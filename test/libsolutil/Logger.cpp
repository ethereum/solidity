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
/**
 * Unit tests for the logging registry.
 */

#include <libsolutil/Logger.h>

#include <boost/test/unit_test.hpp>

#include <sstream>

using namespace solidity::log;

namespace solidity::log::test
{

namespace
{

/// Resets the registry before and after each test so the process-wide singleton stays isolated.
struct LoggerFixture
{
	LoggerFixture() { LoggerRegistry::singleton().reset(); }
	~LoggerFixture() { LoggerRegistry::singleton().reset(); }
};

}

BOOST_FIXTURE_TEST_SUITE(LoggerTest, LoggerFixture)

BOOST_AUTO_TEST_CASE(loggers_default_to_off)
{
	Logger const& logger = LoggerRegistry::singleton().get("yul.ssa");
	BOOST_CHECK(!logger.shouldLog(Level::trace));
	BOOST_CHECK(!logger.shouldLog(Level::debug));
	BOOST_CHECK(!logger.shouldLog(Level::warn));
}

BOOST_AUTO_TEST_CASE(comparison_direction)
{
	auto& registry = LoggerRegistry::singleton();
	Logger const& logger = registry.get("a");

	registry.setLevel("a", Level::trace);
	BOOST_CHECK(logger.shouldLog(Level::trace));
	BOOST_CHECK(logger.shouldLog(Level::debug));
	BOOST_CHECK(logger.shouldLog(Level::warn));

	registry.setLevel("a", Level::debug);
	BOOST_CHECK(!logger.shouldLog(Level::trace));
	BOOST_CHECK(logger.shouldLog(Level::debug));
	BOOST_CHECK(logger.shouldLog(Level::warn));

	registry.setLevel("a", Level::warn);
	BOOST_CHECK(!logger.shouldLog(Level::trace));
	BOOST_CHECK(!logger.shouldLog(Level::debug));
	BOOST_CHECK(logger.shouldLog(Level::warn));

	registry.setLevel("a", Level::off);
	BOOST_CHECK(!logger.shouldLog(Level::trace));
	BOOST_CHECK(!logger.shouldLog(Level::debug));
	BOOST_CHECK(!logger.shouldLog(Level::warn));
}

BOOST_AUTO_TEST_CASE(hierarchy_most_specific_wins)
{
	auto& registry = LoggerRegistry::singleton();
	registry.setLevel("", Level::warn);
	registry.setLevel("yul.ssa", Level::debug);
	registry.setLevel("yul.ssa.codetransform.shuffler", Level::off);

	// Global default applies.
	BOOST_CHECK(registry.get("yul").shouldLog(Level::warn));
	BOOST_CHECK(!registry.get("yul").shouldLog(Level::debug));

	// "yul.ssa" rule applies to it and descendants.
	BOOST_CHECK(registry.get("yul.ssa").shouldLog(Level::debug));
	BOOST_CHECK(registry.get("yul.ssa.codetransform").shouldLog(Level::debug));

	// Most specific rule mutes the shuffler.
	BOOST_CHECK(!registry.get("yul.ssa.codetransform.shuffler").shouldLog(Level::warn));
}

BOOST_AUTO_TEST_CASE(prefix_matching_is_segment_wise)
{
	auto& registry = LoggerRegistry::singleton();
	registry.setLevel("yul", Level::debug);

	// "yul" matches "yul.ssa" but must not match "yulish".
	BOOST_CHECK(registry.get("yul.ssa").shouldLog(Level::debug));
	BOOST_CHECK(!registry.get("yulish").shouldLog(Level::debug));
}

BOOST_AUTO_TEST_CASE(level_change_propagates_to_existing_loggers)
{
	auto& registry = LoggerRegistry::singleton();
	Logger const& logger = registry.get("yul.ssa.stacklayout");
	BOOST_CHECK(!logger.shouldLog(Level::debug));

	registry.setLevel("yul.ssa", Level::debug);
	BOOST_CHECK(logger.shouldLog(Level::debug));
}

BOOST_AUTO_TEST_CASE(broader_rule_does_not_override_more_specific)
{
	auto& registry = LoggerRegistry::singleton();
	registry.setLevel("yul.ssa", Level::off);
	Logger const& logger = registry.get("yul.ssa"); // exists before the broader rule is set

	registry.setLevel("yul", Level::debug); // broader prefix, arrives later

	// Most-specific match wins regardless of the order rules arrive in: "yul.ssa"=off still applies.
	BOOST_CHECK(!logger.shouldLog(Level::debug));
}

BOOST_AUTO_TEST_CASE(sibling_inherits_parent_and_is_unaffected_by_other_subtrees)
{
	auto& registry = LoggerRegistry::singleton();
	registry.setLevel("yul", Level::warn);
	registry.setLevel("yul.ssa", Level::debug);

	// "yul.ir" has no rule of its own, so it inherits its parent "yul"=warn.
	Logger const& ir = registry.get("yul.ir");
	BOOST_CHECK(ir.shouldLog(Level::warn));
	BOOST_CHECK(!ir.shouldLog(Level::debug));

	// Changing the sibling subtree "yul.ssa" must not affect "yul.ir".
	registry.setLevel("yul.ssa", Level::trace);
	BOOST_CHECK(ir.shouldLog(Level::warn));
	BOOST_CHECK(!ir.shouldLog(Level::debug));
}

BOOST_AUTO_TEST_CASE(output_has_prefix_and_formats_arguments)
{
	auto& registry = LoggerRegistry::singleton();
	std::ostringstream out;
	registry.setOutput(out);
	registry.setLevel("c", Level::debug);

	Logger const& logger = registry.get("c");
	solDebug(logger, "x={}", 1);
	// Every line carries the standard "[<level> <category>] " prefix.
	BOOST_CHECK_EQUAL(out.str(), "[debug c] x=1\n");
}

BOOST_AUTO_TEST_CASE(prefix_carries_message_level_not_logger_level)
{
	auto& registry = LoggerRegistry::singleton();
	std::ostringstream out;
	registry.setOutput(out);
	registry.setLevel("yul.ssa", Level::debug);

	Logger const& logger = registry.get("yul.ssa");
	solWarn(logger, "oops {}", 7);
	// The prefix carries the level of the individual message, not the logger's configured level.
	BOOST_CHECK_EQUAL(out.str(), "[warn yul.ssa] oops 7\n");
}

BOOST_AUTO_TEST_CASE(disabled_logger_does_not_evaluate_arguments)
{
	auto& registry = LoggerRegistry::singleton();
	std::ostringstream out;
	registry.setOutput(out);

	Logger const& logger = registry.get("d"); // defaults to off

	int evaluations = 0;
	auto sideEffect = [&]() { ++evaluations; return 42; };

	solDebug(logger, "{}", sideEffect());
	BOOST_CHECK_EQUAL(evaluations, 0);
	BOOST_CHECK(out.str().empty());

	registry.setLevel("d", Level::debug);
	solDebug(logger, "{}", sideEffect());
	BOOST_CHECK_EQUAL(evaluations, 1);
	BOOST_CHECK_EQUAL(out.str(), "[debug d] 42\n");
}

BOOST_AUTO_TEST_CASE(level_string_round_trip)
{
	BOOST_CHECK(levelFromString("trace") == Level::trace);
	BOOST_CHECK(levelFromString("debug") == Level::debug);
	BOOST_CHECK(levelFromString("warn") == Level::warn);
	BOOST_CHECK(levelFromString("off") == Level::off);
	BOOST_CHECK(levelFromString("bogus") == std::nullopt);

	BOOST_CHECK_EQUAL(levelToString(Level::trace), "trace");
	BOOST_CHECK_EQUAL(levelToString(Level::off), "off");
}

BOOST_AUTO_TEST_SUITE_END()

}
