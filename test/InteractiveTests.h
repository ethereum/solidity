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

#pragma once

#include <test/TestCase.h>
#include <test/libsolidity/ABIJsonTest.h>
#include <test/libsolidity/ASTJSONTest.h>
#include <test/libsolidity/GasTest.h>
#include <test/libsolidity/SyntaxTest.h>
#include <test/libsolidity/SemanticTest.h>
#include <test/libsolidity/SMTCheckerJSONTest.h>
#include <test/libyul/YulOptimizerTest.h>
#include <test/libyul/YulInterpreterTest.h>
#include <test/libyul/ObjectCompilerTest.h>

#include <boost/filesystem.hpp>

namespace dev
{
namespace solidity
{
namespace test
{

/** Container for all information regarding a testsuite */
struct Testsuite
{
	char const* title;
	boost::filesystem::path const path;
	boost::filesystem::path const subpath;
	bool smt;
	bool needsVM;
	TestCase::TestCaseCreator testCaseCreator;
};


/// Array of testsuits that can be run interactively as well as automatically
Testsuite const g_interactiveTestsuites[] = {
/*
	Title                   Path           Subpath                SMT   NeedsVM Creator function */
	{"Yul Optimizer",       "libyul",      "yulOptimizerTests",   false, false, &yul::test::YulOptimizerTest::create},
	{"Yul Interpreter",     "libyul",      "yulInterpreterTests", false, false, &yul::test::YulInterpreterTest::create},
	{"Yul Object Compiler", "libyul",      "objectCompiler",      false, false, &yul::test::ObjectCompilerTest::create},
	{"Syntax",              "libsolidity", "syntaxTests",         false, false, &SyntaxTest::create},
	{"Error Recovery",      "libsolidity", "errorRecoveryTests",  false, false, &SyntaxTest::createErrorRecovery},
	{"Semantic",            "libsolidity", "semanticTests",       false, true,  &SemanticTest::create},
	{"JSON AST",            "libsolidity", "ASTJSON",             false, false, &ASTJSONTest::create},
	{"JSON ABI",            "libsolidity", "ABIJson",             false, false, &ABIJsonTest::create},
	{"SMT Checker",         "libsolidity", "smtCheckerTests",     true,  false, &SyntaxTest::create},
	{"SMT Checker JSON",    "libsolidity", "smtCheckerTestsJSON", true,  false, &SMTCheckerTest::create},
	{"Gas Estimates",       "libsolidity", "gasTests",            false, false, &GasTest::create}
};

}
}
}
