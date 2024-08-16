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
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>
#include <optional>
#include <string>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;

BOOST_TEST_DONT_PRINT_LOG_VALUE(ErrorId)
BOOST_TEST_DONT_PRINT_LOG_VALUE(Error::Type)

namespace solidity::yul::test
{

namespace
{

std::shared_ptr<AST> parse(std::string const& _source, Dialect const& _dialect, ErrorReporter& errorReporter)
{
	auto stream = CharStream(_source, "");
	std::map<unsigned, std::shared_ptr<std::string const>> indicesToSourceNames;
	indicesToSourceNames[0] = std::make_shared<std::string const>("source0");
	indicesToSourceNames[1] = std::make_shared<std::string const>("source1");

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
		).analyze(parserResult->root()))
			return parserResult;
	}
	return {};
}

std::optional<Error> parseAndReturnFirstError(std::string const& _source, Dialect const& _dialect, bool _allowWarningsAndInfos = true)
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

bool successParse(std::string const& _source, Dialect const& _dialect, bool _allowWarningsAndInfos = true)
{
	return !parseAndReturnFirstError(_source, _dialect, _allowWarningsAndInfos);
}

Error expectError(std::string const& _source, Dialect const& _dialect, bool _allowWarningsAndInfos = false)
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
		BuiltinFunction const* builtin(YulName _name) const override
		{
			return _name == "builtin"_yulname ? &f : nullptr;
		}
		BuiltinFunction f{"builtin"_yulname, 2, 3, {}, {}, false, {}};
	};

	SimpleDialect dialect;
	BOOST_CHECK(successParse("{ let a, b, c := builtin(1, 2) }", dialect));
	CHECK_ERROR_DIALECT("{ let a, b, c := builtin(1) }", TypeError, "Function \"builtin\" expects 2 arguments but got 1", dialect);
	CHECK_ERROR_DIALECT("{ let a, b := builtin(1, 2) }", DeclarationError, "Variable count mismatch for declaration of \"a, b\": 2 variables and 3 values.", dialect);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 234, 543);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_block_with_children)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{\n"
			"let x := true\n"
			"/// @src 0:123:432\n"
			"let z := true\n"
			"let y := add(1, 2)\n"
		"}\n";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(3, result->root().statements.size());
	CHECK_LOCATION(originLocationOf(result->root().statements.at(0)), "source0", 234, 543);
	CHECK_LOCATION(originLocationOf(result->root().statements.at(1)), "source0", 123, 432);
	// [2] is inherited source location
	CHECK_LOCATION(originLocationOf(result->root().statements.at(2)), "source0", 123, 432);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_block_different_sources)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"/// @src 0:234:543\n"
		"{\n"
			"let x := true\n"
			"/// @src 1:123:432\n"
			"let z := true\n"
			"let y := add(1, 2)\n"
		"}\n";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(3, result->root().statements.size());
	CHECK_LOCATION(originLocationOf(result->root().statements.at(0)), "source0", 234, 543);
	CHECK_LOCATION(originLocationOf(result->root().statements.at(1)), "source1", 123, 432);
	// [2] is inherited source location
	CHECK_LOCATION(originLocationOf(result->root().statements.at(2)), "source1", 123, 432);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 234, 543);
	BOOST_REQUIRE_EQUAL(2, result->root().statements.size());
	CHECK_LOCATION(originLocationOf(result->root().statements.at(1)), "source0", 343, 434);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 234, 543);

	BOOST_REQUIRE_EQUAL(2, result->root().statements.size());
	BOOST_REQUIRE(std::holds_alternative<Switch>(result->root().statements.at(1)));
	auto const& switchStmt = std::get<Switch>(result->root().statements.at(1));

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
				"let x := true\n"
			"}\n"
			"let z := true\n"
			"let y := add(1, 2)\n"
		"}\n";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);

	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 1, 100);

	BOOST_REQUIRE_EQUAL(3, result->root().statements.size());
	CHECK_LOCATION(originLocationOf(result->root().statements.at(0)), "source0", 1, 100);

	// First child element must be a block itself with one statement.
	BOOST_REQUIRE(std::holds_alternative<Block>(result->root().statements.at(0)));
	BOOST_REQUIRE_EQUAL(std::get<Block>(result->root().statements.at(0)).statements.size(), 1);
	CHECK_LOCATION(originLocationOf(std::get<Block>(result->root().statements.at(0)).statements.at(0)), "source0", 123, 432);

	// The next two elements have an inherited source location from the prior inner scope.
	CHECK_LOCATION(originLocationOf(result->root().statements.at(1)), "source0", 123, 432);
	CHECK_LOCATION(originLocationOf(result->root().statements.at(2)), "source0", 123, 432);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_assign_empty)
{
	// Tests single AST node (e.g. VariableDeclaration) with different source locations for each child.
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"{\n"
			"/// @src 0:123:432\n"
			"let a\n"
			"/// @src 1:1:10\n"
			"a := true\n"
		"}\n";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0); // should still parse
	BOOST_REQUIRE_EQUAL(2, result->root().statements.size());
	CHECK_LOCATION(originLocationOf(result->root().statements.at(0)), "source0", 123, 432);
	CHECK_LOCATION(originLocationOf(result->root().statements.at(1)), "source1", 1, 10);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_source_index)
{
	// Tests single AST node (e.g. VariableDeclaration) with different source locations for each child.
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText =
		"{\n"
			"/// @src 1:123:432\n"
			"let a := true\n"
			"/// @src 2345:0:8\n"
			"let b := true\n"
			"\n"
		"}\n";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
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
			"let x \n"
			"/// @src 0:234:2026\n"
			":= true\n"
		"}\n";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);

	BOOST_REQUIRE_EQUAL(1, result->root().statements.size());
	CHECK_LOCATION(originLocationOf(result->root().statements.at(0)), "source0", 123, 432);
	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varDecl = std::get<VariableDeclaration>(result->root().statements.at(0));
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(1, result->root().statements.size());
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 0, 5);

	// `let x := add(1, `
	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varDecl = std::get<VariableDeclaration>(result->root().statements.at(0));
	CHECK_LOCATION(varDecl.debugData->originLocation, "source0", 0, 5);
	BOOST_REQUIRE(!!varDecl.value);
	BOOST_REQUIRE(std::holds_alternative<FunctionCall>(*varDecl.value));
	FunctionCall const& call = std::get<FunctionCall>(*varDecl.value);
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
		{                               // Block
			{                           // Block
				sstore(0, 1)            // FunctionCall
				/// @src 0:420:680
			}
			mstore(1, 2)                // FunctionCall
		}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(2, result->root().statements.size());
	CHECK_LOCATION(result->root().debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE(std::holds_alternative<Block>(result->root().statements.at(0)));
	Block const& innerBlock = std::get<Block>(result->root().statements.at(0));
	CHECK_LOCATION(innerBlock.debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE_EQUAL(1, innerBlock.statements.size());
	BOOST_REQUIRE(std::holds_alternative<ExpressionStatement>(result->root().statements.at(1)));
	ExpressionStatement const& sstoreStmt = std::get<ExpressionStatement>(innerBlock.statements.at(0));
	BOOST_REQUIRE(std::holds_alternative<FunctionCall>(sstoreStmt.expression));
	FunctionCall const& sstoreCall = std::get<FunctionCall>(sstoreStmt.expression);
	CHECK_LOCATION(sstoreCall.debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE(std::holds_alternative<ExpressionStatement>(result->root().statements.at(1)));
	ExpressionStatement mstoreStmt = std::get<ExpressionStatement>(result->root().statements.at(1));
	BOOST_REQUIRE(std::holds_alternative<FunctionCall>(mstoreStmt.expression));
	FunctionCall const& mstoreCall = std::get<FunctionCall>(mstoreStmt.expression);
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
			let a := true
		}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(1, result->root().statements.size());
	CHECK_LOCATION(result->root().debugData->originLocation, "source1", 23, 45);

	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varDecl = std::get<VariableDeclaration>(result->root().statements.at(0));
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_prefix)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// abc@src 0:111:222
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_unspecified)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_non_integer)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src a:b:c
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_bad_integer)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 111111111111111111111:222222222222222222222:333333333333333333333
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 6367_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
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
			let x := true
		}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varDecl = std::get<VariableDeclaration>(result->root().statements.at(0));

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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_separated_with_single_space)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 @src 1:333:444
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source1", 333, 444);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_leading_trailing_whitespace)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = "///     @src 0:111:222    \n{}";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_reference_original_sloc)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 1:2:3
		{
			/// @src -1:10:20
			let x := true
		}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varDecl = std::get<VariableDeclaration>(result->root().statements.at(0));

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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(result->root().statements.size(), 2);

	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varX = std::get<VariableDeclaration>(result->root().statements.at(0));
	CHECK_LOCATION(varX.debugData->originLocation, "source0", 149, 156);

	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(1)));
	VariableDeclaration const& varY = std::get<VariableDeclaration>(result->root().statements.at(1));
	BOOST_REQUIRE(!!varY.value);
	BOOST_REQUIRE(std::holds_alternative<Literal>(*varY.value));
	Literal const& literal128 = std::get<Literal>(*varY.value);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets_no_whitespace_before_snippet)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222"abc" def
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 8387_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_with_code_snippets_no_whitespace_after_snippet)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 "abc"def
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_with_snippets_no_whitespace)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 "abc"@src 1:333:444 "abc"
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source1", 333, 444);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_locations_with_snippets_unterminated_quote)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 " abc @src 1:333:444
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1544_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_single_quote)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 "
		///
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1544_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_two_snippets_with_hex_comment)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 hex"abc"@src 1:333:444 "abc"
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	// the second source location is not parsed as such, as the hex string isn't interpreted as snippet but
	// as the beginning of the tail in AsmParser
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_invalid_escapes)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 "\n\\x\x\w\uö\xy\z\y\fq"
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 111, 222);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_single_quote_snippet_with_whitespaces_and_escapes)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 0:111:222 '\n\\x\x\w\uö\xy\z\y\fq'
		/// @src 1 :		222 : 333 '\x33\u1234\t\n'
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	CHECK_LOCATION(result->root().debugData->originLocation, "source1", 222, 333);
}

BOOST_DATA_TEST_CASE(customSourceLocations_scanner_errors_outside_string_lits_are_ignored, boost::unit_test::data::make({"0x ", "/** unterminated comment", "1_23_4"}), invalid)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = fmt::format(R"(
		/// @src 0:111:222 {}
		/// @src 1:222:333
		{{}}
	)", invalid);
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.empty());
	CHECK_LOCATION(result->root().debugData->originLocation, "source1", 222, 333);
}

BOOST_AUTO_TEST_CASE(customSourceLocations_multi_line_source_loc)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src 1	: 111:
		/// 222 "
		/// abc\"def
		///
		/// " @src 0:333:444
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.empty());
	CHECK_LOCATION(result->root().debugData->originLocation, "source0", 333, 444);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(result->root().statements.size(), 2);

	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varX = std::get<VariableDeclaration>(result->root().statements.at(0));
	CHECK_LOCATION(varX.debugData->originLocation, "source0", 149, 156);

	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(1)));
	VariableDeclaration const& varY = std::get<VariableDeclaration>(result->root().statements.at(1));
	BOOST_REQUIRE(!!varY.value);
	BOOST_REQUIRE(std::holds_alternative<Literal>(*varY.value));
	Literal const& literal128 = std::get<Literal>(*varY.value);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_CHECK(result->root().debugData->astID == int64_t(7));
	auto const& funDef = std::get<FunctionDefinition>(result->root().statements.at(0));
	BOOST_CHECK(funDef.debugData->astID == int64_t(2));
	BOOST_CHECK(funDef.parameters.at(0).debugData->astID == std::nullopt);
	BOOST_CHECK(debugDataOf(result->root().statements.at(1))->astID == std::nullopt);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_CHECK(result->root().debugData->astID == int64_t(7));
	auto const& funDef = std::get<FunctionDefinition>(result->root().statements.at(0));
	BOOST_CHECK(funDef.debugData->astID == int64_t(2));
	BOOST_CHECK(funDef.parameters.at(0).debugData->astID == std::nullopt);
	BOOST_CHECK(debugDataOf(result->root().statements.at(1))->astID == std::nullopt);
}

BOOST_AUTO_TEST_CASE(astid_multi)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1 @ast-id 7 @src 1:1:1 @ast-id 8
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_CHECK(result->root().debugData->astID == int64_t(8));
}

BOOST_AUTO_TEST_CASE(astid_invalid)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @src -1:-1:-1 @ast-id abc @src 1:1:1
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result);
	BOOST_REQUIRE(errorList.size() == 1);
	BOOST_TEST(errorList[0]->type() == Error::Type::SyntaxError);
	BOOST_TEST(errorList[0]->errorId() == 1749_error);
	CHECK_LOCATION(result->root().debugData->originLocation, "", -1, -1);
}

BOOST_AUTO_TEST_CASE(astid_too_large)
{
	ErrorList errorList;
	ErrorReporter reporter(errorList);
	auto const sourceText = R"(
		/// @ast-id 9223372036854775808
		{}
	)";
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
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
	auto const& dialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	std::shared_ptr<AST> result = parse(sourceText, dialect, reporter);
	BOOST_REQUIRE(!!result && errorList.size() == 0);
	BOOST_REQUIRE_EQUAL(result->root().statements.size(), 1);

	BOOST_REQUIRE(std::holds_alternative<VariableDeclaration>(result->root().statements.at(0)));
	VariableDeclaration const& varX = std::get<VariableDeclaration>(result->root().statements.at(0));
	CHECK_LOCATION(varX.debugData->originLocation, "source1", 4, 5);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
