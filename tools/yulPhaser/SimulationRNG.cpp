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

#include <tools/yulPhaser/SimulationRNG.h>

#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <ctime>

using namespace solidity;
using namespace solidity::phaser;

thread_local boost::random::mt19937 SimulationRNG::s_generator(SimulationRNG::generateSeed());

bool SimulationRNG::bernoulliTrial(double _successProbability)
{
	boost::random::bernoulli_distribution<> distribution(_successProbability);

	return static_cast<bool>(distribution(s_generator));
}

uint32_t SimulationRNG::uniformInt(uint32_t _min, uint32_t _max)
{
	boost::random::uniform_int_distribution<> distribution(_min, _max);
	return distribution(s_generator);
}

uint32_t SimulationRNG::binomialInt(uint32_t _numTrials, double _successProbability)
{
	boost::random::binomial_distribution<> distribution(_numTrials, _successProbability);
	return distribution(s_generator);
}

uint32_t SimulationRNG::generateSeed()
{
	// This is not a secure way to seed the generator but it's good enough for simulation purposes.
	// The only thing that matters for us is that the sequence is different on each run and that
	// it fits the expected distribution. It does not have to be 100% unpredictable.
	return time(nullptr);
}
