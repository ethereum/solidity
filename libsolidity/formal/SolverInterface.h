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
	SATISFIABLE, UNSATISFIABLE, UNKNOWN, ERROR
};

enum class Sort
{
	Int, Bool
};

/// C++ representation of an SMTLIB2 expression.
class Expression
{
	friend class SolverInterface;
public:
	Expression(size_t _number): name(std::to_string(_number)) {}
	Expression(u256 const& _number): name(_number.str()) {}
	Expression(bigint const& _number): name(_number.str()) {}

	Expression(Expression const&) = default;
	Expression(Expression&&) = default;
	Expression& operator=(Expression const&) = default;
	Expression& operator=(Expression&&) = default;

	static Expression ite(Expression _condition, Expression _trueValue, Expression _falseValue)
	{
		return Expression("ite", std::vector<Expression>{
			std::move(_condition), std::move(_trueValue), std::move(_falseValue)
		});
	}

	friend Expression operator!(Expression _a)
	{
		return Expression("not", std::move(_a));
	}
	friend Expression operator&&(Expression _a, Expression _b)
	{
		return Expression("and", std::move(_a), std::move(_b));
	}
	friend Expression operator||(Expression _a, Expression _b)
	{
		return Expression("or", std::move(_a), std::move(_b));
	}
	friend Expression operator==(Expression _a, Expression _b)
	{
		return Expression("=", std::move(_a), std::move(_b));
	}
	friend Expression operator!=(Expression _a, Expression _b)
	{
		return !(std::move(_a) == std::move(_b));
	}
	friend Expression operator<(Expression _a, Expression _b)
	{
		return Expression("<", std::move(_a), std::move(_b));
	}
	friend Expression operator<=(Expression _a, Expression _b)
	{
		return Expression("<=", std::move(_a), std::move(_b));
	}
	friend Expression operator>(Expression _a, Expression _b)
	{
		return Expression(">", std::move(_a), std::move(_b));
	}
	friend Expression operator>=(Expression _a, Expression _b)
	{
		return Expression(">=", std::move(_a), std::move(_b));
	}
	friend Expression operator+(Expression _a, Expression _b)
	{
		return Expression("+", std::move(_a), std::move(_b));
	}
	friend Expression operator-(Expression _a, Expression _b)
	{
		return Expression("-", std::move(_a), std::move(_b));
	}
	friend Expression operator*(Expression _a, Expression _b)
	{
		return Expression("*", std::move(_a), std::move(_b));
	}
	Expression operator()(Expression _a) const
	{
		solAssert(arguments.empty(), "Attempted function application to non-function.");
		return Expression(name, _a);
	}

	std::string const name;
	std::vector<Expression> const arguments;

private:
	/// Manual constructor, should only be used by SolverInterface and this class itself.
	Expression(std::string _name, std::vector<Expression> _arguments):
		name(std::move(_name)), arguments(std::move(_arguments)) {}

	explicit Expression(std::string _name):
		Expression(std::move(_name), std::vector<Expression>{}) {}
	Expression(std::string _name, Expression _arg):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg)}) {}
	Expression(std::string _name, Expression _arg1, Expression _arg2):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg1), std::move(_arg2)}) {}
};

DEV_SIMPLE_EXCEPTION(SolverError);

class SolverInterface
{
public:
	virtual void reset() = 0;

	virtual void push() = 0;
	virtual void pop() = 0;

	virtual Expression newFunction(std::string _name, Sort /*_domain*/, Sort /*_codomain*/)
	{
		// Subclasses should do something here
		return Expression(std::move(_name), {});
	}
	virtual Expression newInteger(std::string _name)
	{
		// Subclasses should do something here
		return Expression(std::move(_name), {});
	}
	virtual Expression newBool(std::string _name)
	{
		// Subclasses should do something here
		return Expression(std::move(_name), {});
	}

	virtual void addAssertion(Expression const& _expr) = 0;

	/// Checks for satisfiability, evaluates the expressions if a model
	/// is available. Throws SMTSolverError on error.
	virtual std::pair<CheckResult, std::vector<std::string>>
	check(std::vector<Expression> const& _expressionsToEvaluate) = 0;
};


}
}
}
