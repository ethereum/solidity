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
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Unit tests for the arithmetic overflow checker.
 */

#include <test/libsolidity/ErrorCheck.h>

#include <test/TestHelper.h>

#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/StaticAnalyzer.h>
#include <libsolidity/analysis/PostTypeChecker.h>
#include <libsolidity/analysis/SyntaxChecker.h>
#include <libsolidity/analysis/OverflowChecker.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <libsolidity/analysis/GlobalContext.h>
#include <libsolidity/analysis/TypeChecker.h>

#include <libdevcore/SHA3.h>

#include <string>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

pair<ASTPointer<SourceUnit>, std::shared_ptr<Error const>>
parseAnalyseAndReturnError(string const& _source, bool _reportWarnings = false, bool _insertVersionPragma = true, bool _allowMultipleErrors = false)
{
	// Turn on integer overflow checking
        string source = "pragma analyzeIntegerOverflow;\n" + _source;
	// Silence compiler version warning
	source = _insertVersionPragma ? "pragma solidity >=0.0;\n" + _source : _source;
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	Parser parser(errorReporter);
	ASTPointer<SourceUnit> sourceUnit;
	// catch exceptions for a transition period
	try
	{
		sourceUnit = parser.parse(std::make_shared<Scanner>(CharStream(source)));
		if(!sourceUnit)
			BOOST_FAIL("Parsing failed in type checker test.");

		SyntaxChecker syntaxChecker(errorReporter);
		if (!syntaxChecker.checkSyntax(*sourceUnit))
			return make_pair(sourceUnit, errorReporter.errors().at(0));

		std::shared_ptr<GlobalContext> globalContext = make_shared<GlobalContext>();
		map<ASTNode const*, shared_ptr<DeclarationContainer>> scopes;
		NameAndTypeResolver resolver(globalContext->declarations(), scopes, errorReporter);
		solAssert(Error::containsOnlyWarnings(errorReporter.errors()), "");
		resolver.registerDeclarations(*sourceUnit);

		bool success = true;
		for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				globalContext->setCurrentContract(*contract);
				resolver.updateDeclaration(*globalContext->currentThis());
				resolver.updateDeclaration(*globalContext->currentSuper());
				if (!resolver.resolveNamesAndTypes(*contract))
					success = false;
			}
		if (success)
			for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
				if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
				{
					globalContext->setCurrentContract(*contract);
					resolver.updateDeclaration(*globalContext->currentThis());

					TypeChecker typeChecker(errorReporter);
					bool success = typeChecker.checkTypeRequirements(*contract);
					BOOST_CHECK(success || !errorReporter.errors().empty());
				}
		if (success)
			if (!PostTypeChecker(errorReporter).check(*sourceUnit))
				success = false;
		if (success)
			if (!StaticAnalyzer(errorReporter).analyze(*sourceUnit))
				success = false;
		if (success)
			OverflowChecker(errorReporter).checkOverflow(*sourceUnit);
		if (errorReporter.errors().size() > 1 && !_allowMultipleErrors)
			BOOST_FAIL("Multiple errors found");
		for (auto const& currentError: errorReporter.errors())
		{
			if (
				(_reportWarnings && currentError->type() == Error::Type::Warning) ||
				(!_reportWarnings && currentError->type() != Error::Type::Warning)
			)
				return make_pair(sourceUnit, currentError);
		}
	}
	catch (InternalCompilerError const& _e)
	{
		string message("Internal compiler error");
		if (string const* description = boost::get_error_info<errinfo_comment>(_e))
			message += ": " + *description;
		BOOST_FAIL(message);
	}
	catch (Error const& _e)
	{
		return make_pair(sourceUnit, std::make_shared<Error const>(_e));
	}
	catch (...)
	{
		BOOST_FAIL("Unexpected exception.");
	}
	return make_pair(sourceUnit, nullptr);
}

bool success(string const& _source)
{
	return !parseAnalyseAndReturnError(_source).second;
}

Error expectError(std::string const& _source, bool _warning = false, bool _allowMultiple = false)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source, _warning, true, _allowMultiple);
	BOOST_REQUIRE(!!sourceAndError.second);
	BOOST_REQUIRE(!!sourceAndError.first);
	return *sourceAndError.second;
}

#define CHECK_ERROR_OR_WARNING(text, typ, substring, warning, allowMulti) \
do \
{ \
	Error err = expectError((text), (warning), (allowMulti)); \
	BOOST_CHECK(err.type() == (Error::Type::typ)); \
	BOOST_CHECK(searchErrorMessage(err, (substring))); \
} while(0)

// [checkError(text, type, substring)] asserts that the compilation down to overflow analysis
// emits an error of type [type] and with a message containing [substring].
#define CHECK_ERROR(text, type, substring) \
CHECK_ERROR_OR_WARNING(text, type, substring, false, false)

// [checkError(text, type, substring)] asserts that the compilation down to overflow analysis
// emits an error of type [type] and with a message containing [substring].
#define CHECK_ERROR_ALLOW_MULTI(text, type, substring) \
CHECK_ERROR_OR_WARNING(text, type, substring, false, true)

// [checkWarning(text, type, substring)] asserts that the compilation down to overflow analysis
// emits a warning of type [type] and with a message containing [substring].
#define CHECK_WARNING(text, substring) \
CHECK_ERROR_OR_WARNING(text, Warning, substring, true, true)

// [checkSuccess(text)] asserts that the compilation down to typechecking succeeds.
#define CHECK_SUCCESS(text) do { BOOST_CHECK(success((text))); } while(0)

#define CHECK_SUCCESS_NO_WARNINGS(text) \
do \
{ \
	auto sourceAndError = parseAnalyseAndReturnError((text), true); \
	BOOST_CHECK(sourceAndError.second == nullptr); \
} \
while(0)


BOOST_AUTO_TEST_SUITE(OverflowChecking)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = R"(
		contract test {
			function fun(uint8 arg1) { uint256 y; y = arg1; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(warn_overflow_arithmetic)
{
	char const* text = R"(
		contract C {
			function f(uint8 a, uint8 b) returns (uint8) {
				uint8 c = a+b;
				return c;
			}
		}
	)";
	CHECK_WARNING(text, "overflow");

	char const* text2 = R"(
		contract C {
			function f(uint8 a, uint8 b) returns (uint8) {
				uint8 c = a-b;
				return c;
			}
		}
	)";
	CHECK_WARNING(text2, "overflow");

	char const* text3 = R"(
		contract C {
			function f(uint8 a, uint8 b) returns (uint8) {
				uint8 c = a*b;
				return c;
			}
		}
	)";
	CHECK_WARNING(text3, "overflow");
}

BOOST_AUTO_TEST_CASE(require_test)
{
	char const* text = R"(
		contract test {
			function fun(uint8 arg1, uint8 arg2) returns (uint8) { uint8 y = arg1 + arg2; return y; }
		}
	)";
	CHECK_WARNING(text, "overflow");

	char const* text2 = R"(
		contract test {
			function fun(uint8 arg1, uint8 arg2) returns (uint8) { require(arg1 < 1); uint8 y = arg1 + arg2; return y; }
		}
	)";
	CHECK_SUCCESS(text2);
}

BOOST_AUTO_TEST_CASE(ifelse_test)
{
	char const* text = R"(
		contract test {
			function fun(uint8 arg1, uint8 arg2) returns (uint8) { uint8 y = arg1 + arg2; return y; }
		}
	)";
	CHECK_WARNING(text, "overflow");

	char const* text2 = R"(
		contract test {
			function fun(uint8 arg1, uint8 arg2) returns (uint8) { uint8 y; if (arg1 < 1) y = arg1 + arg2; else y = arg1; return y; }
		}
	)";
	CHECK_SUCCESS(text2);
}



BOOST_AUTO_TEST_SUITE_END()

}
}
}
} // end namespaces
