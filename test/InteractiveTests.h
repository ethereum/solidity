// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/TestCase.h>
#include <test/libsolidity/ABIJsonTest.h>
#include <test/libsolidity/ASTJSONTest.h>
#include <test/libsolidity/GasTest.h>
#include <test/libsolidity/SyntaxTest.h>
#include <test/libsolidity/SemanticTest.h>
#include <test/libsolidity/SMTCheckerTest.h>
#include <test/libsolidity/SMTCheckerJSONTest.h>
#include <test/libyul/EwasmTranslationTest.h>
#include <test/libyul/YulOptimizerTest.h>
#include <test/libyul/YulInterpreterTest.h>
#include <test/libyul/ObjectCompilerTest.h>
#include <test/libyul/FunctionSideEffects.h>
#include <test/libyul/SyntaxTest.h>

#include <boost/filesystem.hpp>

namespace solidity::frontend::test
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
	{"Ewasm Translation",   "libyul",      "ewasmTranslationTests",false,false, &yul::test::EwasmTranslationTest::create},
	{"Yul Optimizer",       "libyul",      "yulOptimizerTests",   false, false, &yul::test::YulOptimizerTest::create},
	{"Yul Interpreter",     "libyul",      "yulInterpreterTests", false, false, &yul::test::YulInterpreterTest::create},
	{"Yul Object Compiler", "libyul",      "objectCompiler",      false, false, &yul::test::ObjectCompilerTest::create},
	{"Function Side Effects","libyul",     "functionSideEffects", false, false, &yul::test::FunctionSideEffects::create},
	{"Yul Syntax",          "libyul",      "yulSyntaxTests",      false, false, &yul::test::SyntaxTest::create},
	{"Syntax",              "libsolidity", "syntaxTests",         false, false, &SyntaxTest::create},
	{"Error Recovery",      "libsolidity", "errorRecoveryTests",  false, false, &SyntaxTest::createErrorRecovery},
	{"Semantic",            "libsolidity", "semanticTests",       false, true,  &SemanticTest::create},
	{"JSON AST",            "libsolidity", "ASTJSON",             false, false, &ASTJSONTest::create},
	{"JSON ABI",            "libsolidity", "ABIJson",             false, false, &ABIJsonTest::create},
	{"SMT Checker",         "libsolidity", "smtCheckerTests",     true,  false, &SMTCheckerTest::create},
	{"SMT Checker JSON",    "libsolidity", "smtCheckerTestsJSON", true,  false, &SMTCheckerJSONTest::create},
	{"Gas Estimates",       "libsolidity", "gasTests",            false, false, &GasTest::create}
};

}
