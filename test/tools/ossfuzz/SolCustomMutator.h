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

#include <test/tools/ossfuzz/SolidityLexer.h>
#include <test/tools/ossfuzz/SolidityMutator.h>
#include <test/tools/ossfuzz/SolidityParser.h>

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <random>

namespace solidity::test::fuzzer
{
struct SolCustomMutator
{
	SolCustomMutator(uint8_t* _data, size_t _size, size_t _maxSize, unsigned _seed);
	size_t mutate();

	uint8_t* Data = nullptr;
	size_t Size = 0;
	std::string In{};
	std::string Out{};
	size_t MaxMutantSize = 0;
	unsigned int Seed = 0;
};
}