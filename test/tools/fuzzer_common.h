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

#include <libsolutil/Common.h>

#include <map>
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
	/// Compiles @param _input which is a map of input file name to source code
	/// string with optimisation turned on if @param _optimize is true
	/// (off otherwise), a pseudo-random @param _rand that selects the EVM
	/// version to be compiled for, and bool @param _forceSMT that, if true,
	/// adds the experimental SMTChecker pragma to each source file in the
	/// source map.
	static void testCompiler(
		solidity::StringMap& _input,
		bool _optimize,
		unsigned _rand,
		bool _forceSMT,
		bool _compileViaYul
	);
	/// Adds the experimental SMTChecker pragma to each source file in the
	/// source map.
	static void forceSMT(solidity::StringMap& _input);
};
