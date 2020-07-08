// SPDX-License-Identifier: GPL-3.0
/**
 * Component that checks whether all variables are reachable on the stack.
 */

#pragma once

#include <libyul/Dialect.h>
#include <libyul/AsmDataForward.h>
#include <libyul/Object.h>

#include <map>
#include <memory>

namespace solidity::yul
{

/**
 * Component that checks whether all variables are reachable on the stack and
 * returns a mapping from function name to the largest stack difference found
 * in that function (no entry present if that function is compilable).
 *
 * This only works properly if the outermost block is compilable and
 * functions are not nested. Otherwise, it might miss reporting some functions.
 *
 * Only checks the code of the object itself, does not descend into sub-objects.
 */
class CompilabilityChecker
{
public:
	static std::map<YulString, int> run(
		Dialect const& _dialect,
		Object const& _object,
		bool _optimizeStackAllocation
	);
};

}
