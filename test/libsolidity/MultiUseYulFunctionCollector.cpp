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

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>

#include <libyul/Utilities.h>

#include <liblangutil/Exceptions.h>

#include <boost/test/unit_test.hpp>

#include <fmt/format.h>

#include <set>
#include <string>
#include <type_traits>

namespace solidity::frontend::test
{

namespace
{

enum class TestMode { A, B };

/// Functions f_<value> whose names do not encode the mode.
yulcache::FunctionKind<unsigned, TestMode> constexpr modeIgnoringKind{
	.name = [](unsigned const& _value, TestMode const&) { return "f_" + std::to_string(_value); },
	.define = [](
		yulcache::FunctionEnv const&,
		unsigned const& _value,
		TestMode const& _mode
	) {
		return util::Whiskers(R"(
			function <functionName>(a, b) -> r {
				r := <value>
				// mode <mode>
			}
		)")
		("value", std::to_string(_value))
		("mode", std::to_string(static_cast<int>(_mode)));
	},
};

/// Functions f_<value> that call themselves and f_<value - 1>.
yulcache::FunctionKind<unsigned> const recursiveKind{
	.name = [](unsigned const& _value) { return "f_" + std::to_string(_value); },
	.define = [](
		yulcache::FunctionEnv const& _env,
		unsigned const& _value
	) {
		std::string body = _env.request<recursiveKind>(_value) + "()";
		if (_value > 0)
			body += "\n" + _env.request<recursiveKind>(_value - 1) + "()";
		return util::Whiskers(R"(
			function <functionName>() {
				<body>
			}
		)")("body", body);
	},
};

/// Function g, whose name does not encode the argument.
yulcache::FunctionKind<std::optional<unsigned>> constexpr optionalIgnoringKind{
	.name = [](std::optional<unsigned> const&) -> std::string { return "g"; },
	.define = [](
		yulcache::FunctionEnv const&,
		std::optional<unsigned> const&
	) {
		return util::Whiskers("function <functionName>() {}");
	},
};

/// Functions h_<type>.
yulcache::FunctionKind<Type> constexpr typeKind{
	.name = [](Type const& _type) { return "h_" + _type.identifier(); },
	.define = [](yulcache::FunctionEnv const&, Type const&) {
		return util::Whiskers(R"(
			function <functionName>() {
			}
		)");
	},
};

/// Function u, whose template does not contain the function name.
yulcache::FunctionKind<> constexpr unnamedKind{
	.name = []() -> std::string { return "u"; },
	.define = [](yulcache::FunctionEnv const&) { return util::Whiskers("function u() {}"); },
};

/// Function s, whose template sets the function name itself.
yulcache::FunctionKind<> constexpr selfNamingKind{
	.name = []() -> std::string { return "s"; },
	.define = [](yulcache::FunctionEnv const&) {
		return util::Whiskers("function <functionName>() {}")("functionName", "s");
	},
};

using SizeKind = yulcache::FunctionKind<std::size_t>;
// I can use lambdas without capture for the name
static_assert(std::is_convertible_v<
	decltype([](std::size_t const&) { return std::string{}; }),
	decltype(SizeKind::name)
>);
// I can't use lambdas with capture for the name
static_assert(!std::is_convertible_v<
	decltype([x = 0](std::size_t const&) { return std::to_string(x); }),
	decltype(SizeKind::name)
>);
// The argument types have to match the declared ones exactly
static_assert(!std::is_convertible_v<
	decltype([](int const&) { return std::string{}; }),
	decltype(SizeKind::name)
>);
static_assert(!std::is_convertible_v<
	decltype([](std::size_t) { return std::string{}; }),
	decltype(SizeKind::name)
>);
static_assert(!std::is_convertible_v<
	decltype([](std::size_t const&, std::size_t const&) { return std::string{}; }),
	decltype(SizeKind::name)
>);
// The definition receives the environment by const reference
static_assert(std::is_convertible_v<
	decltype([](yulcache::FunctionEnv const&, std::size_t const&) { return util::Whiskers(""); }),
	decltype(SizeKind::define)
>);
static_assert(!std::is_convertible_v<
	decltype([](yulcache::FunctionEnv&, std::size_t const&) { return util::Whiskers(""); }),
	decltype(SizeKind::define)
>);
static_assert(!std::is_convertible_v<
	decltype([](std::size_t const&) { return util::Whiskers(""); }),
	decltype(SizeKind::define)
>);
// Argument types need a key
static_assert(yulcache::RequestArgument<std::size_t>);
static_assert(!yulcache::RequestArgument<std::set<int>>);

}

BOOST_AUTO_TEST_SUITE(MultiUseYulFunctionCollectorTest, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(same_site_reuses_cached_function)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
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
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	auto creator = []() -> std::string { return "function f() {}\n"; };
	collector.createFunction("f", creator);
	BOOST_CHECK(!collector.requestedFunctions().empty());
	BOOST_CHECK_NO_THROW(collector.createFunction("f", creator));
	BOOST_CHECK(!collector.requestedFunctions().empty());
}

BOOST_AUTO_TEST_CASE(same_name_from_different_sites_asserts)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	auto creator = []() -> std::string { return "function f() {}\n"; };
	collector.createFunction("f", creator);
	BOOST_CHECK_THROW(collector.createFunction("f", creator), langutil::InternalCompilerError);
}

BOOST_AUTO_TEST_CASE(same_name_from_different_sites_asserts_for_parameter_creator)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	auto creator = [](std::vector<std::string>&, std::vector<std::string>&) -> std::string {
		return "{}";
	};
	collector.createFunction("f", creator);
	BOOST_CHECK_THROW(collector.createFunction("f", creator), langutil::InternalCompilerError);
}

BOOST_AUTO_TEST_CASE(same_name_from_different_creators_through_a_wrapper_asserts)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	auto request = [&](std::function<std::string()> const& _creator) {
		return collector.createFunction("f", _creator);
	};
	request([]() -> std::string { return "function f() {}\n"; });
	BOOST_CHECK_THROW(
		request([]() -> std::string { return "function f() {}\n"; }),
		langutil::InternalCompilerError
	);
}

BOOST_AUTO_TEST_CASE(request_with_same_arguments_reuses_function)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	BOOST_CHECK_EQUAL(collector.request<modeIgnoringKind>(7u, TestMode::A), "f_7");
	BOOST_CHECK_EQUAL(collector.request<modeIgnoringKind>(7u, TestMode::A), "f_7");
	BOOST_CHECK(collector.contains("f_7"));
	BOOST_CHECK_EQUAL(
		yul::reindent(collector.requestedFunctions()),
		yul::reindent(R"(
			function f_7(a, b) -> r {
				r := 7
				// mode 0
			}
		)")
	);
}

BOOST_AUTO_TEST_CASE(request_with_different_arguments_for_the_same_name_asserts)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	collector.request<modeIgnoringKind>(7u, TestMode::A);
	BOOST_CHECK_EQUAL(collector.request<modeIgnoringKind>(8u, TestMode::B), "f_8");
	BOOST_CHECK_THROW(
		collector.request<modeIgnoringKind>(7u, TestMode::B),
		langutil::InternalCompilerError
	);
}

BOOST_AUTO_TEST_CASE(request_of_different_kinds_for_the_same_name_asserts)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	collector.request<modeIgnoringKind>(0u, TestMode::A);
	BOOST_CHECK_THROW(collector.request<recursiveKind>(0u), langutil::InternalCompilerError);
}

BOOST_AUTO_TEST_CASE(absent_and_zero_optional_arguments_differ)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	collector.request<optionalIgnoringKind>(std::optional<unsigned>{});
	BOOST_CHECK_THROW(
		collector.request<optionalIgnoringKind>(std::optional<unsigned>{0}),
		langutil::InternalCompilerError
	);
}

BOOST_AUTO_TEST_CASE(nested_optional_arguments_have_distinct_keys)
{
	using Nested = std::optional<std::optional<unsigned>>;
	static_assert(yulcache::RequestArgument<Nested>);
	std::set<yulcache::RequestArgumentKey> const keys{
		yulcache::requestArgumentKey(Nested{}),
		yulcache::requestArgumentKey(Nested{std::in_place}),
		yulcache::requestArgumentKey(Nested{std::in_place, 0u}),
		yulcache::requestArgumentKey(Nested{std::in_place, 1u}),
	};
	BOOST_CHECK_EQUAL(keys.size(), 4);
}

BOOST_AUTO_TEST_CASE(equal_types_are_the_same_argument)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	Type const* first = TypeProvider::array(DataLocation::Memory, TypeProvider::uint256());
	Type const* second = TypeProvider::array(DataLocation::Memory, TypeProvider::uint256());
	BOOST_REQUIRE_NE(first, second);
	std::string const name = collector.request<typeKind>(*first);
	BOOST_CHECK_EQUAL(collector.request<typeKind>(*second), name);
	BOOST_CHECK_EQUAL(
		yul::reindent(collector.requestedFunctions()),
		yul::reindent(fmt::format(R"(
			function {}() {{
			}}
		)", name))
	);
}

BOOST_AUTO_TEST_CASE(template_without_function_name_asserts)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	BOOST_CHECK_THROW(collector.request<unnamedKind>(), util::WhiskersError);
}

BOOST_AUTO_TEST_CASE(template_setting_function_name_asserts)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	BOOST_CHECK_THROW(collector.request<selfNamingKind>(), util::WhiskersError);
}

BOOST_AUTO_TEST_CASE(recursive_request_returns_preassigned_name)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	BOOST_CHECK_EQUAL(collector.request<recursiveKind>(1u), "f_1");
	BOOST_CHECK_EQUAL(
		yul::reindent(collector.requestedFunctions()),
		yul::reindent(R"(
			function f_0() {
				f_0()
			}

			function f_1() {
				f_1()
				f_0()
			}
		)")
	);
}

BOOST_AUTO_TEST_CASE(request_and_create_function_for_the_same_name_assert)
{
	std::string const name = "f_7";
	auto const creator = []() -> std::string { return "function f_7() {}\n"; };
	{
		MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
		collector.createFunction(name, creator);
		BOOST_CHECK_THROW(
			collector.request<modeIgnoringKind>(7u, TestMode::A),
			langutil::InternalCompilerError
		);
	}
	{
		MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
		collector.request<modeIgnoringKind>(7u, TestMode::A);
		BOOST_CHECK_THROW(collector.createFunction(name, creator), langutil::InternalCompilerError);
	}
}

BOOST_AUTO_TEST_CASE(requested_names_are_reset_when_the_collector_is_drained)
{
	MultiUseYulFunctionCollector collector{langutil::EVMVersion{}, RevertStrings::Default};
	collector.request<modeIgnoringKind>(7u, TestMode::A);
	BOOST_CHECK(!collector.requestedFunctions().empty());
	BOOST_CHECK(!collector.contains("f_7"));
	collector.request<modeIgnoringKind>(7u, TestMode::B);
	BOOST_CHECK(collector.contains("f_7"));
}

BOOST_AUTO_TEST_SUITE_END()

}
