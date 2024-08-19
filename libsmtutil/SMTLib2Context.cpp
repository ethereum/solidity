/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <libsmtutil/SMTLib2Context.h>

#include <boost/functional/hash.hpp>

#include <range/v3/algorithm/find_if.hpp>

namespace solidity::smtutil
{

std::size_t SortPairHash::operator()(std::pair<SortId, SortId> const& _pair) const
{
	std::size_t seed = 0;
	boost::hash_combine(seed, _pair.first);
	boost::hash_combine(seed, _pair.second);
	return seed;
}

SMTLib2Context::SMTLib2Context()
{
	clear();
}

bool SMTLib2Context::isDeclared(std::string const& _name) const
{
	return m_functions.count(_name) > 0;
}

void SMTLib2Context::declare(std::string const& _name, SortPointer const& _sort)
{
	auto [_, inserted] = m_functions.insert({_name, _sort});
	smtAssert(inserted, "Trying to redeclare SMT function!");
}

SortPointer SMTLib2Context::getDeclaredSort(std::string const& _name) const
{
	smtAssert(isDeclared(_name));
	return m_functions.at(_name);
}

void SMTLib2Context::clear() {
	m_functions.clear();
	m_knownTypes.clear();
	m_arraySorts.clear();
	m_tupleSorts.clear();
	m_bitVectorSorts.clear();
	m_callback = {};
	m_knownTypes.emplace_back(std::make_unique<SMTLibSort>(Kind::Bool, std::string("Bool"), std::vector<SortId>{}, SortId{0u}));
	m_knownTypes.emplace_back(std::make_unique<SMTLibSort>(Kind::Int, std::string("Int"), std::vector<SortId>{}, SortId{1u}));
	assert(m_boolSort == m_knownTypes[0]->id);
	assert(m_intSort == m_knownTypes[1]->id);
}

SortId SMTLib2Context::resolve(SortPointer const& _sort)
{
	switch (_sort->kind)
	{
	case Kind::Int:
		return m_intSort;
	case Kind::Bool:
		return m_boolSort;
	case Kind::BitVector:
		return resolveBitVectorSort(dynamic_cast<BitVectorSort const&>(*_sort));
	case Kind::Array:
		return resolveArraySort(dynamic_cast<ArraySort const&>(*_sort));
	case Kind::Tuple:
		return resolveTupleSort(dynamic_cast<TupleSort const&>(*_sort));
	default:
		smtAssert(false, "Invalid SMT sort");
	}
}

SortPointer SMTLib2Context::unresolve(SortId _sortId) const
{
	smtAssert(_sortId < m_knownTypes.size());
	auto const& type = *m_knownTypes[_sortId];
	switch (type.kind)
	{
	case Kind::Int:
		return SortProvider::sintSort;
	case Kind::Bool:
		return SortProvider::boolSort;
	case Kind::BitVector:
	{
		auto it = ranges::find_if(m_bitVectorSorts, [&](auto const& entry) { return entry.second == _sortId; });
		smtAssert(it != m_bitVectorSorts.end());
		return std::make_shared<BitVectorSort>(it->first);
	}
	case Kind::Array:
	{
		auto it = ranges::find_if(m_arraySorts, [&](auto const& entry) { return entry.second == _sortId; });
		smtAssert(it != m_arraySorts.end());
		return std::make_shared<ArraySort>(unresolve(it->first.first), unresolve(it->first.second));
	}
	case Kind::Tuple:
	{
		auto const& tupleType = dynamic_cast<TupleType const&>(type);
		std::vector<std::string> memberNames;
		std::vector<SortPointer> memberTypes;
		for (auto&& [name, sortId] : tupleType.accessors)
		{
			memberNames.push_back(name);
			memberTypes.push_back(unresolve(sortId));
		}
		return std::make_shared<TupleSort>(tupleType.name, std::move(memberNames), std::move(memberTypes));
	}
	default:
		smtAssert(false, "Invalid SMT sort");
	}
}

SortId SMTLib2Context::resolveBitVectorSort(BitVectorSort const& _sort)
{
	auto size = _sort.size;
	auto it = m_bitVectorSorts.find(size);
	if (it == m_bitVectorSorts.end())
	{
		auto newId = static_cast<uint32_t>(m_knownTypes.size());
		m_knownTypes.emplace_back(std::make_unique<SMTLibSort>(Kind::BitVector, "(_ BitVec " + std::to_string(size) + ')', std::vector<SortId>{}, SortId{newId}));
		auto&& [newIt, inserted] = m_bitVectorSorts.emplace(size, SortId{newId});
		smtAssert(inserted);
		return newIt->second;
	}
	return it->second;
}

SortId SMTLib2Context::resolveArraySort(ArraySort const& _sort)
{
	smtAssert(_sort.domain && _sort.range);
	auto domainSort = resolve(_sort.domain);
	auto rangeSort = resolve(_sort.range);
	auto pair = std::make_pair(domainSort, rangeSort);
	auto it = m_arraySorts.find(pair);
	if (it == m_arraySorts.end())
	{
		auto newId = static_cast<uint32_t>(m_knownTypes.size());
		m_knownTypes.emplace_back(std::make_unique<SMTLibSort>(Kind::Array, "Array", std::vector<SortId>{domainSort, rangeSort}, SortId{newId}));
		auto&& [newIt, inserted] = m_arraySorts.emplace(pair, SortId{newId});
		smtAssert(inserted);
		return newIt->second;
	}
	return it->second;
}

SortId SMTLib2Context::resolveTupleSort(TupleSort const& _sort)
{
	auto const& tupleName = _sort.name;
	auto it = m_tupleSorts.find(tupleName);
	if (it == m_tupleSorts.end())
	{
		std::vector<std::pair<std::string, SortId>> accessors;
		smtAssert(_sort.members.size() == _sort.components.size());
		for (std::size_t i = 0u; i < _sort.members.size(); ++i)
			accessors.emplace_back(_sort.members[i], resolve(_sort.components[i]));
		auto newId = static_cast<uint32_t>(m_knownTypes.size());
		m_knownTypes.emplace_back(std::make_unique<TupleType>(tupleName, std::move(accessors), SortId{newId}));
		auto&& [newIt, inserted] = m_tupleSorts.emplace(tupleName, SortId{newId});
		smtAssert(inserted);
		if (m_callback)
			m_callback(_sort);
		return newIt->second;
	}
	return it->second;
}

std::string SMTLib2Context::toString(SortId _id)
{
	auto const& sort = m_knownTypes.at(_id);
	switch (sort->kind)
	{
	case Kind::Int:
		return "Int";
	case Kind::Bool:
		return "Bool";
	case Kind::BitVector:
		return dynamic_cast<SMTLibSort const&>(*sort).name;
	case Kind::Array:
	{
		auto const& arraySort = dynamic_cast<SMTLibSort const&>(*sort);
		smtAssert(arraySort.args.size() == 2);
		return "(Array " + toString(arraySort.args.at(0)) + ' ' + toString(arraySort.args.at(1)) + ')';
	}
	case Kind::Tuple:
	{
		auto const& tupleType = dynamic_cast<TupleType const&>(*sort);
		return '|' + tupleType.name + '|';
	}
	default:
		smtAssert(false, "Invalid SMT sort");
	}
}

std::string SMTLib2Context::toSmtLibSort(solidity::smtutil::SortPointer const& _sort)
{
	return toString(resolve(_sort));
}

std::string SMTLib2Context::toSExpr(Expression const& _expr)
{
	if (_expr.arguments.empty())
		return _expr.name;

	std::string sexpr = "(";
	if (_expr.name == "int2bv")
	{
		size_t size = std::stoul(_expr.arguments[1].name);
		auto arg = toSExpr(_expr.arguments.front());
		auto int2bv = "(_ int2bv " + std::to_string(size) + ")";
		// Some solvers treat all BVs as unsigned, so we need to manually apply 2's complement if needed.
		sexpr += std::string("ite ") +
			"(>= " + arg + " 0) " +
			"(" + int2bv + " " + arg + ") " +
			"(bvneg (" + int2bv + " (- " + arg + ")))";
	}
	else if (_expr.name == "bv2int")
	{
		auto intSort = std::dynamic_pointer_cast<IntSort>(_expr.sort);
		smtAssert(intSort, "");

		auto arg = toSExpr(_expr.arguments.front());
		auto nat = "(bv2nat " + arg + ")";

		if (!intSort->isSigned)
			return nat;

		auto bvSort = std::dynamic_pointer_cast<BitVectorSort>(_expr.arguments.front().sort);
		smtAssert(bvSort, "");
		auto size = std::to_string(bvSort->size);
		auto pos = std::to_string(bvSort->size - 1);

		// Some solvers treat all BVs as unsigned, so we need to manually apply 2's complement if needed.
		sexpr += std::string("ite ") +
			"(= ((_ extract " + pos + " " + pos + ")" + arg + ") #b0) " +
			nat + " " +
			"(- (bv2nat (bvneg " + arg + ")))";
	}
	else if (_expr.name == "const_array")
	{
		smtAssert(_expr.arguments.size() == 2, "");
		auto sortSort = std::dynamic_pointer_cast<SortSort>(_expr.arguments.at(0).sort);
		smtAssert(sortSort, "");
		auto arraySort = std::dynamic_pointer_cast<ArraySort>(sortSort->inner);
		smtAssert(arraySort, "");
		sexpr += "(as const " + toSmtLibSort(arraySort) + ") ";
		sexpr += toSExpr(_expr.arguments.at(1));
	}
	else if (_expr.name == "tuple_get")
	{
		smtAssert(_expr.arguments.size() == 2, "");
		auto tupleSort = std::dynamic_pointer_cast<TupleSort>(_expr.arguments.at(0).sort);
		size_t index = std::stoul(_expr.arguments.at(1).name);
		smtAssert(index < tupleSort->members.size(), "");
		sexpr += "|" + tupleSort->members.at(index) + "| " + toSExpr(_expr.arguments.at(0));
	}
	else if (_expr.name == "tuple_constructor")
	{
		auto tupleSort = std::dynamic_pointer_cast<TupleSort>(_expr.sort);
		smtAssert(tupleSort, "");
		sexpr += "|" + tupleSort->name + "|";
		for (auto const& arg: _expr.arguments)
			sexpr += " " + toSExpr(arg);
	}
	else
	{
		sexpr += _expr.name;
		for (auto const& arg: _expr.arguments)
			sexpr += " " + toSExpr(arg);
	}
	sexpr += ")";
	return sexpr;
}

std::optional<SortPointer> SMTLib2Context::getTupleType(std::string const& _name) const
{
	auto it = m_tupleSorts.find(_name);
	return it == m_tupleSorts.end() ? std::nullopt : std::optional<SortPointer>(unresolve(it->second));
}

std::optional<std::pair<std::string, SortPointer>> SMTLib2Context::getTupleAccessor(std::string const& _name) const
{
	for (auto&& [_, sortId] : m_tupleSorts)
	{
		auto const& type = m_knownTypes.at(sortId);
		smtAssert(type->kind == Kind::Tuple);
		auto const& tupleType = dynamic_cast<TupleType const&>(*type);
		for (auto&& [memberName, memberSort] : tupleType.accessors)
			if (memberName == _name)
				return std::make_pair(memberName, unresolve(memberSort));
	}
	return std::nullopt;
}

void SMTLib2Context::setTupleDeclarationCallback(TupleDeclarationCallback _callback)
{
	m_callback = std::move(_callback);
}
} // namespace solidity::smtutil
