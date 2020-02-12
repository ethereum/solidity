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
 * The numbers are not cryptographically secure so do not use this for anything that requires
 * them to be truly unpredictable.
 */
class SimulationRNG
{
public:
	static uint32_t uniformInt(uint32_t _min, uint32_t _max);
	static uint32_t binomialInt(uint32_t _numTrials, double _successProbability);
};

}
