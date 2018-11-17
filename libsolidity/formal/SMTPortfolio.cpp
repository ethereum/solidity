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

#include <libsolidity/formal/SMTPortfolio.h>

#ifdef HAVE_Z3
#include <libsolidity/formal/Z3Interface.h>
#endif
#ifdef HAVE_CVC4
#include <libsolidity/formal/CVC4Interface.h>
#endif
#if !defined (HAVE_Z3) && !defined (HAVE_CVC4)
#include <libsolidity/formal/SMTLib2Interface.h>
#endif

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::smt;

SMTPortfolio::SMTPortfolio(ReadCallback::Callback const& _readCallback)
{
#ifdef HAVE_Z3
	m_solvers.emplace_back(make_shared<smt::Z3Interface>());
#endif
#ifdef HAVE_CVC4
	m_solvers.emplace_back(make_shared<smt::CVC4Interface>());
#endif
#if !defined (HAVE_Z3) && !defined (HAVE_CVC4)
	m_solvers.emplace_back(make_shared<smt::SMTLib2Interface>(_readCallback)),
#endif
	(void)_readCallback;
}

void SMTPortfolio::reset()
{
	for (auto s : m_solvers)
		s->reset();
}

void SMTPortfolio::push()
{
	for (auto s : m_solvers)
		s->push();
}

void SMTPortfolio::pop()
{
	for (auto s : m_solvers)
		s->pop();
}

void SMTPortfolio::declareFunction(string _name, vector<Sort> const& _domain, Sort _codomain)
{
	for (auto s : m_solvers)
		s->declareFunction(_name, _domain, _codomain);
}

void SMTPortfolio::declareInteger(string _name)
{
	for (auto s : m_solvers)
		s->declareInteger(_name);
}

void SMTPortfolio::declareBool(string _name)
{
	for (auto s : m_solvers)
		s->declareBool(_name);
}

void SMTPortfolio::addAssertion(Expression const& _expr)
{
	for (auto s : m_solvers)
		s->addAssertion(_expr);
}

/*
 * Broadcasts the SMT query to all solvers and returns a single result.
 * This comment explains how this result is decided.
 *
 * When a solver is queried, there are four possible answers:
 *   SATISFIABLE (SAT), UNSATISFIABLE (UNSAT), UNKNOWN, CONFLICTING, ERROR
 * We say that a solver _answered_ the query if it returns either:
 *   SAT or UNSAT
 * A solver did not answer the query if it returns either:
 *   UNKNOWN (it tried but couldn't solve it) or ERROR (crash, internal error, API error, etc).
 *
 * Ideally all solvers answer the query and agree on what the answer is
 * (all say SAT or all say UNSAT).
 *
 * The actual logic as as follows:
 * 1) If at least one solver answers the query, all the non-answer results are ignored.
 *   Here SAT/UNSAT is preferred over UNKNOWN since it's an actual answer, and over ERROR
 *   because one buggy solver/integration shouldn't break the portfolio.
 *
 * 2) If at least one solver answers SAT and at least one answers UNSAT, at least one of them is buggy
 * and the result is CONFLICTING.
 *   In the future if we have more than 2 solvers enabled we could go with the majority.
 *
 * 3) If NO solver answers the query:
 *   If at least one solver returned UNKNOWN (where the rest returned ERROR), the result is UNKNOWN.
 *   This is preferred over ERROR since the SMTChecker might decide to abstract the query
 *   when it is told that this is a hard query to solve.
 *
 *   If all solvers return ERROR, the result is ERROR.
*/
pair<CheckResult, vector<string>> SMTPortfolio::check(vector<Expression> const& _expressionsToEvaluate)
{
	CheckResult lastResult = CheckResult::ERROR;
	vector<string> finalValues;
	for (auto s : m_solvers)
	{
		CheckResult result;
		vector<string> values;
		tie(result, values) = s->check(_expressionsToEvaluate);
		if (solverAnswered(result))
		{
			if (!solverAnswered(lastResult))
			{
				lastResult = result;
				finalValues = std::move(values);
			}
			else if (lastResult != result)
			{
				lastResult = CheckResult::CONFLICTING;
				break;
			}
		}
		else if (result == CheckResult::UNKNOWN && lastResult == CheckResult::ERROR)
			lastResult = result;
	}
	return make_pair(lastResult, finalValues);
}

bool SMTPortfolio::solverAnswered(CheckResult result)
{
	return result == CheckResult::SATISFIABLE || result == CheckResult::UNSATISFIABLE;
}
