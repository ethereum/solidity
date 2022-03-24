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

/**
 * Simplex tableau.
 */
struct Tableau
{
	/// The factors of the objective function (first row of the tableau)
	LinearExpression objective;
	/// The tableau matrix (equational form).
	std::vector<LinearExpression> data;
};

string toString(rational const& _x)
{
	if (_x.denominator() == 1)
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

/*
string toString(LinearExpression const& _expr)
{
	vector<string> items;
	for (auto&& multiplier: _expr)
		 if (multiplier != 0)
			items.emplace_back(::toString(multiplier));
		else
			items.emplace_back("_");
	for (string& item: items)
		while (item.size() < 3)
			item = " " + item;
	return joinHumanReadable(items, " ");
}

string toString(Tableau const& _tableau)
{
	string s = toString(_tableau.objective) + "\n";
	for (auto&& d: _tableau.data)
		s += toString(d) + "\n";
	return s;
}
*/

/// Adds slack variables to remove non-equality costraints from a set of constraints
/// and returns the data part of the tableau / constraints.
/// The second return variable is true if the original input had any equality constraints.
pair<vector<LinearExpression>, bool> toEquationalForm(vector<Constraint> _constraints)
{
	size_t varsNeeded = static_cast<size_t>(ranges::count_if(_constraints, [](Constraint const& _c) { return !_c.equality; }));
	if (varsNeeded > 0)
	{
		size_t columns = _constraints.at(0).data.size();
		size_t varsAdded = 0;
		for (Constraint& constraint: _constraints)
		{
			solAssert(constraint.data.size() == columns, "");
			constraint.data.resize(columns + varsNeeded);
			if (!constraint.equality)
			{
				constraint.equality = true;
				constraint.data[columns + varsAdded] = bigint(1);
				varsAdded++;
			}
		}
		solAssert(varsAdded == varsNeeded);
	}

	vector<LinearExpression> data;
	for (Constraint& c: _constraints)
		data.emplace_back(move(c.data));

	return make_pair(move(data), varsNeeded < _constraints.size());
}

/// Finds the simplex pivot column: The column with the largest positive objective factor.
/// If all objective factors are zero or negative, the optimum has been found and nullopt is returned.
optional<size_t> findPivotColumn(Tableau const& _tableau)
{
	auto&& [maxColumn, maxValue] = ranges::max(
		_tableau.objective.enumerateTail(),
		{},
		[](std::pair<size_t, rational> const& _x) { return _x.second; }
	);

	if (maxValue <= rational{0})
		return nullopt; // found optimum
	else
		return maxColumn;
}

/// Finds the simplex pivot row, given the column:
/// If there is no positive factor in the column, the problem is unbounded, nullopt is returned.
/// Otherwise, returns the row i such that c[i] / x[i] is minimal and x[i] is positive, where
/// c[i] is the constant factor (not the objective factor!) in row i.
optional<size_t> findPivotRow(Tableau const& _tableau, size_t _pivotColumn)
{
	auto positiveColumnEntries =
		ranges::views::iota(size_t(0), _tableau.data.size()) |
		ranges::views::transform([&](size_t i) {
			return make_pair(i, _tableau.data[i][_pivotColumn]);
		}) |
		ranges::views::filter([](pair<size_t, rational> const& _entry) {
			return _entry.second.numerator() > 0;
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
/// the column @a _pivotColumn is all zeros (including the objective row) except for @a _pivotRow,
/// where it is 1.
void performPivot(Tableau& _tableau, size_t _pivotRow, size_t _pivotColumn)
{
	rational pivot = _tableau.data[_pivotRow][_pivotColumn];
	solAssert(pivot != 0, "");
	if (pivot != 1)
		_tableau.data[_pivotRow] /= pivot;
	solAssert(_tableau.data[_pivotRow][_pivotColumn] == rational(1), "");

	LinearExpression const& _pivotRowData = _tableau.data[_pivotRow];
	auto subtractMultipleOfPivotRow = [&](LinearExpression& _row) {
		if (_row[_pivotColumn] == rational{1})
			_row -= _pivotRowData;
		else if (_row[_pivotColumn])
			_row -= _row[_pivotColumn] * _pivotRowData;
	};

	subtractMultipleOfPivotRow(_tableau.objective);
	for (size_t i = 0; i < _tableau.data.size(); ++i)
		if (i != _pivotRow)
			subtractMultipleOfPivotRow(_tableau.data[i]);
}

/// Transforms the tableau such that the last vectors are basis vectors
/// and their objective coefficients are zero.
/// Makes various assumptions and should only be used after adding
/// a certain number of slack variables.
void selectLastVectorsAsBasis(Tableau& _tableau)
{
	// We might skip the operation for a column if it is already the correct
	// unit vector and its objective coefficient is zero.
	size_t columns = _tableau.objective.size();
	size_t rows = _tableau.data.size();
	for (size_t i = 0; i < rows; ++i)
		performPivot(_tableau, i, columns - rows + i);
}

/// If column @a _column inside tableau is a basis vector
/// (i.e. one entry is 1, the others are 0), returns the index
/// of the 1, otherwise nullopt.
optional<size_t> basisIndex(Tableau const& _tableau, size_t _column)
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
		optional<size_t> row = basisIndex(_tableau, j);
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
/// Tries for a number of iterations and then gives up.
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
		_tableau.data[i].resize(columns + rows);
		_tableau.data[i][columns + i] = 1;
	}
	_tableau.objective = {};
	_tableau.objective.resize(columns);
	_tableau.objective.resize(columns + rows, rational{-1});

	// This sets the objective factors of the slack variables
	// to zero (and thus selects a basic feasible solution).
	selectLastVectorsAsBasis(_tableau);

	LPResult result;
	tie(result, _tableau) = simplexEq(move(_tableau));
	if (result == LPResult::Unknown)
		return make_pair(LPResult::Unknown, Tableau{});
	solAssert(result != LPResult::Infeasible, "");

	vector<rational> optimum = solutionVector(_tableau);

	// If the solution needs a nonzero factor for a slack variable,
	// the original system is infeasible.
	for (size_t i = columns - 1; i < optimum.size(); ++i)
		if (optimum[i] != 0)
			return make_pair(LPResult::Infeasible, Tableau{});

	// Restore original objective and remove slack variables.
	_tableau.objective = move(originalObjective);
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
	tie(tableau.data, hasEquations) = toEquationalForm(_constraints);
	tableau.objective.resize(tableau.data.at(0).size());
	//cout << "Running simplex on " << tableau.objective.size() << " x " << tableau.data.size() << endl;

	if (hasEquations || needsPhaseI(tableau))
	{
		LPResult result;
		tie(result, tableau) = simplexPhaseI(move(tableau));
		if (result == LPResult::Infeasible || result == LPResult::Unknown)
			return make_pair(result, vector<rational>{});
		solAssert(result == LPResult::Feasible, "");
	}
	// We know that the system is satisfiable and we know a solution,
	// but it might not be optimal.
	LPResult result;
	tie(result, tableau) = simplexEq(move(tableau));
	solAssert(result != LPResult::Infeasible, "");
	return make_pair(result, solutionVector(tableau));
}

/// Turns all bounds into constraints.
/// @returns false if the bounds make the state infeasible.
optional<ReasonSet> boundsToConstraints(SolvingState& _state)
{
	size_t columns = _state.variableNames.size();

	// Bound zero should not exist because the variable zero does not exist.
	for (auto const& [varIndex, bounds]: _state.bounds | ranges::views::enumerate | ranges::views::tail)
	{
		if (bounds.lower && bounds.upper)
		{
			if (*bounds.lower > *bounds.upper)
				return bounds.lowerReasons + bounds.upperReasons;
			if (*bounds.lower == *bounds.upper)
			{
				LinearExpression c;
				c.resize(columns);
				c[0] = *bounds.lower;
				c[varIndex] = bigint(1);
				_state.constraints.emplace_back(Constraint{move(c), true, bounds.lowerReasons + bounds.upperReasons});
				continue;
			}
		}
		if (bounds.lower && *bounds.lower > 0)
		{
			LinearExpression c;
			c.resize(columns);
			c[0] = -*bounds.lower;
			c[varIndex] = bigint(-1);
			_state.constraints.emplace_back(Constraint{move(c), false, move(bounds.lowerReasons)});
		}
		if (bounds.upper)
		{
			LinearExpression c;
			c.resize(columns);
			c[0] = *bounds.upper;
			c[varIndex] = bigint(1);
			_state.constraints.emplace_back(Constraint{move(c), false, move(bounds.upperReasons)});
		}
	}
	_state.bounds.clear();
	return nullopt;
}

/// Removes incides set to true from a vector-like data structure.
template <class T>
void eraseIndices(T& _data, vector<bool> const& _indicesToRemove)
{
	T result;
	for (size_t i = 0; i < _data.size(); i++)
		if (!_indicesToRemove[i])
			result.push_back(move(_data[i]));
	_data = move(result);
}


void removeColumns(SolvingState& _state, vector<bool> const& _columnsToRemove)
{
	eraseIndices(_state.bounds, _columnsToRemove);
	for (Constraint& constraint: _state.constraints)
		eraseIndices(constraint.data, _columnsToRemove);
	eraseIndices(_state.variableNames, _columnsToRemove);
}

auto nonZeroEntriesInColumn(SolvingState const& _state, size_t _column)
{
	return
		_state.constraints |
		ranges::views::enumerate |
		ranges::views::filter([=](auto const& _entry) { return _entry.second.data[_column]; }) |
		ranges::views::transform([](auto const& _entry) { return _entry.first; });
}

/// @returns vectors of column- and row-indices that are connected to the given column,
/// in the sense of variables occurring in a constraint and constraints for variables.
pair<vector<bool>, vector<bool>> connectedComponent(SolvingState const& _state, size_t _column)
{
	solAssert(_state.variableNames.size() >= 2, "");

	vector<bool> includedColumns(_state.variableNames.size(), false);
	vector<bool> seenColumns(_state.variableNames.size(), false);
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
			for (auto const& [index, entry]: _state.constraints[row].data.enumerateTail())
				if (entry && !seenColumns[index])
				{
					seenColumns[index] = true;
					columnsToProcess.push(index);
				}
		}
	}
	return make_pair(move(includedColumns), move(includedRows));
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
	for (Constraint const& c: constraints)
		ret += c.reasons;
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
			reasonToString(constraint.reasons, reasonLength) +
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

pair<LPResult, variant<map<size_t, rational>, ReasonSet>> SolvingStateSimplifier::simplify()
{
	do
	{
		m_changed = false;
		if (auto conflict = removeFixedVariables())
			return {LPResult::Infeasible, move(*conflict)};
		if (auto conflict = extractDirectConstraints())
			return {LPResult::Infeasible, move(*conflict)};
		// TODO we cannot do this anymore because it would
		// mess up the variable numbering
		// removeEmptyColumns();
	}
	while (m_changed);

	return {LPResult::Unknown, move(m_fixedVariables)};
}

optional<ReasonSet> SolvingStateSimplifier::removeFixedVariables()
{
	for (auto const& [index, bounds]: m_state.bounds | ranges::views::enumerate)
	{
		if (!bounds.upper || (!bounds.lower && bounds.upper->numerator() > 0))
			continue;
		// Lower bound must be at least zero.
		rational lower = max(rational{}, bounds.lower ? *bounds.lower : rational{});
		rational upper = *bounds.upper;
		if (upper < lower)
			// Infeasible.
			return bounds.lowerReasons + bounds.upperReasons;
		if (upper != lower)
			continue;
		set<size_t> reasons = bounds.lowerReasons + bounds.upperReasons;
		m_fixedVariables[index] = lower;
		//cout << "Fixed " << m_state.variableNames.at(index) << " to " << ::toString(lower) << endl;
		m_state.bounds[index] = {};
		m_changed = true;

		// substitute variable
		// -> add the bounds to the literals for the conflict
		// (maybe only if one of these constraints is used)
		for (Constraint& constraint: m_state.constraints)
			if (constraint.data[index])
			{
				constraint.data[0] -= constraint.data[index] * lower;
				constraint.data[index] = 0;
				constraint.reasons += reasons;
			}
	}

	return nullopt;
}

optional<ReasonSet> SolvingStateSimplifier::extractDirectConstraints()
{
	vector<bool> constraintsToRemove(m_state.constraints.size(), false);
	bool needsRemoval = false;
	for (auto const& [index, constraint]: m_state.constraints | ranges::views::enumerate)
	{
		auto nonzeroCoefficients = constraint.data.enumerateTail() | ranges::views::filter(
			[](std::pair<size_t, rational> const& _x) { return !!_x.second; }
		);
		// TODO we can exit early on in the loop above since we only care about zero, one or more than one nonzero entries.
		// TODO could also use iterators and exit if we can advance it twice.
		auto numNonzero = ranges::distance(nonzeroCoefficients);
		if (numNonzero > 1)
			continue;
		constraintsToRemove[index] = true;
		needsRemoval = true;
		if (numNonzero == 0)
		{
			// 0 <= b or 0 = b
			if (
				constraint.data.front().numerator() < 0 ||
				(constraint.equality && constraint.data.front())
			)
				return constraint.reasons;
		}
		else
		{
			auto&& [varIndex, factor] = nonzeroCoefficients.front();
			// a * x <= b
			rational bound = constraint.data[0] / factor;
			if (
				(factor >= 0 || constraint.equality) &&
				(!m_state.bounds[varIndex].upper || bound < m_state.bounds[varIndex].upper)
			)
			{
				m_state.bounds[varIndex].upper = bound;
				m_state.bounds[varIndex].upperReasons = constraint.reasons;
			}
			if (
				(factor <= 0 || constraint.equality) &&
				bound >= 0 &&
				(!m_state.bounds[varIndex].lower || bound > m_state.bounds[varIndex].lower)
			)
			{
				m_state.bounds[varIndex].lower = bound;
				m_state.bounds[varIndex].lowerReasons = constraint.reasons;
			}
		}
	}
	if (needsRemoval)
	{
		m_changed = true;
		eraseIndices(m_state.constraints, constraintsToRemove);
	}
	return nullopt;
}

void SolvingStateSimplifier::removeEmptyColumns()
{
	vector<bool> variablesSeen(m_state.bounds.size(), false);
	for (auto const& constraint: m_state.constraints)
	{
		for (auto&& [index, factor]: constraint.data.enumerateTail())
			if (factor)
				variablesSeen[index] = true;
	}

	vector<bool> variablesToRemove(variablesSeen.size(), false);
	bool needsRemoval = false;
	for (auto&& [i, seen]: variablesSeen | ranges::views::enumerate | ranges::views::tail)
		if (!seen)
		{
			variablesToRemove[i] = true;
			needsRemoval = true;
			SolvingState::Bounds const& bounds = m_state.bounds.at(i);
			// TODO actually it is unbounded if m_state.bounds.at(i).upper is nullopt.
			if (bounds.lower || bounds.upper)
			{
				solAssert(!bounds.upper || bounds.upper >= 0);
				if (bounds.lower && bounds.upper)
					solAssert(*bounds.lower <= *bounds.upper);
				m_fixedVariables[i] =
					bounds.upper ?
					*bounds.upper :
					*bounds.lower;
			}
		}
	if (needsRemoval)
	{
		m_changed = true;
		removeColumns(m_state, variablesToRemove);
	}
}

ProblemSplitter::ProblemSplitter(const SolvingState& _state):
	m_state(_state),
	m_column(1),
	m_seenColumns(std::vector<bool>(m_state.variableNames.size(), false))
{
	while (m_column < m_state.variableNames.size() && nonZeroEntriesInColumn(_state, m_column).empty())
		m_column++;
}

pair<vector<bool>, vector<bool>> ProblemSplitter::next()
{
	vector<bool> includedColumns;
	vector<bool> includedRows;
	tie(includedColumns, includedRows) = connectedComponent(m_state, m_column);

	// Update state.
	m_seenColumns |= includedColumns;
	++m_column;
	while (m_column < m_state.variableNames.size() && (
		m_seenColumns[m_column] ||
		nonZeroEntriesInColumn(m_state, m_column).empty()
	))
		++m_column;

	return {move(includedColumns), move(includedRows)};
/*
	SolvingState splitOff;

	splitOff.variableNames.emplace_back();
	splitOff.bounds.emplace_back();

	for (auto&& [i, included]: includedColumns | ranges::views::enumerate | ranges::views::tail)
		if (included)
		{
			splitOff.variableNames.emplace_back(move(m_state.variableNames[i]));
			splitOff.bounds.emplace_back(move(m_state.bounds[i]));
		}

	for (auto&& [i, included]: includedRows | ranges::views::enumerate)
		if (included)
		{
			// Use const& on purpose because moving from the state can lead
			// to undefined behaviour for connectedComponent
			Constraint const& constraint = m_state.constraints[i];
			Constraint splitRow{{}, constraint.equality, constraint.reasons};
			for (size_t j = 0; j < constraint.data.size(); j++)
				if (j == 0 || includedColumns[j])
					splitRow.data.push_back(constraint.data[j]);
			splitOff.constraints.push_back(move(splitRow));
		}

	return {includedColumns, splitOff};
	*/
}

LPSolver::LPSolver(bool)
{
}

LPResult LPSolver::setState(SolvingState _state)
{
	//cout << "Set state:\n" << _state.toString() << endl;
	m_state = make_shared<SolvingState>(move(_state));
	m_subProblems.clear();
	m_subProblemsPerVariable.resize(m_state->variableNames.size(), static_cast<size_t>(-1));
	m_subProblemsPerConstraint.resize(m_state->constraints.size(), static_cast<size_t>(-1));

	normalizeRowLengths(*m_state);
	auto&& [result, modelOrReasonSet] = SolvingStateSimplifier(*m_state).simplify();
	if (result == LPResult::Infeasible)
		return result;

	// We do not need to store reasons because at this point, we do not have any reasons yet.
	// We can add this and just need to store the reasons together with the variables.
	m_fixedVariables = make_shared<std::map<size_t, rational>>(std::get<std::map<size_t, rational>>(modelOrReasonSet));

	//cout << "Splitting..." << endl;
	ProblemSplitter splitter(*m_state);
	while (splitter)
	{
		auto&& [variables, constraints] = splitter.next();
		SubProblem& problem = *m_subProblems.emplace_back(make_shared<SubProblem>());
		solAssert(problem.dirty);
		for (auto&& [i, included]: variables | ranges::views::enumerate)
			if (included)
			{
				m_subProblemsPerVariable[i] = m_subProblems.size() - 1;
				problem.variables.emplace(i);
			}
		for (auto&& [i, included]: constraints | ranges::views::enumerate)
			if (included)
				m_subProblemsPerConstraint[i] = m_subProblems.size() - 1;
		//cout << "Adding new sub problem with " << m_subProblems.back()->variables.size() << " vars and " << m_subProblems.back()->constraints.size() << " constraints\n" << endl;
		//cout << "-------------------- Split out" << endl;
		//cout << m_subProblems.back()->state.toString() << endl;
	}
	//cout << "Done splitting." << endl;
	return LPResult::Unknown;
}

void LPSolver::addConstraint(Constraint _constraint)
{
	//cout << "Adding constraint " << endl;
	set<size_t> touchedProblems;
	for (auto const& [index, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			if (m_fixedVariables->count(index))
			{
				// This can directly lead to a conflict. We will check it later during the
				// simplify run on the split problems.
				_constraint.data[0] -= _constraint.data[index] * m_fixedVariables->at(index);
				_constraint.data[index] = {};
			}
			else if (m_subProblemsPerVariable[index] != static_cast<size_t>(-1))
				touchedProblems.emplace(m_subProblemsPerVariable[index]);
		}
	if (touchedProblems.empty())
	{
		//cout << "Creating new sub problem." << endl;
		// TODO we could find an empty spot for the pointer.
		m_subProblems.emplace_back(make_shared<SubProblem>());
		solAssert(m_subProblems.back()->dirty);
		touchedProblems.emplace(m_subProblems.size() - 1);
	}
	for (size_t problemToErase: touchedProblems | ranges::views::tail | ranges::views::reverse)
		combineSubProblems(*touchedProblems.begin(), problemToErase);
	addConstraintToSubProblem(*touchedProblems.begin(), _constraint);
//	cout << "subproblems: " << (
//		m_subProblems | ranges::views::transform([&](auto&& p) { return !!p; })).size() <<
//		" (total: " << m_subProblems.size() << endl;
	//cout << "done" << endl;
}

pair<LPResult, variant<Model, ReasonSet>> LPSolver::check()
{
	//cout << "Checking" << endl;
	for (auto&& [index, problem]: m_subProblems | ranges::views::enumerate)
	{
		if (!problem)
			continue;
		if (!problem->dirty)
		{
			solAssert(problem->result != LPResult::Infeasible);
			continue;
		}

		//cout << "Updating sub problem" << endl;
		SolvingState state = stateFromSubProblem(index);
		normalizeRowLengths(state);

		// The simplify run is important because it detects conflicts
		// due to fixed variables.
		auto&& [result, modelOrReasonSet] = SolvingStateSimplifier(state).simplify();
		if (result == LPResult::Infeasible)
		{
			problem->result = LPResult::Infeasible;
			problem->model = {};
			problem->dirty = false;
			// TODO we could use the improved reason set above.
			return {LPResult::Infeasible, reasonSetForSubProblem(*problem)};
		}
		//cout << state.toString() << endl;

		if (auto conflict = boundsToConstraints(state))
		{
			problem->result = LPResult::Infeasible;
			problem->model = {};
			problem->dirty = false;
			return {LPResult::Infeasible, reasonSetForSubProblem(*problem)};
		}
		else if (state.constraints.empty())
		{
			problem->result = LPResult::Feasible;
			problem->model = {};
			problem->dirty = false;
		}
		else
		{
			optional<LPResult> result;
			if (m_cache)
			{
				auto it = m_cache->find(state);
				if (it != m_cache->end())
				{
					//cout << "Cache hit" << endl;
					result = it->second;
				}
			}
			if (!result)
			{
				LinearExpression objectives;
				objectives.resize(1);
				objectives.resize(state.variableNames.size(), rational(bigint(1)));
				// TODO the model relies on the variable numbering.
				result = LPResult::Unknown;
				tie(*result, problem->model) = simplex(state.constraints, move(objectives));
				if (m_cache)
				{
					(*m_cache)[state] = *result;
					//cout << "Cache size " << m_cache->size() << endl;
				}
			}
			problem->dirty = false;
			problem->result = *result;
			if (problem->result == LPResult::Infeasible)
				return {LPResult::Infeasible, reasonSetForSubProblem(*problem)};
		}
	}

	return {LPResult::Unknown, Model{}};
}

void LPSolver::combineSubProblems(size_t _combineInto, size_t _combineFrom)
{
	//cout << "Combining " << _combineInto << " <- " << _combineFrom << endl;
	// TOOD creating a copy and setting dirty is on operation.
	m_subProblems[_combineInto] = make_shared<SubProblem>(*m_subProblems[_combineInto]);
	m_subProblems[_combineInto]->dirty = true;

	for (size_t& item: m_subProblemsPerVariable)
		if (item == _combineFrom)
			item = _combineInto;
	for (size_t& item: m_subProblemsPerConstraint)
		if (item == _combineFrom)
			item = _combineInto;
	m_subProblems[_combineInto]->variables += move(m_subProblems[_combineFrom]->variables);

	m_subProblems[_combineFrom].reset();
}

void LPSolver::addConstraintToSubProblem(size_t _subProblem, Constraint _constraint)
{
	m_subProblems[_subProblem] = make_shared<SubProblem>(*m_subProblems[_subProblem]);
	SubProblem& problem = *m_subProblems[_subProblem];
	problem.dirty = true;
	for (auto const& [index, entry]: _constraint.data.enumerateTail())
		if (entry)
		{
			solAssert(m_subProblemsPerVariable[index] == static_cast<size_t>(-1) || m_subProblemsPerVariable[index] == _subProblem);
			m_subProblemsPerVariable[index] = _subProblem;
			problem.variables.emplace(index);
		}
	problem.removableConstraints.emplace_back(move(_constraint));
}

SolvingState LPSolver::stateFromSubProblem(size_t _index) const
{
	SolvingState split;

	split.variableNames.emplace_back();
	split.bounds.emplace_back();

	SubProblem const& problem = *m_subProblems[_index];
	for (size_t varIndex: problem.variables)
	{
		split.variableNames.emplace_back(m_state->variableNames[varIndex]);
		split.bounds.emplace_back(m_state->bounds[varIndex]);
	}
	for (auto&& item: m_subProblemsPerConstraint | ranges::views::enumerate)
		if (item.second == _index)
		{
			Constraint const& constraint = m_state->constraints[item.first];
			Constraint splitRow{{}, constraint.equality, constraint.reasons};
			splitRow.data.push_back(constraint.data.get(0));
			for (size_t varIndex: problem.variables)
				splitRow.data.push_back(constraint.data.get(varIndex));
			split.constraints.push_back(move(splitRow));
		}

	for (Constraint const& constraint: m_subProblems[_index]->removableConstraints)
	{
		Constraint splitRow{{}, constraint.equality, constraint.reasons};
		splitRow.data.push_back(constraint.data.get(0));
		for (size_t varIndex: problem.variables)
			splitRow.data.push_back(constraint.data.get(varIndex));
		split.constraints.push_back(move(splitRow));
	}

	return split;
}

ReasonSet LPSolver::reasonSetForSubProblem(LPSolver::SubProblem const& _subProblem)
{
	ReasonSet reasons;
	for (Constraint const& c: _subProblem.removableConstraints)
		reasons += c.reasons;
	return reasons;
}
