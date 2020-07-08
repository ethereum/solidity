// SPDX-License-Identifier: GPL-3.0

#include <string>

/**
 * Functions to be used for fuzz-testing of various components.
 * They throw exceptions or error.
 */
struct FuzzerUtil
{
	static void runCompiler(std::string const& _input, bool _quiet);
	static void testCompilerJsonInterface(std::string const& _input, bool _optimize, bool _quiet);
	static void testConstantOptimizer(std::string const& _input, bool _quiet);
	static void testStandardCompiler(std::string const& _input, bool _quiet);
	static void testCompiler(std::string const& _input, bool _optimize);
};
