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
/**
 * @date 2018
 * Unit tests for the Yul object parser.
 */

#include <test/Common.h>

#include <test/libsolidity/ErrorCheck.h>

#include <libyul/AssemblyStack.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>
#include <optional>
#include <string>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::langutil;

namespace solidity::yul::test
{

namespace
{

std::pair<bool, ErrorList> parse(string const& _source)
{
	try
	{
		AssemblyStack asmStack(
			solidity::test::CommonOptions::get().evmVersion(),
			AssemblyStack::Language::StrictAssembly,
			solidity::frontend::OptimiserSettings::none()
		);
		bool success = asmStack.parseAndAnalyze("source", _source);
		return {success, asmStack.errors()};
	}
	catch (FatalError const&)
	{
		BOOST_FAIL("Fatal error leaked.");
	}
	return {false, {}};
}

std::optional<Error> parseAndReturnFirstError(string const& _source, bool _allowWarnings = true)
{
	bool success;
	ErrorList errors;
	tie(success, errors) = parse(_source);
	if (!success)
	{
		BOOST_REQUIRE_EQUAL(errors.size(), 1);
		return *errors.front();
	}
	else
	{
		// If success is true, there might still be an error in the assembly stage.
		if (_allowWarnings && Error::containsOnlyWarnings(errors))
			return {};
		else if (!errors.empty())
		{
			if (!_allowWarnings)
				BOOST_CHECK_EQUAL(errors.size(), 1);
			return *errors.front();
		}
	}
	return {};
}

bool successParse(std::string const& _source, bool _allowWarnings = true)
{
	return !parseAndReturnFirstError(_source, _allowWarnings);
}

Error expectError(std::string const& _source, bool _allowWarnings = false)
{

	auto error = parseAndReturnFirstError(_source, _allowWarnings);
	BOOST_REQUIRE(error);
	return *error;
}

}

#define CHECK_ERROR(text, typ, substring) \
do \
{ \
	Error err = expectError((text), false); \
	BOOST_CHECK(err.type() == (Error::Type::typ)); \
	BOOST_CHECK(::solidity::frontend::test::searchErrorMessage(err, (substring))); \
} while(0)

BOOST_AUTO_TEST_SUITE(YulObjectParser)

BOOST_AUTO_TEST_CASE(empty_code)
{
	BOOST_CHECK(successParse("{ }"));
}

BOOST_AUTO_TEST_CASE(recursion_depth)
{
	string input;
	for (size_t i = 0; i < 20000; i++)
		input += "object \"a" + to_string(i) + "\" { code {} ";
	for (size_t i = 0; i < 20000; i++)
		input += "}";

	CHECK_ERROR(input, ParserError, "recursion");
}

BOOST_AUTO_TEST_CASE(to_string)
{
	string code = R"(
		object "O" {
			code { let x := mload(0) if x { sstore(0, 1) } }
			object "i" { code {} data "j" "def" }
			data "j" "abc"
			data "k" hex"010203"
		}
	)";
	string expectation = R"(object "O" {
	code {
		let x := mload(0)
		if x { sstore(0, 1) }
	}
	object "i" {
		code { }
		data "j" hex"646566"
	}
	data "j" hex"616263"
	data "k" hex"010203"
}
)";
	expectation = boost::replace_all_copy(expectation, "\t", "    ");
	AssemblyStack asmStack(
		solidity::test::CommonOptions::get().evmVersion(),
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::none()
	);
	BOOST_REQUIRE(asmStack.parseAndAnalyze("source", code));
	BOOST_CHECK_EQUAL(asmStack.print(), expectation);
}

BOOST_AUTO_TEST_SUITE_END()

}
