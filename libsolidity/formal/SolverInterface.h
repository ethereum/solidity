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

#include <libsolidity/interface/ReadFile.h>
#include <liblangutil/Exceptions.h>
#include <libdevcore/Common.h>
#include <libdevcore/Exceptions.h>

#include <boost/noncopyable.hpp>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace dev
{
namespace solidity
{
namespace smt
{

enum class CheckResult
{
	SATISFIABLE, UNSATISFIABLE, UNKNOWN, CONFLICTING, ERROR
};

enum class Kind
{
	Int,
	Bool,
	Function,
	Array
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

/// C++ representation of an SMTLIB2 expression.
class Expression
{
	friend class SolverInterface;
public:
	explicit Expression(bool _v): Expression(_v ? "true" : "false", Kind::Bool) {}
	Expression(size_t _number): Expression(std::to_string(_number), Kind::Int) {}
	Expression(u256 const& _number): Expression(_number.str(), Kind::Int) {}
	Expression(bigint const& _number): Expression(_number.str(), Kind::Int) {}

	Expression(Expression const&) = default;
	Expression(Expression&&) = default;
	Expression& operator=(Expression const&) = default;
	Expression& operator=(Expression&&) = default;

	bool hasCorrectArity() const
	{
		static std::map<std::string, unsigned> const operatorsArity{
			{"ite", 3},
			{"not", 1},
			{"and", 2},
			{"or", 2},
			{"=", 2},
			{"<", 2},
			{"<=", 2},
			{">", 2},
			{">=", 2},
			{"+", 2},
			{"-", 2},
			{"*", 2},
			{"/", 2},
			{"mod", 2},
			{"select", 2},
			{"store", 3}
		};
		return operatorsArity.count(name) && operatorsArity.at(name) == arguments.size();
	}

	static Expression ite(Expression _condition, Expression _trueValue, Expression _falseValue)
	{
		solAssert(*_trueValue.sort == *_falseValue.sort, "");
		SortPointer sort = _trueValue.sort;
		return Expression("ite", std::vector<Expression>{
			std::move(_condition), std::move(_trueValue), std::move(_falseValue)
		}, std::move(sort));
	}

	static Expression implies(Expression _a, Expression _b)
	{
		return !std::move(_a) || std::move(_b);
	}

	/// select is the SMT representation of an array index access.
	static Expression select(Expression _array, Expression _index)
	{
		solAssert(_array.sort->kind == Kind::Array, "");
		std::shared_ptr<ArraySort> arraySort = std::dynamic_pointer_cast<ArraySort>(_array.sort);
		solAssert(arraySort, "");
		solAssert(_index.sort, "");
		solAssert(*arraySort->domain == *_index.sort, "");
		return Expression(
			"select",
			std::vector<Expression>{std::move(_array), std::move(_index)},
			arraySort->range
		);
	}

	/// store is the SMT representation of an assignment to array index.
	/// The function is pure and returns the modified array.
	static Expression store(Expression _array, Expression _index, Expression _element)
	{
		solAssert(_array.sort->kind == Kind::Array, "");
		std::shared_ptr<ArraySort> arraySort = std::dynamic_pointer_cast<ArraySort>(_array.sort);
		solAssert(arraySort, "");
		solAssert(_index.sort, "");
		solAssert(_element.sort, "");
		solAssert(*arraySort->domain == *_index.sort, "");
		solAssert(*arraySort->range == *_element.sort, "");
		return Expression(
			"store",
			std::vector<Expression>{std::move(_array), std::move(_index), std::move(_element)},
			arraySort
		);
	}

	friend Expression operator!(Expression _a)
	{
		return Expression("not", std::move(_a), Kind::Bool);
	}
	friend Expression operator&&(Expression _a, Expression _b)
	{
		return Expression("and", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator||(Expression _a, Expression _b)
	{
		return Expression("or", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator==(Expression _a, Expression _b)
	{
		return Expression("=", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator!=(Expression _a, Expression _b)
	{
		return !(std::move(_a) == std::move(_b));
	}
	friend Expression operator<(Expression _a, Expression _b)
	{
		return Expression("<", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator<=(Expression _a, Expression _b)
	{
		return Expression("<=", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator>(Expression _a, Expression _b)
	{
		return Expression(">", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator>=(Expression _a, Expression _b)
	{
		return Expression(">=", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator+(Expression _a, Expression _b)
	{
		return Expression("+", std::move(_a), std::move(_b), Kind::Int);
	}
	friend Expression operator-(Expression _a, Expression _b)
	{
		return Expression("-", std::move(_a), std::move(_b), Kind::Int);
	}
	friend Expression operator*(Expression _a, Expression _b)
	{
		return Expression("*", std::move(_a), std::move(_b), Kind::Int);
	}
	friend Expression operator/(Expression _a, Expression _b)
	{
		return Expression("/", std::move(_a), std::move(_b), Kind::Int);
	}
	friend Expression operator%(Expression _a, Expression _b)
	{
		return Expression("mod", std::move(_a), std::move(_b), Kind::Int);
	}
	Expression operator()(std::vector<Expression> _arguments) const
	{
		solAssert(
			sort->kind == Kind::Function,
			"Attempted function application to non-function."
		);
		auto fSort = dynamic_cast<FunctionSort const*>(sort.get());
		solAssert(fSort, "");
		return Expression(name, std::move(_arguments), fSort->codomain);
	}

	std::string name;
	std::vector<Expression> arguments;
	SortPointer sort;

private:
	/// Manual constructors, should only be used by SolverInterface and this class itself.
	Expression(std::string _name, std::vector<Expression> _arguments, SortPointer _sort):
		name(std::move(_name)), arguments(std::move(_arguments)), sort(std::move(_sort)) {}
	Expression(std::string _name, std::vector<Expression> _arguments, Kind _kind):
		Expression(std::move(_name), std::move(_arguments), std::make_shared<Sort>(_kind)) {}

	explicit Expression(std::string _name, Kind _kind):
		Expression(std::move(_name), std::vector<Expression>{}, _kind) {}
	Expression(std::string _name, Expression _arg, Kind _kind):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg)}, _kind) {}
	Expression(std::string _name, Expression _arg1, Expression _arg2, Kind _kind):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg1), std::move(_arg2)}, _kind) {}
};

DEV_SIMPLE_EXCEPTION(SolverError);

class SolverInterface
{
public:
	virtual ~SolverInterface() = default;
	virtual void reset() = 0;

	virtual void push() = 0;
	virtual void pop() = 0;

	virtual void declareVariable(std::string const& _name, Sort const& _sort) = 0;
	Expression newVariable(std::string _name, SortPointer _sort)
	{
		// Subclasses should do something here
		declareVariable(_name, *_sort);
		return Expression(std::move(_name), {}, std::move(_sort));
	}

	virtual void addAssertion(Expression const& _expr) = 0;

	/// Checks for satisfiability, evaluates the expressions if a model
	/// is available. Throws SMTSolverError on error.
	virtual std::pair<CheckResult, std::vector<std::string>>
	check(std::vector<Expression> const& _expressionsToEvaluate) = 0;

	/// @returns a list of queries that the system was not able to respond to.
	virtual std::vector<std::string> unhandledQueries() { return {}; }

	/// @returns how many SMT solvers this interface has.
	virtual unsigned solvers() { return 1; }

protected:
	// SMT query timeout in milliseconds.
	static int const queryTimeout = 10000;
};

}
}
}
