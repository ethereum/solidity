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
 * Unit tests for the collector of generated Yul helper functions.
 */

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <liblangutil/Exceptions.h>

#include <boost/test/unit_test.hpp>

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(MultiUseYulFunctionCollectorTest, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(same_site_reuses_cached_function)
{
	MultiUseYulFunctionCollector collector;
	std::size_t creatorCalls = 0;
	auto creator = [&]() -> std::string {
		++creatorCalls;
		return "function f() {}\n";
	};
	auto request = [&] { return collector.createFunction("f", creator); };
	BOOST_CHECK_EQUAL(request(), "f");
	BOOST_CHECK_EQUAL(request(), "f");
	BOOST_CHECK_EQUAL(creatorCalls, 1);
}

BOOST_AUTO_TEST_CASE(sites_are_reset_when_the_collector_is_drained)
{
	MultiUseYulFunctionCollector collector;
	auto creator = []() -> std::string { return "function f() {}\n"; };
	collector.createFunction("f", creator);
	BOOST_CHECK(!collector.requestedFunctions().empty());
	BOOST_CHECK_NO_THROW(collector.createFunction("f", creator));
	BOOST_CHECK(!collector.requestedFunctions().empty());
}

BOOST_AUTO_TEST_CASE(same_name_from_different_sites_asserts)
{
	MultiUseYulFunctionCollector collector;
	auto creator = []() -> std::string { return "function f() {}\n"; };
	collector.createFunction("f", creator);
	BOOST_CHECK_THROW(collector.createFunction("f", creator), langutil::InternalCompilerError);
}

BOOST_AUTO_TEST_CASE(same_name_from_different_sites_asserts_for_parameter_creator)
{
	MultiUseYulFunctionCollector collector;
	auto creator = [](std::vector<std::string>&, std::vector<std::string>&) -> std::string {
		return "{}";
	};
	collector.createFunction("f", creator);
	BOOST_CHECK_THROW(collector.createFunction("f", creator), langutil::InternalCompilerError);
}

BOOST_AUTO_TEST_CASE(same_name_from_different_creators_through_a_wrapper_asserts)
{
	MultiUseYulFunctionCollector collector;
	auto request = [&](std::function<std::string()> const& _creator) {
		return collector.createFunction("f", _creator);
	};
	request([]() -> std::string { return "function f() {}\n"; });
	BOOST_CHECK_THROW(
		request([]() -> std::string { return "function f() {}\n"; }),
		langutil::InternalCompilerError
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
