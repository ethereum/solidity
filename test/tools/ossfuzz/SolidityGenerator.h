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
/**
 * Implements generators for synthesizing mostly syntactically valid
 * Solidity test programs.
 */

#pragma once

#include <random>

namespace solidity::test::fuzzer
{
using RandomEngine = std::mt19937_64;

class SolidityGenerator
{
public:
	SolidityGenerator(uint64_t _seed): m_rand(_seed)
	{}
	/// @returns a pseudo randomly generated test program
	std::string generateTestProgram();
private:
	/// Random number generator
	RandomEngine const m_rand;
};
}
