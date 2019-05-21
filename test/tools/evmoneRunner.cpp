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
#include <fstream>
#include <limits>

using namespace external::evmone;
using namespace std;

evmc_host_interface EvmOneVM::m_interface{
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		[](evmc_context* ctx) { return static_cast<EvmOneVM*>(ctx)->m_txContext; },
		{},
		{},
};

EvmOneVM::EvmOneVM(evmc_revision const& _evmcRevision): evmc_context{&m_interface}, m_vm{evmc_create_evmone()}
{
	m_rev = _evmcRevision;
}

void EvmOneVM::execute(evmc_message const& _m, std::basic_string<uint8_t> _code)
{
	evmc_result result;
	result = m_vm->execute(m_vm, this, m_rev, &_m, _code.data(), _code.size());
	assert(result.status_code == EVMC_SUCCESS);
	// Checking if execution output is true or 1
	//	assert(result.output_data[31] == '\x01');
}

void EvmOneVM::execute(int64_t gas, std::basic_string<uint8_t> _code, std::string _hexInput)
{
	auto input = fromHex(_hexInput);
	evmc_message msg = {};
	msg.gas = gas;
	msg.input_data = input.data();
	msg.input_size = input.size();
	execute(msg, _code);
}

void EvmOneVM::execute(std::basic_string<uint8_t> _code, std::string _hexInput)
{
	execute(std::numeric_limits<int64_t>::max(), _code, _hexInput);
}

void EvmOneVM::execute(int64_t _gas, std::string _code, std::string _input)
{
	execute(_gas, fromHex(_code), _input);
}

void EvmOneVM::execute(std::string _code, std::string _hexInput)
{
	execute(std::numeric_limits<int64_t>::max(), _code, _hexInput);
}

// Converts hex encoded string to its raw byte representation
std::basic_string<uint8_t> EvmOneVM::fromHex(std::string _hex)
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