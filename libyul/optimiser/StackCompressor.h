// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that aggressively rematerializes certain variables ina a function to free
 * space on the stack until it is compilable.
 */

#pragma once

#include <memory>

namespace solidity::yul
{

struct Dialect;
struct Object;
struct FunctionDefinition;

/**
 * Optimisation stage that aggressively rematerializes certain variables in a function to free
 * space on the stack until it is compilable.
 *
 * Only runs on the code of the object itself, does not descend into sub-objects.
 *
 * Prerequisite: Disambiguator, Function Grouper
 */
class StackCompressor
{
public:
	/// Try to remove local variables until the AST is compilable.
	/// @returns true if it was successful.
	static bool run(
		Dialect const& _dialect,
		Object& _object,
		bool _optimizeStackAllocation,
		size_t _maxIterations
	);
};

}
