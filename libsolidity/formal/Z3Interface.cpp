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

#include <libsolidity/formal/Z3Interface.h>

#include <libsolidity/interface/Exceptions.h>

#include <libdevcore/CommonIO.h>

using namespace std;
using namespace dev;
using namespace dev::solidity::smt;

Z3Interface::Z3Interface():
	m_solver(m_context)
{
	// This needs to be set globally.
	z3::set_param("rewriter.pull_cheap_ite", true);
	// This needs to be set in the context.
	m_context.set("timeout", queryTimeout);
}

void Z3Interface::reset()
{
	m_constants.clear();
	m_functions.clear();
	m_solver.reset();
}

void Z3Interface::push()
{
	m_solver.push();
}

void Z3Interface::pop()
{
	m_solver.pop();
}

void Z3Interface::declareFunction(string _name, Sort _domain, Sort _codomain)
{
	m_functions.insert({_name, m_context.function(_name.c_str(), z3Sort(_domain), z3Sort(_codomain))});
}

void Z3Interface::declareInteger(string _name)
{
	m_constants.insert({_name, m_context.int_const(_name.c_str())});
}

void Z3Interface::declareBool(string _name)
{
	m_constants.insert({_name, m_context.bool_const(_name.c_str())});
}

void Z3Interface::addAssertion(Expression const& _expr)
{
	m_solver.add(toZ3Expr(_expr));
}

pair<CheckResult, vector<string>> Z3Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	CheckResult result;
	vector<string> values;
	try
	{
		switch (m_solver.check())
		{
		case z3::check_result::sat:
			result = CheckResult::SATISFIABLE;
			break;
		case z3::check_result::unsat:
			result = CheckResult::UNSATISFIABLE;
			break;
		case z3::check_result::unknown:
			result = CheckResult::UNKNOWN;
			break;
		default:
			solAssert(false, "");
		}

		if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
		{
			z3::model m = m_solver.get_model();
			for (Expression const& e: _expressionsToEvaluate)
				values.push_back(toString(m.eval(toZ3Expr(e))));
		}
	}
	catch (z3::exception const&)
	{
		result = CheckResult::ERROR;
		values.clear();
	}

	return make_pair(result, values);
}

z3::expr Z3Interface::toZ3Expr(Expression const& _expr)
{
	if (_expr.arguments.empty() && m_constants.count(_expr.name))
		return m_constants.at(_expr.name);
	z3::expr_vector arguments(m_context);
	for (auto const& arg: _expr.arguments)
		arguments.push_back(toZ3Expr(arg));

	string const& n = _expr.name;
	if (m_functions.count(n))
		return m_functions.at(n)(arguments);
	else if (m_constants.count(n))
	{
		solAssert(arguments.empty(), "");
		return m_constants.at(n);
	}
	else if (arguments.empty())
	{
		if (n == "true")
			return m_context.bool_val(true);
		else if (n == "false")
			return m_context.bool_val(false);
		else
			// We assume it is an integer...
			return m_context.int_val(n.c_str());
	}

	solAssert(_expr.hasCorrectArity(), "");
	if (n == "ite")
		return z3::ite(arguments[0], arguments[1], arguments[2]);
	else if (n == "not")
		return !arguments[0];
	else if (n == "and")
		return arguments[0] && arguments[1];
	else if (n == "or")
		return arguments[0] || arguments[1];
	else if (n == "=")
		return arguments[0] == arguments[1];
	else if (n == "<")
		return arguments[0] < arguments[1];
	else if (n == "<=")
		return arguments[0] <= arguments[1];
	else if (n == ">")
		return arguments[0] > arguments[1];
	else if (n == ">=")
		return arguments[0] >= arguments[1];
	else if (n == "+")
		return arguments[0] + arguments[1];
	else if (n == "-")
		return arguments[0] - arguments[1];
	else if (n == "*")
		return arguments[0] * arguments[1];
	else if (n == "/")
		return arguments[0] / arguments[1];
	// Cannot reach here.
	solAssert(false, "");
	return arguments[0];
}

z3::sort Z3Interface::z3Sort(Sort _sort)
{
	switch (_sort)
	{
	case Sort::Bool:
		return m_context.bool_sort();
	case Sort::Int:
		return m_context.int_sort();
	default:
		break;
	}
	solAssert(false, "");
	// Cannot be reached.
	return m_context.int_sort();
}
