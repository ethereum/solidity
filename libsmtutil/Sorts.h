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

#pragma once

#include <liblangutil/Exceptions.h>
#include <libsolutil/Common.h>
#include <libsolutil/Exceptions.h>

#include <memory>
#include <vector>

namespace solidity::frontend::smt
{

enum class Kind
{
	Int,
	Bool,
	Function,
	Array,
	Sort,
	Tuple
};

struct Sort
{
	Sort(Kind _kind):
		kind(_kind) {}
	virtual ~Sort() = default;
	virtual bool operator==(Sort const& _other) const { return kind == _other.kind; }

	Kind const kind;
};
using SortPointer = std::shared_ptr<Sort>;

struct FunctionSort: public Sort
{
	FunctionSort(std::vector<SortPointer> _domain, SortPointer _codomain):
		Sort(Kind::Function), domain(std::move(_domain)), codomain(std::move(_codomain)) {}
	bool operator==(Sort const& _other) const override
	{
		if (!Sort::operator==(_other))
			return false;
		auto _otherFunction = dynamic_cast<FunctionSort const*>(&_other);
		solAssert(_otherFunction, "");
		if (domain.size() != _otherFunction->domain.size())
			return false;
		if (!std::equal(
			domain.begin(),
			domain.end(),
			_otherFunction->domain.begin(),
			[&](SortPointer _a, SortPointer _b) { return *_a == *_b; }
		))
			return false;
		solAssert(codomain, "");
		solAssert(_otherFunction->codomain, "");
		return *codomain == *_otherFunction->codomain;
	}

	std::vector<SortPointer> domain;
	SortPointer codomain;
};

struct ArraySort: public Sort
{
	/// _domain is the sort of the indices
	/// _range is the sort of the values
	ArraySort(SortPointer _domain, SortPointer _range):
		Sort(Kind::Array), domain(std::move(_domain)), range(std::move(_range)) {}
	bool operator==(Sort const& _other) const override
	{
		if (!Sort::operator==(_other))
			return false;
		auto _otherArray = dynamic_cast<ArraySort const*>(&_other);
		solAssert(_otherArray, "");
		solAssert(_otherArray->domain, "");
		solAssert(_otherArray->range, "");
		solAssert(domain, "");
		solAssert(range, "");
		return *domain == *_otherArray->domain && *range == *_otherArray->range;
	}

	SortPointer domain;
	SortPointer range;
};

struct SortSort: public Sort
{
	SortSort(SortPointer _inner): Sort(Kind::Sort), inner(std::move(_inner)) {}
	bool operator==(Sort const& _other) const override
	{
		if (!Sort::operator==(_other))
			return false;
		auto _otherSort = dynamic_cast<SortSort const*>(&_other);
		solAssert(_otherSort, "");
		solAssert(_otherSort->inner, "");
		solAssert(inner, "");
		return *inner == *_otherSort->inner;
	}

	SortPointer inner;
};

struct TupleSort: public Sort
{
	TupleSort(
		std::string _name,
		std::vector<std::string> _members,
		std::vector<SortPointer> _components
	):
		Sort(Kind::Tuple),
		name(std::move(_name)),
		members(std::move(_members)),
		components(std::move(_components))
	{}

	bool operator==(Sort const& _other) const override
	{
		if (!Sort::operator==(_other))
			return false;
		auto _otherTuple = dynamic_cast<TupleSort const*>(&_other);
		solAssert(_otherTuple, "");
		if (name != _otherTuple->name)
			return false;
		if (members != _otherTuple->members)
			return false;
		if (components.size() != _otherTuple->components.size())
			return false;
		if (!std::equal(
			components.begin(),
			components.end(),
			_otherTuple->components.begin(),
			[&](SortPointer _a, SortPointer _b) { return *_a == *_b; }
		))
			return false;
		return true;
	}

	std::string const name;
	std::vector<std::string> const members;
	std::vector<SortPointer> const components;
};

/** Frequently used sorts.*/
struct SortProvider
{
	static std::shared_ptr<Sort> const boolSort;
	static std::shared_ptr<Sort> const intSort;
};

}
