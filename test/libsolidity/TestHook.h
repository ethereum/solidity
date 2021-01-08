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

#pragma once

#include <test/libsolidity/util/SoltestErrors.h>

#include <memory>
#include <vector>

namespace solidity::frontend::test
{

class TestFunctionCall;

/**
 * The `TestHook` class can be used to implement checks for semantic tests.
 *
 * A `TestHook` normally gets registered in `test/libsolidity/SemanticTest.cpp`.
 * If instantiated there, they have the same live-time as the SemanticTest instance.
 *
 * NOTE: A single `SemanticTest` instance is used per `*.sol` file in the semantic tests.
 *       A test-case defined by `SemanticTest` may have multiple test-runs, e.g. Yul and/or Ewasm test-runs.
 *
 * Pseudo-code how `TestHooks` are used during test-execution
 * (see test/libsolidity/SemanticTest.cpp, SemanticTest::run & SemanticTest::runTest):
 *
 *  // Multiple test-runs are possible.
 *  // Additional tests_runs are needed for yul and/or ewasm.
 *  foreach test in test_runs:
 *      success = true
 *      foreach hook in test.hooks:
 *          hook->beginTestCase();
 *
 *      foreach call in test.calls:
 *          foreach hook in test.hooks:
 *              hook->beforeFunctionCall(call);
 *          call();
 *          foreach hook in test.hooks:
 *              hook->afterFunctionCall(call);
 *
 *       foreach hook in test.hooks:
 *          hook->endTestCase();
 *
 *      foreach hook in tests.hook:
 *          foreach call in test.calls:
 *              success &= hook->verifyFunctionCall(call);
 *
 *      // printing errors/warnings & update expectations
 *      if !success:
 *          foreach hook in tests.hooks:
 *              foreach call in tests.calls:
 *                  print(hook->formatFunctionCall(call));
 */
class TestHook
{
public:
	TestHook() = default;
	virtual ~TestHook() = default;

	/// If the test-case consists multiple test-runs (e.g. yul and/or ewasm), this function is called per test-run.
	/// This function is called at the begin of each test-run.
	virtual void beginTestCase() = 0;

	/// This function gets called, before the actual `TestFunctionCall` got executed.
	/// @param _call current `TestFunctionCall` defined in the test-case.
	virtual void beforeFunctionCall(TestFunctionCall const& _call) = 0;

	/// This function gets called, after the actual `TestFunctionCall` got executed.
	/// @param _call current `TestFunctionCall` defined in the test-case.
	virtual void afterFunctionCall(TestFunctionCall const& _call) = 0;

	/// If the test-case consists multiple test-runs (e.g. yul and/or ewasm), this function is called per test-run.
	/// This function is called at the end of each test-run.
	virtual void endTestCase() = 0;

	/// Normally the methods `beginTestCase`, `beforeFunctionCall`, `afterFunctionCall` and `endTestCase` are used
	/// to collect information of the test-calls. `verifyFunctionCall` can be used to check whether this information
	/// is complete and doesn't contain any errors.
	/// NOTE: if any of the test-call verification fails, the whole test-case fails.
	/// This function is called on each of the `TestFunctionCall` defined within the test-case.
	/// @param _call current `TestFunctionCall` defined in the test-case.
	/// @returns true, if verification of `_call` was successful, false otherwise.
	virtual bool verifyFunctionCall(TestFunctionCall const& _call) = 0;

	/// This function is used to format a single `TestFunctionCall`.
	/// It is responsible to generate the textual representation of `_call`
	/// that will be used to show and/or update expectations.
	/// @param _call current `TestFunctionCall` defined in the test-case.
	/// @param _errorReporter ErrorReporter to be used.
	/// @param _linePrefix contains the indentation prefix of the line.
	/// @param _renderResult false, the expected result of the call will be returned, otherwise the actual result is
	/// returned.
	/// @param _highlight false, if it's formatted without colorized highlighting. If it's true, AnsiColorized used to
	/// apply a colorized highlighting.
	/// @returns string that contains the textual representation of `_call`.
	virtual std::string formatFunctionCall(
		const TestFunctionCall& _call,
		ErrorReporter& _errorReporter,
		std::string const& _linePrefix,
		bool const _renderResult,
		bool const _highlight) const = 0;
};

} // namespace solidity::frontend::test
