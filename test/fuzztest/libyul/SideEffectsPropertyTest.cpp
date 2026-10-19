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

/// Property test for the soundness of the SideEffectsCollector and the ControlFlowSideEffectsCollector
///
/// What the collectors claim about a function call (it cannot revert, does not write storage, is movable, ...)
/// must hold whenever the call is evaluated. The test generates a program and executes it in a Yul interpreter
/// that checks the claims about every call it evaluates, see SideEffectsAuditor.h.
///
/// The programs come from YulProgramGenerator.h, which also lists what they do not cover. They are generated
/// with ProgramProfile::Calls, so that most of them call functions the collectors make claims about.
///
/// Limitations:
///   - claims are only checked on the paths that execute, for four calldata inputs per program
///   - claims about transient storage and other state, and that a call cannot loop, are not checked
///   - no optimizer step runs on the programs first, other than the FunctionHoister, which the
///     ControlFlowSideEffectsCollector needs

#include <test/fuzztest/libyul/OptimizerStepSequence.h>
#include <test/fuzztest/libyul/YulProgramGenerator.h>

#include <test/tools/yulInterpreter/Interpreter.h>
#include <test/tools/yulInterpreter/SideEffectsAuditor.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Object.h>
#include <libyul/YulStack.h>

#include <libsolutil/CommonData.h>

#include <boost/exception/diagnostic_information.hpp>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using namespace solidity;
using namespace solidity::yul;

namespace solidity::yul::test
{

namespace
{

std::size_t constexpr runsPerProgram = 4;
std::size_t constexpr maxInterpreterSteps = 2000000;

struct Statistics: testing::Environment
{
	std::size_t programs = 0;
	std::size_t runs = 0;
	std::map<Claim, std::size_t> checks;
	std::map<Claim, std::size_t> checksInvolvingUserFunctions;
	GeneratorStatistics generator;

	void TearDown() override
	{
		generator.print();
		if (runs == 0)
			return;
		fmt::print(stderr, "\n== side effects check statistics ==\n");
		fmt::print(stderr, "programs: {}, runs: {}\n\n", programs, runs);
		fmt::print(
			stderr,
			"{:<28}{:>12}{:>10}{:>32}{:>10}\n",
			"claim",
			"checks",
			"per run",
			"involving a user function",
			"per run"
		);
		for (auto const& [claim, count]: checks)
		{
			std::size_t const involvingUserFunctions =
				checksInvolvingUserFunctions.contains(claim) ? checksInvolvingUserFunctions.at(claim) : 0;
			fmt::print(
				stderr,
				"{:<28}{:>12}{:>10.1f}{:>32}{:>10.2f}\n",
				claim,
				count,
				static_cast<double>(count) / static_cast<double>(runs),
				involvingUserFunctions,
				static_cast<double>(involvingUserFunctions) / static_cast<double>(runs)
			);
		}
	}
};

Statistics& statistics = *static_cast<Statistics*>(testing::AddGlobalTestEnvironment(new Statistics));

/// Asserts that all claims about the calls of @a _source hold when executing it on each of @a _calldataWordsPerRun, given
/// as one value per word.
/// Adds to @a _checksInvolvingUserFunctions how often each kind of claim was checked on a call that involves a
/// user-defined function.
void checkClaimsHold(
	std::string const& _source,
	std::vector<CalldataWords> const& _calldataWordsPerRun,
	std::map<Claim, std::size_t>& _checksInvolvingUserFunctions
)
{
	YulStack stack;
	ASSERT_TRUE(stack.parseAndAnalyze("source", _source)) << _source;
	std::shared_ptr<Object const> const object = stack.parserResult();
	Block const ast = runSteps(*object, "h");
	++statistics.programs;

	for (CalldataWords const& calldataWords: _calldataWordsPerRun)
	{
		InterpreterState state;
		state.calldata = toCalldata(calldataWords);
		state.maxSteps = maxInterpreterSteps;
		state.maxTraceSize = maxInterpreterSteps;
		SideEffectsAudit audit;
		try
		{
			audit = auditSideEffects(state, *object->dialect(), ast);
		}
		catch (InterpreterTerminatedGeneric const&)
		{
			FAIL() << fmt::format(
				"The interpreter hit a limit before all claims were checked, maxInterpreterSteps may need raising.\n"
				"source:\n{}",
				_source
			);
		}
		++statistics.runs;
		for (auto const& [claim, count]: audit.checks)
			statistics.checks[claim] += count;
		for (auto const& [claim, count]: audit.checksInvolvingUserFunctions)
		{
			_checksInvolvingUserFunctions[claim] += count;
			statistics.checksInvolvingUserFunctions[claim] += count;
		}
		ASSERT_FALSE(audit.violation.has_value()) << fmt::format(
			"A claim about the side effects of a call does not hold at runtime: {}\n"
			"calldata: {}\n"
			"source:\n{}",
			*audit.violation,
			util::toHex(state.calldata),
			_source
		);
	}
}

struct FuzzInput
{
	std::vector<Instruction> instructions;
	std::vector<CalldataWords> calldataWordsPerRun;
};

auto inputDomain()
{
	return fuzztest::StructOf<FuzzInput>(
		instructionsDomain(ProgramProfile::Calls),
		fuzztest::VectorOf(calldataWordsDomain()).WithSize(runsPerProgram)
	);
}

}

static void SideEffectClaimsHoldAtRuntime(FuzzInput const& _input)
{
	std::string const source = buildProgram(_input.instructions, ProgramProfile::Calls, &statistics.generator);
	std::map<Claim, std::size_t> checksInvolvingUserFunctions;
	try
	{
		checkClaimsHold(source, _input.calldataWordsPerRun, checksInvolvingUserFunctions);
	}
	catch (...)
	{
		FAIL() << fmt::format(
			"Exception: {}\ncalldata words: {}\nsource:\n{}",
			boost::current_exception_diagnostic_information(),
			_input.calldataWordsPerRun,
			source
		);
	}
}

FUZZ_TEST(SideEffectsProperty, SideEffectClaimsHoldAtRuntime)
	.WithDomains(inputDomain());

/// Every kind of claim gets checked on a user-defined function, so the property test cannot pass vacuously.
TEST(SideEffectsProperty, EveryKindOfClaimIsChecked)
{
	std::string const source = R"({
		function pure(a) -> r { r := add(a, 1) }
		function reader(a) -> r { r := add(sload(a), mload(a)) }
		function writer(a) -> r { sstore(a, 1) mstore(a, 2) }
		function fail() { revert(0, 0) }
		let x := pure(calldataload(0))
		let y := reader(x)
		let z := writer(y)
		if calldataload(32) { fail() }
		return(0, 32)
	})";
	// calldataload(32) is nonzero, so the call that cannot continue is evaluated.
	std::map<Claim, std::size_t> checks;
	ASSERT_NO_FATAL_FAILURE(checkClaimsHold(source, {CalldataWords{{1, 1}}}, checks));
	for (Claim const claim: allClaims)
		EXPECT_TRUE(checks.contains(claim)) << format_as(claim);
}

}
