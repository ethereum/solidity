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
	/// Names of variables, the index zero should be left empty
	/// (because zero corresponds to constants).
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

	bool operator<(SolvingState const& _other) const;
	bool operator==(SolvingState const& _other) const;
	std::string toString() const;
};

enum class LPResult
{
	Unknown,
	Unbounded,
	Feasible,
	Infeasible
};

/**
 * LP solver for rational problems.
 *
 * Does not solve integer problems!
 *
 * Tries to split a given problem into sub-problems and utilizes a cache to quickly solve
 * similar problems.
 */
class LPSolver
{
public:
	std::pair<LPResult, std::map<std::string, boost::rational<bigint>>> check(SolvingState _state);

private:
	// TODO check if the model is requested in production. If not, we do not need to cache it.
	std::map<SolvingState, std::pair<LPResult, std::vector<boost::rational<bigint>>>> m_cache;
};

}
