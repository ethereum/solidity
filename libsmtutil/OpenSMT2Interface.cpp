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

#include <libsmtutil/OpenSMT2Interface.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;

OpenSMT2Interface::OpenSMT2Interface(optional<unsigned> /*_queryTimeout*/):
	m_opensmt(make_unique<Opensmt>(qf_lia, "OpenSMT")),
	m_logic(m_opensmt->getLIALogic()),
	m_solver(m_opensmt->getMainSolver())
{
}

void OpenSMT2Interface::reset()
{
}

void OpenSMT2Interface::push()
{
	m_solver.push();
}

void OpenSMT2Interface::pop()
{
	m_solver.pop();
}

void OpenSMT2Interface::declareVariable(string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort, "");
	m_variables[_name] = m_logic.mkVar(openSMT2Sort(*_sort), _name.c_str());
}

void OpenSMT2Interface::addAssertion(Expression const& _expr)
{
	m_solver.insertFormula(toOpenSMT2Expr(_expr));
}

pair<CheckResult, vector<string>> OpenSMT2Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	CheckResult result;
	vector<string> values;

	sstat res = m_solver.check();
	if (res == s_True)
		result = CheckResult::SATISFIABLE;
	else if (res == s_False)
		result = CheckResult::UNSATISFIABLE;
	else if (res == s_Undef)
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
	{
		for (Expression const& e: _expressionsToEvaluate)
			values.push_back(m_solver.getValue(toOpenSMT2Expr(e)).val);
	}

	return make_pair(result, values);
}

PTRef OpenSMT2Interface::toOpenSMT2Expr(Expression const& _expr)
{
	// Variable
	if (_expr.arguments.empty() && m_variables.count(_expr.name))
		return m_variables.at(_expr.name);

	vector<PTRef> arguments;
	for (auto const& arg: _expr.arguments)
		arguments.push_back(toOpenSMT2Expr(arg));

	try
	{
		string const& n = _expr.name;
		// Function application
		if (!arguments.empty() && m_variables.count(_expr.name))
			smtAssert(false, "Not implemented.");
		// Literal
		else if (arguments.empty())
		{
			if (n == "true")
				return m_logic.getTerm_true();
			else if (n == "false")
				return m_logic.getTerm_false();
			else if (auto sortSort = dynamic_pointer_cast<SortSort>(_expr.sort))
				return m_logic.mkVar(openSMT2Sort(*sortSort->inner), n.c_str());
			else
				try
				{
					return m_logic.mkConst(n.c_str());
				}
				catch (std::exception const& _e)
				{
					smtAssert(false, _e.what());
				}
		}

		smtAssert(_expr.hasCorrectArity(), "");
		if (n == "ite")
			return m_logic.mkIte(arguments[0], arguments[1], arguments[2]);
		else if (n == "not")
			return m_logic.mkNot(arguments[0]);
		else if (n == "and")
			return m_logic.mkAnd(arguments[0], arguments[1]);
		else if (n == "or")
			return m_logic.mkOr(arguments[0], arguments[1]);
		else if (n == "implies")
			return m_logic.mkImpl(arguments[0], arguments[1]);
		else if (n == "=")
			return m_logic.mkEq(arguments[0], arguments[1]);
		else if (n == "<")
			return m_logic.mkNumLt(arguments[0], arguments[1]);
		else if (n == "<=")
			return m_logic.mkNumLeq(arguments[0], arguments[1]);
		else if (n == ">")
			return m_logic.mkNumGt(arguments[0], arguments[1]);
		else if (n == ">=")
			return m_logic.mkNumGeq(arguments[0], arguments[1]);
		else if (n == "+")
			return m_logic.mkNumPlus(arguments[0], arguments[1]);
		else if (n == "-")
			return m_logic.mkNumMinus(arguments[0], arguments[1]);
		else if (n == "*")
			return m_logic.mkNumTimes(arguments[0], arguments[1]);
		else if (n == "/")
			return m_logic.mkNumDiv(arguments[0], arguments[1]);
		else if (n == "mod")
			smtAssert(false, "Not implemented.");
		else
			smtAssert(false, "Not implemented.");

		smtAssert(false, "");
	}
	catch (std::exception const& _e)
	{
		smtAssert(false, _e.what());
	}

	smtAssert(false, "");
}

SRef OpenSMT2Interface::openSMT2Sort(Sort const& _sort)
{
	switch (_sort.kind)
	{
	case Kind::Bool:
		return m_logic.getSort_bool();
	case Kind::Int:
		return m_logic.getSort_num();
	default:
		smtAssert(false, "Not implemented.");
	}
	smtAssert(false, "");
}

vector<SRef> OpenSMT2Interface::openSMT2Sort(vector<SortPointer> const& _sorts)
{
	vector<SRef> opensmt2Sorts;
	for (auto const& _sort: _sorts)
		opensmt2Sorts.push_back(openSMT2Sort(*_sort));
	return opensmt2Sorts;
}
