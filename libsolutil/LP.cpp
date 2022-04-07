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

#include <libsolutil/LP.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/LinearExpression.h>
#include <libsolutil/cxx20.h>

#include <liblangutil/Exceptions.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/iterator/operations.hpp>

#include <boost/range/algorithm_ext/erase.hpp>

#include <optional>
#include <stack>

using namespace std;
using namespace solidity;
using namespace solidity::util;

using rational = boost::rational<bigint>;


namespace
{

/// Disjunctively combined two vectors of bools.
inline std::vector<bool>& operator|=(std::vector<bool>& _x, std::vector<bool> const& _y)
{
	solAssert(_x.size() == _y.size(), "");
	for (size_t i = 0; i < _x.size(); ++i)
		if (_y[i])
			_x[i] = true;
	return _x;
}

string toString(rational const& _x)
{
	if (_x == bigint(1) << 256)
		return "2**256";
	else if (_x == (bigint(1) << 256) - 1)
		return "2**256-1";
	else if (_x.denominator() == 1)
		return ::toString(_x.numerator());
	else
		return ::toString(_x.numerator()) + "/" + ::toString(_x.denominator());
}

string reasonToString(ReasonSet const& _reasons, size_t _minSize)
{
	auto reasonsAsStrings = _reasons | ranges::views::transform([](size_t _r) { return to_string(_r); });
	string result = "[" + joinHumanReadable(reasonsAsStrings) + "]";
	if (result.size() < _minSize)
		result.resize(_minSize, ' ');
	return result;
}

}


bool Constraint::operator<(Constraint const& _other) const
{
	if (equality != _other.equality)
		return equality < _other.equality;

	for (size_t i = 0; i < max(data.size(), _other.data.size()); ++i)
		if (rational diff = data.get(i) - _other.data.get(i))
		{
			//cout << "Exit after " << i << endl;
			return diff < 0;
		}
	//cout << "full traversal of " << max(data.size(), _other.data.size()) << endl;

	return false;
}

bool Constraint::operator==(Constraint const& _other) const
{
	if (equality != _other.equality)
		return false;

	for (size_t i = 0; i < max(data.size(), _other.data.size()); ++i)
		if (data.get(i) != _other.data.get(i))
		{
			//cout << "Exit after " << i << endl;
			return false;
		}
	//cout << "full traversal of " << max(data.size(), _other.data.size()) << endl;

	return true;
}

bool SolvingState::Compare::operator()(SolvingState const& _a, SolvingState const& _b) const
{
	if (!considerVariableNames || _a.variableNames == _b.variableNames)
	{
		if (_a.bounds == _b.bounds)
			return _a.constraints < _b.constraints;
		else
			return _a.bounds < _b.bounds;
	}
	else
		return _a.variableNames < _b.variableNames;
}

set<size_t> SolvingState::reasons() const
{
	set<size_t> ret;
	for (Bounds const& b: bounds)
		ret += b.lowerReasons + b.upperReasons;
	return ret;
}

string SolvingState::toString() const
{
	size_t const reasonLength = 10;
	string result;
	for (Constraint const& constraint: constraints)
	{
		vector<string> line;
		for (auto&& [index, multiplier]: constraint.data.enumerate())
			if (index > 0 && multiplier != 0)
			{
				string mult =
					multiplier == -1 ?
					"-" :
					multiplier == 1 ?
					"" :
					::toString(multiplier) + " ";
				line.emplace_back(mult + variableNames.at(index));
			}
		result +=
			joinHumanReadable(line, " + ") +
			(constraint.equality ? "  = " : " <= ") +
			::toString(constraint.data.front()) +
			"\n";
	}
	result += "Bounds:\n";
	for (auto&& [index, bounds]: bounds | ranges::views::enumerate)
	{
		if (!bounds.lower && !bounds.upper)
			continue;
		if (bounds.lower)
			result +=
				reasonToString(bounds.lowerReasons, reasonLength) +
				::toString(*bounds.lower) + " <= ";
		result += variableNames.at(index);
		if (bounds.upper)
			result += " <= "s + ::toString(*bounds.upper) + " " + reasonToString(bounds.upperReasons, 0);
		result += "\n";
	}
	return result;
}


void LPSolver::addConstraint(Constraint const& _constraint, optional<size_t> _reason)
{
	// TODO at this point, we could also determine if it is a fixed variable.
	// (maybe even taking the bounds on existing variables into account)
	// If we do this, we have to take reasons into account properly!
	set<size_t> touchedProblems;
	for (auto const& [index, entry]: _constraint.data.enumerateTail())
		if (entry)
			if (m_subProblemsPerVariable.count(index))
				touchedProblems.emplace(m_subProblemsPerVariable.at(index));

	if (touchedProblems.empty())
	{
		//cout << "Creating new sub problem." << endl;
		// TODO we could find an empty spot for the pointer.
		m_subProblems.emplace_back(make_shared<SubProblem>());
		solAssert(!m_subProblems.back()->sealed);
		touchedProblems.emplace(m_subProblems.size() - 1);
	}
	for (size_t problemToErase: touchedProblems | ranges::views::tail | ranges::views::reverse)
		combineSubProblems(*touchedProblems.begin(), problemToErase);
	addConstraintToSubProblem(*touchedProblems.begin(), _constraint, move(_reason));
	//cout << "Added constraint:\n" << toString() << endl;
}

void LPSolver::setVariableName(size_t _variable, string _name)
{
	// TODO it might be constly to do this before we know hich variables relate to which

	SubProblem& p = unsealForVariable(_variable);
	p.variables[p.varMapping.at(_variable)].name = move(_name);
}

void LPSolver::addLowerBound(size_t _variable, rational _bound)
{
	SubProblem& p = unsealForVariable(_variable);
	size_t innerIndex = p.varMapping.at(_variable);
	Variable& var = p.variables[innerIndex];
	if (!var.bounds.lower || *var.bounds.lower < _bound)
	{
		var.bounds.lower = move(_bound);
		p.variablesPotentiallyOutOfBounds.insert(innerIndex);
	}
}

void LPSolver::addUpperBound(size_t _variable, rational _bound)
{
	SubProblem& p = unsealForVariable(_variable);
	size_t innerIndex = p.varMapping.at(_variable);
	Variable& var = p.variables[innerIndex];
	if (!var.bounds.upper || *var.bounds.upper > _bound)
	{
		var.bounds.upper = move(_bound);
		p.variablesPotentiallyOutOfBounds.insert(innerIndex);
	}
}

pair<LPResult, ReasonSet> LPSolver::check()
{
	for (auto&& [index, problem]: m_subProblems | ranges::views::enumerate)
		if (problem)
			problem->sealed = true;

	for (auto&& [index, problem]: m_subProblems | ranges::views::enumerate)
	{
		if (!problem)
			continue;
		if (!problem->result)
			problem->result = problem->check();

		if (*problem->result == LPResult::Infeasible)
			return {LPResult::Infeasible, problem->reasons};
	}
	//cout << "Feasible:\n" << toString() << endl;
	return {LPResult::Feasible, {}};
}

string LPSolver::toString() const
{
	string result = "LP Solver state:\n";
	for (auto const& problem: m_subProblems)
		if (problem)
			result += problem->toString();
	return result;
}

map<string, rational> LPSolver::model() const
{
	map<string, rational> result;
	for (auto const& problem: m_subProblems)
		if (problem)
			for (auto&& [outerIndex, innerIndex]: problem->varMapping)
				result[problem->variables[innerIndex].name] = problem->variables[innerIndex].value;
	return result;
}

LPSolver::SubProblem& LPSolver::unseal(size_t _problemIndex)
{
	shared_ptr<SubProblem>& problem = m_subProblems[_problemIndex];
	solAssert(problem);
	if (problem->sealed)
		problem = make_shared<SubProblem>(*problem);
	problem->sealed = false;
	problem->result = nullopt;
	return *problem;
}

LPSolver::SubProblem& LPSolver::unsealForVariable(size_t _outerIndex)
{
	if (!m_subProblemsPerVariable.count(_outerIndex))
	{
		m_subProblems.emplace_back(make_shared<SubProblem>());
		addOuterVariableToSubProblem(m_subProblems.size() - 1, _outerIndex);
	}
	return unseal(m_subProblemsPerVariable.at(_outerIndex));
}

void LPSolver::combineSubProblems(size_t _combineInto, size_t _combineFrom)
{
	//cout << "Combining\n" << m_subProblems.at(_combineFrom)->toString();
	//cout << "\ninto\n" << m_subProblems.at(_combineInto)->toString();
	SubProblem& combineInto = unseal(_combineInto);
	SubProblem const& combineFrom = *m_subProblems[_combineFrom];

	size_t varShift = combineInto.variables.size();
	size_t rowShift = combineInto.factors.size();
	size_t newRowLength = combineInto.variables.size() + combineFrom.variables.size();
	for (LinearExpression& row: combineInto.factors)
		row.resize(newRowLength);
	for (LinearExpression const& row: combineFrom.factors)
	{
		LinearExpression shiftedRow;
		shiftedRow.resize(newRowLength);
		for (auto&& [index, f]: row.enumerate())
			shiftedRow[varShift + index] = f;
		combineInto.factors.emplace_back(move(shiftedRow));
	}
	combineInto.variables += combineFrom.variables;
	for (auto const& index: combineFrom.variablesPotentiallyOutOfBounds)
		combineInto.variablesPotentiallyOutOfBounds.insert(index + varShift);
	for (auto&& [index, row]: combineFrom.basicVariables)
		combineInto.basicVariables.emplace(index + varShift, row + rowShift);
	for (auto&& [outerIndex, innerIndex]: combineFrom.varMapping)
		combineInto.varMapping.emplace(outerIndex, innerIndex + varShift);
	combineInto.reasons += combineFrom.reasons;

	for (auto& item: m_subProblemsPerVariable)
		if (item.second == _combineFrom)
			item.second = _combineInto;

	m_subProblems[_combineFrom].reset();
	//cout << "result: \n" << m_subProblems.at(_combineInto)->toString();
	//cout << "------------------------------\n";
}

// TODO move this function into the problem struct and make it erturn set of vaiables added

void LPSolver::addConstraintToSubProblem(
	size_t _subProblem,
	Constraint const& _constraint,
	std::optional<size_t> _reason
)
{
	// TODO opt:
	// Add "fixed variables" at general state (above sub problems)
	// replace all fixed variables in constraint by their values
	// If constaint is direct constraint on variable, just add it to its bounds
	// If constraint results in variable being fixed,
	// then push that to the general state as 'fixed variables'
	// Remove the variable from the subproblem (so we can more efficiently split)
	// If we remove the var, it is a bit more tricky because we have to store the reason
	// together with the var.

	SubProblem& problem = unseal(_subProblem);
	if (_reason)
		problem.reasons.insert(*_reason);

	size_t numVariables = 0;
	size_t latestVariableIndex = size_t(-1);
	// Make all variables available and check if it is a simple bound on a variable.
	for (auto const& [index, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			latestVariableIndex = index;
			numVariables++;
			if (!problem.varMapping.count(index))
				addOuterVariableToSubProblem(_subProblem, index);
		}
	if (numVariables == 1)
	{
		// Add this as direct bound.
		// TODO we could avoid some of the steps by introducing an "addUpperBound"
		// function on the subproblem.
		rational factor = _constraint.data[latestVariableIndex];
		rational bound = _constraint.data.front() / factor;
		if (factor > 0 || _constraint.equality)
			addUpperBound(latestVariableIndex, bound);
		if (factor < 0 || _constraint.equality)
			addLowerBound(latestVariableIndex, bound);
		return;
	}

	// Introduce the slack variable.
	size_t slackIndex = addNewVariableToSubProblem(_subProblem);
	// Name is only needed for printing
	//problem.variables[slackIndex].name = "_s" + to_string(m_slackVariableCounter++);
	problem.basicVariables[slackIndex] = problem.factors.size();
	if (_constraint.equality)
		problem.variables[slackIndex].bounds.lower = _constraint.data[0];
	problem.variables[slackIndex].bounds.upper = _constraint.data[0];
	// TODO it is a basic var, so we don't add it, unless we use this for basic vars.
	//problem.variablesPotentiallyOutOfBounds.insert(slackIndex);

	// Compress the constraint, i.e. turn outer variable indices into
	// inner variable indices.
	rational valueForSlack;
	LinearExpression compressedConstraint;
	LinearExpression basicVarNullifier;
	compressedConstraint.resize(problem.variables.size());
	for (auto const& [outerIndex, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			size_t innerIndex = problem.varMapping.at(outerIndex);
			if (problem.basicVariables.count(innerIndex))
			{
				// We cannot add basic variables directly, so replace them by their row.
				basicVarNullifier += entry * problem.factors.at(problem.basicVariables[innerIndex]);
				basicVarNullifier[innerIndex] = {};
			}
			else
				compressedConstraint[innerIndex] = entry;
			valueForSlack += problem.variables[innerIndex].value * entry;
		}
	compressedConstraint += move(basicVarNullifier);

	compressedConstraint[slackIndex] = -1;
	problem.factors.emplace_back(move(compressedConstraint));
	problem.basicVariables[slackIndex] = problem.factors.size() - 1;
	problem.variables[slackIndex].value = valueForSlack;
}

void LPSolver::addOuterVariableToSubProblem(size_t _subProblem, size_t _outerIndex)
{
	size_t index = addNewVariableToSubProblem(_subProblem);
	unseal(_subProblem).varMapping.emplace(_outerIndex, index);
	m_subProblemsPerVariable[_outerIndex] = _subProblem;
}

size_t LPSolver::addNewVariableToSubProblem(size_t _subProblem)
{
	SubProblem& problem = unseal(_subProblem);
	size_t index = problem.variables.size();
	for (LinearExpression& c: problem.factors)
		c.resize(index + 1);
	problem.variables.emplace_back();
	return index;
}

LPResult LPSolver::SubProblem::check()
{

	// TODO one third of the computing time (inclusive) in this function
	// is spent on "operator<" - maybe we can cache "is in bounds" for variables
	// and invalidate that in the update procedures.

//	cout << "checking..." << endl;
//	cout << toString() << endl;
//	cout << "----------------------------" << endl;
//	cout << "fixing non-basic..." << endl;
	// Adjust the assignments so we satisfy the bounds of the non-basic variables.
	if (!correctNonbasic())
		return LPResult::Infeasible;

	// Now try to make the basic variables happy, pivoting if necessary.

//	cout << "fixed non-basic." << endl;
//	cout << toString() << endl;
//	cout << "----------------------------" << endl;

	// TODO bound number of iterations
	while (auto bvi = firstConflictingBasicVariable())
	{
		Variable const& basicVar = variables[*bvi];
//		cout << toString() << endl;
//		cout << "Fixing basic " << basicVar.name << endl;
//		cout << "----------------------------" << endl;
		if (basicVar.bounds.lower && basicVar.bounds.upper)
			solAssert(*basicVar.bounds.lower <= *basicVar.bounds.upper);
		if (basicVar.bounds.lower && basicVar.value < *basicVar.bounds.lower)
		{
			if (auto replacementVar = firstReplacementVar(*bvi, true))
			{
//				cout << "Replacing by " << variables[*replacementVar].name << endl;

				pivotAndUpdate(*bvi, *basicVar.bounds.lower, *replacementVar);
			}
			else
				return LPResult::Infeasible;
		}
		else if (basicVar.bounds.upper && basicVar.value > *basicVar.bounds.upper)
		{
			if (auto replacementVar = firstReplacementVar(*bvi, false))
			{
//				cout << "Replacing by " << variables[*replacementVar].name << endl;
				pivotAndUpdate(*bvi, *basicVar.bounds.upper, *replacementVar);
			}
			else
				return LPResult::Infeasible;
		}
//		cout << "Fixed basic " << basicVar.name << endl;
//		cout << toString() << endl;
	}

	return LPResult::Feasible;
}

string LPSolver::SubProblem::toString() const
{
	string resultString;
	for (auto&& [i, v]: variables | ranges::views::enumerate)
	{
		if (v.bounds.lower)
			resultString += ::toString(*v.bounds.lower) + " <= ";
		else
			resultString += "       ";
		resultString += v.name;
		if (v.bounds.upper)
			resultString += " <= " + ::toString(*v.bounds.upper);
		else
			resultString += "       ";
		resultString += "   := " + ::toString(v.value) + "\n";
	}
	for (auto&& [rowIndex, row]: factors | ranges::views::enumerate)
	{
		string basicVarPrefix;
		string rowString;
		for (auto&& [i, f]: row.enumerate())
		{
			if (basicVariables.count(i) && basicVariables.at(i) == rowIndex)
			{
				solAssert(f == -1);
				solAssert(basicVarPrefix.empty());
				basicVarPrefix = variables[i].name + " = ";
			}
			else if (f != 0)
			{
				string joiner = f < 0 ? " - " : f > 0 && !rowString.empty() ? " + " : " ";
				string factor = f == 1 || f == -1 ? "" : ::toString(abs(f)) + " ";
				string var = variables[i].name;
				rowString += joiner + factor + var;
			}
		}
		resultString += basicVarPrefix + rowString + "\n";
	}
	if (result)
	{
		if (*result == LPResult::Feasible)
			resultString += "result: feasible\n";
		else
			resultString += "result: infeasible\n";
	}
	else
		resultString += "result: unknown\n";


	return resultString + "----\n";
}

bool LPSolver::SubProblem::correctNonbasic()
{
	set<size_t> toCorrect;
	swap(toCorrect, variablesPotentiallyOutOfBounds);
	for (size_t i: toCorrect)
	{
		Variable& var = variables.at(i);
		if (var.bounds.lower && var.bounds.upper && *var.bounds.lower > *var.bounds.upper)
			return false;
		if (basicVariables.count(i))
		{
			variablesPotentiallyOutOfBounds.insert(i);
			continue;
		}
		if (!var.bounds.lower && !var.bounds.upper)
			continue;
		if (var.bounds.lower && var.value < *var.bounds.lower)
			update(i, *var.bounds.lower);
		else if (var.bounds.upper && var.value > *var.bounds.upper)
			update(i, *var.bounds.upper);
	}
	return true;
}

void LPSolver::SubProblem::update(size_t _varIndex, rational const& _value)
{
	rational delta = _value - variables[_varIndex].value;
	variables[_varIndex].value = _value;
	for (size_t j = 0; j < variables.size(); j++)
		if (basicVariables.count(j) && factors[basicVariables.at(j)][_varIndex])
		{
			variables[j].value += delta * factors[basicVariables.at(j)][_varIndex];
			//variablesPotentiallyOutOfBounds.insert(j);
		}

}

optional<size_t> LPSolver::SubProblem::firstConflictingBasicVariable() const
{
	// TODO we could use "variablesPotentiallyOutOfBounds" here.
	for (auto&& [i, row]: basicVariables)
	{
		Variable const& variable = variables[i];
		if (
			(variable.bounds.lower && variable.value < *variable.bounds.lower) ||
			(variable.bounds.upper && variable.value > *variable.bounds.upper)
		)
			return i;
	}
	return nullopt;
}

optional<size_t> LPSolver::SubProblem::firstReplacementVar(
	size_t _basicVarToReplace,
	bool _increasing
) const
{
	LinearExpression const& basicVarEquation = factors[basicVariables.at(_basicVarToReplace)];
	for (auto const& [i, factor]: basicVarEquation.enumerate())
	{
		if (i == _basicVarToReplace || !factor)
			continue;
		bool positive = factor > 0;
		if (!_increasing)
			positive = !positive;
		Variable const& candidate = variables.at(i);
		if (positive && (!candidate.bounds.upper || candidate.value < *candidate.bounds.upper))
			return i;
		if (!positive && (!candidate.bounds.lower || candidate.value > *candidate.bounds.lower))
			return i;
	}
	return nullopt;
}

void LPSolver::SubProblem::pivot(size_t _old, size_t _new)
{
	// Transform pivotRow such that the coefficient for _new is -1
	// Then use that to set all other coefficients for _new to zero.
	size_t pivotRow = basicVariables[_old];
	LinearExpression& pivotRowData = factors[pivotRow];

	rational pivot = pivotRowData[_new];
	solAssert(pivot != 0, "");
	if (pivot != -1)
		pivotRowData /= -pivot;
	solAssert(pivotRowData[_new] == rational(-1), "");

	auto subtractMultipleOfPivotRow = [&](LinearExpression& _row) {
		if (_row[_new] == 0)
			return;
		else if (_row[_new] == rational{1})
			_row += pivotRowData;
		else if (_row[_new] == rational{-1})
			_row -= pivotRowData;
		else
			_row += _row[_new] * pivotRowData;
	};

	for (size_t i = 0; i < factors.size(); ++i)
		if (i != pivotRow)
			subtractMultipleOfPivotRow(factors[i]);

	basicVariables.erase(_old);
	basicVariables[_new] = pivotRow;
}

void LPSolver::SubProblem::pivotAndUpdate(
	size_t _oldBasicVar,
	rational const& _newValue,
	size_t _newBasicVar
)
{
	rational theta = (_newValue - variables[_oldBasicVar].value) / factors[basicVariables[_oldBasicVar]][_newBasicVar];

	variables[_oldBasicVar].value = _newValue;
	variables[_newBasicVar].value += theta;

	for (auto const& [i, row]: basicVariables)
		if (i != _oldBasicVar && factors[row][_newBasicVar])
			variables[i].value += factors[row][_newBasicVar] * theta;

	pivot(_oldBasicVar, _newBasicVar);
}
