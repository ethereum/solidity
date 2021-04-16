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
#include <libsolutil/StringUtils.h>
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

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;

using rational = boost::rational<bigint>;


namespace
{

string toString(rational const& _value, size_t _paddedLength = 2)
{
	string result;
	if (_value == rational(bigint(1) << 256))
		result = "2**256";
	else if (_value == rational(bigint(1) << 256) - 1)
		result = "2**256-1";
	else if (_value.denominator() == bigint(1))
		result = _value.numerator().str();
	else
		result = to_string(_value);
	if (result.length() < _paddedLength)
		result = string(_paddedLength - result.length(), ' ') + result;
	return result;
}

vector<rational> factorForVariable(size_t _index, rational _factor)
{
	vector<rational> result(_index + 1);
	result[_index] = move(_factor);
	return result;
}

rational const& get(vector<rational> const& _data, size_t _index)
{
	static rational const zero;
	return _index < _data.size() ? _data[_index] : zero;
}

template <class T>
void resizeAndSet(vector<T>& _data, size_t _index, T _value)
{
	if (_data.size() <= _index)
		_data.resize(_index + 1);
	_data[_index] = move(_value);
}

vector<rational>& operator/=(vector<rational>& _data, rational const& _divisor)
{
	for (rational& x: _data)
		if (x.numerator())
			x /= _divisor;
	return _data;
}

vector<rational>& operator*=(vector<rational>& _data, rational const& _factor)
{
	for (rational& x: _data)
		if (x.numerator())
			x *= _factor;
	return _data;
}

vector<rational> operator*(rational const& _factor, vector<rational> _data)
{
	for (rational& x: _data)
		if (x.numerator())
			x *= _factor;
	return _data;
}

vector<rational> operator-(vector<rational> const& _x, vector<rational> const& _y)
{
	vector<rational> result;
	for (size_t i = 0; i < max(_x.size(), _y.size()); ++i)
		result.emplace_back(get(_x, i) - get(_y, i));
	return result;
}

vector<rational>& operator-=(vector<rational>& _x, vector<rational> const& _y)
{
	solAssert(_x.size() == _y.size(), "");
	for (size_t i = 0; i < _x.size(); ++i)
		if (_y[i].numerator())
			_x[i] -= _y[i];
	return _x;
}

vector<rational> add(vector<rational> const& _x, vector<rational> const& _y)
{
	vector<rational> result;
	for (size_t i = 0; i < max(_x.size(), _y.size()); ++i)
		result.emplace_back(get(_x, i) + get(_y, i));
	return result;
}

bool isConstant(vector<rational> const& _x)
{
	return ranges::all_of(_x | ranges::views::tail, [](rational const& _v) { return _v == 0; });
}

/// Multiply two vectors where the first element of each vector is a constant factor.
/// Only works if at most one of the vector has a nonzero element after the first.
/// If this condition is violated, returns nullopt.
optional<vector<rational>> vectorProduct(optional<vector<rational>> _x, optional<vector<rational>> _y)
{
	if (!_x || !_y)
		return std::nullopt;
	if (!isConstant(*_y))
		swap(_x, _y);
	if (!isConstant(*_y))
		return std::nullopt;

	rational factor = _y->front();

	for (rational& element: *_x)
		element *= factor;
	return *_x;
}

vector<bool>& operator|=(vector<bool>& _x, vector<bool> const& _y)
{
	solAssert(_x.size() == _y.size(), "");
	for (size_t i = 0; i < _x.size(); ++i)
		if (_y[i])
			_x[i] = true;
	return _x;
}


/**
 * Simplex tableau with the first row representing the objective function.
 */
struct Tableau
{
	std::vector<std::vector<rational>> data;
};


pair<vector<Constraint>, bool> toEquationalForm(vector<Constraint> _constraints)
{
	size_t varsNeeded = static_cast<size_t>(ranges::count_if(_constraints, [](Constraint const& _c) { return !_c.equality; }));

	vector<Constraint> result;

	size_t columns = _constraints.at(0).data.size();
	size_t currentVariable = 0;
	for (Constraint& constraint: _constraints)
	{
		solAssert(constraint.data.size() == columns, "");
		result.emplace_back(Constraint{move(constraint.data) + vector<rational>(varsNeeded, rational{}), true});
		if (!constraint.equality)
		{
			result.back().data[columns + currentVariable] = bigint(1);
			currentVariable++;
		}
	}

	return make_pair(move(result), varsNeeded > 0);
}

optional<size_t> findPivotColumn(Tableau const& _tableau)
{
	auto&& [maxColumn, maxValue] = ranges::max(
		_tableau.data[0] | ranges::views::enumerate | ranges::views::tail,
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
		ranges::views::iota(size_t(1), _tableau.data.size()) |
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

void performPivot(Tableau& _tableau, size_t _pivotRow, size_t _pivotColumn)
{
	rational pivot = _tableau.data[_pivotRow][_pivotColumn];
	solAssert(pivot != 0, "");
	if (pivot != 1)
		_tableau.data[_pivotRow] /= pivot;
	solAssert(_tableau.data[_pivotRow][_pivotColumn] == rational(1), "");

	for (size_t i = 0; i < _tableau.data.size(); ++i)
		if (i != _pivotRow)
		{
			if (_tableau.data[i][_pivotColumn] == rational{})
			{
			}
			else if (_tableau.data[i][_pivotColumn] == rational{1})
				_tableau.data[i] -= _tableau.data[_pivotRow];
			else
				_tableau.data[i] -= _tableau.data[i][_pivotColumn] * _tableau.data[_pivotRow];
		}
}

/*
void printVector(vector<rational> const& _v)
{
	for (auto const& element: _v)
		cout << toString(element, 3) << ", ";
	cout << endl;
}
*/
vector<rational> optimalVector(Tableau const& _tableau);


/*
 * void printTableau(Tableau _tableau)
{
	cout << "------------" << endl;
	for (auto const& row: _tableau.data)
		printVector(row);
	cout << "------------" << endl;
	cout << "Solution: ";
	printVector(optimalVector(_tableau));
}
*/

void selectLastVectorsAsBasis(Tableau& _tableau)
{
	// We might skip the operation for a column if it is already the correct
	// unit vector and its cost coefficient is zero.
	size_t columns = _tableau.data.at(0).size();
	size_t rows = _tableau.data.size();
	for (size_t i = 1; i < rows; ++i)
		performPivot(_tableau, i, columns - rows + i);
}

/// Returns the row containing 1 if all other rows are zero.
optional<size_t> basisVariable(Tableau const& _tableau, size_t _column)
{
	optional<size_t> row;
	for (size_t i = 1; i < _tableau.data.size(); ++i)
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

vector<rational> optimalVector(Tableau const& _tableau)
{
	vector<rational> result;
	set<size_t> rowsSeen;
	for (size_t j = 1; j < _tableau.data[0].size(); j++)
	{
		optional<size_t> row = basisVariable(_tableau, j);
		if (row && rowsSeen.count(*row))
			row = nullopt;
		result.emplace_back(row ? _tableau.data[*row][0] : rational{});
		if (row)
			rowsSeen.insert(*row);
	}
	//solAssert(rowsSeen.size() == _tableau.data.size() - 1, "");
	return result;
}


/// Solve the LP Ax = b s.t. min c^Tx
/// The first row encodes the objective function
/// The first column encodes b
/// Assumes the tableau has a trivial basic feasible solution.
pair<LPResult, Tableau> simplexEq(Tableau _tableau)
{
	size_t iterations = 50 + _tableau.data[0].size() * 2;
	if (iterations > 60)
		iterations = 60;
	for (size_t step = 0; step <= iterations; ++step)
	{
		optional<size_t> pivotColumn = findPivotColumn(_tableau);
		if (!pivotColumn)
		{
			////cout << "Optimum: ";
			//vector<rational> optimum = optimalVector(_tableau);
			//printVector(optimum);

//			cout << "Feasible after " << step << " steps." << endl;
//			cout << "Constraints: " << (_tableau.data.size() - 1) << endl;
//			cout << "Variables: " << (_tableau.data[0].size() - 1) << endl;
			return make_pair(LPResult::Feasible, move(_tableau));
		}
		////cout << "Pivot column: " << *pivotColumn << endl;
		optional<size_t> pivotRow = findPivotRow(_tableau, *pivotColumn);
		if (!pivotRow)
		{
			////cout << "unbounded" << endl;
			return make_pair(LPResult::Unbounded, move(_tableau));
		}
		////cout << "Pivot row: " << *pivotRow << endl;
		performPivot(_tableau, *pivotRow, *pivotColumn);
		////cout << "After step " << step << endl;
		//printTableau(_tableau);
	}
	cout << "LP: Too many iterations: " << iterations << endl;
	return make_pair(LPResult::Unknown, Tableau{});
}

pair<LPResult, Tableau> simplexPhaseI(Tableau _tableau)
{
	vector<rational> originalObjective = _tableau.data[0];

	size_t rows = _tableau.data.size();
	size_t columns = _tableau.data.at(0).size();
	for (size_t i = 1; i < rows; ++i)
	{
		if (_tableau.data[i][0] < 0)
			_tableau.data[i] *= -1;
		_tableau.data[i] += vector<bigint>(rows - 1, bigint{});
		_tableau.data[i][columns + i - 1] = 1;
	}
	_tableau.data[0] =
		vector<rational>(columns, rational{}) +
		vector<rational>(rows - 1, rational{-1});

	//cout << "Phase I tableau: " << endl;
	//printTableau(_tableau);

	selectLastVectorsAsBasis(_tableau);

	//cout << "After basis selection: " << endl;
	//printTableau(_tableau);

	LPResult result;
	tie(result, _tableau) = simplexEq(move(_tableau));

	// TODO This should actually not happen.
	if (result != LPResult::Feasible && result != LPResult::Unbounded)
	{
		//cout << "Unknown because phaseI resulted in " << static_cast<int>(result) << endl;
		return make_pair(LPResult::Unknown, Tableau{});
	}
	vector<rational> optimum = optimalVector(_tableau);
	//cout << "PhaseI solution: ";
	//printVector(optimum);

	for (size_t i = columns - 1; i < optimum.size(); ++i)
		if (optimum[i] != 0)
		{
			//cout << "Phase I => infeasible." << endl;
			return make_pair(LPResult::Infeasible, Tableau{});
		}

	_tableau.data[0] = originalObjective;
	for (size_t i = 1; i < rows; ++i)
		_tableau.data[i].resize(columns);

	//cout << "Tableau after Phase I: " << endl;
	//printTableau(_tableau);

	// TODO do we need to select a basis?
	//_tableau = selectLastVectorsAsBasis(move(_tableau));
	////cout << "After basis correction: " << endl;
	//printTableau(_tableau);

	return make_pair(LPResult::Feasible, move(_tableau));
}

bool needsPhaseI(Tableau const& _tableau)
{
	// TODO with equality constraints, this may need refinement.
	for (size_t i = 1; i < _tableau.data.size(); ++i)
		if (_tableau.data[i][0] < 0)
			return true;
	return false;
}

/// Solve the LP Ax <= b s.t. min c^Tx
/// The first row encodes the objective function
/// The first column encodes b
pair<LPResult, vector<rational>> simplex(vector<Constraint> _constraints, vector<rational> _objectives)
{
	Tableau tableau;
	tableau.data.emplace_back(move(_objectives));
	bool hasEquations = false;
	tie(_constraints, hasEquations) = toEquationalForm(_constraints);
	for (Constraint& c: _constraints)
		tableau.data.emplace_back(move(c.data));
	tableau.data.front().resize(tableau.data.at(1).size());
	//cout << "Equational: " << endl;
	//printTableau(tableau);
	if (hasEquations || needsPhaseI(tableau))
	{
		LPResult result;
		tie(result, tableau) = simplexPhaseI(move(tableau));
		if (result == LPResult::Infeasible || result == LPResult::Unknown)
			return make_pair(result, vector<rational>{});
		solAssert(result == LPResult::Feasible, "");
		//cout << "Phase I returned feasible." << endl;
	}
	// We know that the system is satisfyable and we know a solution,
	// but it is not optimal.
	// TODO is the tableau always in a form to compute a solution?
	LPResult result;
	tie(result, tableau) = simplexEq(move(tableau));
	solAssert(result == LPResult::Feasible || result == LPResult::Unbounded, "");
	return make_pair(result, optimalVector(tableau));
}

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
void eraseIndices(vector<T>& _data, set<size_t> const& _indices)
{
	// TODO make this more efficient.
	vector<T> result;
	for (size_t i = 0; i < _data.size(); i++)
		if (!_indices.count(i))
			result.emplace_back(move(_data[i]));
	_data = move(result);
}


void removeColumns(SolvingState& _state, set<size_t> const& _columnsToRemove)
{
	eraseIndices(_state.bounds, _columnsToRemove);
	for (Constraint& constraint: _state.constraints)
		eraseIndices(constraint.data, _columnsToRemove);
	eraseIndices(_state.variableNames, _columnsToRemove);
}

bool extractDirectConstraints(SolvingState& _state, bool& _changed)
{
	// Turn constraints of the form ax <= b into an upper bound on x.
	// TODO this could be a `vector<bool>`
	set<size_t> constraintsToRemove;
	for (auto const& [index, constraint]: _state.constraints | ranges::views::enumerate)
	{
		auto nonzero = constraint.data | ranges::views::enumerate | ranges::views::tail | ranges::view::filter(
			[](std::pair<size_t, rational> const& _x) { return !!_x.second; }
		);
		// TODO we can exit early on in the loop above.
		// TODO could also use iterators and exit if we can advance it twice.
		auto numNonzero = ranges::distance(nonzero);
		if (numNonzero > 1)
			continue;
		constraintsToRemove.insert(index);
		if (numNonzero == 0)
		{
			// 0 <= b or 0 = b
			if (
				constraint.data.front() < 0 ||
				(constraint.equality && constraint.data.front() != 0)
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
	if (!constraintsToRemove.empty())
	{
		_changed = true;
		eraseIndices(_state.constraints, constraintsToRemove);
	}
	return true;
}

bool removeFixedVariables(SolvingState& _state, map<string, rational>& _model, bool& _changed)
{
	//set<size_t> variablesToRemove;
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
		// cout << "Fixing " << _state.variableNames.at(index) << " to " << lower << endl;
		_model[_state.variableNames.at(index)] = lower;
		//variablesToRemove.insert(index);
		_state.bounds[index] = {};
		_changed = true;
		////cout << "Removing variable " << _state.variableNames[index] << endl;

		// substitute variable
		for (Constraint& constraint: _state.constraints)
			if (constraint.data.at(index) != 0)
			{
				constraint.data[0] -= constraint.data[index] * lower;
				constraint.data[index] = 0;
			}
	}

/*	if (!variablesToRemove.empty())
	{
		_changed = true;
		//removeColumns(_state, variablesToRemove);
	}*/
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

	set<size_t> variablesToRemove;
	for (auto&& [i, seen]: variablesSeen | ranges::views::enumerate | ranges::views::tail)
		if (!seen)
		{
			variablesToRemove.insert(i);
			// TODO actually it is unbounded if _state.bounds.at(i)[1] is nullopt.
			if (_state.bounds.at(i)[0] || _state.bounds.at(i)[1])
				_model[_state.variableNames.at(i)] =
					_state.bounds.at(i)[1] ?
					*_state.bounds.at(i)[1] :
					*_state.bounds.at(i)[0];
		}
	if (!variablesToRemove.empty())
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
			for (auto const& [index, entry]: _state.constraints[row].data | ranges::view::enumerate | ranges::views::tail)
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
					splitRow.data.push_back(state.constraints[i].data[j]);
			splitOff.constraints.push_back(move(splitRow));
		}

		return splitOff;
	}

	SolvingState const& state;
	size_t column = 1;
	vector<bool> seenColumns;
};


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
		 rational const& a = get(data, i);
		 rational const& b = get(_other.data, i);
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
		 if (get(data, i) != get(_other.data, i))
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
		result += joinHumanReadable(line, " + ") + (constraint.equality ? "  = " : " <= ") + ::toString(constraint.data.front()) + "\n";
	}
	result += "Bounds:\n";
	for (auto&& [index, bounds]: bounds | ranges::view::enumerate)
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


pair<bool, map<size_t, bool>> DPLL::simplify()
{
	// TODO use vector?
	map<size_t, bool> assignments;
	while (!clauses.empty() && !ranges::all_of(clauses, [](Clause const& _c) { return _c.literals.size() > 1; }))
	{
		// TODO this assumes that no clause contains more than one constraint
		// TODO add an assertion for that.

		for (Clause const& c: clauses)
			if (c.literals.empty())
				return {false, {}};
			else if (c.literals.size() == 1)
			{
				Literal const& literal = c.literals.front();
				if (literal.kind == Literal::Constraint)
					constraints.push_back(literal.index);
				else
				{
					bool value = literal.kind == Literal::PositiveVariable ? true : false;
					if (assignments.count(literal.index) && assignments.at(literal.index) != value)
						return {false, {}};
					//cout << "Assigning" << variableName(literal.index) << " " << value << endl;
					assignments[literal.index] = value;
				}
			}

		if (!setVariables(assignments, true))
			return {false, {}};
	}
	return {true, move(assignments)};
}

size_t DPLL::findUnassignedVariable() const
{
	for (Clause const& c: clauses)
		for (Literal const& l: c.literals)
			if (l.kind != Literal::Constraint)
				return l.index;
	solAssert(false, "");
	return 0;
}

bool DPLL::setVariable(size_t _index, bool _value)
{
	return setVariables({{_index, _value}});
}

bool DPLL::setVariables(map<size_t, bool> const& _assignments, bool _removeSingleConstraintClauses)
{
	// TODO do this in a way so that we do not have to copy twice.
	vector<Clause> prunedClauses;
	for (Clause const& c: clauses)
	{
		bool skipClause = false;
		vector<Literal> newClause;
		if (_removeSingleConstraintClauses && c.literals.size() == 1 && c.literals.front().kind == Literal::Constraint)
			continue;
		for (Literal const& l: c.literals)
			if (l.kind != Literal::Constraint && _assignments.count(l.index))
			{
				if (_assignments.at(l.index) == (l.kind == Literal::PositiveVariable))
				{
					skipClause = true;
					break;
				}
			}
			else
				newClause.push_back(l);
		if (skipClause)
			continue;
		if (newClause.empty())
			return false;
		prunedClauses.push_back(Clause{move(newClause)});
	}
	clauses = move(prunedClauses);
	return true;
}

void BooleanLPSolver::reset()
{
	m_state = vector<State>{{State{}}};
	m_internalVariableCounter = 0;
	m_constraintCounter = 0;
	// Do not reset the solver, it should retain its cache.
}

void BooleanLPSolver::push()
{
	if (m_state.back().infeasible)
	{
		m_state.emplace_back();
		m_state.back().infeasible = true;
		return;
	}
	map<string, size_t> variables = m_state.back().variables;
	map<size_t, array<optional<boost::rational<bigint>>, 2>> bounds = m_state.back().bounds;
	vector<bool> isBooleanVariable = m_state.back().isBooleanVariable;
	m_state.emplace_back();
	m_state.back().variables = move(variables);
	m_state.back().isBooleanVariable = move(isBooleanVariable);
	m_state.back().bounds = move(bounds);
}

void BooleanLPSolver::pop()
{
	m_state.pop_back();
	solAssert(!m_state.empty(), "");
}

void BooleanLPSolver::declareVariable(string const& _name, SortPointer const& _sort)
{
	// Internal variables are '$<number>'
	string name = (_name.empty() || _name.at(0) != '$') ? _name : "$$" + _name;
	// TODO This will not be an integer variable in our model.
	// Introduce a new kind?
	solAssert(_sort && (_sort->kind == Kind::Int || _sort->kind == Kind::Bool), "");
	solAssert(!m_state.back().variables.count(name), "");
	declareVariable(name, _sort->kind == Kind::Bool);
}

void BooleanLPSolver::addAssertion(Expression const& _expr)
{
	if (_expr.arguments.empty())
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{*parseLiteral(_expr)}});
	else if (_expr.name == "=")
	{
		// Try to see if both sides are linear.
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (left && right)
		{
			vector<rational> data = *left - *right;
			data[0] *= -1;
			Constraint c{move(data), _expr.name == "="};
			if (!tryAddDirectBounds(c))
			{
				size_t index = addConstraint(move(c));
				m_state.back().clauses.emplace_back(Clause{vector<Literal>{Literal{Literal::Constraint, index}}});
			}
		}
		else if (_expr.arguments.at(0).arguments.empty() && isBooleanVariable(_expr.arguments.at(0).name))
			addBooleanEquality(*parseLiteral(_expr.arguments.at(0)), _expr.arguments.at(1));
		else if (_expr.arguments.at(1).arguments.empty() && isBooleanVariable(_expr.arguments.at(1).name))
			addBooleanEquality(*parseLiteral(_expr.arguments.at(1)), _expr.arguments.at(0));
		else
		{
			Literal const& newBoolean = *parseLiteral(declareInternalBoolean());
			addBooleanEquality(newBoolean, _expr.arguments.at(0));
			addBooleanEquality(newBoolean, _expr.arguments.at(1));
		}
	}
	else if (_expr.name == "and")
	{
		addAssertion(_expr.arguments.at(0));
		addAssertion(_expr.arguments.at(1));
	}
	else if (_expr.name == "or")
	{
		// We could try to parse a full clause here.
		Literal left = parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0));
		Literal right = parseLiteralOrReturnEqualBoolean(_expr.arguments.at(1));
		if (left.kind == Literal::Constraint && right.kind == Literal::Constraint)
		{
			// We cannot have more than one constraint per clause.
			right = *parseLiteral(declareInternalBoolean());
			addBooleanEquality(right, _expr.arguments.at(1));
		}
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{left, right}});
	}
	else if (_expr.name == "not")
	{
		Literal l = negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0)));
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{l}});
	}
	else if (_expr.name == "<=")
	{
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return;

		vector<rational> data = *left - *right;
		data[0] *= -1;
		Constraint c{move(data), _expr.name == "="};
		if (!tryAddDirectBounds(c))
		{
			size_t index = addConstraint(move(c));
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{Literal{Literal::Constraint, index}}});
		}
	}
	else if (_expr.name == ">=")
		addAssertion(_expr.arguments.at(1) <= _expr.arguments.at(0));
	else if (_expr.name == "<")
		addAssertion(_expr.arguments.at(0) <= _expr.arguments.at(1) - 1);
	else if (_expr.name == ">")
		addAssertion(_expr.arguments.at(1) < _expr.arguments.at(0));
}

pair<CheckResult, map<string, rational>> LPSolver::check(SolvingState _state)
{
	SolvingState state = move(_state);
	normalizeRowLengths(state);

	//cout << endl;
	//cout << "----------------------------------------" << endl;
	//cout << "Solving LP:\n" << toString(state) << endl;
	map<string, rational> model;

	if (!simplifySolvingState(state, model))
	{
		//cout << "LP: infeasible." << endl;
		return {CheckResult::UNSATISFIABLE, {}};
	}
	//cout << "Simplified to:\n" << toString(state) << endl;
	//cout << "----------------------------------------" << endl;

	bool canOnlyBeUnknown = false;
	ProblemSplitter splitter(state);
	while (splitter)
	{
		SolvingState split = splitter.next();
		solAssert(!split.constraints.empty(), "");
		solAssert(split.variableNames.size() >= 2, "");

		// cout << "Split off:\n" << split.toString() << endl;
		// cout << "----------------------------------------" << endl;

		LPResult lpResult;
		vector<rational> solution;
		auto it = m_cache.find(split);
		if (it != m_cache.end())
		{
			// cout << "Cache hit for" << split.toString() << endl;
			tie(lpResult, solution) = it->second;
		}
		else
		{
			// cout << "Cache miss for" << split.toString() << endl;
			SolvingState orig = split;
			if (!boundsToConstraints(split))
				lpResult = LPResult::Infeasible;
			else
			{
				// cout << "simplex query with " << split.variableNames.size() << " variables" << endl;
				tie(lpResult, solution) = simplex(split.constraints, vector<rational>(1, rational(bigint(0))) + vector<rational>(split.constraints.front().data.size() - 1, rational(bigint(1))));
			}
			m_cache.emplace(move(orig), make_pair(lpResult, solution));
			// cout << "simplex model: " <<endl;
			// for (auto const& value: solution)
				// cout << value << ", ";
			// cout << endl;
		}

		switch (lpResult)
		{
		case LPResult::Feasible:
		case LPResult::Unbounded:
			//cout << "feasible or unbounded after simplex" << endl;
			break;
		case LPResult::Infeasible:
			//cout << "infeasible after simplex" << endl;
			return {CheckResult::UNSATISFIABLE, {}};
		case LPResult::Unknown:
			//cout << "unknown after simplex" << endl;
			// We do not stop here, because another independent query can still be infeasible.
			canOnlyBeUnknown = true;
			break;
		}
		for (auto&& [index, value]: solution | ranges::views::enumerate)
			if (index + 1 < split.variableNames.size())
				model[split.variableNames.at(index + 1)] = value;
	}
//	cout << "No more sub-problems to split off." << endl;
//	cout << "----------------------------------------" << endl;

	bool solveInteger = false;

	//cout << "LP: feasible / unbounded, because no constraints left." << endl;
	if (solveInteger || canOnlyBeUnknown)
		return {CheckResult::UNKNOWN, {}};

	return {CheckResult::SATISFIABLE, move(model)};
}


pair<CheckResult, vector<string>> BooleanLPSolver::check(vector<Expression> const& _expressionsToEvaluate)
{
	cout << "Solving boolean constraint system" << endl;
	cout << toString() << endl;

	if (m_state.back().infeasible)
		return make_pair(CheckResult::UNSATISFIABLE, vector<string>{});

	// TODO compress variables because we ignore boolean variables
	SolvingState state;
	for (auto&& [name, index]: m_state.back().variables)
		resizeAndSet(state.variableNames, index, name);
	for (auto&& [index, value]: m_state.back().bounds)
		resizeAndSet(state.bounds, index, value);

	std::vector<Clause> clauses;
	for (State const& s: m_state)
		for (Clause const& clause: s.clauses)
			clauses.push_back(clause);

	auto&& [result, model] = runDPLL(state, DPLL{move(clauses), {}});
	if (result == CheckResult::SATISFIABLE)
	{
		vector<string> requestedModel;
		for (Expression const& e: _expressionsToEvaluate)
			requestedModel.emplace_back(
				e.arguments.empty() && model.count(e.name) ?
				::toString(model[e.name], 0) :
				string("unknown")
			);
		return {result, move(requestedModel)};
	}
	else
		return {result, {}};
}

string BooleanLPSolver::toString() const
{
	string result;

	for (State const& s: m_state)
		for (Clause const& c: s.clauses)
			result += toString(c);
	result += "-- Bounds:\n";
	for (State const& s: m_state)
		for (auto&& [index, bounds]: s.bounds)
		{
			if (!bounds[0] && !bounds[1])
				continue;
			if (bounds[0])
				result += ::toString(*bounds[0]) + " <= ";
			result += variableName(index);
			if (bounds[1])
				result += " <= " + ::toString(*bounds[1]);
			result += "\n";
		}
	return result;
}

Expression BooleanLPSolver::declareInternalBoolean()
{
	string name = "$" + to_string(m_internalVariableCounter++);
	declareVariable(name, true);
	return smtutil::Expression(name, {}, SortProvider::boolSort);
}

void BooleanLPSolver::declareVariable(string const& _name, bool _boolean)
{
	size_t index = m_state.back().variables.size() + 1;
	m_state.back().variables[_name] = index;
	resizeAndSet(m_state.back().isBooleanVariable, index, _boolean);
}

optional<Literal> BooleanLPSolver::parseLiteral(smtutil::Expression const& _expr)
{
	// TODO constanst true/false?

	if (_expr.arguments.empty())
	{
		if (isBooleanVariable(_expr.name))
			return Literal{
				Literal::PositiveVariable,
				m_state.back().variables.at(_expr.name)
			};
	}
	else if (_expr.name == "not")
		return negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0)));
	else if (_expr.name == "<=")
	{
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return {};

		vector<rational> data = *left - *right;
		data[0] *= -1;

		return Literal{Literal::Constraint, addConstraint(Constraint{move(data), false})};
	}
	else if (_expr.name == ">=")
		return parseLiteral(_expr.arguments.at(1) <= _expr.arguments.at(0));
	else if (_expr.name == "<")
		return parseLiteral(_expr.arguments.at(0) <= _expr.arguments.at(1) - 1);
	else if (_expr.name == ">")
		return parseLiteral(_expr.arguments.at(1) < _expr.arguments.at(0));

	return {};
}

Literal BooleanLPSolver::negate(Literal const& _lit)
{
	switch (_lit.kind)
	{
	case Literal::NegativeVariable:
		return Literal{Literal::PositiveVariable, _lit.index};
	case Literal::PositiveVariable:
		return Literal{Literal::NegativeVariable, _lit.index};
	default:
		break;
	}

	Constraint const& c = constraint(_lit.index);

	solAssert(!c.equality, "");

	// X > b
	// -x < -b
	// -x <= -b - 1

	Constraint negated = c;
	negated.data *= -1;
	negated.data[0] -= 1;

	return Literal{Literal::Constraint, addConstraint(move(negated))};
}

Literal BooleanLPSolver::parseLiteralOrReturnEqualBoolean(Expression const& _expr)
{
	if (optional<Literal> literal = parseLiteral(_expr))
		return *literal;
	else
	{
		Literal newBoolean = *parseLiteral(declareInternalBoolean());
		addBooleanEquality(newBoolean, _expr);
		return newBoolean;
	}
}

optional<vector<rational>> BooleanLPSolver::parseLinearSum(smtutil::Expression const& _expr) const
{
	if (_expr.arguments.empty() || _expr.name == "*")
		return parseProduct(_expr);
	else if (_expr.name == "+" || _expr.name == "-")
	{
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return std::nullopt;
		return _expr.name == "+" ? add(*left, *right) : *left - *right;
	}
	else
		return std::nullopt;
}

optional<vector<rational>> BooleanLPSolver::parseProduct(smtutil::Expression const& _expr) const
{
	if (_expr.arguments.empty())
		return parseFactor(_expr);
	else if (_expr.name == "*")
		// The multiplication ensures that only one of them can be a variable.
		return vectorProduct(parseFactor(_expr.arguments.at(0)), parseFactor(_expr.arguments.at(1)));
	else
		return std::nullopt;
}

optional<vector<rational>> BooleanLPSolver::parseFactor(smtutil::Expression const& _expr) const
{
	solAssert(_expr.arguments.empty(), "");
	solAssert(!_expr.name.empty(), "");
	if ('0' <= _expr.name[0] && _expr.name[0] <= '9')
		return vector<rational>{rational(bigint(_expr.name))};
	else if (_expr.name == "true")
		return vector<rational>{rational(bigint(1))};
	else if (_expr.name == "false")
		return vector<rational>{rational(bigint(0))};

	size_t index = m_state.back().variables.at(_expr.name);
	solAssert(index > 0, "");
	if (isBooleanVariable(index))
		return nullopt;
	return factorForVariable(index, rational(bigint(1)));
}

bool BooleanLPSolver::tryAddDirectBounds(Constraint const& _constraint)
{
	auto nonzero = _constraint.data | ranges::views::enumerate | ranges::views::tail | ranges::view::filter(
		[](std::pair<size_t, rational> const& _x) { return !!_x.second; }
	);
	// TODO we can exit early on in the loop above.
	if (ranges::distance(nonzero) > 1)
		return false;

	//cout << "adding direct bound." << endl;
	if (ranges::distance(nonzero) == 0)
	{
		// 0 <= b or 0 = b
		if (
			_constraint.data.front() < 0 ||
			(_constraint.equality && _constraint.data.front() != 0)
		)
		{
//			cout << "SETTING INF" << endl;
			m_state.back().infeasible = true;
		}
	}
	else
	{
		auto&& [varIndex, factor] = nonzero.front();
		// a * x <= b
		rational bound = _constraint.data[0] / factor;
		if (factor > 0 || _constraint.equality)
			addUpperBound(varIndex, bound);
		if (factor < 0 || _constraint.equality)
			addLowerBound(varIndex, bound);
	}
	return true;
}

void BooleanLPSolver::addUpperBound(size_t _index, rational _value)
{
	//cout << "adding " << variableName(_index) << " <= " << toString(_value) << endl;
	if (!m_state.back().bounds[_index][1] || _value < *m_state.back().bounds[_index][1])
		m_state.back().bounds[_index][1] = move(_value);
}

void BooleanLPSolver::addLowerBound(size_t _index, rational _value)
{
	// Lower bound must be at least zero.
	_value = max(_value, rational{});
	//cout << "adding " << variableName(_index) << " >= " << toString(_value) << endl;
	if (!m_state.back().bounds[_index][0] || _value > *m_state.back().bounds[_index][0])
		m_state.back().bounds[_index][0] = move(_value);
}

size_t BooleanLPSolver::addConstraint(Constraint _constraint)
{
	size_t index = ++m_constraintCounter;
	m_state.back().constraints[index] = move(_constraint);
	return index;
}

void BooleanLPSolver::addBooleanEquality(Literal const& _left, smtutil::Expression const& _right)
{
	if (optional<Literal> right = parseLiteral(_right))
	{
		 // includes: not, <=, <, >=, >, boolean variables.
		 // a = b <=> (-a \/ b) /\ (a \/ -b)
		 Literal negLeft = negate(_left);
		 Literal negRight = negate(*right);
		 m_state.back().clauses.emplace_back(Clause{vector<Literal>{negLeft, *right}});
		 m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negRight}});
	}
	else if (_right.name == "=" && parseLinearSum(_right.arguments.at(0)) && parseLinearSum(_right.arguments.at(1)))
		// a = (x = y)  <=>  a = (x <= y && x >= y)
		addBooleanEquality(
			_left,
			_right.arguments.at(0) <= _right.arguments.at(1) &&
			_right.arguments.at(1) <= _right.arguments.at(0)
		);
	else
	{
		Literal a = parseLiteralOrReturnEqualBoolean(_right.arguments.at(0));
		Literal b = parseLiteralOrReturnEqualBoolean(_right.arguments.at(1));
		if (a.kind == Literal::Constraint && b.kind == Literal::Constraint)
		{
			// We cannot have more than one constraint per clause.
			b = *parseLiteral(declareInternalBoolean());
			addBooleanEquality(b, _right.arguments.at(1));
		}

		if (_right.name == "and")
		{
			// a = and(x, y) <=> (-a \/ x) /\ ( -a \/ y) /\ (a \/ -x \/ -y)
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), b}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a), negate(b)}});
		}
		else if (_right.name == "or")
		{
			// a = or(x, y) <=> (-a \/ x \/ y) /\ (a \/ -x) /\ (a \/ -y)
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a, b}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a)}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(b)}});
		}
		else if (_right.name == "=")
		{
			// l = eq(a, b) <=> (-l or -a or b) and (-l or a or -b) and (l or -a or -b) and (l or a or b)
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), negate(a), b}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a, negate(b)}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a), negate(b)}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, a, b}});
		}
		else
			solAssert(false, "Unsupported operation: " + _right.name);
	}
}


// TODO as input we do not need the full solving state, only the bounds
pair<CheckResult, map<string, rational>> BooleanLPSolver::runDPLL(SolvingState& _solvingState, DPLL _dpll)
{
	//cout << "Running dpll on" << endl << toString(_solvingState.bounds) << "\n" << toString(_dpll) << endl;

	auto&& [simplifyResult, booleanModel] = _dpll.simplify();
	if (!simplifyResult)
		return {CheckResult::UNSATISFIABLE, {}};

	//cout << "Simplified to" << endl << toString(_solvingState.bounds) << "\n" << toString(_dpll) << endl;

	CheckResult result = CheckResult::UNKNOWN;
	map<string, rational> model;
	// TODO could run this check already even though not all variables are assigned
	// and return unsat if it is already unsat.
	if (_dpll.clauses.empty())
	{
		//cout << "Invoking LP..." << endl;
		_solvingState.constraints.clear();
		for (size_t c: _dpll.constraints)
			_solvingState.constraints.emplace_back(constraint(c));
		tie(result, model) = m_lpSolver.check(_solvingState);
		// cout << "Model: " << endl;
		// for (auto const& [index, value]: model)
			// cout << index << " = " << value << endl;
	}
	else
	{
		size_t varIndex = _dpll.findUnassignedVariable();

		DPLL copy = _dpll;
		if (_dpll.setVariable(varIndex, true))
		{
			booleanModel[varIndex] = true;
			// cout << "Trying " << variableName(varIndex) << " = true\n";
			tie(result, model) = runDPLL(_solvingState, move(_dpll));
			// TODO actually we should also handle UNKNOWN here.
		}
		if (result != CheckResult::SATISFIABLE)
		{
			// cout << "Trying " << variableName(varIndex) << " = false\n";
			if (!copy.setVariable(varIndex, false))
				return {CheckResult::UNSATISFIABLE, {}};
			booleanModel[varIndex] = false;
			auto&& [result, model] = runDPLL(_solvingState, move(copy));
		}
	}
	if (result == CheckResult::SATISFIABLE)
	{
		for (auto const& [index, value]: booleanModel)
			model[variableName(index)] = value ? 1 : 0;
		return {result, move(model)};
	}
	else
		return {result, {}};
}

string BooleanLPSolver::toString(DPLL const& _dpll) const
{
	string result;
	for (size_t c: _dpll.constraints)
		result += toString(Clause{{Literal{Literal::Constraint, c}}});
	for (Clause const& c: _dpll.clauses)
		result += toString(c);
	return result;
}

string BooleanLPSolver::toString(std::vector<std::array<std::optional<boost::rational<bigint>>, 2>> const& _bounds) const
{
	string result;
	for (auto&& [index, bounds]: _bounds | ranges::view::enumerate)
	{
		if (!bounds[0] && !bounds[1])
			continue;
		if (bounds[0])
			result += ::toString(*bounds[0]) + " <= ";
		// TODO If the variables are compressed, this does no longer work.
		result += variableName(index);
		if (bounds[1])
			result += " <= " + ::toString(*bounds[1]);
		result += "\n";
	}
	return result;
}

string BooleanLPSolver::toString(Clause const& _clause) const
{
	vector<string> literals;
	for (Literal const& l: _clause.literals)
		if (l.kind == Literal::Constraint)
		{
			Constraint const& constr = constraint(l.index);
			vector<string> line;
			for (auto&& [index, multiplier]: constr.data | ranges::views::enumerate)
				if (index > 0 && multiplier != 0)
				{
					string mult =
						multiplier == -1 ?
						"-" :
						multiplier == 1 ?
						"" :
						::toString(multiplier) + " ";
					line.emplace_back(mult + variableName(index));
				}
			literals.emplace_back(joinHumanReadable(line, " + ") + (constr.equality ? "  = " : " <= ") + ::toString(constr.data.front()));
		}
		else
			literals.emplace_back((l.kind == Literal::NegativeVariable ? "!" : "") + variableName(l.index));
	return joinHumanReadable(literals, "  \\/  ") + "\n";
}

Constraint const& BooleanLPSolver::constraint(size_t _index) const
{
	for (State const& state: m_state)
		if (state.constraints.count(_index))
			return state.constraints.at(_index);
	solAssert(false, "");
}


string BooleanLPSolver::variableName(size_t _index) const
{
	for (auto const& v: m_state.back().variables)
		if (v.second == _index)
			return v.first;
	return {};
}

bool BooleanLPSolver::isBooleanVariable(string const& _name) const
{
	if (!m_state.back().variables.count(_name))
		return false;
	size_t index = m_state.back().variables.at(_name);
	solAssert(index > 0, "");
	return isBooleanVariable(index);
}

bool BooleanLPSolver::isBooleanVariable(size_t _index) const
{
	return
		_index < m_state.back().isBooleanVariable.size() &&
		m_state.back().isBooleanVariable.at(_index);
}
