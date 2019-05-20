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

	auto code = from_hex(_runtimeCode.data());
	result = vm->execute(vm, this, rev, &_msg, &code[0], code.size());
	cout << result.status_code << endl;
	assert(result.status_code == EVMC_SUCCESS);
	std::string output = std::string{reinterpret_cast<const char*>(result.output_data), result.output_size};
	assert(output == "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01");
	cout << result.output_data << endl;
}

std::basic_string<uint8_t> EvmOneVM::from_hex(std::string _hex)
{
    if (_hex.length() % 2 == 1)
        throw std::length_error{"the length of the input is odd"};

	std::basic_string<uint8_t> bs;
    int b = 0;
    for (size_t i = 0; i < _hex.size(); ++i)
    {
        auto h = _hex[i];
        int v;
        if (h >= '0' && h <= '9')
            v = h - '0';
        else if (h >= 'a' && h <= 'f')
            v = h - 'a' + 10;
        else if (h >= 'A' && h <= 'F')
            v = h - 'A' + 10;
        else
            throw std::out_of_range{"not a hex digit"};

        if (i % 2 == 0)
            b = v << 4;
        else
            bs.push_back(static_cast<uint8_t>(b | v));
    }
    return bs;
}