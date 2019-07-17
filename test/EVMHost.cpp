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
/**
 * EVM execution host, i.e. component that implements a simulated Ethereum blockchain
 * for testing purposes.
 */

#include <test/EVMHost.h>

#include <test/evmc/helpers.hpp>
#include <test/evmc/loader.h>

#include <libdevcore/Exceptions.h>
#include <libdevcore/Assertions.h>
#include <libdevcore/Keccak256.h>
#include <libdevcore/picosha2.h>

using namespace std;
using namespace dev;
using namespace dev::test;

EVMHost::EVMHost(langutil::EVMVersion _evmVersion, evmc::vm& _vmInstance):
	m_vm(_vmInstance)
{
	if (_evmVersion == langutil::EVMVersion::homestead())
		m_evmVersion = EVMC_HOMESTEAD;
	else if (_evmVersion == langutil::EVMVersion::tangerineWhistle())
		m_evmVersion = EVMC_TANGERINE_WHISTLE;
	else if (_evmVersion == langutil::EVMVersion::spuriousDragon())
		m_evmVersion = EVMC_SPURIOUS_DRAGON;
	else if (_evmVersion == langutil::EVMVersion::byzantium())
		m_evmVersion = EVMC_BYZANTIUM;
	else if (_evmVersion == langutil::EVMVersion::constantinople())
		m_evmVersion = EVMC_CONSTANTINOPLE;
	else //if (_evmVersion == langutil::EVMVersion::petersburg())
		m_evmVersion = EVMC_PETERSBURG;
}

evmc_storage_status EVMHost::set_storage(const evmc_address& _addr, const evmc_bytes32& _key, const evmc_bytes32& _value) noexcept
{
	evmc_bytes32 previousValue = m_state.accounts[_addr].storage[_key];
	m_state.accounts[_addr].storage[_key] = _value;

	// TODO EVMC_STORAGE_MODIFIED_AGAIN should be also used
	if (previousValue == _value)
		return EVMC_STORAGE_UNCHANGED;
	else if (previousValue == evmc_bytes32{})
		return EVMC_STORAGE_ADDED;
	else if (_value == evmc_bytes32{})
		return EVMC_STORAGE_DELETED;
	else
		return EVMC_STORAGE_MODIFIED;

}

void EVMHost::selfdestruct(const evmc_address& _addr, const evmc_address& _beneficiary) noexcept
{
	// TODO actual selfdestruct is even more complicated.
	evmc_uint256be balance = m_state.accounts[_addr].balance;
	m_state.accounts.erase(_addr);
	m_state.accounts[_beneficiary].balance = balance;
}

evmc::result EVMHost::call(evmc_message const& _message) noexcept
{
	if (_message.destination == convertToEVMC(Address(2)))
		return precompileSha256(_message);

	State stateBackup = m_state;

	u256 value{convertFromEVMC(_message.value)};
	Account& sender = m_state.accounts[_message.sender];

	bytes code;

	evmc_message message = _message;
	if (message.kind == EVMC_CREATE)
	{
		// TODO this is not the right formula
		// TODO is the nonce incremented on failure, too?
		Address createAddress(keccak256(
			bytes(begin(message.sender.bytes), end(message.sender.bytes)) +
			asBytes(to_string(sender.nonce++))
		));
		message.destination = convertToEVMC(createAddress);
		code = bytes(message.input_data, message.input_data + message.input_size);
	}
	else if (message.kind == EVMC_DELEGATECALL)
	{
		code = m_state.accounts[message.destination].code;
		message.destination = m_currentAddress;
	}
	else if (message.kind == EVMC_CALLCODE)
	{
		code = m_state.accounts[message.destination].code;
		message.destination = m_currentAddress;
	}
	else
		code = m_state.accounts[message.destination].code;
	//TODO CREATE2

	Account& destination = m_state.accounts[message.destination];

	if (value != 0 && message.kind != EVMC_DELEGATECALL && message.kind != EVMC_CALLCODE)
	{
		sender.balance = convertToEVMC(u256(convertFromEVMC(sender.balance)) - value);
		destination.balance = convertToEVMC(u256(convertFromEVMC(destination.balance)) + value);
	}

	evmc_address currentAddress = m_currentAddress;
	m_currentAddress = message.destination;
	evmc::result result = m_vm.execute(*this, m_evmVersion, message, code.data(), code.size());
	m_currentAddress = currentAddress;

	if (result.status_code != EVMC_SUCCESS)
		m_state = stateBackup;
	else if (message.kind == EVMC_CREATE)
	{
		result.create_address = message.destination;
		destination.code = bytes(result.output_data, result.output_data + result.output_size);
		destination.codeHash = convertToEVMC(keccak256(destination.code));
	}

	return result;
}

evmc_tx_context EVMHost::get_tx_context() noexcept
{
	evmc_tx_context ctx = {};
	ctx.block_timestamp = m_state.timestamp;
	ctx.block_number = m_state.blockNumber;
	ctx.block_coinbase = m_coinbase;
	ctx.block_difficulty = convertToEVMC(u256("200000000"));
	ctx.block_gas_limit = 20000000;
	ctx.tx_gas_price = convertToEVMC(u256("3000000000"));
	ctx.tx_origin = convertToEVMC(Address("0x9292929292929292929292929292929292929292"));
	return ctx;
}

evmc_bytes32 EVMHost::get_block_hash(int64_t _number) noexcept
{
	return convertToEVMC(u256("0x3737373737373737373737373737373737373737373737373737373737373737") + _number);
}

void EVMHost::emit_log(
	evmc_address const& _addr,
	uint8_t const* _data,
	size_t _dataSize,
	evmc_bytes32 const _topics[],
	size_t _topicsCount
) noexcept
{
	LogEntry entry;
	entry.address = convertFromEVMC(_addr);
	for (size_t i = 0; i < _topicsCount; ++i)
		entry.topics.emplace_back(convertFromEVMC(_topics[i]));
	entry.data = bytes(_data, _data + _dataSize);
	m_state.logs.emplace_back(std::move(entry));
}


Address EVMHost::convertFromEVMC(evmc_address const& _addr)
{
	return Address(bytes(begin(_addr.bytes), end(_addr.bytes)));
}

evmc_address EVMHost::convertToEVMC(Address const& _addr)
{
	evmc_address a;
	for (size_t i = 0; i < 20; ++i)
		a.bytes[i] = _addr[i];
	return a;
}

h256 EVMHost::convertFromEVMC(evmc_bytes32 const& _data)
{
	return h256(bytes(begin(_data.bytes), end(_data.bytes)));
}

evmc_bytes32 EVMHost::convertToEVMC(h256 const& _data)
{
	evmc_bytes32 d;
	for (size_t i = 0; i < 32; ++i)
		d.bytes[i] = _data[i];
	return d;
}

evmc::result EVMHost::precompileSha256(evmc_message const& _message) noexcept
{
	// static data so that we do not need a release routine...
	bytes static hash;
	hash = picosha2::hash256(bytes(
		_message.input_data,
		_message.input_data + _message.input_size
	));

	evmc::result result({});
	result.output_data = hash.data();
	result.output_size = hash.size();
	return result;
}
