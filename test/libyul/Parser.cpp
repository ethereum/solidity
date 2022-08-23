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

#include <libyul/AST.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Dialect.h>
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

BOOST_TEST_DONT_PRINT_LOG_VALUE(ErrorId)
BOOST_TEST_DONT_PRINT_LOG_VALUE(Error::Type)

namespace solidity::yul::test
{

namespace
{

shared_ptr<Block> parse(string const& _source, Dialect const& _dialect, ErrorReporter& errorReporter)
{
	try
	{
		auto stream = CharStream(_source, "");
		map<unsigned, shared_ptr<string const>> indicesToSourceNames;
		indicesToSourceNames[0] = make_shared<string const>("source0");
		indicesToSourceNames[1] = make_shared<string const>("source1");

		auto parserResult = yul::Parser(
			errorReporter,
			_dialect,
			std::move(indicesToSourceNames)
		).parse(stream);
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

std::optional<Error> parseAndReturnFirstError(string const& _source, Dialect const& _dialect, bool _allowWarningsAndInfos = true)
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
		if (_allowWarningsAndInfos && !Error::containsErrors(errors))
			return {};
		else if (!errors.empty())
		{
			if (!_allowWarningsAndInfos)
				BOOST_CHECK_EQUAL(errors.size(), 1);
			return *errors.front();
		}
	}
	return {};
}

bool successParse(std::string const& _source, Dialect const& _dialect = Dialect::yulDeprecated(), bool _allowWarningsAndInfos = true)
{
	return !parseAndReturnFirstError(_source, _dialect, _allowWarningsAndInfos);
}

Error expectError(std::string const& _source, Dialect const& _dialect = Dialect::yulDeprecated(), bool _allowWarningsAndInfos = false)
{

	auto error = parseAndReturnFirstError(_source, _dialect, _allowWarningsAndInfos);
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
	CHECK_ERROR_DIALECT("{ let a, b, c := builtin(1) }", TypeError, "Function \"builtin\" expects 2 arguments but got 1", dialect);
	CHECK_ERROR_DIALECT("{ let a, b := builtin(1, 2) }", DeclarationError, "Variable count mismatch for declaration of \"a, b\": 2 variables and 3 values.", dialect);
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
	BOOST_REQUIRE(!!result && errorList.size() == 0);

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

#define CHECK_LOCATION(_actual, _sourceName, _start, _end) \
	do { \
		BOOST_CHECK_EQUAL((_sourceName), ((_actual).sourceName ? *(_actual).sourceName : "")); \
		BOOST_CHECK_EQUAL((_start), (_actual).start); \
		BOOST_CHECK_EQUAL((_end), (_actual).end); \
	} while (0)

BOOST_AUTO_TEST_CASE(customSourceLocations_empty_block)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 234, 543);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_block_with_children)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{\n"
			"let x:bool := true:bool\n"
			"/// @src 0:123:432\n"
			"let z:bool := true\n"
			"let y := add(1, 2)\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(3, result->statements.size());
	CHECK_LOCATION(originLocationOf(result->statements.at(0)), "source0", 234, 543);
	CHECK_LOCATION(originLocationOf(result->statements.at(1)), "source0", 123, 432);
	// [2] is inherited source location
	CHECK_LOCATION(originLocationOf(result->statements.at(2)), "source0", 123, 432);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_block_different_sources)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{\n"
			"let x:bool := true:bool\n"
			"/// @src 1:123:432\n"
			"let z:bool := true\n"
			"let y := add(1, 2)\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(3, result->statements.size());
	CHECK_LOCATION(originLocationOf(result->statements.at(0)), "source0", 234, 543);
	CHECK_LOCATION(originLocationOf(result->statements.at(1)), "source1", 123, 432);
	// [2] is inherited source location
	CHECK_LOCATION(originLocationOf(result->statements.at(2)), "source1", 123, 432);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_block_nested)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{\n"
			"let y := add(1, 2)\n"
			"/// @src 0:343:434\n"
			"switch y case 0 {} default {}\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	CHECK_LOCATION(originLocationOf(result->statements.at(1)), "source0", 343, 434);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_block_switch_case)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{\n"
			"let y := add(1, 2)\n"
			"/// @src 0:343:434\n"
			"switch y\n"
			"/// @src 0:3141:59265\n"
			"case 0 {\n"
			"    /// @src 0:271:828\n"
			"    let z := add(3, 4)\n"
			"}\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 234, 543);

	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	BOOST_REQUIRE(holds_alternative<Switch>(result->statements.at(1)));
	auto const& switchStmt = get<Switch>(result->statements.at(1));

	CHECK_LOCATION(switchStmt.debugData->originLocation, "source0", 343, 434);
	BOOST_REQUIRE_EQUAL(1, switchStmt.cases.size());
	CHECK_LOCATION(switchStmt.cases.at(0).debugData->originLocation, "source0", 3141, 59265);

	auto const& caseBody = switchStmt.cases.at(0).body;
	BOOST_REQUIRE_EQUAL(1, caseBody.statements.size());
	CHECK_LOCATION(originLocationOf(caseBody.statements.at(0)), "source0", 271, 828);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_inherit_into_outer_scope)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:1:100\n"
		"{\n"
			"{\n"
				"/// @src 0:123:432\n"
				"let x:bool := true:bool\n"
			"}\n"
			"let z:bool := true\n"
			"let y := add(1, 2)\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);

	CHECK_LOCATION(result->debugData->originLocation, "source0", 1, 100);

	BOOST_REQUIRE_EQUAL(3, result->statements.size());
	CHECK_LOCATION(originLocationOf(result->statements.at(0)), "source0", 1, 100);

	// First child element must be a block itself with one statement.
	BOOST_REQUIRE(holds_alternative<Block>(result->statements.at(0)));
	BOOST_REQUIRE_EQUAL(get<Block>(result->statements.at(0)).statements.size(), 1);
	CHECK_LOCATION(originLocationOf(get<Block>(result->statements.at(0)).statements.at(0)), "source0", 123, 432);

	// The next two elements have an inherited source location from the prior inner scope.
	CHECK_LOCATION(originLocationOf(result->statements.at(1)), "source0", 123, 432);
	CHECK_LOCATION(originLocationOf(result->statements.at(2)), "source0", 123, 432);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_assign_empty)
{
	// Tests single AST node (e.g. VariableDeclaration) with different source locations for each child.
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"{\n"
			"/// @src 0:123:432\n"
			"let a:bool\n"
			"/// @src 1:1:10\n"
			"a := true:bool\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0); // should still parse
	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	CHECK_LOCATION(originLocationOf(result->statements.at(0)), "source0", 123, 432);
	CHECK_LOCATION(originLocationOf(result->statements.at(1)), "source1", 1, 10);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_source_index)
{
	// Tests single AST node (e.g. VariableDeclaration) with different source locations for each child.
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"{\n"
			"/// @src 1:123:432\n"
			"let a:bool := true:bool\n"
			"/// @src 2345:0:8\n"
			"let b:bool := true:bool\n"
			"\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result); // should still parse
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 2674_error);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_mixed_locations_1)
{
	// Tests single AST node (e.g. VariableDeclaration) with different source locations for each child.
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"{\n"
			"/// @src 0:123:432\n"
			"let x:bool \n"
			"/// @src 0:234:2026\n"
			":= true:bool\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);

	BOOST_REQUIRE_EQUAL(1, result->statements.size());
	CHECK_LOCATION(originLocationOf(result->statements.at(0)), "source0", 123, 432);
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(originLocationOf(*varDecl.value), "source0", 234, 2026);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_mixed_locations_2)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:0:5
		{
			let x := /// @src 1:2:3
			add(1,   /// @src 0:4:8
			2)
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(1, result->statements.size());
	CHECK_LOCATION(result->debugData->originLocation, "source0", 0, 5);

	// `let x := add(1, `
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varDecl.debugData->originLocation, "source0", 0, 5);
	BOOST_REQUIRE(!!varDecl.value);
	BOOST_REQUIRE(holds_alternative<FunctionCall>(*varDecl.value));
	FunctionCall const& call = get<FunctionCall>(*varDecl.value);
	CHECK_LOCATION(call.debugData->originLocation, "source1", 2, 3);

	// `2`
	BOOST_REQUIRE_EQUAL(2, call.arguments.size());
	CHECK_LOCATION(originLocationOf(call.arguments.at(1)), "source0", 4, 8);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_mixed_locations_3)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 1:23:45
		{								// Block
			{							// Block
				sstore(0, 1)			// FunctionCall
				/// @src 0:420:680
			}
			mstore(1, 2)				// FunctionCall
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	CHECK_LOCATION(result->debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE(holds_alternative<Block>(result->statements.at(0)));
	Block const& innerBlock = get<Block>(result->statements.at(0));
	CHECK_LOCATION(innerBlock.debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE_EQUAL(1, innerBlock.statements.size());
	BOOST_REQUIRE(holds_alternative<ExpressionStatement>(result->statements.at(1)));
	ExpressionStatement const& sstoreStmt = get<ExpressionStatement>(innerBlock.statements.at(0));
	BOOST_REQUIRE(holds_alternative<FunctionCall>(sstoreStmt.expression));
	FunctionCall const& sstoreCall = get<FunctionCall>(sstoreStmt.expression);
	CHECK_LOCATION(sstoreCall.debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE(holds_alternative<ExpressionStatement>(result->statements.at(1)));
	ExpressionStatement mstoreStmt = get<ExpressionStatement>(result->statements.at(1));
	BOOST_REQUIRE(holds_alternative<FunctionCall>(mstoreStmt.expression));
	FunctionCall const& mstoreCall = get<FunctionCall>(mstoreStmt.expression);
	CHECK_LOCATION(mstoreCall.debugData->originLocation, "source0", 420, 680);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_comments_after_valid)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 1:23:45
		{
			/// @src 0:420:680
			/// @invalid
			let a:bool := true
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(1, result->statements.size());
	CHECK_LOCATION(result->debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varDecl.debugData->originLocation, "source0", 420, 680);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_suffix)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:420:680foo
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_prefix)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// abc@src 0:111:222
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_unspecified)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_non_integer)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src a:b:c
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_bad_integer)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 111111111111111111111:222222222222222222222:333333333333333333333
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 6367_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_ensure_last_match)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:123:432
		{
			/// @src 1:10:20
			/// @src 0:30:40
			let x:bool := true
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));

	// Ensure the latest @src per documentation-comment is used (0:30:40).
	CHECK_LOCATION(varDecl.debugData->originLocation, "source0", 30, 40);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_no_whitespace)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222@src 1:333:444
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_separated_with_single_space)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 @src 1:333:444
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source1", 333, 444);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_leading_trailing_whitespace)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = "///     @src 0:111:222    \n{}";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_reference_original_sloc)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 1:2:3
		{
			/// @src -1:10:20
			let x:bool := true
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));

	// -1 points to original source code, which in this case is `"source0"` (which is also
	CHECK_LOCATION(varDecl.debugData->originLocation, "", 10, 20);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"~~~(
		{
			/// @src 0:149:156  "new C(\"123\")"
			let x := 123

			let y := /** @src 1:96:165  "contract D {..." */ 128
		}
	)~~~";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(result->statements.size(), 2);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varX = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varX.debugData->originLocation, "source0", 149, 156);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(1)));
	VariableDeclaration const& varY = get<VariableDeclaration>(result->statements.at(1));
	BOOST_REQUIRE(!!varY.value);
	BOOST_REQUIRE(holds_alternative<Literal>(*varY.value));
	Literal const& literal128 = get<Literal>(*varY.value);
	CHECK_LOCATION(literal128.debugData->originLocation, "source1", 96, 165);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets_empty_snippet)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 ""
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets_no_whitespace_before_snippet)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222"abc" def
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets_no_whitespace_after_snippet)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 "abc"def
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_with_snippets_no_whitespace)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 "abc"@src 1:333:444 "abc"
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->debugData->originLocation, "source1", 333, 444);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_with_snippets_unterminated_quote)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 " abc @src 1:333:444
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1544_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets_with_nested_locations)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"~~~(
		{
			/// @src 0:149:156  "new C(\"123\") /// @src 1:3:4 "
			let x := 123

			let y := /** @src 1:96:165  "function f() internal { \"\/** @src 0:6:7 *\/\"; }" */ 128
		}
	)~~~";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(result->statements.size(), 2);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varX = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varX.debugData->originLocation, "source0", 149, 156);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(1)));
	VariableDeclaration const& varY = get<VariableDeclaration>(result->statements.at(1));
	BOOST_REQUIRE(!!varY.value);
	BOOST_REQUIRE(holds_alternative<Literal>(*varY.value));
	Literal const& literal128 = get<Literal>(*varY.value);
	CHECK_LOCATION(literal128.debugData->originLocation, "source1", 96, 165);
}

BOOST_AUTO_TEST_CASE(astid)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1 @ast-id 7
		{
			/** @ast-id 2 */
			function f(x) -> y {}
			mstore(1, 2)
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_CHECK(result->debugData->astID == int64_t(7));
	auto const& funDef = get<FunctionDefinition>(result->statements.at(0));
	BOOST_CHECK(funDef.debugData->astID == int64_t(2));
	BOOST_CHECK(funDef.parameters.at(0).debugData->astID == nullopt);
	BOOST_CHECK(debugDataOf(result->statements.at(1))->astID == nullopt);
}

BOOST_AUTO_TEST_CASE(astid_reset)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1 @ast-id 7 @src 1:1:1
		{
			/** @ast-id 2 */
			function f(x) -> y {}
			mstore(1, 2)
		}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_CHECK(result->debugData->astID == int64_t(7));
	auto const& funDef = get<FunctionDefinition>(result->statements.at(0));
	BOOST_CHECK(funDef.debugData->astID == int64_t(2));
	BOOST_CHECK(funDef.parameters.at(0).debugData->astID == nullopt);
	BOOST_CHECK(debugDataOf(result->statements.at(1))->astID == nullopt);
}

BOOST_AUTO_TEST_CASE(astid_multi)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1 @ast-id 7 @src 1:1:1 @ast-id 8
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_CHECK(result->debugData->astID == int64_t(8));
}

BOOST_AUTO_TEST_CASE(astid_invalid)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1 @ast-id abc @src 1:1:1
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1749_error);
	CHECK_LOCATION(result->debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(astid_too_large)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @ast-id 9223372036854775808
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1749_error);
}

BOOST_AUTO_TEST_CASE(astid_way_too_large)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @ast-id 999999999999999999999999999999999999999
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1749_error);
}

BOOST_AUTO_TEST_CASE(astid_not_fully_numeric)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @ast-id 9x
		{}
	)";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1749_error);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_multiple_src_tags_on_one_line)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"{\n"
		"    /// "
			R"~~(@src 1:2:3 ""@src 1:2:4 @src-1:2:5@src 1:2:6 @src 1:2:7     "" @src 1:2:8)~~"
			R"~~( X "@src 0:10:20 "new C(\"123\") /// @src 1:4:5 "" XYZ)~~"
			R"~~( @src0:20:30 "abc"@src0:2:4 @src-0:2:5@)~~"
			R"~~( @some text with random @ signs @@@ @- @** 1:6:7 "src 1:8:9")~~"
		"\n"
		"    let x := 123\n"
		"}\n";
	EVMDialectTyped const& dialect = EVMDialectTyped::instance(EVMVersion{});
	shared_ptr<Block> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(result->statements.size(), 1);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varX = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varX.debugData->originLocation, "source1", 4, 5);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
