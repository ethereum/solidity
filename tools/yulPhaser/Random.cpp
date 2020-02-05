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

#include <tools/yulPhaser/Random.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>

#include <ctime>

using namespace solidity;

uint32_t phaser::uniformRandomInt(uint32_t _min, uint32_t _max)
{
	// TODO: Seed must be configurable
	static boost::random::mt19937 generator(time(0));
	boost::random::uniform_int_distribution<> distribution(_min, _max);

	return distribution(generator);
}

uint32_t phaser::binomialRandomInt(uint32_t _numTrials, double _successProbability)
{
	// TODO: Seed must be configurable
	static boost::random::mt19937 generator(time(0));
	boost::random::binomial_distribution<> distribution(_numTrials, _successProbability);

	return distribution(generator);
}
