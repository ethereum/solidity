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
 * @date 2017
 * Unit tests for parsing Julia.
 */

#include "../TestHelper.h"

#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/parsing/Scanner.h>
#include <test/libsolidity/ErrorCheck.h>

#include <boost/optional.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <string>
#include <memory>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

bool parse(string const& _source, ErrorList& errors)
{
	try
	{
		auto scanner = make_shared<Scanner>(CharStream(_source));
		auto parserResult = assembly::Parser(errors, true).parse(scanner);
		if (parserResult)
		{
			assembly::AsmAnalysisInfo analysisInfo;
			if (assembly::AsmAnalyzer(analysisInfo, errors).analyze(*parserResult))
				return true;
		}
	}
	catch (FatalError const&)
	{
		BOOST_FAIL("Fatal error leaked.");
	}
	return false;
}

boost::optional<Error> parseAndReturnFirstError(string const& _source, bool _allowWarnings = true)
{
	ErrorList errors;
	if (!parse(_source, errors))
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

bool successAssemble(string const& _source, bool _allowWarnings = true)
{
	return successParse(_source, _allowWarnings);
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
	BOOST_CHECK(searchErrorMessage(err, (substring))); \
} while(0)


BOOST_AUTO_TEST_SUITE(JuliaParser)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	BOOST_CHECK(successParse("{ }"));
}

BOOST_AUTO_TEST_CASE(vardecl)
{
	BOOST_CHECK(successParse("{ let x := 7 }"));
}

BOOST_AUTO_TEST_CASE(assignment)
{
	BOOST_CHECK(successParse("{ let x := 2 let y := x }"));
}

BOOST_AUTO_TEST_CASE(function_call)
{
	BOOST_CHECK(successParse("{ fun() fun(fun()) }"));
}

BOOST_AUTO_TEST_CASE(vardecl_complex)
{
	BOOST_CHECK(successParse("{ let y := 2 let x := add(7, mul(6, y)) }"));
}

BOOST_AUTO_TEST_CASE(variable_use_before_decl)
{
	CHECK_ERROR("{ x := 2 let x := 3 }", DeclarationError, "Variable x used before it was declared.");
	CHECK_ERROR("{ function mul(y) -> z {} let x := mul(x) }", DeclarationError, "Variable x used before it was declared.");
}

BOOST_AUTO_TEST_CASE(blocks)
{
	BOOST_CHECK(successParse("{ let x := 7 { let y := 3 } { let z := 2 } }"));
}

BOOST_AUTO_TEST_CASE(function_definitions)
{
	BOOST_CHECK(successParse("{ function f() { } function g(a) -> x { } }"));
}

BOOST_AUTO_TEST_CASE(function_definitions_multiple_args)
{
	BOOST_CHECK(successParse("{ function f(a, d) { } function g(a, d) -> x, y { } }"));
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	BOOST_CHECK(successParse("{ function f(a) -> b {} function g(a, b, c) {} function x() { g(1, 2, f(mul(2, 3))) x() } }"));
}

BOOST_AUTO_TEST_CASE(name_clashes)
{
	CHECK_ERROR("{ let g := 2 function g() { } }", DeclarationError, "Function name g already taken in this scope");
}

BOOST_AUTO_TEST_CASE(variable_access_cross_functions)
{
	CHECK_ERROR("{ let x := 2 function g() { let y := x } }", DeclarationError, "Identifier not found.");
}

BOOST_AUTO_TEST_CASE(label)
{
	CHECK_ERROR("{ label: }", ParserError, "Labels are not supported.");
}

BOOST_AUTO_TEST_CASE(instructions)
{
	CHECK_ERROR("{ pop }", ParserError, "Call or assignment expected.");
}

BOOST_AUTO_TEST_CASE(push)
{
	CHECK_ERROR("{ 0x42 }", ParserError, "Call or assignment expected.");
}

BOOST_AUTO_TEST_CASE(assign_from_stack)
{
	CHECK_ERROR("{ =: x }", ParserError, "Literal or identifier expected.");
}

BOOST_AUTO_TEST_CASE(empty_call)
{
	CHECK_ERROR("{ () }", ParserError, "Literal or identifier expected.");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
