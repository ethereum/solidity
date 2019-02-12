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

#include <string>

struct FuzzerUtil
{
	static void runCompiler(std::string const& _input, bool _quiet);
	static void testCompiler(std::string const& _input, bool _optimize, bool _quiet);
	static void testConstantOptimizer(std::string const& _input, bool _quiet);
	static void testStandardCompiler(std::string const& _input, bool _quiet);
};
