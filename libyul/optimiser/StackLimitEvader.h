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
 * Optimisation stage that assigns memory offsets to variables that would become unreachable if
 * assigned a stack slot as usual and replaces references and assignments to them by mload and mstore calls.
 */

#pragma once

#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/backends/evm/StackLayoutGenerator.h>

namespace solidity::yul
{

struct Object;

/**
 * Optimisation stage that assigns memory offsets to variables that would become unreachable if
 * assigned a stack slot as usual.
 *
 * Uses CompilabilityChecker to determine which variables in which functions are unreachable.
 *
 * Only variables outside of functions contained in cycles in the call graph are considered. Thereby it is possible
 * to assign globally fixed memory offsets to the variable. If a variable in a function contained in a cycle in the
 * call graph is reported as unreachable, the process is aborted.
 *
 * Offsets are assigned to the variables, s.t. on every path through the call graph each variable gets a unique offset
 * in memory. However, distinct paths through the call graph can use the same memory offsets for their variables.
 *
 * The current arguments to the ``memoryguard`` calls are used as base memory offset and then replaced by the offset past
 * the last memory offset used for a variable on any path through the call graph.
 *
 * Finally, the StackToMemoryMover is called to actually move the variables to their offsets in memory.
 *
 * Prerequisite: Disambiguator
 */
class StackLimitEvader
{
public:
	/// @a _unreachableVariables can be determined by the CompilabilityChecker.
	/// Can only be run on the EVM dialect with objects.
	/// Abort and do nothing, if no ``memoryguard`` call or several ``memoryguard`` calls
	/// with non-matching arguments are found, or if any of the @a _unreachableVariables
	/// are contained in a recursive function.
	static void run(
		OptimiserStepContext& _context,
		Object& _object,
		std::map<YulString, std::set<YulString>> const& _unreachableVariables
	);
	/// @a _stackTooDeepErrors can be determined by the StackLayoutGenerator.
	/// Can only be run on the EVM dialect with objects.
	/// Abort and do nothing, if no ``memoryguard`` call or several ``memoryguard`` calls
	/// with non-matching arguments are found, or if any of the @a _stackTooDeepErrors
	/// are contained in a recursive function.
	static void run(
		OptimiserStepContext& _context,
		Object& _object,
		std::map<YulString, std::vector<StackLayoutGenerator::StackTooDeep>> const& _stackTooDeepErrors
	);
	/// Determines stack too deep errors using the appropriate code generation backend.
	/// Can only be run on the EVM dialect with objects.
	/// Abort and do nothing, if no ``memoryguard`` call or several ``memoryguard`` calls
	/// with non-matching arguments are found, or if any of the unreachable variables
	/// are contained in a recursive function.
	static void run(
		OptimiserStepContext& _context,
		Object& _object
	);
};

}
