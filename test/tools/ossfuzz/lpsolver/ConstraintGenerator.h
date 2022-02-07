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
/*
 * Generates an integer matrix of dimension n x m
 */

#pragma once

#include <limits>
#include <memory>
#include <random>
#include <string>

namespace solidity::test::fuzzer::lpsolver
{

using RandomEngine = std::mt19937;
using Distribution = std::uniform_int_distribution<int>;
using Bernoulli = std::bernoulli_distribution;

struct ConstraintGenerator
{
	explicit ConstraintGenerator(unsigned int _seed);

	/// @returns generated constraint.
	std::string generate();

	/// @returns random number of factors.
	int numFactors()
	{
		return Distribution(s_minFactors, s_maxFactors)(*prng);
	}

	/// @returns random number of constraints.
	int numConstraints()
	{
		return Distribution(s_minConstraints, s_maxConstraints)(*prng);
	}

	/// @returns an integer chosen uniformly at random.
	int randomInteger()
	{
		return Distribution(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())(*prng);
	}

	/// @returns an integer in the range [-1, 1] chosen uniformly at random.
	int randomMinusOneToOne()
	{
		return Distribution(-1, 1)(*prng);
	}

	/// @returns zero or one with a probability of occurance of 0.5 each.
	int zeroOrOne()
	{
		return Distribution(0, 1)(*prng);
	}

	/// @returns true with a probability @param _p, false otherwise.
	bool bernoulliDist(double _truthProbability)
	{
		return Bernoulli(_truthProbability)(*prng);
	}


	std::shared_ptr<RandomEngine> prng;

	static constexpr int s_minFactors = 2;
	static constexpr int s_maxFactors = 10;
	static constexpr int s_minConstraints = 1;
	static constexpr int s_maxConstraints = 10;
	static constexpr double s_piecewiseConstantProb = 0.25;
};

}
