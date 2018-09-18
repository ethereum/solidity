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
/**
 * @file ExpressionClasses.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Container for equivalence classes of expressions for use in common subexpression elimination.
 */

#include <libevmasm/ExpressionClasses.h>
#include <utility>
#include <tuple>
#include <functional>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/noncopyable.hpp>
#include <libevmasm/Assembly.h>
#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/SimplificationRules.h>

using namespace std;
using namespace dev;
using namespace dev::eth;


bool ExpressionClasses::Expression::operator<(ExpressionClasses::Expression const& _other) const
{
	assertThrow(!!item && !!_other.item, OptimizerException, "");
	auto type = item->type();
	auto otherType = _other.item->type();
	if (type != otherType)
		return type < otherType;
	else if (type == Operation)
	{
		auto instr = item->instruction();
		auto otherInstr = _other.item->instruction();
		return std::tie(instr, arguments, sequenceNumber) <
			std::tie(otherInstr, _other.arguments, _other.sequenceNumber);
	}
	else
		return std::tie(item->data(), arguments, sequenceNumber) <
			std::tie(_other.item->data(), _other.arguments, _other.sequenceNumber);
}

ExpressionClasses::Id ExpressionClasses::find(
	AssemblyItem const& _item,
	Ids const& _arguments,
	bool _copyItem,
	unsigned _sequenceNumber
)
{
	Expression exp;
	exp.id = Id(-1);
	exp.item = &_item;
	exp.arguments = _arguments;
	exp.sequenceNumber = _sequenceNumber;

	if (SemanticInformation::isCommutativeOperation(_item))
		sort(exp.arguments.begin(), exp.arguments.end());

	if (SemanticInformation::isDeterministic(_item))
	{
		auto it = m_expressions.find(exp);
		if (it != m_expressions.end())
			return it->id;
	}

	if (_copyItem)
		exp.item = storeItem(_item);

	ExpressionClasses::Id id = tryToSimplify(exp);
	if (id < m_representatives.size())
		exp.id = id;
	else
	{
		exp.id = m_representatives.size();
		m_representatives.push_back(exp);
	}
	m_expressions.insert(exp);
	return exp.id;
}

void ExpressionClasses::forceEqual(
	ExpressionClasses::Id _id,
	AssemblyItem const& _item,
	ExpressionClasses::Ids const& _arguments,
	bool _copyItem
)
{
	Expression exp;
	exp.id = _id;
	exp.item = &_item;
	exp.arguments = _arguments;

	if (SemanticInformation::isCommutativeOperation(_item))
		sort(exp.arguments.begin(), exp.arguments.end());

	if (_copyItem)
		exp.item = storeItem(_item);

	m_expressions.insert(exp);
}

ExpressionClasses::Id ExpressionClasses::newClass(SourceLocation const& _location)
{
	Expression exp;
	exp.id = m_representatives.size();
	exp.item = storeItem(AssemblyItem(UndefinedItem, (u256(1) << 255) + exp.id, _location));
	m_representatives.push_back(exp);
	m_expressions.insert(exp);
	return exp.id;
}

bool ExpressionClasses::knownToBeDifferent(ExpressionClasses::Id _a, ExpressionClasses::Id _b)
{
	// Try to simplify "_a - _b" and return true iff the value is a non-zero constant.
	return knownNonZero(find(Instruction::SUB, {_a, _b}));
}

bool ExpressionClasses::knownToBeDifferentBy32(ExpressionClasses::Id _a, ExpressionClasses::Id _b)
{
	// Try to simplify "_a - _b" and return true iff the value is at least 32 away from zero.
	u256 const* v = knownConstant(find(Instruction::SUB, {_a, _b}));
	// forbidden interval is ["-31", 31]
	return v && *v + 31 > u256(62);
}

bool ExpressionClasses::knownZero(Id _c)
{
	return Pattern(u256(0)).matches(representative(_c), *this);
}

bool ExpressionClasses::knownNonZero(Id _c)
{
	return Pattern(u256(0)).matches(representative(find(Instruction::ISZERO, {_c})), *this);
}

u256 const* ExpressionClasses::knownConstant(Id _c)
{
	map<unsigned, Expression const*> matchGroups;
	Pattern constant(Push);
	constant.setMatchGroup(1, matchGroups);
	if (!constant.matches(representative(_c), *this))
		return nullptr;
	return &constant.d();
}

AssemblyItem const* ExpressionClasses::storeItem(AssemblyItem const& _item)
{
	m_spareAssemblyItems.push_back(make_shared<AssemblyItem>(_item));
	return m_spareAssemblyItems.back().get();
}

string ExpressionClasses::fullDAGToString(ExpressionClasses::Id _id) const
{
	Expression const& expr = representative(_id);
	stringstream str;
	str << dec << expr.id << ":";
	if (expr.item)
	{
		str << *expr.item << "(";
		for (Id arg: expr.arguments)
			str << fullDAGToString(arg) << ",";
		str << ")";
	}
	else
		str << " UNIQUE";
	return str.str();
}

ExpressionClasses::Id ExpressionClasses::tryToSimplify(Expression const& _expr)
{
	static Rules rules;
	assertThrow(rules.isInitialized(), OptimizerException, "Rule list not properly initialized.");

	if (
		!_expr.item ||
		_expr.item->type() != Operation ||
		!SemanticInformation::isDeterministic(*_expr.item)
	)
		return -1;

	if (auto match = rules.findFirstMatch(_expr, *this))
	{
		// Debug info
		//cout << "Simplifying " << *_expr.item << "(";
		//for (Id arg: _expr.arguments)
		//	cout << fullDAGToString(arg) << ", ";
		//cout << ")" << endl;
		//cout << "with rule " << match->first.toString() << endl;
		//ExpressionTemplate t(match->second());
		//cout << "to " << match->second().toString() << endl;
		return rebuildExpression(ExpressionTemplate(match->action(), _expr.item->location()));
	}

	return -1;
}

ExpressionClasses::Id ExpressionClasses::rebuildExpression(ExpressionTemplate const& _template)
{
	if (_template.hasId)
		return _template.id;

	Ids arguments;
	for (ExpressionTemplate const& t: _template.arguments)
		arguments.push_back(rebuildExpression(t));
	return find(_template.item, arguments);
}
