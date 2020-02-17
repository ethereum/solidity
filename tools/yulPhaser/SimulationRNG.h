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

#pragma once

#include <boost/random/mersenne_twister.hpp>

#include <cstdint>

namespace solidity::phaser
{

/**
 * A class that provides functions for generating random numbers good enough for simulation purposes.
 *
 * The functions share a common instance of the generator which can be reset with a known seed
 * to deterministically generate a given sequence of numbers. Initially the generator is seeded with
 * a value from @a generateSeed() which is different on each run.
 *
 * The numbers are not cryptographically secure so do not use this for anything that requires
 * them to be truly unpredictable.
 */
class SimulationRNG
{
public:
	static bool bernoulliTrial(double _successProbability);
	static uint32_t uniformInt(uint32_t _min, uint32_t _max);
	static uint32_t binomialInt(uint32_t _numTrials, double _successProbability);

	/// Resets generator to a known state given by the @a seed. Given the same seed, a fixed
	/// sequence of calls to the members generating random values is guaranteed to produce the
	/// same results.
	static void reset(uint32_t seed) { s_generator = boost::random::mt19937(seed); }

	/// Generates a seed that's different on each run of the program.
	/// Does **not** use the generator and is not affected by @a reset().
	static uint32_t generateSeed();

private:
	thread_local static boost::random::mt19937 s_generator;
};

}
