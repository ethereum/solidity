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

#include <test/tools/evmoneRunner.h>
#include <iostream>
#include <assert.h>

using namespace external::evmone;
using namespace std;

evmc_host_interface EvmOneVM::interface{
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		[](evmc_context* ctx) { return static_cast<EvmOneVM*>(ctx)->tx_context; },
		{},
		{},
};

void EvmOneVM::execute(evmc_message const& _msg, std::string _runtimeCode)
{
	evmc_result result = {};

	result = vm->execute(vm, this, rev, &_msg, reinterpret_cast<uint8_t const*>(_runtimeCode.data()), _runtimeCode.size());
	cout << result.status_code << endl;
	assert(result.status_code == EVMC_SUCCESS);
	std::string output= reinterpret_cast<const char*>(result.output_data);
	assert(output == "0000000000000000000000000000000000000000000000000000000000000001");
	cout << result.output_data << endl;
}