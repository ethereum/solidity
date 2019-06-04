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

#include <libsolidity/formal/SolverInterface.h>

namespace dev
{
namespace solidity
{
namespace smt
{

/* Interface for constrained Horn solvers.
 * Derives from SolverInterface to use variable handling.
 */
class CHCSolverInterface: public SolverInterface
{
public:
	virtual ~CHCSolverInterface() = default;

	void reset() override = 0;
	void push() override { solAssert(false, "Horn solver does not support push."); }
	void pop() override { solAssert(false, "Horn solver does not support pop."); }
	void addAssertion(Expression const&) override { solAssert(false, "Horn solver does not support assertions."); }

	/// Takes a function declaration as a relation.
	virtual void registerRelation(Expression const& _expr) = 0;

	/// Takes an implication and adds as rule.
	/// Needs to bound all vars as universally quantified.
	virtual void addRule(Expression const& _expr, std::string const& _name) = 0;

	/// Takes a function application and checks for reachability.
	std::pair<CheckResult, std::vector<std::string>> check(
		std::vector<Expression> const& _expressionsToEvaluate
	) override
	{
		solAssert(_expressionsToEvaluate.size() == 1, "");
		return query(_expressionsToEvaluate.front());
	}

	virtual std::pair<CheckResult, std::vector<std::string>> query(
		Expression const& _expr
	) = 0;
};

}
}
}
