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
#include <functional>

namespace solidity::util
{

using Model = std::map<std::string, rational>;
using ReasonSet = std::set<size_t>;

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
	/// Set of literals the conjunction of which implies this constraint.
	std::set<size_t> reasons = {};

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

		// TODO this is currently not used

		/// Set of literals the conjunction of which implies the lower bonud.
		std::set<size_t> lowerReasons;
		/// Set of literals the conjunction of which implies the upper bonud.
		std::set<size_t> upperReasons;
	};
	/// Lower and upper bounds for variables (in the sense of >= / <=).
	std::vector<Bounds> bounds;
	std::vector<Constraint> constraints;
	// For each bound and constraint, store an index of the literal
	// that implies it.

	std::set<size_t> reasons() const;

	struct Compare
	{
		explicit Compare(bool _considerVariableNames = false): considerVariableNames(_considerVariableNames) {}
		bool operator()(SolvingState const& _a, SolvingState const& _b) const;
		bool considerVariableNames;
	};

	bool operator==(SolvingState const& _other) const noexcept {
		return bounds == _other.bounds && constraints == _other.constraints;
	}

	std::string toString() const;
};

}

template <class T>
inline void hashCombine(std::size_t& _seed, T const& _v)
{
	std::hash<T> hasher;
	_seed ^= hasher(_v) + 0x9e3779b9 + (_seed << 6) + (_seed >> 2);
}

template <class T>
inline void hashCombineVector(std::size_t& _seed, std::vector<T> const& _v)
{
	hashCombine(_seed, _v.size());
	for (auto const& x: _v)
		hashCombine(_seed, x);
}

template<>
struct std::hash<solidity::util::SolvingState::Bounds>
{
	std::size_t operator()(solidity::util::SolvingState::Bounds const& _bounds) const noexcept
	{
		std::size_t result = 0;
		hashCombine(result, _bounds.lower);
		hashCombine(result, _bounds.upper);
		return result;
	}
};

template<>
struct std::hash<solidity::util::LinearExpression>
{
	std::size_t operator()(solidity::util::LinearExpression const& _linearExpression) const noexcept
	{
		std::size_t result = 0;
		hashCombine(result, _linearExpression.size());
		for (auto const& x: _linearExpression.enumerate())
			hashCombine(result, x.second);
		return result;
	}
};

template<>
struct std::hash<solidity::util::Constraint>
{
	std::size_t operator()(solidity::util::Constraint const& _constraint) const noexcept
	{
		std::size_t result = 0;
		hashCombine(result, _constraint.equality);
		hashCombine(result, _constraint.data);
		return result;
	}
};

template<>
struct std::hash<solidity::util::SolvingState>
{
	std::size_t operator()(solidity::util::SolvingState const& _solvingState) const noexcept
	{
		std::size_t result = 0;
		hashCombineVector(result, _solvingState.bounds);
		hashCombineVector(result, _solvingState.constraints);
		return result;
	}
};


namespace solidity::util
{

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

	std::pair<LPResult, std::variant<std::map<size_t, rational>, ReasonSet>> simplify();

private:
	/// Remove variables that have equal lower and upper bound.
	/// @returns reason / set of conflicting clauses if infeasible.
	std::optional<ReasonSet> removeFixedVariables();

	/// Removes constraints of the form 0 <= b or 0 == b (no variables) and
	/// turns constraints of the form a * x <= b (one variable) into bounds.
	/// @returns reason / set of conflicting clauses if infeasible.
	std::optional<ReasonSet> extractDirectConstraints();

	/// Removes all-zeros columns.
	void removeEmptyColumns();

	/// Set to true by the strategies if they performed some changes.
	bool m_changed = false;

	SolvingState& m_state;
	std::map<size_t, rational> m_fixedVariables;
};

/**
 * Splits a given linear program into multiple linear programs with disjoint sets of variables.
 * The initial program is feasible if and only if all sub-programs are feasible.
 */
class ProblemSplitter
{
public:
	explicit ProblemSplitter(SolvingState const& _state);

	/// @returns true if there are still sub-problems to split out.
	operator bool() const { return m_column < m_state.variableNames.size(); }

	/// @returns the next sub-problem.
	std::pair<std::vector<bool>, std::vector<bool>> next();

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
	explicit LPSolver(std::unordered_map<SolvingState, LPResult>* _cache):
		m_cache(_cache) {}


	LPResult setState(SolvingState _state);
	void addConstraint(Constraint _constraint);
	std::pair<LPResult, std::variant<Model, ReasonSet>> check();

private:
	void combineSubProblems(size_t _combineInto, size_t _combineFrom);
	void addConstraintToSubProblem(size_t _subProblem, Constraint _constraint);
	void updateSubProblems();

	/// Ground state for CDCL. This is shared by copies of the solver.
	/// Only ``setState`` changes the state. Copies will only use
	/// ``addConstraint`` which does not change m_state.
	std::shared_ptr<SolvingState> m_state;
	struct SubProblem
	{
		// TODO now we could actually put the constraints here again.
		std::vector<Constraint> removableConstraints;
		bool dirty = true;
		LPResult result = LPResult::Unknown;
		std::vector<boost::rational<bigint>> model = {};
		std::set<size_t> variables = {};
	};

	SolvingState stateFromSubProblem(size_t _index) const;
	ReasonSet reasonSetForSubProblem(SubProblem const& _subProblem);

	std::shared_ptr<std::map<size_t, rational>> m_fixedVariables;
	/// These use "copy on write".
	std::vector<std::shared_ptr<SubProblem>> m_subProblems;
	std::vector<size_t> m_subProblemsPerVariable;
	std::vector<size_t> m_subProblemsPerConstraint;
	/// TODO also store the first infeasible subproblem?
	/// TODO still retain the cache?
	std::unordered_map<SolvingState, LPResult>* m_cache = nullptr;

};

}
