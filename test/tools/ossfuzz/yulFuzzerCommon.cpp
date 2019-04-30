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
#include <test/tools/ossfuzz/yulFuzzerCommon.h>

using namespace std;
using namespace yul;
using namespace yul::test::yul_fuzzer;

void yulFuzzerUtil::interpret(
	ostream& _os,
	shared_ptr<yul::Block> _ast,
	Dialect const& _dialect,
	size_t _maxSteps,
	size_t _maxTraceSize,
	size_t _maxMemory
)
{
	InterpreterState state;
	state.maxTraceSize = _maxTraceSize;
	state.maxSteps = _maxSteps;
	state.maxMemSize = _maxMemory;
	Interpreter interpreter(state, _dialect);
	interpreter(*_ast);
	_os << "Trace:" << endl;
	for (auto const& line: interpreter.trace())
		_os << "  " << line << endl;
}
