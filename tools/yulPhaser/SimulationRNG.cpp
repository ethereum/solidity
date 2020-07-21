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

#include <tools/yulPhaser/SimulationRNG.h>

// NOTE: The code would work with std::random but the results for a given seed would not be reproducible
// across different STL implementations. Boost does not guarantee this either but at least it has only one
// implementation. Reproducibility is not a hard requirement for yul-phaser but it's nice to have.
#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <ctime>
#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::phaser;

thread_local boost::random::mt19937 SimulationRNG::s_generator(SimulationRNG::generateSeed());

bool SimulationRNG::bernoulliTrial(double _successProbability)
{
	boost::random::bernoulli_distribution<double> distribution(_successProbability);

	return distribution(s_generator);
}

size_t SimulationRNG::uniformInt(size_t _min, size_t _max)
{
	boost::random::uniform_int_distribution<size_t> distribution(_min, _max);
	return distribution(s_generator);
}

size_t SimulationRNG::binomialInt(size_t _numTrials, double _successProbability)
{
	// NOTE: binomial_distribution<size_t> would not work because it internally tries to use abs()
	// and fails to compile due to ambiguous conversion.
	assert(_numTrials <= static_cast<size_t>(numeric_limits<long>::max()));

	boost::random::binomial_distribution<long> distribution(static_cast<long>(_numTrials), _successProbability);
	return static_cast<size_t>(distribution(s_generator));
}

uint32_t SimulationRNG::generateSeed()
{
	// This is not a secure way to seed the generator but it's good enough for simulation purposes.
	// The only thing that matters for us is that the sequence is different on each run and that
	// it fits the expected distribution. It does not have to be 100% unpredictable.
	return time(nullptr);
}
