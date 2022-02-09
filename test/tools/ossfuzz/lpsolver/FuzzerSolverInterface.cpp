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

#include <test/tools/ossfuzz/lpsolver/FuzzerSolverInterface.h>

#include <range/v3/view/enumerate.hpp>

using namespace solidity::test::fuzzer::lpsolver;
using namespace solidity::util;
using namespace std;

FuzzerSolverInterface::FuzzerSolverInterface(bool _supportModels):
	m_solver(_supportModels)
{
	m_solvingState.variableNames.emplace_back("");
}

LinearExpression FuzzerSolverInterface::linearExpression(vector<int> _factors)
{
	LinearExpression lexp;
	lexp.resize(_factors.size());
	for (auto&& [index, value]: _factors | ranges::views::enumerate)
		lexp[index] = rational{value};
	return lexp;
}

void FuzzerSolverInterface::addConstraint(pair<bool, vector<int>> _constraint)
{
	m_solvingState.constraints.push_back({linearExpression(move(_constraint.second)), _constraint.first});
}

void FuzzerSolverInterface::addConstraints(vector<pair<bool, vector<int>>> _constraints)
{
	for (auto c: _constraints)
		addConstraint(c);
}

solution FuzzerSolverInterface::check()
{
	return m_solver.check(m_solvingState);
}

string FuzzerSolverInterface::checkResult()
{
	auto r = check();
	return lpResult(r.first);
}

string FuzzerSolverInterface::lpResult(LPResult _result)
{
	switch (_result)
	{
	case LPResult::Unknown:
		return "unknown";
	case LPResult::Unbounded:
		return "unbounded";
	case LPResult::Feasible:
		return "feasible";
	case LPResult::Infeasible:
		return "infeasible";
	}
}
