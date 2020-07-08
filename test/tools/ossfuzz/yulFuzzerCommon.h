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
		None
	};

	static TerminationReason interpret(
		std::ostream& _os,
		std::shared_ptr<yul::Block> _ast,
		Dialect const& _dialect,
		size_t _maxSteps = maxSteps,
		size_t _maxTraceSize = maxTraceSize
	);
	static size_t constexpr maxSteps = 100;
	static size_t constexpr maxTraceSize = 75;
};

}
