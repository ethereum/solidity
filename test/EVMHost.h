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

#pragma once

#include <test/evmc/evmc.hpp>
#include <test/evmc/evmc.h>
#include <test/evmc/helpers.hpp>

#include <liblangutil/EVMVersion.h>

#include <libdevcore/FixedHash.h>


namespace dev
{
namespace test
{
using Address = h160;

class EVMHost: public evmc::Host
{
public:
	explicit EVMHost(langutil::EVMVersion _evmVersion, evmc::vm& _vmInstance);

	struct Account
	{
		evmc_uint256be balance = {};
		size_t nonce = 0;
		bytes code;
		evmc_bytes32 codeHash = {};
		std::map<evmc_bytes32, evmc_bytes32> storage;
	};

	struct LogEntry
	{
		Address address;
		std::vector<h256> topics;
		bytes data;
	};

	struct State
	{
		size_t blockNumber;
		uint64_t timestamp;
		std::map<evmc_address, Account> accounts;
		std::vector<LogEntry> logs;
	};

	Account* account(evmc_address const& _address)
	{
		auto it = m_state.accounts.find(_address);
		return it == m_state.accounts.end() ? nullptr : &it->second;
	}

	void reset() { m_state = State{}; m_currentAddress = {}; }
	void newBlock()
	{
		m_state.blockNumber++;
		m_state.timestamp += 15;
		m_state.logs.clear();
	}

	bool account_exists(evmc_address const& _addr) noexcept final
	{
		return account(_addr) != nullptr;
	}

	evmc_bytes32 get_storage(evmc_address const& _addr, evmc_bytes32 const& _key) noexcept final
	{
		if (Account* acc = account(_addr))
			return acc->storage[_key];
		return {};
	}

	evmc_storage_status set_storage(
		evmc_address const& _addr,
		evmc_bytes32 const& _key,
		evmc_bytes32 const& _value
	) noexcept;

	evmc_uint256be get_balance(evmc_address const& _addr) noexcept final
	{
		if (Account const* acc = account(_addr))
			return acc->balance;
		return {};
	}

	size_t get_code_size(evmc_address const& _addr) noexcept final
	{
		if (Account const* acc = account(_addr))
			return acc->code.size();
		return 0;
	}

	evmc_bytes32 get_code_hash(evmc_address const& _addr) noexcept final
	{
		if (Account const* acc = account(_addr))
			return acc->codeHash;
		return {};
	}

	size_t copy_code(
		evmc_address const& _addr,
		size_t _codeOffset,
		uint8_t* _bufferData,
		size_t _bufferSize
	) noexcept final
	{
		size_t i = 0;
		if (Account const* acc = account(_addr))
			for (; i < _bufferSize && _codeOffset + i < acc->code.size(); i++)
				_bufferData[i] = acc->code[_codeOffset + i];
		return i;
	}

	void selfdestruct(evmc_address const& _addr, evmc_address const& _beneficiary) noexcept;

	evmc::result call(evmc_message const& _message) noexcept;

	evmc_tx_context get_tx_context() noexcept;

	evmc_bytes32 get_block_hash(int64_t number) noexcept;

	void emit_log(
		evmc_address const& _addr,
		uint8_t const* _data,
		size_t _dataSize,
		evmc_bytes32 const _topics[],
		size_t _topicsCount
	) noexcept;

	evmc_revision getRevision()
	{
		return m_evmVersion;
	}

	static Address convertFromEVMC(evmc_address const& _addr);
	static evmc_address convertToEVMC(Address const& _addr);
	static h256 convertFromEVMC(evmc_bytes32 const& _data);
	static evmc_bytes32 convertToEVMC(h256 const& _data);


	State m_state;
	evmc_address m_currentAddress = {};
	evmc_address m_coinbase = convertToEVMC(Address("0x7878787878787878787878787878787878787878"));

private:
	evmc::result precompileSha256(evmc_message const& _message) noexcept;

	evmc::vm& m_vm;
	evmc_revision m_evmVersion;
};


}
}
