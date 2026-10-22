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

/// Property test for the soundness of the DataFlowAnalyzer and its KnowledgeBase
///
/// Every fact the analyzer claims before a statement (known storage and memory contents, variable values,
/// constant offsets and disequalities between locations) must hold whenever execution reaches that statement.
/// The test generates a program, runs a random sequence of optimizer steps on it, turns every fact claimed
/// about the result into a runtime check and executes that in the Yul interpreter. Checking the claims directly,
/// instead of comparing optimized against unoptimized behaviour, catches unsound facts even if no optimizer step
/// happens to exploit them in an observable way.
///
/// The programs come from YulProgramGenerator.h, which also lists what they do not cover.
///
/// Limitations of the check:
///   - facts are only checked on the paths that execute, for four calldata inputs per program
///   - facts mentioning a variable that is out of scope are not checked
///   - facts about two variables (`a - b` is constant, `a != b`, `keccak256(a, b)` is known) are only checked if
///     both are an argument of a storage, memory or `keccak256` builtin somewhere in the program

#include <test/fuzztest/libyul/OptimizerStepSequence.h>
#include <test/fuzztest/libyul/YulProgramGenerator.h>

#include <test/tools/yulFactAuditor/FactAuditor.h>
#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/AST.h>
#include <libyul/AsmPrinter.h>
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
#include <string_view>
#include <utility>
#include <vector>

using namespace solidity;
using namespace solidity::yul;

namespace solidity::yul::test
{

namespace
{

std::size_t constexpr maxRandomSteps = 12;
std::size_t constexpr runsPerProgram = 4;
std::size_t constexpr maxInterpreterSteps = 2000000;

/// The steps the analyzer itself needs, in the order they have to run in
std::string_view constexpr analyzerPrerequisites = "ho";

/// One of these sequences runs before the random steps, to mimic the default sequence:
///   x = ExpressionSplitter
///   a = SSATransform
///   c = CommonSubexpressionEliminator
///   s = ExpressionSimplifier
std::vector<std::string> const preparationSequences{"", "x", "xa", "xc", "xcs", "xac", "xacs"};

struct Statistics: testing::Environment
{
	std::size_t checkedPrograms = 0;
	std::size_t facts = 0;
	std::size_t randomSteps = 0;
	std::map<std::string, std::size_t> preparationSteps;
	GeneratorStatistics generator;

	void TearDown() override
	{
		generator.print();
		if (checkedPrograms == 0)
			return;
		auto const perProgram = [&](std::size_t const _count) {
			return static_cast<double>(_count) / static_cast<double>(checkedPrograms);
		};
		fmt::print(stderr, "\n== fact check statistics ==\n");
		fmt::print(stderr, "checked programs: {}\n", checkedPrograms);
		fmt::print(
			stderr,
			"per program: {:.1f} facts, {:.1f} random steps\n",
			perProgram(facts),
			perProgram(randomSteps)
		);
		fmt::print(stderr, "preparation steps:");
		for (auto const& [steps, count]: preparationSteps)
			fmt::print(stderr, " \"{}\": {:.1f}%", steps, 100 * perProgram(count));
		fmt::print(stderr, "\n");
	}
};

Statistics& statistics = *static_cast<Statistics*>(testing::AddGlobalTestEnvironment(new Statistics));

/// @returns the final state, or nothing if a limit of the interpreter cut the run short.
std::optional<InterpreterState> interpret(AST const& _ast, bytes const& _calldata)
{
	InterpreterState state;
	state.calldata = _calldata;
	state.maxSteps = maxInterpreterSteps;
	state.maxTraceSize = maxInterpreterSteps;
	try
	{
		Interpreter::run(state, _ast, /*disableExternalCalls=*/true, /*disableMemoryTracing=*/true);
	}
	catch (ExplicitlyTerminated const&)
	{
		// Ended by the program itself, e.g. by its return or by the stop of a failed check.
	}
	catch (InterpreterTerminatedGeneric const&)
	{
		return std::nullopt;
	}
	return state;
}

/// Runs @a _steps on @a _source and asserts that all facts the analyzer claims about the result hold when
/// executing it on each of @a _calldataWordsPerRun, given as one value per word.
void checkFactsHold(
	std::string const& _source,
	std::string const& _steps,
	std::vector<CalldataWords> const& _calldataWordsPerRun
)
{
	YulStack stack;
	ASSERT_TRUE(stack.parseAndAnalyze("source", _source)) << _source;
	std::shared_ptr<Object const> const object = stack.parserResult();
	ASSERT_TRUE(object && object->hasCode() && object->analysisInfo) << _source;
	Dialect const& dialect = *object->dialect();

	Block ast = runSteps(*object, _steps);

	std::string const analyzedCode = AsmPrinter{dialect}(ast);
	std::vector<std::string> const facts = instrumentWithFactChecks(dialect, ast);
	++statistics.checkedPrograms;
	statistics.facts += facts.size();
	AST const instrumented(dialect, std::move(ast));

	for (CalldataWords const& calldataWords: _calldataWordsPerRun)
	{
		bytes const calldata = toCalldata(calldataWords);
		std::optional<InterpreterState> const state = interpret(instrumented, calldata);
		ASSERT_TRUE(state.has_value()) << fmt::format(
			"The interpreter hit a limit before all facts were checked, maxInterpreterSteps may need raising.\n"
			"steps: {}\n"
			"source:\n{}",
			_steps,
			_source
		);
		std::optional<std::size_t> const violated = violatedFactIndex(state->storage);
		ASSERT_FALSE(violated.has_value()) << fmt::format(
			"The analyzer claims a fact that does not hold at runtime: {}\n"
			"fact number: {}\n"
			"calldata: {}\n"
			"steps: {}\n"
			"code the analyzer ran on:\n{}\n"
			"source:\n{}",
			facts.at(*violated),
			*violated + 1,
			util::toHex(calldata),
			_steps,
			analyzedCode,
			_source
		);
	}
}

/// Prepends only those prerequisites that @a _steps need, so that the other steps also get to run on code that
/// is not hoisted, not grouped or has loop initializers. The analyzerPrerequisites no step needs run last.
std::string withPrerequisites(std::string const& _steps)
{
	std::string const needed = prerequisitesOf(_steps);
	std::string sequence = needed + _steps;
	for (char const prerequisite: analyzerPrerequisites)
		if (needed.find(prerequisite) == std::string::npos)
			sequence += prerequisite;
	return sequence;
}

struct FuzzInput
{
	std::vector<Instruction> instructions;
	std::vector<CalldataWords> calldataWordsPerRun;
	std::string preparationSteps;
	std::string randomSteps;
};

auto inputDomain()
{
	return fuzztest::StructOf<FuzzInput>(
		instructionsDomain(),
		fuzztest::VectorOf(calldataWordsDomain()).WithSize(runsPerProgram),
		fuzztest::ElementOf(preparationSequences),
		stepsDomain(maxRandomSteps)
	);
}

}

static void AnalyzerFactsHoldAtRuntime(FuzzInput const& _input)
{
	std::string const source = buildProgram(_input.instructions, ProgramProfile::Aliasing, &statistics.generator);
	std::string const steps = withPrerequisites(_input.preparationSteps + _input.randomSteps);
	++statistics.preparationSteps[_input.preparationSteps];
	statistics.randomSteps += _input.randomSteps.size();
	try
	{
		checkFactsHold(source, steps, _input.calldataWordsPerRun);
	}
	catch (...)
	{
		FAIL() << fmt::format(
			"Exception: {}\nsteps: {}\ncalldata words: {}\nsource:\n{}",
			boost::current_exception_diagnostic_information(),
			steps,
			_input.calldataWordsPerRun,
			source
		);
	}
}

FUZZ_TEST(DataFlowAnalyzerProperty, AnalyzerFactsHoldAtRuntime)
	.WithDomains(inputDomain());

}
