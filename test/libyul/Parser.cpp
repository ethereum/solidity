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
 * Unit tests for parsing Yul.
 */

#include <test/Common.h>

#include <test/libsolidity/ErrorCheck.h>
#include <test/libyul/Common.h>

#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Dialect.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ErrorReporter.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>
#include <optional>
#include <string>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;

namespace solidity::yul::test
{

namespace
{

shared_ptr<Block> parse(string const& _source, Dialect const& _dialect, ErrorReporter& errorReporter)
{
	try
	{
		auto scanner = make_shared<Scanner>(CharStream(_source, ""));
		auto parserResult = yul::Parser(errorReporter, _dialect).parse(scanner, false);
		if (parserResult)
		{
			yul::AsmAnalysisInfo analysisInfo;
			if (yul::AsmAnalyzer(
				analysisInfo,
				errorReporter,
				_dialect
			).analyze(*parserResult))
				return parserResult;
		}
	}
	catch (FatalError const&)
	{
		BOOST_FAIL("Fatal error leaked.");
	}
	return {};
}

std::optional<Error> parseAndReturnFirstError(string const& _source, Dialect const& _dialect, bool _allowWarnings = true)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	if (!parse(_source, _dialect, errorReporter))
	{
		BOOST_REQUIRE(!errors.empty());
		BOOST_CHECK_EQUAL(errors.size(), 1);
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

bool successParse(std::string const& _source, Dialect const& _dialect = Dialect::yulDeprecated(), bool _allowWarnings = true)
{
	return !parseAndReturnFirstError(_source, _dialect, _allowWarnings);
}

Error expectError(std::string const& _source, Dialect const& _dialect = Dialect::yulDeprecated(), bool _allowWarnings = false)
{

	auto error = parseAndReturnFirstError(_source, _dialect, _allowWarnings);
	BOOST_REQUIRE(error);
	return *error;
}

}

#define CHECK_ERROR_DIALECT(text, typ, substring, dialect) \
do \
{ \
	Error err = expectError((text), dialect, false); \
	BOOST_CHECK(err.type() == (Error::Type::typ)); \
	BOOST_CHECK(solidity::frontend::test::searchErrorMessage(err, (substring))); \
} while(0)

#define CHECK_ERROR(text, typ, substring) CHECK_ERROR_DIALECT(text, typ, substring, Dialect::yulDeprecated())

BOOST_AUTO_TEST_SUITE(YulParser)

BOOST_AUTO_TEST_CASE(recursion_depth)
{
	string input;
	for (size_t i = 0; i < 20000; i++)
		input += "{";
	input += "let x:u256 := 0:u256";
	for (size_t i = 0; i < 20000; i++)
		input += "}";

	CHECK_ERROR(input, ParserError, "recursion");
}

BOOST_AUTO_TEST_CASE(multiple_assignment)
{
	CHECK_ERROR("{ let x:u256 function f() -> a:u256, b:u256 {} 123:u256, x := f() }", ParserError, "Variable name must precede \",\" in multiple assignment.");
	CHECK_ERROR("{ let x:u256 function f() -> a:u256, b:u256 {} x, 123:u256 := f() }", ParserError, "Variable name must precede \":=\" in assignment.");

	/// NOTE: Travis hiccups if not having a variable
	char const* text = R"(
	{
		function f(a:u256) -> r1:u256, r2:u256 {
			r1 := a
			r2 := 7:u256
		}
		let x:u256 := 9:u256
		let y:u256 := 2:u256
		x, y := f(x)
	}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_defined_in_init_block)
{
	auto const& dialect = EVMDialect::strictAssemblyForEVMObjects(EVMVersion{});
	BOOST_CHECK(successParse("{ for { } 1 { function f() {} } {} }", dialect));
	BOOST_CHECK(successParse("{ for { } 1 {} { function f() {} } }", dialect));
	CHECK_ERROR_DIALECT(
		"{ for { function f() {} } 1 {} {} }",
		SyntaxError,
		"Functions cannot be defined inside a for-loop init block.",
		dialect
	);
}

BOOST_AUTO_TEST_CASE(function_defined_in_init_nested)
{
	auto const& dialect = EVMDialect::strictAssemblyForEVMObjects(EVMVersion{});
	BOOST_CHECK(successParse(
		"{ for {"
			"for { } 1 { function f() {} } {}"
		"} 1 {} {} }", dialect));
	CHECK_ERROR_DIALECT(
		"{ for { for {function foo() {}} 1 {} {} } 1 {} {} }",
		SyntaxError,
		"Functions cannot be defined inside a for-loop init block.",
		dialect
	);
	CHECK_ERROR_DIALECT(
		"{ for {} 1 {for {function foo() {}} 1 {} {} } {} }",
		SyntaxError,
		"Functions cannot be defined inside a for-loop init block.",
		dialect
	);
}

BOOST_AUTO_TEST_CASE(function_shadowing_outside_vars)
{
	CHECK_ERROR("{ let x:u256 function f() -> x:u256 {} }", DeclarationError, "already taken in this scope");
	BOOST_CHECK(successParse("{ { let x:u256 } function f() -> x:u256 {} }"));
}

BOOST_AUTO_TEST_CASE(builtins_parser)
{
	struct SimpleDialect: public Dialect
	{
		BuiltinFunction const* builtin(YulString _name) const override
		{
			return _name == "builtin"_yulstring ? &f : nullptr;
		}
		BuiltinFunction f;
	};

	SimpleDialect dialect;
	CHECK_ERROR_DIALECT("{ let builtin := 6 }", ParserError, "Cannot use builtin function name \"builtin\" as identifier name.", dialect);
	CHECK_ERROR_DIALECT("{ function builtin() {} }", ParserError, "Cannot use builtin function name \"builtin\" as identifier name.", dialect);
	CHECK_ERROR_DIALECT("{ function f(x) { f(builtin) } }", ParserError, "Expected '(' but got ')'", dialect);
	CHECK_ERROR_DIALECT("{ function f(builtin) {}", ParserError, "Cannot use builtin function name \"builtin\" as identifier name.", dialect);
	CHECK_ERROR_DIALECT("{ function f() -> builtin {}", ParserError, "Cannot use builtin function name \"builtin\" as identifier name.", dialect);
}

BOOST_AUTO_TEST_CASE(builtins_analysis)
{
	struct SimpleDialect: public Dialect
	{
		BuiltinFunction const* builtin(YulString _name) const override
		{
			return _name == "builtin"_yulstring ? &f : nullptr;
		}
		BuiltinFunction f{"builtin"_yulstring, vector<Type>(2), vector<Type>(3), {}, {}, false, {}};
	};

	SimpleDialect dialect;
	BOOST_CHECK(successParse("{ let a, b, c := builtin(1, 2) }", dialect));
	CHECK_ERROR_DIALECT("{ let a, b, c := builtin(1) }", TypeError, "Function expects 2 arguments but got 1", dialect);
	CHECK_ERROR_DIALECT("{ let a, b := builtin(1, 2) }", DeclarationError, "Variable count mismatch: 2 variables and 3 values.", dialect);
}

BOOST_AUTO_TEST_CASE(default_types_set)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	shared_ptr<Block> result = parse(
		"{"
			"let x:bool := true:bool "
			"let z:bool := true "
			"let y := add(1, 2) "
			"switch y case 0 {} default {} "
		"}",
		EVMDialectTyped::instance(EVMVersion{}),
		reporter
	);
	BOOST_REQUIRE(!!result);

	// Use no dialect so that all types are printed.
	// This tests that the default types are properly assigned.
	BOOST_CHECK_EQUAL(AsmPrinter{}(*result),
		"{\n"
		"    let x:bool := true:bool\n"
		"    let z:bool := true:bool\n"
		"    let y:u256 := add(1:u256, 2:u256)\n"
		"    switch y\n"
		"    case 0:u256 { }\n"
		"    default { }\n"
		"}"
	);

	// Now test again with type dialect. Now the default types
	// should be omitted.
	BOOST_CHECK_EQUAL(AsmPrinter{EVMDialectTyped::instance(EVMVersion{})}(*result),
		"{\n"
		"    let x:bool := true\n"
		"    let z:bool := true\n"
		"    let y := add(1, 2)\n"
		"    switch y\n"
		"    case 0 { }\n"
		"    default { }\n"
		"}"
	);
}



BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
