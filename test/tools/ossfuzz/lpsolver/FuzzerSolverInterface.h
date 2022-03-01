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

#include <z3++.h>

#include <string>

namespace solidity::test::fuzzer::lpsolver
{

using Model = std::map<std::string, solidity::util::rational>;
using ReasonSet = std::set<size_t>;
using Solution = std::pair<solidity::util::LPResult, std::variant<Model, ReasonSet>>;

class FuzzerSolverInterface
{
public:
	FuzzerSolverInterface(bool _supportModels);

	/// Adds @param _constraint to LP solver.
	void addLPConstraint(std::pair<bool, std::vector<int>> _constraint);

	/// Adds @param _constraint to Z3 solver.
	void addZ3Constraint(std::pair<bool, std::vector<int>> _constraint);

	/// Adds @param _constraints to LP solver.
	void addLPConstraints(std::vector<std::pair<bool, std::vector<int>>> _constraints);

	/// Adds @param _constraints to Z3 solver.
	void addZ3Constraints(std::vector<std::pair<bool, std::vector<int>>> _constraints);

	/// @returns linear expression created from @param _factors.
	solidity::util::LinearExpression linearExpression(std::vector<int> _factors);

	/// Queries LP solver and @returns solution.
	Solution checkLP();

	/// Queries Z3 solver and @returns solution.
	z3::check_result checkZ3();

	/// Queries LP solver and @returns sat result as string.
	std::string checkLPResult();

	/// Queries Z3 solver and @returns sat result as string.
	std::string checkZ3Result();

	/// @returns true if both the LP and the Z3 solver return an identical result on
	/// @param _constraints, false otherwise.
	bool differentialCheck(std::vector<std::pair<bool, std::vector<int>>> _constraints);
private:
	/// @returns LP result as string.
	std::string lpResult(solidity::util::LPResult _result);

	/// @returns Z3 result as string.
	std::string z3Result(z3::check_result _result);

	/// Adds variable name to LP solver solving state.
	void addLPVariable(std::string _varName);

	solidity::util::LPSolver m_lpSolver;
	solidity::util::SolvingState m_lpSolvingState;
	z3::context m_z3Ctx;
	z3::solver m_z3Solver;
public:
	std::string m_lpResult;
	std::string m_z3Result;
};
}
