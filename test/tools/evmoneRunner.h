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

#pragma once

#include <deps/evmone/include/evmone/evmone.h>
#include <string>

namespace external
{
namespace evmone
{
class EvmOneVM: public evmc_context
{
public:
	explicit EvmOneVM(evmc_revision const& _evmcRevision = EVMC_PETERSBURG);
	void execute(std::basic_string<uint8_t> _rawByteCode, std::string _hexInput = {});
	void execute(std::string _hexCode, std::string _hexInput = {});
	void execute(const evmc_message& _m, std::basic_string<uint8_t> _rawByteCode);
	void execute(int64_t _gas, std::basic_string<uint8_t> _rawByteCode, std::string _hexInput = {});
	void execute(int64_t _gas, std::string _code, std::string _input);
	static std::basic_string<uint8_t> fromHex(std::string _hex);

private:
	evmc_instance* m_vm;
	evmc_revision m_rev;
	static evmc_host_interface m_interface;
	// TODO: Initialize transaction
	evmc_tx_context m_txContext;
};
}
}