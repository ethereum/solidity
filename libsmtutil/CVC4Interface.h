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

#pragma once

#include <libsmtutil/SolverInterface.h>

#include <cvc5/cvc5.h>

namespace solidity::smtutil
{

class CVC5Interface: public SolverInterface
{
public:
	/// Noncopyable.
	CVC5Interface(CVC5Interface const&) = delete;
	CVC5Interface& operator=(CVC5Interface const&) = delete;

	CVC5Interface(std::optional<unsigned> _queryTimeout = {});

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, SortPointer const&) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	cvc5::Term toCVC5Expr(Expression const& _expr);
	cvc5::Sort CVC5Sort(Sort const& _sort);
	std::vector<cvc5::Sort> CVC5Sort(std::vector<SortPointer> const& _sorts);

	cvc5::Solver m_solver;
	std::map<std::string, cvc5::Term> m_variables;

	// CVC5 "basic resources" limit.
	// This is used to make the runs more deterministic and platform/machine independent.
	// The tests start failing for CVC5 with less than 6000,
	// so using double that.
	static int const resourceLimit = 12000;
};

}
