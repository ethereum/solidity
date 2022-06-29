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
 *  - data[1] * x_1 + data[2] * x_2 + ... <= data[0]  (LESS_OR_EQUAL)
 *  - data[1] * x_1 + data[2] * x_2 + ...  < data[0]  (LESS_THAN)
 *  - data[1] * x_1 + data[2] * x_2 + ...  = data[0]  (EQUAL)
 * The set and order of variables is implied.
 */
struct Constraint
{
	LinearExpression data;
	enum Kind { EQUAL, LESS_THAN, LESS_OR_EQUAL };
	Kind kind = LESS_OR_EQUAL;

	bool operator<(Constraint const& _other) const;
	bool operator==(Constraint const& _other) const;
};

/**
 * A two-dimensional rational number "a + b*delta" that can be used to perform strict comparisons:
 * x > 0 is transformed into x >= 1*delta, where delta is assumed to be "small". Its value
 * is never explicitly computed / set, it is just a symbolic parameter.
 */
struct RationalWithDelta
{
	RationalWithDelta(rational _x = {}): m_main(move(_x)) {}
	static RationalWithDelta delta()
	{
		RationalWithDelta x(0);
		x.m_delta = 1;
		return x;
	}

	RationalWithDelta& operator+=(RationalWithDelta const& _other)
	{
		m_main += _other.m_main;
		m_delta += _other.m_delta;
		return *this;
	}
	RationalWithDelta& operator-=(RationalWithDelta const& _other)
	{
		m_main -= _other.m_main;
		m_delta -= _other.m_delta;
		return *this;
	}
	RationalWithDelta operator-(RationalWithDelta const& _other) const
	{
		RationalWithDelta ret = *this;
		ret -= _other;
		return ret;
	}
	RationalWithDelta& operator*=(rational const& _factor)
	{
		m_main *= _factor;
		m_delta *= _factor;
		return *this;
	}
	RationalWithDelta operator*(rational const& _factor) const
	{
		RationalWithDelta ret = *this;
		ret *= _factor;
		return ret;
	}
	RationalWithDelta& operator/=(rational const& _factor)
	{
		m_main /= _factor;
		m_delta /= _factor;
		return *this;
	}
	RationalWithDelta operator/(rational const& _factor) const
	{
		RationalWithDelta ret = *this;
		ret /= _factor;
		return ret;
	}
	bool operator<=(RationalWithDelta const& _other) const
	{
		return std::tie(m_main, m_delta) <= std::tie(_other.m_main, _other.m_delta);
	}
	bool operator>=(RationalWithDelta const& _other) const
	{
		return std::tie(m_main, m_delta) >= std::tie(_other.m_main, _other.m_delta);
	}
	bool operator<(RationalWithDelta const& _other) const
	{
		return std::tie(m_main, m_delta) < std::tie(_other.m_main, _other.m_delta);
	}
	bool operator>(RationalWithDelta const& _other) const
	{
		return std::tie(m_main, m_delta) > std::tie(_other.m_main, _other.m_delta);
	}
	bool operator==(RationalWithDelta const& _other) const
	{
		return std::tie(m_main, m_delta) == std::tie(_other.m_main, _other.m_delta);
	}
	bool operator!=(RationalWithDelta const& _other) const
	{
		return std::tie(m_main, m_delta) != std::tie(_other.m_main, _other.m_delta);
	}

	std::string toString() const;

	rational m_main;
	rational m_delta;
};

}


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
 * LP solver for rational problems, based on "A Fast Linear-Arithmetic Solver for DPLL(T)*"
 * by Dutertre and Moura.
 *
 * Does not solve integer problems!
 *
 * Tries to split incoming bounds and constraints into unrelated sub-problems.
 * Maintains lower/upper bounds for all variables.
 * Adds one slack variable per constraint and stores all constraints as "= 0" equations.
 * Splits variables into basic and non-basic. For each row there is exactly one
 * basic variable that has a factor of -1.
 * The equations are satisfied at all times and non-basic variables are always within their bounds.
 * Non-basic variables might violate their bounds.
 * It attempts to resolve these violations in turn, swapping a basic variables with a non-basic
 * variables that can still move in the required direction.
 *
 * It is perfectly fine to add new bounds, variable or constraints after a call to "check".
 * The solver can be copied at low cost and it uses a "copy on write" mechanism for the sub-problems.
 */
class LPSolver
{
public:
	void addConstraint(Constraint const& _constraint, std::optional<size_t> _reason = std::nullopt);
	void setVariableName(size_t _variable, std::string _name);
	void addLowerBound(size_t _variable, RationalWithDelta _bound, std::optional<size_t> _reason = std::nullopt);
	void addUpperBound(size_t _variable, RationalWithDelta _bound, std::optional<size_t> _reason = std::nullopt);

	std::pair<LPResult, ReasonSet> check();

	std::string toString() const;
	std::map<std::string, rational> model() const;

private:
	struct Bounds
	{
		std::optional<RationalWithDelta> lower;
		std::optional<RationalWithDelta> upper;
		std::optional<size_t> lowerReason;
		std::optional<size_t> upperReason;
	};
	struct Variable
	{
#ifdef DEBUG
		std::string name = {};
#endif
		RationalWithDelta value = {};
		Bounds bounds = {};
	};
	struct SubProblem
	{
		/// Set to true on "check". Needs a copy for adding a constraint or bound if set to true.
		bool sealed = false;
		std::optional<LPResult> result = std::nullopt;
		std::vector<LinearExpression> factors;
		std::vector<Variable> variables;
		std::set<size_t> variablesPotentiallyOutOfBounds;
		/// Variable index to constraint it controls.
		std::map<size_t, size_t> basicVariables;
		/// Maps outer indices to inner indices.
		std::map<size_t, size_t> varMapping = {};
		std::set<size_t> reasons;

		LPResult check();
		std::string toString() const;
	private:
		bool correctNonbasic();
		/// Set value of non-basic variable.
		void update(size_t _varIndex, RationalWithDelta const& _value);
		/// @returns the index of the first basic variable violating its bounds.
		std::optional<size_t> firstConflictingBasicVariable() const;
		std::optional<size_t> firstReplacementVar(size_t _basicVarToReplace, bool _increasing) const;
		/// @returns the set of reasons in case "firstReplacementVar" failed.
		std::set<size_t> reasonsForUnsat(size_t _basicVarToReplace, bool _increasing) const;

		void pivot(size_t _old, size_t _new);
		void pivotAndUpdate(size_t _oldBasicVar, RationalWithDelta const& _newValue, size_t _newBasicVar);
	};


	SubProblem& unseal(size_t _problem);
	/// Unseals the problem for the given variable or creates a new one.
	SubProblem& unsealForVariable(size_t _outerIndex);
	void combineSubProblems(size_t _combineInto, size_t _combineFrom);
	void addConstraintToSubProblem(size_t _subProblem, Constraint const& _constraint, std::optional<size_t> _reason);
	void addOuterVariableToSubProblem(size_t _subProblem, size_t _outerIndex);
	size_t addNewVariableToSubProblem(size_t _subProblem);

	/// These use "copy on write".
	std::vector<std::shared_ptr<SubProblem>> m_subProblems;
	/// Maps outer indices to sub problems.
	std::map<size_t, size_t> m_subProblemsPerVariable;
	/// Counter to enable unique names for the slack variables.
	size_t m_slackVariableCounter = 0;

};

}
