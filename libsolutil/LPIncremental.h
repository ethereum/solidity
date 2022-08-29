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

//#define DEBUG 1

#include <libsolutil/Numeric.h>
#include <libsolutil/LinearExpression.h>

#include <boost/rational.hpp>

#include <vector>
#include <variant>
#include <functional>

namespace solidity::util
{

using rational = boost::rational<bigint>;
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

class LPSolver
{
public:
	void addConstraint(Constraint const& _constraint);
	void addLowerBound(size_t _variable, RationalWithDelta _bound);
	void addUpperBound(size_t _variable, RationalWithDelta _bound);
	/// Add the conditional constraint but do not activate it yet.
	void addConditionalConstraint(Constraint const& _constraint, size_t _reason);
	void activateConstraint(size_t _reason);
	void setTrailSize(size_t _trailSize);

	void setVariableName(size_t _variable, std::string _name);

	std::optional<bool> recommendedPolarity(size_t _reason) const;

	std::pair<LPResult, ReasonSet> check();

	std::string toString() const;
	std::map<std::string, rational> model() const;

private:
	struct Bounds
	{
		std::optional<RationalWithDelta> lower;
		std::optional<RationalWithDelta> upper;
	};
	struct Variable
	{
#ifdef DEBUG
		std::string name = {};
#endif
		RationalWithDelta value = {};
		Bounds bounds = {};
		std::optional<size_t> lowerReason;
		std::optional<size_t> upperReason;
	};

	/// Consumes a constraint and returns a controlling variable (can be a new slack
	/// but does not need to) and corresponding bounds.
	/// If it adds a slack variable, updates the factors and properly sets the value
	/// for the slack variable (which will be a new basic variable).
	std::pair<size_t, Bounds> constraintIntoVariableBounds(Constraint const& _constraint);
	void addBounds(size_t _variable, Bounds _bounds);
	std::set<size_t> collectReasonsForVariable(size_t _variable);

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

	void addOuterVariable(size_t _outerIndex);
	/// Adds a new outer variable if it is not known yet and returns the inner index in any case.
	size_t maybeAddOuterVariable(size_t _outerIndex);
	size_t addNewVariable();

	/// Counter to enable unique names for the slack variables.
	size_t m_slackVariableCounter = 0;
	std::optional<LPResult> result = std::nullopt;
	size_t trailSize = 0;
	SparseMatrix factors;
	std::vector<Variable> variables;
	/// Stack of (trail size, variable index, bounds, lower reason, upper reason).
	std::vector<std::tuple<size_t, size_t, Bounds, std::optional<size_t>, std::optional<size_t>>> storedBounds;
	/// Last known satisfying values for variables.
	std::vector<RationalWithDelta> previousGoodValues;
	std::set<size_t> variablesPotentiallyOutOfBounds;
	/// Variable index to constraint it controls.
	std::map<size_t, size_t> basicVariables;
	/// Maps outer indices to inner indices.
	std::map<size_t, size_t> varMapping = {};
	/// Mapping from reason (constraint ID) to variable it controls and bounds for it.
	/// A variable can be controlled by multiple constraints.
	/// TODO do we want to store the reverse mapping?
	std::map<size_t, std::pair<size_t, Bounds>> reasonToBounds;
	std::set<size_t> reasons;


};

}
