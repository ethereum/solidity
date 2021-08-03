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
			move(indicesToSourceNames)
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
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->debugData->location, "source0", 234, 543);
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
	CHECK_LOCATION(result->debugData->location, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(3, result->statements.size());
	CHECK_LOCATION(locationOf(result->statements.at(0)), "source0", 234, 543);
	CHECK_LOCATION(locationOf(result->statements.at(1)), "source0", 123, 432);
	// [2] is inherited source location
	CHECK_LOCATION(locationOf(result->statements.at(2)), "source0", 123, 432);
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
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->debugData->location, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(3, result->statements.size());
	CHECK_LOCATION(locationOf(result->statements.at(0)), "source0", 234, 543);
	CHECK_LOCATION(locationOf(result->statements.at(1)), "source1", 123, 432);
	// [2] is inherited source location
	CHECK_LOCATION(locationOf(result->statements.at(2)), "source1", 123, 432);
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
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->debugData->location, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	CHECK_LOCATION(locationOf(result->statements.at(1)), "source0", 343, 434);
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
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->debugData->location, "source0", 234, 543);

	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	BOOST_REQUIRE(holds_alternative<Switch>(result->statements.at(1)));
	auto const& switchStmt = get<Switch>(result->statements.at(1));

	CHECK_LOCATION(switchStmt.debugData->location, "source0", 343, 434);
	BOOST_REQUIRE_EQUAL(1, switchStmt.cases.size());
	CHECK_LOCATION(switchStmt.cases.at(0).debugData->location, "source0", 3141, 59265);

	auto const& caseBody = switchStmt.cases.at(0).body;
	BOOST_REQUIRE_EQUAL(1, caseBody.statements.size());
	CHECK_LOCATION(locationOf(caseBody.statements.at(0)), "source0", 271, 828);
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
	BOOST_REQUIRE(!!result);

	CHECK_LOCATION(result->debugData->location, "source0", 1, 100);

	BOOST_REQUIRE_EQUAL(3, result->statements.size());
	CHECK_LOCATION(locationOf(result->statements.at(0)), "source0", 1, 100);

	// First child element must be a block itself with one statement.
	BOOST_REQUIRE(holds_alternative<Block>(result->statements.at(0)));
	BOOST_REQUIRE_EQUAL(get<Block>(result->statements.at(0)).statements.size(), 1);
	CHECK_LOCATION(locationOf(get<Block>(result->statements.at(0)).statements.at(0)), "source0", 123, 432);

	// The next two elements have an inherited source location from the prior inner scope.
	CHECK_LOCATION(locationOf(result->statements.at(1)), "source0", 123, 432);
	CHECK_LOCATION(locationOf(result->statements.at(2)), "source0", 123, 432);
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
	BOOST_REQUIRE(!!result); // should still parse
	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	CHECK_LOCATION(locationOf(result->statements.at(0)), "source0", 123, 432);
	CHECK_LOCATION(locationOf(result->statements.at(1)), "source1", 1, 10);
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
	BOOST_REQUIRE(!!result);

	BOOST_REQUIRE_EQUAL(1, result->statements.size());
	CHECK_LOCATION(locationOf(result->statements.at(0)), "source0", 123, 432);
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(locationOf(*varDecl.value), "source0", 234, 2026);
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
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE_EQUAL(1, result->statements.size());
	CHECK_LOCATION(result->debugData->location, "source0", 0, 5);

	// `let x := add(1, `
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varDecl.debugData->location, "source0", 0, 5);
	BOOST_REQUIRE(!!varDecl.value);
	BOOST_REQUIRE(holds_alternative<FunctionCall>(*varDecl.value));
	FunctionCall const& call = get<FunctionCall>(*varDecl.value);
	CHECK_LOCATION(call.debugData->location, "source1", 2, 3);

	// `2`
	BOOST_REQUIRE_EQUAL(2, call.arguments.size());
	CHECK_LOCATION(locationOf(call.arguments.at(1)), "source0", 4, 8);
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
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE_EQUAL(2, result->statements.size());
	CHECK_LOCATION(result->debugData->location, "source1", 23, 45);

	BOOST_REQUIRE(holds_alternative<Block>(result->statements.at(0)));
	Block const& innerBlock = get<Block>(result->statements.at(0));
	CHECK_LOCATION(innerBlock.debugData->location, "source1", 23, 45);

	BOOST_REQUIRE_EQUAL(1, innerBlock.statements.size());
	BOOST_REQUIRE(holds_alternative<ExpressionStatement>(result->statements.at(1)));
	ExpressionStatement const& sstoreStmt = get<ExpressionStatement>(innerBlock.statements.at(0));
	BOOST_REQUIRE(holds_alternative<FunctionCall>(sstoreStmt.expression));
	FunctionCall const& sstoreCall = get<FunctionCall>(sstoreStmt.expression);
	CHECK_LOCATION(sstoreCall.debugData->location, "source1", 23, 45);

	BOOST_REQUIRE(holds_alternative<ExpressionStatement>(result->statements.at(1)));
	ExpressionStatement mstoreStmt = get<ExpressionStatement>(result->statements.at(1));
	BOOST_REQUIRE(holds_alternative<FunctionCall>(mstoreStmt.expression));
	FunctionCall const& mstoreCall = get<FunctionCall>(mstoreStmt.expression);
	CHECK_LOCATION(mstoreCall.debugData->location, "source0", 420, 680);
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
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE_EQUAL(1, result->statements.size());
	CHECK_LOCATION(result->debugData->location, "source1", 23, 45);

	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));
	CHECK_LOCATION(varDecl.debugData->location, "source0", 420, 680);
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
	CHECK_LOCATION(result->debugData->location, "", -1, -1);
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
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->debugData->location, "", -1, -1);
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
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));

	// Ensure the latest @src per documentation-comment is used (0:30:40).
	CHECK_LOCATION(varDecl.debugData->location, "source0", 30, 40);
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
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(holds_alternative<VariableDeclaration>(result->statements.at(0)));
	VariableDeclaration const& varDecl = get<VariableDeclaration>(result->statements.at(0));

	// -1 points to original source code, which in this case is `"source0"` (which is also
	CHECK_LOCATION(varDecl.debugData->location, "", 10, 20);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
