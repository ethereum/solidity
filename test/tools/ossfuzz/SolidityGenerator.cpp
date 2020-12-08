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

#include <test/tools/ossfuzz/SolidityGenerator.h>

#include <libsolutil/Whiskers.h>

using namespace solidity::test::fuzzer;
using namespace solidity::util;
using namespace std;

string SolidityGenerator::generateTestProgram()
{
	// TODO: Add generators for grammar elements of
	// Solidity antlr4 grammar. Currently, the generated
	// test program consists of a version pragma only.
	return Whiskers(R"(pragma <directive>;)")
		("directive", "solidity >= 0.0.0")
		.render();
}
