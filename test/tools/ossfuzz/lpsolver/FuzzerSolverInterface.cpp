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

#include <set>

using namespace solidity::test::fuzzer::lpsolver;
using namespace solidity::util;
using namespace std;
using namespace z3;

FuzzerSolverInterface::FuzzerSolverInterface(bool _supportModels):
	m_lpSolver(_supportModels),
	m_z3Solver(m_z3Ctx)
{
	m_lpSolvingState.variableNames.emplace_back("");
	params z3Params(m_z3Ctx);
	z3Params.set(":timeout", static_cast<unsigned>(10)); // in milliseconds
	m_z3Solver.set(z3Params);
}

LinearExpression FuzzerSolverInterface::linearExpression(vector<int> _factors)
{
	LinearExpression lexp;
	lexp.resize(_factors.size());
	for (auto&& [index, value]: _factors | ranges::views::enumerate)
		lexp[index] = rational{value};
	return lexp;
}

void FuzzerSolverInterface::addLPConstraint(pair<bool, vector<int>> _constraint)
{
	m_lpSolvingState.constraints.push_back(
		{linearExpression(move(_constraint.second)), _constraint.first, {}}
	);
}

void FuzzerSolverInterface::addLPVariable(string _varName)
{
	if (
		find(
			m_lpSolvingState.variableNames.begin(),
			m_lpSolvingState.variableNames.end(),
			_varName
		) == m_lpSolvingState.variableNames.end()
	)
		m_lpSolvingState.variableNames.emplace_back(_varName);
}

void FuzzerSolverInterface::addZ3Constraint(pair<bool, vector<int>> _constraint)
{
	bool isEquality = _constraint.first;
	expr reduce = m_z3Ctx.real_val(0);
	for (auto&& [index, value]: _constraint.second | ranges::views::enumerate)
	{
		if (index != 0 && value != 0)
		{
			string varName = "x" + to_string(index - 1);
			// Add variable name to LP solving state to aid debugging
			addLPVariable(varName);
			expr var = m_z3Ctx.real_const(varName.c_str());
			expr factor = m_z3Ctx.int_val(value);
			reduce = reduce + var * factor;
			m_z3Solver.add(var >= 0);
		}
	}
	if (isEquality)
		m_z3Solver.add(reduce == _constraint.second[0]);
	else
		m_z3Solver.add(reduce <= _constraint.second[0]);
}

void FuzzerSolverInterface::addLPConstraints(vector<pair<bool, vector<int>>> _constraints)
{
	for (auto c: _constraints)
		addLPConstraint(c);
}

void FuzzerSolverInterface::addZ3Constraints(vector<pair<bool, vector<int>>> _constraints)
{
	for (auto c: _constraints)
		addZ3Constraint(c);
}

Solution FuzzerSolverInterface::checkLP()
{
	return m_lpSolver.check(m_lpSolvingState);
}

check_result FuzzerSolverInterface::checkZ3()
{
	return m_z3Solver.check();
}

string FuzzerSolverInterface::checkLPResult()
{
	m_lpResult = lpResult(checkLP().first);
	return m_lpResult;
}

string FuzzerSolverInterface::checkZ3Result()
{
	m_z3Result = z3Result(checkZ3());
	return m_z3Result;
}

bool FuzzerSolverInterface::differentialCheck(vector<pair<bool, vector<int>>> _constraints)
{
	addZ3Constraints(_constraints);
	string z3Result = checkZ3Result();
	// There is no point in continuing if z3 (the quicker solver) returns
	// unknown.
	if (z3Result == "unknown")
		return true;
	addLPConstraints(_constraints);
	string lpResult = checkLPResult();
	bool checkFailed = ((z3Result == "infeasible") && (lpResult == "feasible")) ||
		((z3Result == "feasible") && (lpResult == "infeasible"));
	return !checkFailed;
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

string FuzzerSolverInterface::z3Result(check_result _result)
{
	switch (_result)
	{
	case check_result::unsat:
		return "infeasible";
	case check_result::sat:
		return "feasible";
	case check_result::unknown:
		return "unknown";
	default:
		solAssert(false, "Invalid Z3 result");
	}
}
