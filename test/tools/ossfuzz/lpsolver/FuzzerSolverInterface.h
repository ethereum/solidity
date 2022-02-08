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
/**
 * Implements the Fuzzer-Solver interface.
 */

#pragma once

#include <libsolutil/LP.h>
#include <libsolutil/LinearExpression.h>

#include <string>

namespace solidity::test::fuzzer::lpsolver
{

using solution = std::pair<
    solidity::util::LPResult,
	std::map<std::string, solidity::util::rational>
>;

class FuzzerSolverInterface
{
public:
	FuzzerSolverInterface(bool _supportModels);

	/// Adds @param _constraint to LP solver.
	void addConstraint(std::pair<bool, std::vector<int>> _constraint);

	/// Adds @param _constraints to LP solver.
	void addConstraints(std::vector<std::pair<bool, std::vector<int>>> _constraints);

	/// @returns linear expression created from @param _factors.
	solidity::util::LinearExpression linearExpression(std::vector<int> _factors);

	/// Queries LP solver and @returns solution.
	solution check();

	/// Queries LP solver and @returns sat result as string.
	std::string checkResult();

private:
	/// @returns LP result as string.
	std::string lpResult(solidity::util::LPResult _result);

	solidity::util::LPSolver m_solver;
	solidity::util::SolvingState m_solvingState;
};
}
