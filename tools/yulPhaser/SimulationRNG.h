// SPDX-License-Identifier: GPL-3.0

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
	static size_t uniformInt(size_t _min, size_t _max);
	static size_t binomialInt(size_t _numTrials, double _successProbability);

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
