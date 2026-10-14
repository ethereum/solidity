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

#pragma once

#include <test/TestCase.h>
#include <test/libsolidity/ABIJsonTest.h>
#include <test/libsolidity/ASTJSONTest.h>
#include <test/libsolidity/ASTPropertyTest.h>
#include <test/libsolidity/EthdebugTest.h>
#include <test/libsolidity/GasTest.h>
#include <test/libsolidity/MemoryGuardTest.h>
#include <test/libsolidity/NatspecJSONTest.h>
#include <test/libsolidity/OptimizedIRCachingTest.h>
#include <test/libsolidity/SyntaxTest.h>
#include <test/libsolidity/SemanticTest.h>
#include <test/libsolidity/SMTCheckerTest.h>

#include <test/libyul/ssa/CallGraphTest.h>
#include <test/libyul/ssa/ControlFlowGraphTest.h>
#include <test/libyul/ssa/ParserTest.h>
#include <test/libyul/ssa/PrinterTest.h>
#include <test/libyul/ssa/SpillTest.h>
#include <test/libyul/ssa/StackLayoutGeneratorTest.h>
#include <test/libyul/ssa/StackShufflerTest.h>
#include <test/libyul/ControlFlowGraphTest.h>
#include <test/libyul/EVMCodeTransformTest.h>
#include <test/libyul/YulOptimizerTest.h>
#include <test/libyul/CallGraphTest.h>
#include <test/libyul/YulInterpreterTest.h>
#include <test/libyul/ObjectCompilerTest.h>
#include <test/libyul/ControlFlowSideEffectsTest.h>
#include <test/libyul/FunctionSideEffects.h>
#include <test/libyul/StackLayoutGeneratorTest.h>
#include <test/libyul/StackShufflingTest.h>
#include <test/libyul/SyntaxTest.h>

#include <test/libevmasm/EVMAssemblyTest.h>

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
	std::vector<std::string> labels{};
};


/// Array of testsuits that can be run interactively as well as automatically
Testsuite const g_interactiveTestsuites[] = {
/*
	Title                           Path           Subpath                          SMT   NeedsVM Creator function */
	{"EVM Assembly",                "libevmasm",   "evmAssemblyTests",              false, false, &evmasm::test::EVMAssemblyTest::create},
	{"Yul Optimizer",               "libyul",      "yulOptimizerTests",             false, false, &yul::test::YulOptimizerTest::create},
	{"Yul Interpreter",             "libyul",      "yulInterpreterTests",           false, false, &yul::test::YulInterpreterTest::create},
	{"Yul Object Compiler",         "libyul",      "objectCompiler",                false, false, &yul::test::ObjectCompilerTest::create},
	{"Yul Control Flow Graph",      "libyul",      "yulControlFlowGraph",           false, false, &yul::test::ControlFlowGraphTest::create},
	{"Yul Call Graph",              "libyul",      "yulCallGraph",                  false, false, &yul::test::CallGraphTest::create},
	{"Yul SSA Control Flow Graph",  "libyul",      "ssa/controlFlowGraph",          false, false, &yul::test::ssa::ControlFlowGraphTest::create},
	{"Yul SSA Call Graph",          "libyul",      "ssa/callGraph",                 false, false, &yul::test::ssa::CallGraphTest::create},
	{"Yul SSA Printer",             "libyul",      "ssa/printer",                   false, false, &yul::test::ssa::PrinterTest::create},
	{"Yul SSA Parser",              "libyul",      "ssa/parser",                    false, false, &yul::test::ssa::ParserTest::create},
	{"Yul SSA StackShuffling",      "libyul",      "ssa/stackShuffler",             false, false, &yul::test::ssa::ShufflingTest::create},
	{"Yul SSA StackLayoutGenerator","libyul",      "ssa/stackLayoutGenerator",      false, false, &yul::test::ssa::StackLayoutGeneratorTest::create},
	{"Yul SSA Spill",               "libyul",      "ssa/spill",                     false, false, &yul::test::ssa::SpillTest::create},
	{"Yul Stack Layout",            "libyul",      "yulStackLayout",                false, false, &yul::test::StackLayoutGeneratorTest::create},
	{"Yul Stack Shuffling",         "libyul",      "yulStackShuffling",             false, false, &yul::test::StackShufflingTest::create},
	{"Control Flow Side Effects",   "libyul",      "controlFlowSideEffects",        false, false, &yul::test::ControlFlowSideEffectsTest::create},
	{"Function Side Effects",       "libyul",      "functionSideEffects",           false, false, &yul::test::FunctionSideEffects::create},
	{"Yul Syntax",                  "libyul",      "yulSyntaxTests",                false, false, &yul::test::SyntaxTest::create},
	{"EVM Code Transform",          "libyul",      "evmCodeTransform",              false, false, &yul::test::EVMCodeTransformTest::create, {"nooptions"}},
	{"Syntax",                      "libsolidity", "syntaxTests",                   false, false, &SyntaxTest::create},
	{"Semantic",                    "libsolidity", "semanticTests",                 false, true,  &SemanticTest::create},
	{"JSON AST",                    "libsolidity", "ASTJSON",                       false, false, &ASTJSONTest::create},
	{"JSON ABI",                    "libsolidity", "ABIJson",                       false, false, &ABIJsonTest::create},
	{"JSON Natspec",                "libsolidity", "natspecJSON",                   false, false, &NatspecJSONTest::create},
	{"SMT Checker",                 "libsolidity", "smtCheckerTests",               true,  false, &SMTCheckerTest::create},
	{"Ethdebug",                    "libsolidity", "ethdebugTests",                 false, false, &EthdebugTest::create},
	{"Gas Estimates",               "libsolidity", "gasTests",                      false, false, &GasTest::create},
	{"Memory Guard",                "libsolidity", "memoryGuardTests",              false, false, &MemoryGuardTest::create},
	{"AST Properties",              "libsolidity", "astPropertyTests",              false, false, &ASTPropertyTest::create},
	{"Optimized IR Caching",        "libsolidity", "optimizedIRCaching",            false, false, &OptimizedIRCachingTest::create},
};

}
