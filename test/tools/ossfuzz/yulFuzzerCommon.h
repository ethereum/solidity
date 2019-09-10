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
#include <test/tools/yulInterpreter/Interpreter.h>
#include <libyul/backends/evm/EVMDialect.h>

namespace yul
{
namespace test
{
namespace yul_fuzzer
{
struct yulFuzzerUtil
{
	static void interpret(
		std::ostream& _os,
		std::shared_ptr<yul::Block> _ast,
		Dialect const& _dialect,
		size_t _maxSteps = maxSteps,
		size_t _maxTraceSize = maxTraceSize,
		size_t _maxMemory = maxMemory
	);
	static size_t constexpr maxSteps = 100;
	static size_t constexpr maxTraceSize = 75;
	static size_t constexpr maxMemory = 0x200;
};
}
}
}
