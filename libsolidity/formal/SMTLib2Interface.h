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

#include <libsolidity/formal/SMTSolverCommunicator.h>

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/ReadFile.h>

#include <libdevcore/Common.h>

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
	SAT, UNSAT, UNKNOWN, ERROR
};

enum class Sort
{
	Int, Bool
};

/// C++ representation of an SMTLIB2 expression.
class Expression
{
	friend class SMTLib2Interface;
	/// Manual constructor, should only be used by SMTLib2Interface and the class itself.
	Expression(std::string _name, std::vector<Expression> _arguments):
		m_name(std::move(_name)), m_arguments(std::move(_arguments)) {}

public:
	Expression(size_t _number): m_name(std::to_string(_number)) {}
	Expression(u256 const& _number): m_name(_number.str()) {}
	Expression(bigint const& _number): m_name(_number.str()) {}

	Expression(Expression const& _other) = default;
	Expression(Expression&& _other) = default;
	Expression& operator=(Expression const& _other) = default;
	Expression& operator=(Expression&& _other) = default;

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
		solAssert(m_arguments.empty(), "Attempted function application to non-function.");
		return Expression(m_name, _a);
	}

	std::string toSExpr() const
	{
		if (m_arguments.empty())
			return m_name;
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

class SMTLib2Interface: public boost::noncopyable
{
public:
	SMTLib2Interface(ReadFile::Callback const& _readFileCallback);

	void reset();

	void push();
	void pop();

	Expression newFunction(std::string _name, Sort _domain, Sort _codomain);
	Expression newInteger(std::string _name);
	Expression newBool(std::string _name);

	void addAssertion(Expression const& _expr);
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate);

private:
	void write(std::string _data);

	std::string checkSatAndGetValuesCommand(std::vector<Expression> const& _expressionsToEvaluate);
	std::vector<std::string> parseValues(std::string::const_iterator _start, std::string::const_iterator _end);

	SMTSolverCommunicator m_communicator;
	std::vector<std::string> m_accumulatedOutput;
};


}
}
}
