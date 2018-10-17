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

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/ReadFile.h>

#include <libdevcore/Common.h>
#include <libdevcore/Exceptions.h>

#include <boost/noncopyable.hpp>

#include <map>
#include <string>
#include <vector>
#include <cstdio>

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

enum class Sort
{
	Int,
	Bool,
	IntIntFun, // Function of one Int returning a single Int
	IntBoolFun // Function of one Int returning a single Bool
};

/// C++ representation of an SMTLIB2 expression.
class Expression
{
	friend class SolverInterface;
public:
	explicit Expression(bool _v): name(_v ? "true" : "false"), sort(Sort::Bool) {}
	Expression(size_t _number): name(std::to_string(_number)), sort(Sort::Int) {}
	Expression(u256 const& _number): name(_number.str()), sort(Sort::Int) {}
	Expression(bigint const& _number): name(_number.str()), sort(Sort::Int) {}

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
			{"/", 2}
		};
		return operatorsArity.count(name) && operatorsArity.at(name) == arguments.size();
	}

	static Expression ite(Expression _condition, Expression _trueValue, Expression _falseValue)
	{
		solAssert(_trueValue.sort == _falseValue.sort, "");
		return Expression("ite", std::vector<Expression>{
			std::move(_condition), std::move(_trueValue), std::move(_falseValue)
		}, _trueValue.sort);
	}

	static Expression implies(Expression _a, Expression _b)
	{
		return !std::move(_a) || std::move(_b);
	}

	friend Expression operator!(Expression _a)
	{
		return Expression("not", std::move(_a), Sort::Bool);
	}
	friend Expression operator&&(Expression _a, Expression _b)
	{
		return Expression("and", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator||(Expression _a, Expression _b)
	{
		return Expression("or", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator==(Expression _a, Expression _b)
	{
		return Expression("=", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator!=(Expression _a, Expression _b)
	{
		return !(std::move(_a) == std::move(_b));
	}
	friend Expression operator<(Expression _a, Expression _b)
	{
		return Expression("<", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator<=(Expression _a, Expression _b)
	{
		return Expression("<=", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator>(Expression _a, Expression _b)
	{
		return Expression(">", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator>=(Expression _a, Expression _b)
	{
		return Expression(">=", std::move(_a), std::move(_b), Sort::Bool);
	}
	friend Expression operator+(Expression _a, Expression _b)
	{
		return Expression("+", std::move(_a), std::move(_b), Sort::Int);
	}
	friend Expression operator-(Expression _a, Expression _b)
	{
		return Expression("-", std::move(_a), std::move(_b), Sort::Int);
	}
	friend Expression operator*(Expression _a, Expression _b)
	{
		return Expression("*", std::move(_a), std::move(_b), Sort::Int);
	}
	friend Expression operator/(Expression _a, Expression _b)
	{
		return Expression("/", std::move(_a), std::move(_b), Sort::Int);
	}
	Expression operator()(Expression _a) const
	{
		solAssert(
			arguments.empty(),
			"Attempted function application to non-function."
		);
		switch (sort)
		{
		case Sort::IntIntFun:
			return Expression(name, _a, Sort::Int);
		case Sort::IntBoolFun:
			return Expression(name, _a, Sort::Bool);
		default:
			solAssert(
				false,
				"Attempted function application to invalid type."
			);
			break;
		}
	}

	std::string name;
	std::vector<Expression> arguments;
	Sort sort;

private:
	/// Manual constructor, should only be used by SolverInterface and this class itself.
	Expression(std::string _name, std::vector<Expression> _arguments, Sort _sort):
		name(std::move(_name)), arguments(std::move(_arguments)), sort(_sort) {}

	explicit Expression(std::string _name, Sort _sort):
		Expression(std::move(_name), std::vector<Expression>{}, _sort) {}
	Expression(std::string _name, Expression _arg, Sort _sort):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg)}, _sort) {}
	Expression(std::string _name, Expression _arg1, Expression _arg2, Sort _sort):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg1), std::move(_arg2)}, _sort) {}
};

DEV_SIMPLE_EXCEPTION(SolverError);

class SolverInterface
{
public:
	virtual ~SolverInterface() = default;
	virtual void reset() = 0;

	virtual void push() = 0;
	virtual void pop() = 0;

	virtual void declareFunction(std::string _name, Sort _domain, Sort _codomain) = 0;
	Expression newFunction(std::string _name, Sort _domain, Sort _codomain)
	{
		declareFunction(_name, _domain, _codomain);
		solAssert(_domain == Sort::Int, "Function sort not supported.");
		// Subclasses should do something here
		switch (_codomain)
		{
		case Sort::Int:
			return Expression(std::move(_name), {}, Sort::IntIntFun);
		case Sort::Bool:
			return Expression(std::move(_name), {}, Sort::IntBoolFun);
		default:
			solAssert(false, "Function sort not supported.");
			break;
		}
	}
	virtual void declareInteger(std::string _name) = 0;
	Expression newInteger(std::string _name)
	{
		// Subclasses should do something here
		declareInteger(_name);
		return Expression(std::move(_name), {}, Sort::Int);
	}
	virtual void declareBool(std::string _name) = 0;
	Expression newBool(std::string _name)
	{
		// Subclasses should do something here
		declareBool(_name);
		return Expression(std::move(_name), {}, Sort::Bool);
	}

	virtual void addAssertion(Expression const& _expr) = 0;

	/// Checks for satisfiability, evaluates the expressions if a model
	/// is available. Throws SMTSolverError on error.
	virtual std::pair<CheckResult, std::vector<std::string>>
	check(std::vector<Expression> const& _expressionsToEvaluate) = 0;

protected:
	// SMT query timeout in milliseconds.
	static int const queryTimeout = 10000;
};

}
}
}
