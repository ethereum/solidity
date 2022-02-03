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
#pragma once

#include <libsolutil/Numeric.h>
#include <libsolutil/LinearExpression.h>

#include <boost/rational.hpp>

#include <vector>
#include <variant>

namespace solidity::util
{

/**
 * Constraint of the form
 *  - data[1] * x_1 + data[2] * x_2 + ... <= data[0]  (equality == false)
 *  - data[1] * x_1 + data[2] * x_2 + ...  = data[0]  (equality == true)
 * The set and order of variables is implied.
 */
struct Constraint
{
	LinearExpression data;
	bool equality = false;

	bool operator<(Constraint const& _other) const;
	bool operator==(Constraint const& _other) const;
};

/**
 * State used when solving an LP problem.
 */
struct SolvingState
{
	/// Names of variables. The index zero should be left empty
	/// because zero corresponds to constants.
	std::vector<std::string> variableNames;
	struct Bounds
	{
		std::optional<rational> lower;
		std::optional<rational> upper;
		bool operator<(Bounds const& _other) const { return make_pair(lower, upper) < make_pair(_other.lower, _other.upper); }
		bool operator==(Bounds const& _other) const { return make_pair(lower, upper) == make_pair(_other.lower, _other.upper); }
	};
	/// Lower and upper bounds for variables (in the sense of >= / <=).
	std::vector<Bounds> bounds;
	std::vector<Constraint> constraints;

	struct Compare
	{
		explicit Compare(bool _considerVariableNames = true): considerVariableNames(_considerVariableNames) {}
		bool operator()(SolvingState const& _a, SolvingState const& _b) const;
		bool considerVariableNames;
	};

	std::string toString() const;
};

enum class LPResult
{
	Unknown,
	Unbounded, ///< System has a solution, but it can have an arbitrary objective value.
	Feasible, ///< System has a solution (it might be unbounded, though).
	Infeasible ///< System does not have any solution.
};


/**
 * Applies several strategies to simplify a given solving state.
 * During these simplifications, it can sometimes already be determined if the
 * state is feasible or not.
 * Since some variables can be fixed to specific values, it returns a
 * (partial) model.
 *
 * - Constraints with exactly one nonzero coefficient represent "a x <= b"
 *   and thus are turned into bounds.
 * - Constraints with zero nonzero coefficients are constant relations.
 *   If such a relation is false, answer "infeasible", otherwise remove the constraint.
 * - Empty columns can be removed.
 * - Variables with matching bounds can be removed from the problem by substitution.
 *
 * Holds a reference to the solving state that is modified during operation.
 */
class SolvingStateSimplifier
{
public:
	SolvingStateSimplifier(SolvingState& _state):
		m_state(_state) {}

	std::pair<LPResult, std::map<std::string, rational>> simplify();

private:
	/// Remove variables that have equal lower and upper bound.
	/// @returns false if the system is infeasible.
	bool removeFixedVariables();

	/// Removes constraints of the form 0 <= b or 0 == b (no variables) and
	/// turns constraints of the form a * x <= b (one variable) into bounds.
	bool extractDirectConstraints();

	/// Removes all-zeros columns.
	bool removeEmptyColumns();

	/// Set to true by the strategies if they performed some changes.
	bool m_changed = false;

	SolvingState& m_state;
	std::map<std::string, rational> m_model;
};

/**
 * Splits a given linear program into multiple linear programs with disjoint sets of variables.
 * The initial program is feasible if and only if all sub-programs are feasible.
 */
class ProblemSplitter
{
public:
	explicit ProblemSplitter(SolvingState const& _state):
		m_state(_state),
		m_column(1),
		m_seenColumns(std::vector<bool>(m_state.variableNames.size(), false))
	{}

	/// @returns true if there are still sub-problems to split out.
	operator bool() const { return m_column < m_state.variableNames.size(); }

	/// @returns the next sub-problem.
	SolvingState next();

private:
	SolvingState const& m_state;
	/// Next column to start the search for a connected component.
	size_t m_column = 1;
	/// The columns we have already split out.
	std::vector<bool> m_seenColumns;
};

/**
 * LP solver for rational problems.
 *
 * Does not solve integer problems!
 *
 * Tries to split a given problem into sub-problems and utilizes a cache to quickly solve
 * similar problems.
 *
 * Can be used in a mode where it does not support returning models. In that case, the
 * cache is more efficient.
 */
class LPSolver
{
public:
	explicit LPSolver(bool _supportModels = true);

	std::pair<LPResult, std::map<std::string, boost::rational<bigint>>> check(SolvingState _state);

private:
	using CacheValue = std::pair<LPResult, std::vector<boost::rational<bigint>>>;

	bool m_supportModels = true;
	std::map<SolvingState, CacheValue, SolvingState::Compare> m_cache;
};

}
