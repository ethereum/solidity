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
#include <test/tools/yulInterpreter/Interpreter.h>
#include <libyul/backends/evm/EVMDialect.h>

namespace solidity::yul::test::yul_fuzzer
{

struct yulFuzzerUtil
{
	enum class TerminationReason
	{
		ExplicitlyTerminated,
		StepLimitReached,
		TraceLimitReached,
		ExpresionNestingLimitReached,
		None
	};

	/// Interprets the Yul AST pointed to by @param _ast. Flag @param _outputStorageOnly
	/// (unset by default) outputs an execution trace of both memory and storage;
	/// if set, only storage contents are output as part of the execution trace. The
	/// latter avoids false positives that will be produced by the fuzzer when certain
	/// optimizer steps are activated e.g., Redundant store eliminator, Equal store
	/// eliminator.
	static TerminationReason interpret(
		std::ostream& _os,
		yul::Block const& _astRoot,
		Dialect const& _dialect,
		bool _disableMemoryTracing = false,
		bool _outputStorageOnly = false,
		size_t _maxSteps = maxSteps,
		size_t _maxTraceSize = maxTraceSize,
		size_t _maxExprNesting = maxExprNesting
	);

	/// @returns true if @param _reason for Yul interpreter terminating is
	/// resource exhaustion of some form e.g., exceeded maximum time-out
	/// threshold, number of nested expressions etc.
	static bool resourceLimitsExceeded(TerminationReason _reason);
	static size_t constexpr maxSteps = 100;
	static size_t constexpr maxTraceSize = 75;
	static size_t constexpr maxExprNesting = 64;
};

}
