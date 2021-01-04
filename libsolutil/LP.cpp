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
#include <liblangutil/Exceptions.h>

#include <range/v3/view/enumerate.hpp>
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

/**
 * Simplex tableau.
 */
struct Tableau
{
	/// The factors of the objective function (first row of the tableau)
	LinearExpression objective;
	/// The tableau matrix (equational forrm).
	std::vector<LinearExpression> data;
};


/// Adds slack variables to remove non-equality costraints from a set of constraints.
/// The second return variable is true if new variables have been added.
pair<vector<Constraint>, bool> toEquationalForm(vector<Constraint> _constraints)
{
	size_t varsNeeded = static_cast<size_t>(ranges::count_if(_constraints, [](Constraint const& _c) { return !_c.equality; }));
	if (varsNeeded > 0)
	{
		size_t columns = _constraints.at(0).data.size();
		size_t currentVariable = 0;
		for (Constraint& constraint: _constraints)
		{
			solAssert(constraint.data.size() == columns, "");
			constraint.data.factors += vector<rational>(varsNeeded, rational{});
			if (!constraint.equality)
			{
				constraint.equality = true;
				constraint.data[columns + currentVariable] = bigint(1);
				currentVariable++;
			}
		}
	}

	return make_pair(move(_constraints), varsNeeded > 0);
}

optional<size_t> findPivotColumn(Tableau const& _tableau)
{
	auto&& [maxColumn, maxValue] = ranges::max(
		_tableau.objective | ranges::views::enumerate | ranges::views::tail,
		{},
		[](std::pair<size_t, rational> const& _x) { return _x.second; }
	);

	if (maxValue <= rational{0})
		return nullopt; // found optimum
	else
		return maxColumn;
}

optional<size_t> findPivotRow(Tableau const& _tableau, size_t _pivotColumn)
{
	auto positiveColumnEntries =
		ranges::views::iota(size_t(0), _tableau.data.size()) |
		ranges::views::transform([&](size_t i) {
			return make_pair(i, _tableau.data[i][_pivotColumn]);
		}) |
		ranges::views::filter([](pair<size_t, rational> const& _entry) {
			return _entry.second > 0;
		});
	if (positiveColumnEntries.empty())
		return nullopt; // unbounded

	return ranges::min(
		positiveColumnEntries,
		{},
		[&](std::pair<size_t, rational> const& _entry) {
			return _tableau.data[_entry.first][0] / _entry.second;
		}
	).first;
}

/// Performs equivalence transform on @a _tableau, so that
/// the column @a _pivotColumn is all zeros except for @a _pivotRow,
/// where it is 1.
void performPivot(Tableau& _tableau, size_t _pivotRow, size_t _pivotColumn)
{
	rational pivot = _tableau.data[_pivotRow][_pivotColumn];
	solAssert(pivot != 0, "");
	if (pivot != 1)
		_tableau.data[_pivotRow] /= pivot;
	solAssert(_tableau.data[_pivotRow][_pivotColumn] == rational(1), "");

	LinearExpression const& _pivotRowData = _tableau.data[_pivotRow];
	auto subtractPivotRow = [&](LinearExpression& _row) {
		if (_row[_pivotColumn] == rational{1})
			_row -= _pivotRowData;
		else if (_row[_pivotColumn] != rational{})
			_row -= _row[_pivotColumn] * _pivotRowData;
	};

	subtractPivotRow(_tableau.objective);
	for (size_t i = 0; i < _tableau.data.size(); ++i)
		if (i != _pivotRow)
			subtractPivotRow(_tableau.data[i]);
}

void selectLastVectorsAsBasis(Tableau& _tableau)
{
	// We might skip the operation for a column if it is already the correct
	// unit vector and its cost coefficient is zero.
	size_t columns = _tableau.objective.size();
	size_t rows = _tableau.data.size();
	for (size_t i = 0; i < rows; ++i)
		performPivot(_tableau, i, columns - rows + i);
}

/// If column @a _column inside tableau is a basis vector
/// (i.e. one entry is 1, the others are 0), returns the index
/// of the 1, otherwise nullopt.
optional<size_t> basisVariable(Tableau const& _tableau, size_t _column)
{
	optional<size_t> row;
	for (size_t i = 0; i < _tableau.data.size(); ++i)
		if (_tableau.data[i][_column] == bigint(1))
		{
			if (row)
				return std::nullopt;
			else
				row = i;
		}
		else if (_tableau.data[i][_column] != 0)
			return std::nullopt;
	return row;
}

/// @returns a solution vector, assuming one exists.
/// The solution vector minimizes the objective function if the tableau
/// is the result of the simplex algorithm.
vector<rational> solutionVector(Tableau const& _tableau)
{
	vector<rational> result;
	vector<bool> rowsSeen(_tableau.data.size(), false);
	for (size_t j = 1; j < _tableau.objective.size(); j++)
	{
		optional<size_t> row = basisVariable(_tableau, j);
		if (row && rowsSeen[*row])
			row = nullopt;
		result.emplace_back(row ? _tableau.data[*row][0] : rational{});
		if (row)
			rowsSeen[*row] = true;
	}
	return result;
}


/// Solve the LP A x = b s.t. min c^T x
/// Here, c is _tableau.objective and the first column of _tableau.data
/// encodes b and the other columns encode A
/// Assumes the tableau has a trivial basic feasible solution.
pair<LPResult, Tableau> simplexEq(Tableau _tableau)
{
	size_t const iterations = min<size_t>(60, 50 + _tableau.objective.size() * 2);
	for (size_t step = 0; step <= iterations; ++step)
	{
		optional<size_t> pivotColumn = findPivotColumn(_tableau);
		if (!pivotColumn)
			return make_pair(LPResult::Feasible, move(_tableau));
		optional<size_t> pivotRow = findPivotRow(_tableau, *pivotColumn);
		if (!pivotRow)
			return make_pair(LPResult::Unbounded, move(_tableau));
		performPivot(_tableau, *pivotRow, *pivotColumn);
	}
	return make_pair(LPResult::Unknown, Tableau{});
}

/// We add slack variables to find a basic feasible solution.
/// In particular, there is a slack variable for each row
/// which is weighted negatively. Setting the new slack
/// variables to one and all other variables to zero yields
/// a basic feasible solution.
/// If the optimal solution has all slack variables set to zero,
/// this is a basic feasible solution. Otherwise, the original
/// problem is infeasible.
/// This function returns the modified tableau with the original
/// objective function and the slack variables removed.
pair<LPResult, Tableau> simplexPhaseI(Tableau _tableau)
{
	LinearExpression originalObjective = _tableau.objective;

	size_t rows = _tableau.data.size();
	size_t columns = _tableau.objective.size();
	for (size_t i = 0; i < rows; ++i)
	{
		if (_tableau.data[i][0] < 0)
			_tableau.data[i] *= -1;
		_tableau.data[i].factors += vector<bigint>(rows, bigint{});
		_tableau.data[i][columns + i] = 1;
	}
	_tableau.objective.factors =
		vector<rational>(columns, rational{}) +
		vector<rational>(rows, rational{-1});

	// This sets the objective factors of the slack variables
	// to zero (and thus selects a basic feasible solution).
	selectLastVectorsAsBasis(_tableau);

	LPResult result;
	tie(result, _tableau) = simplexEq(move(_tableau));
	solAssert(result == LPResult::Feasible || result == LPResult::Unbounded, "");

	vector<rational> optimum = solutionVector(_tableau);

	for (size_t i = columns - 1; i < optimum.size(); ++i)
		if (optimum[i] != 0)
			return make_pair(LPResult::Infeasible, Tableau{});

	_tableau.objective = originalObjective;
	for (auto& row: _tableau.data)
		row.resize(columns);

	return make_pair(LPResult::Feasible, move(_tableau));
}

/// Returns true if the all-zero solution is not a solution for the tableau.
bool needsPhaseI(Tableau const& _tableau)
{
	for (auto const& row: _tableau.data)
		if (row[0] < 0)
			return true;
	return false;
}

/// Solve the LP Ax <= b s.t. min c^Tx
pair<LPResult, vector<rational>> simplex(vector<Constraint> _constraints, LinearExpression _objectives)
{
	Tableau tableau;
	tableau.objective = move(_objectives);
	bool hasEquations = false;
	// TODO change toEquationalForm to directly return the tableau
	tie(_constraints, hasEquations) = toEquationalForm(_constraints);
	for (Constraint& c: _constraints)
		tableau.data.emplace_back(move(c.data));
	tableau.objective.resize(tableau.data.at(0).size());

	if (hasEquations || needsPhaseI(tableau))
	{
		LPResult result;
		tie(result, tableau) = simplexPhaseI(move(tableau));
		if (result == LPResult::Infeasible || result == LPResult::Unknown)
			return make_pair(result, vector<rational>{});
		solAssert(result == LPResult::Feasible, "");
	}
	// We know that the system is satisfiable and we know a solution,
	// but it is not optimal.
	LPResult result;
	tie(result, tableau) = simplexEq(move(tableau));
	solAssert(result == LPResult::Feasible || result == LPResult::Unbounded, "");
	return make_pair(result, solutionVector(tableau));
}

/// Turns all bounds into constraints.
/// @returns false if the bounds make the state infeasible.
bool boundsToConstraints(SolvingState& _state)
{
	size_t columns = _state.variableNames.size();

	// Turn bounds into constraints.
	for (auto const& [index, bounds]: _state.bounds | ranges::views::enumerate | ranges::views::tail)
	{
		if (bounds[0] && bounds[1])
		{
			if (*bounds[0] > *bounds[1])
				return false;
			if (*bounds[0] == *bounds[1])
			{
				vector<rational> c(columns);
				c[0] = *bounds[0];
				c[index] = bigint(1);
				_state.constraints.emplace_back(Constraint{move(c), true});
				continue;
			}
		}
		if (bounds[0] && *bounds[0] > 0)
		{
			vector<rational> c(columns);
			c[0] = -*bounds[0];
			c[index] = bigint(-1);
			_state.constraints.emplace_back(Constraint{move(c), false});
		}
		if (bounds[1])
		{
			vector<rational> c(columns);
			c[0] = *bounds[1];
			c[index] = bigint(1);
			_state.constraints.emplace_back(Constraint{move(c), false});
		}
	}
	_state.bounds.clear();
	return true;
}

template <class T>
void eraseIndices(T& _data, vector<bool> const& _indices)
{
	T result;
	for (size_t i = 0; i < _data.size(); i++)
		if (!_indices[i])
			result.emplace_back(move(_data[i]));
	_data = move(result);
}


void removeColumns(SolvingState& _state, vector<bool> const& _columnsToRemove)
{
	eraseIndices(_state.bounds, _columnsToRemove);
	for (Constraint& constraint: _state.constraints)
		eraseIndices(constraint.data, _columnsToRemove);
	eraseIndices(_state.variableNames, _columnsToRemove);
}

bool extractDirectConstraints(SolvingState& _state, bool& _changed)
{
	// Turn constraints of the form ax <= b into an upper bound on x.
	vector<bool> constraintsToRemove(_state.constraints.size(), false);
	bool needsRemoval = false;
	for (auto const& [index, constraint]: _state.constraints | ranges::views::enumerate)
	{
		auto nonzero = constraint.data | ranges::views::enumerate | ranges::views::tail | ranges::views::filter(
			[](std::pair<size_t, rational> const& _x) { return !!_x.second; }
		);
		// TODO we can exit early on in the loop above since we only care about zero, one or more than one nonzero entries.
		// TODO could also use iterators and exit if we can advance it twice.
		auto numNonzero = ranges::distance(nonzero);
		if (numNonzero > 1)
			continue;
		constraintsToRemove[index] = true;
		needsRemoval = true;
		if (numNonzero == 0)
		{
			// 0 <= b or 0 = b
			if (
				constraint.data.factors.front() < 0 ||
				(constraint.equality && constraint.data.factors.front() != 0)
			)
				return false; // Infeasible.
		}
		else
		{
			auto&& [varIndex, factor] = nonzero.front();
			// a * x <= b
			rational bound = constraint.data[0] / factor;
			if (
				(factor >= 0 || constraint.equality) &&
				(!_state.bounds[varIndex][1] || bound < _state.bounds[varIndex][1])
			)
				_state.bounds[varIndex][1] = bound;
			if (
				(factor <= 0 || constraint.equality) &&
				(!_state.bounds[varIndex][0] || bound > _state.bounds[varIndex][0])
			)
				// Lower bound must be at least zero.
				_state.bounds[varIndex][0] = max(rational{}, bound);
		}
	}
	if (needsRemoval)
	{
		_changed = true;
		eraseIndices(_state.constraints, constraintsToRemove);
	}
	return true;
}

bool removeFixedVariables(SolvingState& _state, map<string, rational>& _model, bool& _changed)
{
	// Remove variables that have equal lower and upper bound.
	for (auto const& [index, bounds]: _state.bounds | ranges::views::enumerate)
	{
		if (!bounds[1] || (!bounds[0] && bounds[1]->numerator() > 0))
			continue;
		// Lower bound must be at least zero.
		rational lower = max(rational{}, bounds[0] ? *bounds[0] : rational{});
		rational upper = *bounds[1];
		if (upper < lower)
			return false; // Infeasible.
		if (upper != lower)
			continue;
		_model[_state.variableNames.at(index)] = lower;
		_state.bounds[index] = {};
		_changed = true;

		// substitute variable
		for (Constraint& constraint: _state.constraints)
			if (constraint.data.factors.at(index) != 0)
			{
				constraint.data[0] -= constraint.data[index] * lower;
				constraint.data[index] = 0;
			}
	}

	return true;
}

bool removeEmptyColumns(SolvingState& _state, map<string, rational>& _model, bool& _changed)
{
	vector<bool> variablesSeen(_state.bounds.size(), false);
	for (auto const& constraint: _state.constraints)
	{
		for (auto&& [index, factor]: constraint.data | ranges::views::enumerate | ranges::views::tail)
			if (factor)
				variablesSeen[index] = true;
	}

	// TODO we could assert that any variable we remove does not have conflicting bounds.
	// (We also remove the bounds).

	vector<bool> variablesToRemove(variablesSeen.size(), false);
	bool needsRemoval = false;
	for (auto&& [i, seen]: variablesSeen | ranges::views::enumerate | ranges::views::tail)
		if (!seen)
		{
			variablesToRemove[i] = true;
			needsRemoval = true;
			// TODO actually it is unbounded if _state.bounds.at(i)[1] is nullopt.
			if (_state.bounds.at(i)[0] || _state.bounds.at(i)[1])
				_model[_state.variableNames.at(i)] =
					_state.bounds.at(i)[1] ?
					*_state.bounds.at(i)[1] :
					*_state.bounds.at(i)[0];
		}
	if (needsRemoval)
	{
		_changed = true;
		removeColumns(_state, variablesToRemove);
	}
	return true;
}

auto nonZeroEntriesInColumn(SolvingState const& _state, size_t _column)
{
	return
		_state.constraints |
		ranges::views::enumerate |
		ranges::views::filter([=](auto const& _entry) { return _entry.second.data[_column] != 0; }) |
		ranges::views::transform([](auto const& _entry) { return _entry.first; });
}

pair<vector<bool>, vector<bool>> connectedComponent(SolvingState const& _state, size_t _column)
{
	solAssert(_state.variableNames.size() >= 2, "");

	vector<bool> includedColumns(_state.variableNames.size(), false);
	vector<bool> includedRows(_state.constraints.size(), false);
	stack<size_t> columnsToProcess;
	columnsToProcess.push(_column);
	while (!columnsToProcess.empty())
	{
		size_t column = columnsToProcess.top();
		columnsToProcess.pop();
		if (includedColumns[column])
			continue;
		includedColumns[column] = true;

		for (size_t row: nonZeroEntriesInColumn(_state, column))
		{
			if (includedRows[row])
				continue;
			includedRows[row] = true;
			for (auto const& [index, entry]: _state.constraints[row].data | ranges::views::enumerate | ranges::views::tail)
				if (entry && !includedColumns[index])
					columnsToProcess.push(index);
		}
	}
	return make_pair(move(includedColumns), move(includedRows));
}

struct ProblemSplitter
{
	ProblemSplitter(SolvingState const& _state):
		state(_state),
		column(1),
		seenColumns(vector<bool>(state.variableNames.size(), false))
	{}

	operator bool() const
	{
		return column < state.variableNames.size();
	}

	SolvingState next()
	{
		vector<bool> includedColumns;
		vector<bool> includedRows;
		tie(includedColumns, includedRows) = connectedComponent(state, column);

		// Update state.
		seenColumns |= includedColumns;
		++column;
		while (column < state.variableNames.size() && seenColumns[column])
			++column;

		// Happens in case of not removed empty column.
		// Currently not happening because those are removed during the simplification stage.
		// TODO If this is the case, we should actually also check the bounds.
		if (includedRows.empty())
			return next();

		SolvingState splitOff;

		splitOff.variableNames.emplace_back();
		splitOff.bounds.emplace_back();

		for (auto&& [i, included]: includedColumns | ranges::views::enumerate | ranges::views::tail)
		{
			if (!included)
				continue;
			splitOff.variableNames.emplace_back(move(state.variableNames[i]));
			splitOff.bounds.emplace_back(move(state.bounds[i]));
		}
		for (auto&& [i, included]: includedRows | ranges::views::enumerate)
		{
			if (!included)
				continue;
			Constraint splitRow{{}, state.constraints[i].equality};
			for (size_t j = 0; j < state.constraints[i].data.size(); j++)
				if (j == 0 || includedColumns[j])
					splitRow.data.factors.push_back(state.constraints[i].data[j]);
			splitOff.constraints.push_back(move(splitRow));
		}

		return splitOff;
	}

	SolvingState const& state;
	size_t column = 1;
	vector<bool> seenColumns;
};


/// Simplifies the solving state according to some rules (remove rows without variables, etc).
/// @returns false if the state is determined to be infeasible during this process.
bool simplifySolvingState(SolvingState& _state, map<string, rational>& _model)
{
	// - Constraints with exactly one nonzero coefficient represent "a x <= b"
	//   and thus are turned into bounds.
	// - Constraints with zero nonzero coefficients are constant relations.
	//   If such a relation is false, answer "infeasible", otherwise remove the constraint.
	// - Empty columns can be removed.
	// - Variables with matching bounds can be removed from the problem by substitution.

	bool changed = true;
	while (changed)
	{
		changed = false;

		if (!removeFixedVariables(_state, _model, changed))
			return false;

		if (!extractDirectConstraints(_state, changed))
			return false;

		if (!removeFixedVariables(_state, _model, changed))
			return false;

		if (!removeEmptyColumns(_state, _model, changed))
			return false;
	}

	// TODO return the values selected for named variables in order to
	// be used when returning the model.
	return true;
}

void normalizeRowLengths(SolvingState& _state)
{
	size_t vars = max(_state.variableNames.size(), _state.bounds.size());
	for (Constraint const& c: _state.constraints)
		vars = max(vars, c.data.size());
	_state.variableNames.resize(vars);
	_state.bounds.resize(vars);
	for (Constraint& c: _state.constraints)
		c.data.resize(vars);
}

}


bool Constraint::operator<(Constraint const& _other) const
{
	if (equality != _other.equality)
		return equality < _other.equality;

	for (size_t i = 0; i < max(data.size(), _other.data.size()); ++i)
	{
		rational const& a = data.get(i);
		rational const& b = _other.data.get(i);
		if (a != b)
			return a < b;
	}
	return false;
}

bool Constraint::operator==(Constraint const& _other) const
{
	if (equality != _other.equality)
		return false;

	for (size_t i = 0; i < max(data.size(), _other.data.size()); ++i)
		if (data.get(i) != _other.data.get(i))
			return false;
	return true;
}

bool SolvingState::operator<(SolvingState const& _other) const
{
	if (variableNames == _other.variableNames)
	{
		if (bounds == _other.bounds)
			return constraints < _other.constraints;
		else
			return bounds < _other.bounds;
	}
	else
		return variableNames < _other.variableNames;
}

bool SolvingState::operator==(SolvingState const& _other) const
{
	return
		variableNames == _other.variableNames &&
		bounds == _other.bounds &&
		constraints == _other.constraints;
}

string SolvingState::toString() const
{
	string result;

	for (Constraint const& constraint: constraints)
	{
		vector<string> line;
		for (auto&& [index, multiplier]: constraint.data | ranges::views::enumerate)
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
			::toString(constraint.data.factors.front()) +
			"\n";
	}
	result += "Bounds:\n";
	for (auto&& [index, bounds]: bounds | ranges::views::enumerate)
	{
		if (!bounds[0] && !bounds[1])
			continue;
		if (bounds[0])
			result += ::toString(*bounds[0]) + " <= ";
		result += variableNames.at(index);
		if (bounds[1])
			result += " <= " + ::toString(*bounds[1]);
		result += "\n";
	}
	return result;
}


pair<LPResult, map<string, rational>> LPSolver::check(SolvingState _state)
{
	normalizeRowLengths(_state);

	map<string, rational> model;

	if (!simplifySolvingState(_state, model))
		return {LPResult::Infeasible, {}};

	bool canOnlyBeUnknown = false;
	ProblemSplitter splitter(_state);
	while (splitter)
	{
		SolvingState split = splitter.next();
		solAssert(!split.constraints.empty(), "");
		solAssert(split.variableNames.size() >= 2, "");

		LPResult lpResult;
		vector<rational> solution;
		auto it = m_cache.find(split);
		if (it != m_cache.end())
			tie(lpResult, solution) = it->second;
		else
		{
			SolvingState orig = split;
			if (!boundsToConstraints(split))
				lpResult = LPResult::Infeasible;
			else
			{
				LinearExpression objectives;
				objectives.factors =
					vector<rational>(1, rational(bigint(0))) +
					vector<rational>(split.constraints.front().data.size() - 1, rational(bigint(1)));
				tie(lpResult, solution) = simplex(split.constraints, move(objectives));
			}
			m_cache.emplace(move(orig), make_pair(lpResult, solution));
		}

		switch (lpResult)
		{
		case LPResult::Feasible:
		case LPResult::Unbounded:
			break;
		case LPResult::Infeasible:
			return {LPResult::Infeasible, {}};
		case LPResult::Unknown:
			// We do not stop here, because another independent query can still be infeasible.
			canOnlyBeUnknown = true;
			break;
		}
		for (auto&& [index, value]: solution | ranges::views::enumerate)
			if (index + 1 < split.variableNames.size())
				model[split.variableNames.at(index + 1)] = value;
	}

	if (canOnlyBeUnknown)
		return {LPResult::Unknown, {}};

	return {LPResult::Feasible, move(model)};
}

