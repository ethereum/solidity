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

#include <fstream>
#include <test/tools/ossfuzz/abiV2FuzzerCommon.h>

using namespace dev::test::abiv2fuzzer;
using namespace std;
using namespace dev;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	string solidityProgram(reinterpret_cast<char const*>(_data), _size);
	dev::bytes runtimeByteCode;
	std::string hexEncodedInput;

	// If we cannot compile contract, we simply bail.
	try
	{
		SolidityCompilationFramework solCompilationFramework;
		runtimeByteCode = solCompilationFramework.compileContract(solidityProgram);
		Json::Value methodIdentifiers = solCompilationFramework.getMethodIdentifiers();

		// Call the first function
	    for (auto const& id : methodIdentifiers.getMemberNames())
	    {
		    hexEncodedInput = methodIdentifiers[id].asString();
		    break;
	    }
	}
	catch (...)
	{
		return 0;
	}

	std::string hexEncodedRuntimeCode = toHex(runtimeByteCode);
	return 0;
}
