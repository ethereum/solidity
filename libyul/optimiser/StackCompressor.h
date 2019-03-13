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
/**
 * Optimisation stage that aggressively rematerializes certain variables ina a function to free
 * space on the stack until it is compilable.
 */

#pragma once

#include <memory>

namespace yul
{

struct Dialect;
struct Block;
struct FunctionDefinition;

/**
 * Optimisation stage that aggressively rematerializes certain variables in a function to free
 * space on the stack until it is compilable.
 *
 * Prerequisite: Disambiguator, Function Grouper
 */
class StackCompressor
{
public:
	/// Try to remove local variables until the AST is compilable.
	/// @returns true if it was successful.
	static bool run(std::shared_ptr<Dialect> const& _dialect, Block& _ast, bool _optimizeStackAllocation);
};

}
