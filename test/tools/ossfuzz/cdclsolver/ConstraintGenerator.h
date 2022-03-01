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
 * Generates constraints for the CDCL solver.
 */

#pragma once

#include <limits>
#include <memory>
#include <random>
#include <string>

namespace solidity::test::fuzzer::cdclsolver
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
		return Distribution(s_minNumFactors, s_maxNumFactors)(*prng);
	}

	/// @returns random number of constraints.
	int numConstraints()
	{
		return Distribution(s_minConstraints, s_maxConstraints)(*prng);
	}

	/// @returns an integer chosen uniformly at random.
	int randomInteger()
	{
		return Distribution(s_minFactor, s_maxFactor)(*prng);
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

	/// Smallest number of factors in linear constraint of the form
	/// a*x1 + b*x2 <= c
	static constexpr int s_minNumFactors = 2;
	/// Largest number of factors in linear constraint
	static constexpr int s_maxNumFactors = 100;
	/// Smallest number of linear constraints
	static constexpr int s_minConstraints = 1;
	/// Largest number of linear constraints
	static constexpr int s_maxConstraints = 100;
	/// Smallest value of a factor in linear constraint
	static constexpr int s_minFactor = -100;
	/// Largest value of a factor in linear constraint
	static constexpr int s_maxFactor = 100;
	/// Probability that a factor in the range of [-1, 1] is chosen
	static constexpr double s_piecewiseConstantProb = 0.75;
};
}
