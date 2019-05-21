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
#include <test/tools/evmoneRunner.h>
#include "abiV2FuzzerCommon.h"

using namespace dev::test::abiv2fuzzer;
using namespace external::evmone;
using namespace std;
using namespace dev;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	// Solidity program
	string code(reinterpret_cast<char const*>(_data), _size);
	// evmone VM instance
	EvmOneVM vm;
	// Solidity runtime bytecode
	dev::bytes byteCode;
	// Hex encoded runtime bytecode
	std::string hexEncodedRuntimeCode;
	// Hex encoded input
	std::string hexEncodedInput;

	// If we cannot compile contract, we simply bail.
	try
	{
		SolidityExecutionFramework executionFramework;
		byteCode = executionFramework.compileContract(code);
		Json::Value methodIdentifiers = executionFramework.getMethodIdentifiers();

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

	hexEncodedRuntimeCode = toHex(byteCode);
	vm.execute(hexEncodedRuntimeCode, hexEncodedInput);
	return 0;
}