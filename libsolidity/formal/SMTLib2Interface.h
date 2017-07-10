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

#include <map>
#include <string>
#include <vector>

#include <libdevcore/Common.h>

namespace dev
{
namespace solidity
{
namespace smt
{

enum class CheckResult
{
	SAT, UNSAT, UNKNOWN
};

enum class Sort
{
	Int, Bool
};

class Expression
{
	friend class SMTLib2Interface;
	/// Manual constructor, should only be used by SMTLib2Interface and the class itself.
	Expression(std::string _name, std::vector<Expression> _arguments):
		m_name(std::move(_name)), m_arguments(std::move(_arguments)) {}

public:
	Expression(size_t _number): m_name(std::to_string(_number)) {}
	Expression(u256 const& _number): m_name(std::to_string(_number)) {}

	Expression(Expression const& _other) = default;
	Expression(Expression&& _other) = default;
	Expression& operator=(Expression const& _other) = default;
	Expression& operator=(Expression&& _other) = default;

	friend Expression operator!(Expression _a)
	{
		return Expression("not", _a);
	}
	friend Expression operator&&(Expression _a, Expression _b)
	{
		return Expression("and", _a, _b);
	}
	friend Expression operator||(Expression _a, Expression _b)
	{
		return Expression("or", _a, _b);
	}
	friend Expression operator==(Expression _a, Expression _b)
	{
		return Expression("=", _a, _b);
	}
	friend Expression operator!=(Expression _a, Expression _b)
	{
		return !(_a == _b);
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

	std::string toSExpr() const
	{
		std::string sexpr = "(" + m_name;
		for (auto const& arg: m_arguments)
			sexpr += " " + arg.toSExpr();
		sexpr += ")";
		return sexpr;
	}

private:
	explicit Expression(std::string _name):
		Expression(std::move(_name), std::vector<Expression>{}) {}
	Expression(std::string _name, Expression _arg):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg)}) {}
	Expression(std::string _name, Expression _arg1, Expression _arg2):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg1), std::move(_arg2)}) {}

	std::string const m_name;
	std::vector<Expression> const m_arguments;
};

class SMTLib2Interface
{
public:

	void reset();

	void push();
	void pop();

	Expression newFunction(std::string _name, Sort _domain, Sort _codomain);
	Expression newInteger(std::string _name);
	Expression newBool(std::string _name);

	void addAssertion(Expression _expr);
	CheckResult check();
	std::string eval(Expression _expr);
};


}
}
}
