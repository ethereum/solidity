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

#include <libsolutil/LPIncremental.h>

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

//#define DEBUG

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

/*
string reasonToString(ReasonSet const& _reasons, size_t _minSize)
{
	auto reasonsAsStrings = _reasons | ranges::views::transform([](size_t _r) { return to_string(_r); });
	string result = "[" + joinHumanReadable(reasonsAsStrings) + "]";
	if (result.size() < _minSize)
		result.resize(_minSize, ' ');
	return result;
}
*/

}

bool Constraint::operator<(Constraint const& _other) const
{
	if (kind != _other.kind)
		return kind < _other.kind;

	for (size_t i = 0; i < max(data.size(), _other.data.size()); ++i)
		if (rational diff = data.get(i) - _other.data.get(i))
		{
			//cerr << "Exit after " << i << endl;
			return diff < 0;
		}
	//cerr << "full traversal of " << max(data.size(), _other.data.size()) << endl;

	return false;
}

bool Constraint::operator==(Constraint const& _other) const
{
	if (kind != _other.kind)
		return false;

	for (size_t i = 0; i < max(data.size(), _other.data.size()); ++i)
		if (data.get(i) != _other.data.get(i))
		{
			//cerr << "Exit after " << i << endl;
			return false;
		}
	//cerr << "full traversal of " << max(data.size(), _other.data.size()) << endl;

	return true;
}

string RationalWithDelta::toString() const
{
	string result = ::toString(m_main);
	if (m_delta)
		result +=
			(m_delta > 0 ? "+" : "-") +
			(abs(m_delta) == 1 ? "" : ::toString(abs(m_delta))) +
			"d";
	return result;
}

void LPSolver::addConstraint(Constraint const& _constraint)
{
#ifdef DEBUG
	cerr << "Adding constraint." << endl;
#endif
	result = nullopt;
	auto&& [varIndex, bounds] = constraintIntoVariableBounds(_constraint);
	addBounds(varIndex, bounds);
}

void LPSolver::addLowerBound(size_t _variable, RationalWithDelta _bound)
{
#ifdef DEBUG
	cerr << "Adding lower bound." << endl;
#endif
	result = nullopt;
	size_t innerIndex = maybeAddOuterVariable(_variable);
	addBounds(innerIndex, Bounds{move(_bound), {}});
}

void LPSolver::addUpperBound(size_t _variable, RationalWithDelta _bound)
{
#ifdef DEBUG
	cerr << "Adding upper bound." << endl;
#endif
	// TODO we could only reset the result if the bound changed anything.
	// then we could check if we already have a result insiche "check()"
	// and return early. Although this might be better done inside
	// activateConstraint.
	result = nullopt;
	size_t innerIndex = maybeAddOuterVariable(_variable);
	addBounds(innerIndex, Bounds{{}, move(_bound)});
}

void LPSolver::addConditionalConstraint(Constraint const& _constraint, size_t _reason)
{
#ifdef DEBUG
	cerr << "Adding conditional constraint." << endl;
#endif
	auto&& [varIndex, bounds] = constraintIntoVariableBounds(_constraint);
	solAssert(!reasonToBounds.count(_reason));
	reasonToBounds[_reason] = make_pair(varIndex, move(bounds));
}

void LPSolver::activateConstraint(size_t _reason)
{
#ifdef DEBUG
	cerr << "Activating constraint." << endl;
#endif
	result = nullopt;
	auto&& [varIndex, bounds] = reasonToBounds.at(_reason);
	Variable& var = variables[varIndex];
	bool savedBounds = false;
	if (bounds.lower && (!var.bounds.lower || *var.bounds.lower < *bounds.lower))
	{
		storedBounds.emplace_back(make_tuple(trailSize, varIndex, var.bounds, var.lowerReason, var.upperReason));
		savedBounds = true;
		var.bounds.lower = bounds.lower;
		var.lowerReason = _reason;
		if (var.value < *var.bounds.lower)
			variablesPotentiallyOutOfBounds.insert(varIndex);
	}
	if (bounds.upper && (!var.bounds.upper || *var.bounds.upper > *bounds.upper))
	{
		if (!savedBounds)
			storedBounds.emplace_back(make_tuple(trailSize, varIndex, var.bounds, var.lowerReason, var.upperReason));
		savedBounds = true;
		var.bounds.upper = bounds.upper;
		var.upperReason = _reason;
		if (var.value > *var.bounds.upper)
			variablesPotentiallyOutOfBounds.insert(varIndex);
	}
#ifdef DEBUG
	if (!savedBounds)
		cerr << "Did not change anything." << endl;
#endif
}

void LPSolver::setTrailSize(size_t _trailSize)
{
//	solAssert(_trailSize == 0 || _trailSize != trailSize);
	if (_trailSize > trailSize)
	{
#ifdef DEBUG
		cerr << "=== Advancing from " << trailSize << " to " << _trailSize << endl;
#endif
		solAssert(result == LPResult::Feasible);
		previousGoodValues.resize(variables.size());
		for (size_t i = 0; i < variables.size(); i++)
			previousGoodValues[i] = variables[i].value;
		variablesPotentiallyOutOfBounds.clear();
	}
	else if (_trailSize < trailSize)
	{
#ifdef DEBUG
		cerr << "=== Backtracking from " << trailSize << " to " << _trailSize << endl;
#endif
		while (!storedBounds.empty())
		{
			auto&& [ts, varIndex, bounds, lowerReason, upperReason] = storedBounds.back();
			//TODO should this be "<"?
			if (ts <= _trailSize)
				break;
			variables[varIndex].bounds = bounds;
			variables[varIndex].lowerReason = lowerReason;
			variables[varIndex].upperReason = upperReason;
			// TODO I think this is not needed because of "previousGoodValues
			// we can maybe assert it.
			//variablesPotentiallyOutOfBounds.insert(varIndex);
			storedBounds.pop_back();
		}
		for (size_t i = 0; i < previousGoodValues.size(); i++)
			variables.at(i).value = previousGoodValues[i];
		variablesPotentiallyOutOfBounds.clear();
		result = LPResult::Feasible;
	}
	trailSize = _trailSize;
}

#ifdef DEBUG
void LPSolver::setVariableName(size_t _variable, string _name)
{
	size_t index = maybeAddOuterVariable(_variable);
	variables[index].name = move(_name);
}
#else
void LPSolver::setVariableName(size_t, string)
{
}
#endif

optional<bool> LPSolver::recommendedPolarity(size_t _reason) const
{
	if (!reasonToBounds.count(_reason))
		return {};
	return {};
//	TODO: We cannot actually have a negative polarity for a reason / constraint!
//	We can recommend not to activate it, though...

}

pair<LPResult, ReasonSet> LPSolver::check()
{
	// TODO below is an old comment - but maybe we can optimize something to that effect
	// by moving functionality to 'activateConstraint'.

	// TODO one third of the computing time (inclusive) in this function
	// is spent on "operator<" - maybe we can cache "is in bounds" for variables
	// and invalidate that in the update procedures.

#ifdef DEBUG
	cerr << "checking..." << endl;
	cerr << toString() << endl;
	cerr << "----------------------------" << endl;
//	cerr << "fixing non-basic..." << endl;
#endif
	if (result == LPResult::Feasible)
		return make_pair(LPResult::Feasible, std::set<size_t>());
	result = nullopt;
	// Adjust the assignments so we satisfy the bounds of the non-basic variables.
	if (!correctNonbasic())
	{
#ifdef DEBUG
		cerr << "---> infeasible" << endl;
#endif
		result = LPResult::Infeasible;
		return make_pair(LPResult::Infeasible, reasons);
	}

	// Now try to make the basic variables happy, pivoting if necessary.

#ifdef DEBUG
//	cerr << "fixed non-basic." << endl;
//	cerr << toString() << endl;
//	cerr << "----------------------------" << endl;
#endif

	// TODO bound number of iterations
	while (auto bvi = firstConflictingBasicVariable())
	{
		Variable const& basicVar = variables[*bvi];
#ifdef DEBUG
//		cerr << "----------------------------" << endl;
//		cerr << "Fixing basic " << basicVar.name << endl;
#endif
		if (basicVar.bounds.lower && basicVar.bounds.upper)
			solAssert(*basicVar.bounds.lower <= *basicVar.bounds.upper);
		if (basicVar.bounds.lower && basicVar.value < *basicVar.bounds.lower)
		{
			if (auto replacementVar = firstReplacementVar(*bvi, true))
			{
#ifdef DEBUG
//				cerr << "Replacing by " << variables[*replacementVar].name << endl;
//				cerr << "Setting basic var to to " << basicVar.bounds.lower->m_main << endl;
#endif

				pivotAndUpdate(*bvi, *basicVar.bounds.lower, *replacementVar);
			}
			else
			{
#ifdef DEBUG
				cerr << "---> infeasible" << endl;
#endif
				result = LPResult::Infeasible;
				reasons = reasonsForUnsat(*bvi, true);
				return make_pair(LPResult::Infeasible, reasons);
			}
		}
		else if (basicVar.bounds.upper && basicVar.value > *basicVar.bounds.upper)
		{
			if (auto replacementVar = firstReplacementVar(*bvi, false))
			{
#ifdef DEBUG
//				cerr << "Replacing by " << variables[*replacementVar].name << endl;
#endif
				pivotAndUpdate(*bvi, *basicVar.bounds.upper, *replacementVar);
			}
			else
			{
#ifdef DEBUG
				cerr << "---> infeasible" << endl;
#endif
				result = LPResult::Infeasible;
				reasons = reasonsForUnsat(*bvi, false);
				return make_pair(LPResult::Infeasible, reasons);
			}
		}
#ifdef DEBUG
//		cerr << "Fixed basic " << basicVar.name << endl;
//		cerr << toString() << endl;
#endif
	}

	result = LPResult::Feasible;
#ifdef DEBUG
	cerr << toString() << endl;
	cerr << "---> FEAsible" << endl;
#endif
	return make_pair(LPResult::Feasible, std::set<size_t>());
}

string LPSolver::toString() const
{
	string resultString = "LP Solver state (trail size " + to_string(trailSize) + "):\n";
	auto varName = [&](size_t _i) {
#ifdef DEBUG
		return variables[_i].name;
#else
		return "x" + to_string(_i);
#endif
	};
	for (auto&& [i, v]: variables | ranges::views::enumerate)
	{
		if (v.bounds.lower)
			resultString += v.bounds.lower->toString() + " <= ";
		else
			resultString += "       ";
		resultString += varName(i);
		if (v.bounds.upper)
			resultString += " <= " + v.bounds.upper->toString();
		else
			resultString += "       ";
		resultString += "   := " + v.value.toString() + "\n";
	}
	for (size_t rowIndex = 0; rowIndex < factors.rows(); rowIndex++)
	{
		string basicVarPrefix;
		string rowString;
		for (auto&& entry: const_cast<SparseMatrix&>(factors).iterateRow(rowIndex))
		{
			rational const& f = entry.value;
			solAssert(!!f);
			size_t i = entry.col;
			if (basicVariables.count(i) && basicVariables.at(i) == rowIndex)
			{
				solAssert(f == -1);
				solAssert(basicVarPrefix.empty());
				basicVarPrefix = varName(i) + " = ";
			}
			else if (f != 0)
			{
				string joiner = f < 0 ? " - " : f > 0 && !rowString.empty() ? " + " : " ";
				string factor = f == 1 || f == -1 ? "" : ::toString(abs(f)) + " ";
				string var = varName(i);
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

map<string, rational> LPSolver::model() const
{
	map<string, rational> result;
#ifdef DEBUG
	for (auto&& [outerIndex, innerIndex]: varMapping)
		// TODO assign proper value to "delta"
		result[variables[innerIndex].name] =
			variables[innerIndex].value.m_main +
			variables[innerIndex].value.m_delta / rational(100000);
#endif
	return result;
}


pair<size_t, LPSolver::Bounds> LPSolver::constraintIntoVariableBounds(Constraint const& _constraint)
{
	size_t numVariables = 0;
	size_t latestVariableIndex = size_t(-1);
	// Make all variables available and check if it is a simple bound on a variable.
	for (auto const& [index, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			latestVariableIndex = index;
			numVariables++;
			if (!varMapping.count(index))
				addOuterVariable(index);
		}
	if (numVariables == 1)
	{
		// Add this as direct bound.
		rational factor = _constraint.data[latestVariableIndex];
		RationalWithDelta bound = _constraint.data.front();
		if (_constraint.kind == Constraint::LESS_THAN)
			bound -= RationalWithDelta::delta();
		bound /= factor;
		Bounds bounds;
		if (factor > 0 || _constraint.kind == Constraint::EQUAL)
			bounds.upper = bound;
		if (factor < 0 || _constraint.kind == Constraint::EQUAL)
			bounds.lower = bound;
		return make_pair(varMapping.at(latestVariableIndex), move(bounds));
	}

	// TODO do we need to introduce a slack variable if we have a (potentially new)
	// non-basic variable, or if we have an equality constraint?

	// Introduce the slack variable.
	size_t slackIndex = addNewVariable();
	// Name is only needed for printing
#ifdef DEBUG
	variables[slackIndex].name = "_s" + to_string(m_slackVariableCounter++);
#endif
	basicVariables[slackIndex] = factors.rows();

	// Compress the constraint, i.e. turn outer variable indices into
	// inner variable indices.
	RationalWithDelta valueForSlack;
	size_t row = factors.rows();
	// First, handle the basic variables.
	for (auto const& [outerIndex, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			size_t innerIndex = varMapping.at(outerIndex);
			if (basicVariables.count(innerIndex))
			{
				factors.addMultipleOfRow(
					basicVariables[innerIndex],
					row,
					entry
				);
				factors.remove(factors.entry(row, innerIndex));
			}
		}
	// Now the non-basic.
	for (auto const& [outerIndex, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			size_t innerIndex = varMapping.at(outerIndex);
			if (!basicVariables.count(innerIndex))
			{
				SparseMatrix::Entry& e = factors.entry(row, innerIndex);
				e.value += entry;
				if (!e.value)
					factors.remove(e);
			}
			valueForSlack += variables[innerIndex].value * entry;
		}

	factors.entry(row, slackIndex).value = -1;

	// TODO do we really not need to add this to "potentially out of bounds"?

	basicVariables[slackIndex] = row;
	variables[slackIndex].value = valueForSlack;

	Bounds bounds;
	if (_constraint.kind == Constraint::EQUAL)
		bounds.lower = _constraint.data[0];
	bounds.upper = _constraint.data[0];
	if (_constraint.kind == Constraint::LESS_THAN)
		*bounds.upper -= RationalWithDelta::delta();
	return make_pair(slackIndex, move(bounds));
}

void LPSolver::addBounds(size_t _variable, Bounds _bounds)
{
	Variable& var = variables[_variable];
	if (_bounds.lower && (!var.bounds.lower || *var.bounds.lower < *_bounds.lower))
	{
		var.bounds.lower = move(_bounds.lower);
		if (var.value < var.bounds.lower)
			variablesPotentiallyOutOfBounds.insert(_variable);
	}
	if (_bounds.upper && (!var.bounds.upper || *var.bounds.upper > *_bounds.upper))
	{
		var.bounds.upper = move(_bounds.upper);
		if (var.value > var.bounds.upper)
			variablesPotentiallyOutOfBounds.insert(_variable);
	}
}

set<size_t> LPSolver::collectReasonsForVariable(size_t _variable)
{
	set<size_t> reasons;
	if (variables[_variable].lowerReason)
		reasons.insert(*variables[_variable].lowerReason);
	if (variables[_variable].upperReason)
		reasons.insert(*variables[_variable].upperReason);
	return reasons;
}

void LPSolver::addOuterVariable(size_t _outerIndex)
{
	size_t index = addNewVariable();
	varMapping.emplace(_outerIndex, index);
}

size_t LPSolver::maybeAddOuterVariable(size_t _outerIndex)
{
	if (varMapping.count(_outerIndex))
		return varMapping.at(_outerIndex);
	size_t index = addNewVariable();
	varMapping.emplace(_outerIndex, index);
	return index;
}

size_t LPSolver::addNewVariable()
{
	size_t index = variables.size();
	variables.emplace_back();
	return index;
}


bool LPSolver::correctNonbasic()
{
	set<size_t> toCorrect;
	swap(toCorrect, variablesPotentiallyOutOfBounds);
	for (size_t i: toCorrect)
	{
		Variable& var = variables.at(i);
		if (var.bounds.lower && var.bounds.upper && *var.bounds.lower > *var.bounds.upper)
		{
			reasons = collectReasonsForVariable(i);
			return false;
		}
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

void LPSolver::update(size_t _varIndex, RationalWithDelta const& _value)
{
	RationalWithDelta delta = _value - variables[_varIndex].value;
	variables[_varIndex].value = _value;

	// TODO can we store that?
	map<size_t, size_t> basicVarForRow = invertMap(basicVariables);
	for (auto&& entry: factors.iterateColumn(_varIndex))
		if (entry.value && basicVarForRow.count(entry.row))
		{
			size_t j = basicVarForRow[entry.row];
			variables[j].value += delta * entry.value;
			//variablesPotentiallyOutOfBounds.insert(j);
		}
}

optional<size_t> LPSolver::firstConflictingBasicVariable() const
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

optional<size_t> LPSolver::firstReplacementVar(
	size_t _basicVarToReplace,
	bool _increasing
) const
{
	for (auto&& entry: const_cast<SparseMatrix&>(factors).iterateRow(basicVariables.at(_basicVarToReplace)))
	{
		size_t i = entry.col;
		rational const& factor = entry.value;
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

set<size_t> LPSolver::reasonsForUnsat(
	size_t _basicVarToReplace,
	bool _increasing
) const
{
	set<size_t> r;
	if (_increasing && variables[_basicVarToReplace].lowerReason)
		r.insert(*variables[_basicVarToReplace].lowerReason);
	else if (!_increasing && variables[_basicVarToReplace].upperReason)
		r.insert(*variables[_basicVarToReplace].upperReason);

	for (auto&& entry: const_cast<SparseMatrix&>(factors).iterateRow(basicVariables.at(_basicVarToReplace)))
	{
		size_t i = entry.col;
		rational const& factor = entry.value;
		if (i == _basicVarToReplace || !factor)
			continue;
		bool positive = factor > 0;
		if (!_increasing)
			positive = !positive;
		Variable const& candidate = variables.at(i);
		if (positive && candidate.upperReason)
			r.insert(*candidate.upperReason);
		if (!positive && candidate.lowerReason)
			r.insert(*candidate.lowerReason);
	}
	return r;
}

void LPSolver::pivot(size_t _old, size_t _new)
{
	// Transform pivotRow such that the coefficient for _new is -1
	// Then use that to set all other coefficients for _new to zero.
	size_t pivotRow = basicVariables[_old];

	rational pivot = factors.entry(pivotRow, _new).value;
	solAssert(pivot != 0, "");
	if (pivot != -1)
		factors.multiplyRowByFactor(pivotRow, rational{-1} / pivot);

	for (auto it = factors.iterateColumn(_new).begin(); it != factors.iterateColumn(_new).end(); )
	{
		SparseMatrix::Entry& entry = *it;
		// Increment becasue "addMultipleOfRow" might invalidate the iterator
		++it;
		if (entry.row != pivotRow)
			factors.addMultipleOfRow(pivotRow, entry.row, entry.value);
	}


	basicVariables.erase(_old);
	basicVariables[_new] = pivotRow;
}

void LPSolver::pivotAndUpdate(
	size_t _oldBasicVar,
	RationalWithDelta const& _newValue,
	size_t _newBasicVar
)
{
	RationalWithDelta theta = (_newValue - variables[_oldBasicVar].value) / factors.entry(basicVariables[_oldBasicVar], _newBasicVar).value;

	variables[_oldBasicVar].value = _newValue;
	variables[_newBasicVar].value += theta;

	// TODO can we store that?
	map<size_t, size_t> basicVarForRow = invertMap(basicVariables);
	for (auto&& entry: factors.iterateColumn(_newBasicVar))
		if (basicVarForRow.count(entry.row))
		{
			size_t i = basicVarForRow[entry.row];
			if (i != _oldBasicVar)
				variables[i].value += theta * entry.value;
		}

	pivot(_oldBasicVar, _newBasicVar);
}
