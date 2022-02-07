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

using namespace solidity::test::fuzzer::lpsolver;
using namespace solidity::util;
using namespace std;

FuzzerSolverInterface::FuzzerSolverInterface(bool _supportModels):
	m_solver(_supportModels)
{
	m_solvingState.variableNames.emplace_back("");
}

LinearExpression FuzzerSolverInterface::constant(rational _value)
{
	return LinearExpression::factorForVariable(0, _value);
}

LinearExpression FuzzerSolverInterface::variable(
	rational _factor,
	string const& _variable
)
{
	return LinearExpression::factorForVariable(variableIndex(_variable), _factor);
}

void FuzzerSolverInterface::addLEConstraint(LinearExpression _lhs)
{
	// Move constant to RHS
	if (_lhs[0])
		_lhs[0] = -_lhs[0];
	m_solvingState.constraints.push_back({move(_lhs), false});
}

void FuzzerSolverInterface::addEQConstraint(LinearExpression _lhs)
{
	// Move constant to RHS
	if (_lhs[0])
		_lhs[0] = -_lhs[0];
	m_solvingState.constraints.push_back({move(_lhs), true});
}

LinearExpression FuzzerSolverInterface::linearExpression(vector<int> _factors)
{
	bool first = true;
	unsigned count = 0;
	LinearExpression lexp;
	for (auto f: _factors)
	{
		if (first)
		{
			first = false;
			lexp += constant(f);
		}
		else
			lexp += variable(f, "x" + to_string(count++));
	}
	return lexp;
}

void FuzzerSolverInterface::addEQConstraint(vector<int> _factors)
{
	addEQConstraint(linearExpression(_factors));
}

void FuzzerSolverInterface::addLEConstraint(vector<int> _factors)
{
	addLEConstraint(linearExpression(_factors));
}

void FuzzerSolverInterface::addConstraint(pair<bool, vector<int>> _constraint)
{
	if (_constraint.first)
		addEQConstraint(_constraint.second);
	else
		addLEConstraint(_constraint.second);
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

size_t FuzzerSolverInterface::variableIndex(string const& _name)
{
	if (m_solvingState.variableNames.empty())
		m_solvingState.variableNames.emplace_back("");
	auto index = findOffset(m_solvingState.variableNames, _name);
	if (!index)
	{
		index = m_solvingState.variableNames.size();
		m_solvingState.variableNames.emplace_back(_name);
	}
	return *index;
}
