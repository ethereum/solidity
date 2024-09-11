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
#include <test/tools/ossfuzz/yulFuzzerCommon.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test::yul_fuzzer;

yulFuzzerUtil::TerminationReason yulFuzzerUtil::interpret(
	std::ostream& _os,
	yul::Block const& _astRoot,
	Dialect const& _dialect,
	bool _disableMemoryTracing,
	bool _outputStorageOnly,
	size_t _maxSteps,
	size_t _maxTraceSize,
	size_t _maxExprNesting
)
{
	InterpreterState state;
	state.maxTraceSize = _maxTraceSize;
	state.maxSteps = _maxSteps;
	state.maxExprNesting = _maxExprNesting;
	// Add 64 bytes of pseudo-randomly generated calldata so that
	// calldata opcodes perform non trivial work.
	state.calldata = {
		0xe9, 0x96, 0x40, 0x7d, 0xa5, 0xda, 0xb0, 0x2d,
		0x97, 0xf5, 0xc3, 0x44, 0xd7, 0x65, 0x0a, 0xd8,
		0x2c, 0x14, 0x3a, 0xf3, 0xe7, 0x40, 0x0f, 0x1e,
		0x67, 0xce, 0x90, 0x44, 0x2e, 0x92, 0xdb, 0x88,
		0xb8, 0x43, 0x9c, 0x41, 0x42, 0x08, 0xf1, 0xd7,
		0x65, 0xe9, 0x7f, 0xeb, 0x7b, 0xb9, 0x56, 0x9f,
		0xc7, 0x60, 0x5f, 0x7c, 0xcd, 0xfb, 0x92, 0xcd,
		0x8e, 0xf3, 0x9b, 0xe4, 0x4f, 0x6c, 0x14, 0xde
	};

	TerminationReason reason = TerminationReason::None;
	try
	{
		Interpreter::run(state, _dialect, _astRoot, true, _disableMemoryTracing);
	}
	catch (StepLimitReached const&)
	{
		reason = TerminationReason::StepLimitReached;
	}
	catch (TraceLimitReached const&)
	{
		reason = TerminationReason::TraceLimitReached;
	}
	catch (ExpressionNestingLimitReached const&)
	{
		reason = TerminationReason::ExpresionNestingLimitReached;
	}
	catch (ExplicitlyTerminated const&)
	{
		reason = TerminationReason::ExplicitlyTerminated;
	}

	if (_outputStorageOnly)
		state.dumpStorage(_os);
	else
		state.dumpTraceAndState(_os, _disableMemoryTracing);
	return reason;
}

bool yulFuzzerUtil::resourceLimitsExceeded(TerminationReason _reason)
{
	return
		_reason == yulFuzzerUtil::TerminationReason::StepLimitReached ||
		_reason == yulFuzzerUtil::TerminationReason::TraceLimitReached ||
		_reason == yulFuzzerUtil::TerminationReason::ExpresionNestingLimitReached;
}
