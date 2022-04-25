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

namespace solidity::smtutil
{

class DLSolver: public SolverInterface
{
public:
	/// Noncopyable.
	DLSolver(DLSolver const&) = delete;
	DLSolver& operator=(DLSolver const&) = delete;

	DLSolver(std::optional<unsigned> _queryTimeout = {});

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	CheckResult solve();
	void initGraphs();
	unsigned uniqueId();

	// Stack of DL graphs and unique ids,
	// used for incremental solving.
	std::vector<
		std::map<unsigned, std::map<unsigned, bigint>>
	> m_graphs;

	std::map<std::string, unsigned> m_variables;

	unsigned m_lastId = 0;
};

}
