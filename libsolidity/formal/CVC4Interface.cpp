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

#include <libsolidity/formal/CVC4Interface.h>

#include <liblangutil/Exceptions.h>
#include <libdevcore/CommonIO.h>

using namespace std;
using namespace dev;
using namespace dev::solidity::smt;

CVC4Interface::CVC4Interface():
	m_solver(&m_context)
{
	reset();
}

void CVC4Interface::reset()
{
	m_variables.clear();
	m_solver.reset();
	m_solver.setOption("produce-models", true);
	m_solver.setTimeLimit(queryTimeout);
}

void CVC4Interface::push()
{
	m_solver.push();
}

void CVC4Interface::pop()
{
	m_solver.pop();
}

void CVC4Interface::declareVariable(string const& _name, Sort const& _sort)
{
	if (!m_variables.count(_name))
		m_variables.insert({_name, m_context.mkVar(_name.c_str(), cvc4Sort(_sort))});
}

void CVC4Interface::addAssertion(Expression const& _expr)
{
	try
	{
		m_solver.assertFormula(toCVC4Expr(_expr));
	}
	catch (CVC4::TypeCheckingException const& _e)
	{
		solAssert(false, _e.what());
	}
	catch (CVC4::LogicException const& _e)
	{
		solAssert(false, _e.what());
	}
	catch (CVC4::UnsafeInterruptException const& _e)
	{
		solAssert(false, _e.what());
	}
	catch (CVC4::Exception const& _e)
	{
		solAssert(false, _e.what());
	}
}

pair<CheckResult, vector<string>> CVC4Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	CheckResult result;
	vector<string> values;
	try
	{
		switch (m_solver.checkSat().isSat())
		{
		case CVC4::Result::SAT:
			result = CheckResult::SATISFIABLE;
			break;
		case CVC4::Result::UNSAT:
			result = CheckResult::UNSATISFIABLE;
			break;
		case CVC4::Result::SAT_UNKNOWN:
			result = CheckResult::UNKNOWN;
			break;
		default:
			solAssert(false, "");
		}

		if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
		{
			for (Expression const& e: _expressionsToEvaluate)
				values.push_back(toString(m_solver.getValue(toCVC4Expr(e))));
		}
	}
	catch (CVC4::Exception const&)
	{
		result = CheckResult::ERROR;
		values.clear();
	}

	return make_pair(result, values);
}

CVC4::Expr CVC4Interface::toCVC4Expr(Expression const& _expr)
{
	// Variable
	if (_expr.arguments.empty() && m_variables.count(_expr.name))
		return m_variables.at(_expr.name);

	vector<CVC4::Expr> arguments;
	for (auto const& arg: _expr.arguments)
		arguments.push_back(toCVC4Expr(arg));

	try
	{
		string const& n = _expr.name;
		// Function application
		if (!arguments.empty() && m_variables.count(_expr.name))
			return m_context.mkExpr(CVC4::kind::APPLY_UF, m_variables.at(n), arguments);
		// Literal
		else if (arguments.empty())
		{
			if (n == "true")
				return m_context.mkConst(true);
			else if (n == "false")
				return m_context.mkConst(false);
			else
				try
				{
					return m_context.mkConst(CVC4::Rational(n));
				}
				catch (CVC4::TypeCheckingException const& _e)
				{
					solAssert(false, _e.what());
				}
				catch (CVC4::Exception const& _e)
				{
					solAssert(false, _e.what());
				}
		}

		solAssert(_expr.hasCorrectArity(), "");
		if (n == "ite")
			return arguments[0].iteExpr(arguments[1], arguments[2]);
		else if (n == "not")
			return arguments[0].notExpr();
		else if (n == "and")
			return arguments[0].andExpr(arguments[1]);
		else if (n == "or")
			return arguments[0].orExpr(arguments[1]);
		else if (n == "implies")
			return m_context.mkExpr(CVC4::kind::IMPLIES, arguments[0], arguments[1]);
		else if (n == "=")
			return m_context.mkExpr(CVC4::kind::EQUAL, arguments[0], arguments[1]);
		else if (n == "<")
			return m_context.mkExpr(CVC4::kind::LT, arguments[0], arguments[1]);
		else if (n == "<=")
			return m_context.mkExpr(CVC4::kind::LEQ, arguments[0], arguments[1]);
		else if (n == ">")
			return m_context.mkExpr(CVC4::kind::GT, arguments[0], arguments[1]);
		else if (n == ">=")
			return m_context.mkExpr(CVC4::kind::GEQ, arguments[0], arguments[1]);
		else if (n == "+")
			return m_context.mkExpr(CVC4::kind::PLUS, arguments[0], arguments[1]);
		else if (n == "-")
			return m_context.mkExpr(CVC4::kind::MINUS, arguments[0], arguments[1]);
		else if (n == "*")
			return m_context.mkExpr(CVC4::kind::MULT, arguments[0], arguments[1]);
		else if (n == "/")
			return m_context.mkExpr(CVC4::kind::INTS_DIVISION_TOTAL, arguments[0], arguments[1]);
		else if (n == "mod")
			return m_context.mkExpr(CVC4::kind::INTS_MODULUS, arguments[0], arguments[1]);
		else if (n == "select")
			return m_context.mkExpr(CVC4::kind::SELECT, arguments[0], arguments[1]);
		else if (n == "store")
			return m_context.mkExpr(CVC4::kind::STORE, arguments[0], arguments[1], arguments[2]);

		solAssert(false, "");
	}
	catch (CVC4::TypeCheckingException const& _e)
	{
		solAssert(false, _e.what());
	}
	catch (CVC4::Exception const& _e)
	{
		solAssert(false, _e.what());
	}

	solAssert(false, "");
}

CVC4::Type CVC4Interface::cvc4Sort(Sort const& _sort)
{
	switch (_sort.kind)
	{
	case Kind::Bool:
		return m_context.booleanType();
	case Kind::Int:
		return m_context.integerType();
	case Kind::Function:
	{
		FunctionSort const& fSort = dynamic_cast<FunctionSort const&>(_sort);
		return m_context.mkFunctionType(cvc4Sort(fSort.domain), cvc4Sort(*fSort.codomain));
	}
	case Kind::Array:
	{
		auto const& arraySort = dynamic_cast<ArraySort const&>(_sort);
		return m_context.mkArrayType(cvc4Sort(*arraySort.domain), cvc4Sort(*arraySort.range));
	}
	default:
		break;
	}
	solAssert(false, "");
	// Cannot be reached.
	return m_context.integerType();
}

vector<CVC4::Type> CVC4Interface::cvc4Sort(vector<SortPointer> const& _sorts)
{
	vector<CVC4::Type> cvc4Sorts;
	for (auto const& _sort: _sorts)
		cvc4Sorts.push_back(cvc4Sort(*_sort));
	return cvc4Sorts;
}
