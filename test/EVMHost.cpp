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
// SPDX-License-Identifier: GPL-3.0
/**
 * EVM execution host, i.e. component that implements a simulated Ethereum blockchain
 * for testing purposes.
 */

#include <test/EVMHost.h>

#include <test/evmc/loader.h>

#include <libevmasm/GasMeter.h>

#include <libsolutil/Exceptions.h>
#include <libsolutil/Assertions.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/picosha2.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::test;
using namespace evmc::literals;

evmc::VM& EVMHost::getVM(string const& _path)
{
	static evmc::VM NullVM{nullptr};
	static map<string, unique_ptr<evmc::VM>> vms;
	if (vms.count(_path) == 0)
	{
		evmc_loader_error_code errorCode = {};
		auto vm = evmc::VM{evmc_load_and_configure(_path.c_str(), &errorCode)};
		if (vm && errorCode == EVMC_LOADER_SUCCESS)
		{
			if (vm.get_capabilities() & (EVMC_CAPABILITY_EVM1))
				vms[_path] = make_unique<evmc::VM>(evmc::VM(std::move(vm)));
			else
				cerr << "VM loaded does not support EVM1" << endl;
		}
		else
		{
			cerr << "Error loading VM from " << _path;
			if (char const* errorMsg = evmc_last_error_msg())
				cerr << ":" << endl << errorMsg;
			cerr << endl;
		}
	}

	if (vms.count(_path) > 0)
		return *vms[_path];

	return NullVM;
}

bool EVMHost::checkVmPaths(vector<boost::filesystem::path> const& _vmPaths)
{
	bool evmVmFound = false;
	for (auto const& path: _vmPaths)
	{
		evmc::VM& vm = EVMHost::getVM(path.string());
		if (!vm)
			continue;

		if (vm.has_capability(EVMC_CAPABILITY_EVM1))
		{
			if (evmVmFound)
				BOOST_THROW_EXCEPTION(runtime_error("Multiple evm1 evmc vms defined. Please only define one evm1 evmc vm."));
			evmVmFound = true;
		}
	}
	return evmVmFound;
}

EVMHost::EVMHost(langutil::EVMVersion _evmVersion, evmc::VM& _vm):
	m_vm(_vm),
	m_evmVersion(_evmVersion)
{
	if (!m_vm)
	{
		cerr << "Unable to find evmone library" << endl;
		assertThrow(false, Exception, "");
	}

	if (_evmVersion == langutil::EVMVersion::homestead())
		m_evmRevision = EVMC_HOMESTEAD;
	else if (_evmVersion == langutil::EVMVersion::tangerineWhistle())
		m_evmRevision = EVMC_TANGERINE_WHISTLE;
	else if (_evmVersion == langutil::EVMVersion::spuriousDragon())
		m_evmRevision = EVMC_SPURIOUS_DRAGON;
	else if (_evmVersion == langutil::EVMVersion::byzantium())
		m_evmRevision = EVMC_BYZANTIUM;
	else if (_evmVersion == langutil::EVMVersion::constantinople())
		m_evmRevision = EVMC_CONSTANTINOPLE;
	else if (_evmVersion == langutil::EVMVersion::petersburg())
		m_evmRevision = EVMC_PETERSBURG;
	else if (_evmVersion == langutil::EVMVersion::istanbul())
		m_evmRevision = EVMC_ISTANBUL;
	else if (_evmVersion == langutil::EVMVersion::berlin())
		m_evmRevision = EVMC_BERLIN;
	else if (_evmVersion == langutil::EVMVersion::london())
		m_evmRevision = EVMC_LONDON;
	else if (_evmVersion == langutil::EVMVersion::paris())
		m_evmRevision = EVMC_PARIS;
	else if (_evmVersion == langutil::EVMVersion::shanghai())
		m_evmRevision = EVMC_SHANGHAI;
	else
		assertThrow(false, Exception, "Unsupported EVM version");

	if (m_evmRevision >= EVMC_PARIS)
		// This is the value from the merge block.
		tx_context.block_prev_randao = 0xa86c2e601b6c44eb4848f7d23d9df3113fbcac42041c49cbed5000cb4f118777_bytes32;
	else
		tx_context.block_prev_randao = evmc::uint256be{200000000};
	tx_context.block_gas_limit = 20000000;
	tx_context.block_coinbase = 0x7878787878787878787878787878787878787878_address;
	tx_context.tx_gas_price = evmc::uint256be{3000000000};
	tx_context.tx_origin = 0x9292929292929292929292929292929292929292_address;
	// Mainnet according to EIP-155
	tx_context.chain_id = evmc::uint256be{1};
	// The minimum value of basefee
	tx_context.block_base_fee = evmc::bytes32{7};

	// Reserve space for recording calls.
	if (!recorded_calls.capacity())
		recorded_calls.reserve(max_recorded_calls);

	reset();
}

void EVMHost::reset()
{
	accounts.clear();
	// Clear self destruct records
	recorded_selfdestructs.clear();
	// Clear call records
	recorded_calls.clear();
	// Clear EIP-2929 account access indicator
	recorded_account_accesses.clear();

	// Mark all precompiled contracts as existing. Existing here means to have a balance (as per EIP-161).
	// NOTE: keep this in sync with `EVMHost::call` below.
	//
	// A lot of precompile addresses had a balance before they became valid addresses for precompiles.
	// For example all the precompile addresses allocated in Byzantium had a 1 wei balance sent to them
	// roughly 22 days before the update went live.
	for (unsigned precompiledAddress = 1; precompiledAddress <= 8; precompiledAddress++)
	{
		evmc::address address{precompiledAddress};
		// 1wei
		accounts[address].balance = evmc::uint256be{1};
		// Set according to EIP-1052.
		if (precompiledAddress < 5 || m_evmVersion >= langutil::EVMVersion::byzantium())
			accounts[address].codehash = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470_bytes32;
	}
}

void EVMHost::newTransactionFrame()
{
	// Clear EIP-2929 account access indicator
	recorded_account_accesses.clear();

	for (auto& [address, account]: accounts)
		for (auto& [slot, value]: account.storage)
		{
			value.access_status = EVMC_ACCESS_COLD; // Clear EIP-2929 storage access indicator
			value.original = value.current;			// Clear EIP-2200 dirty slot
		}
	// Process selfdestruct list
	for (auto& [address, _]: recorded_selfdestructs)
		accounts.erase(address);
	recorded_selfdestructs.clear();
}

void EVMHost::transfer(evmc::MockedAccount& _sender, evmc::MockedAccount& _recipient, u256 const& _value) noexcept
{
	assertThrow(u256(convertFromEVMC(_sender.balance)) >= _value, Exception, "Insufficient balance for transfer");
	_sender.balance = convertToEVMC(u256(convertFromEVMC(_sender.balance)) - _value);
	_recipient.balance = convertToEVMC(u256(convertFromEVMC(_recipient.balance)) + _value);
}

bool EVMHost::selfdestruct(const evmc::address& _addr, const evmc::address& _beneficiary) noexcept
{
	// TODO actual selfdestruct is even more complicated.

	transfer(accounts[_addr], accounts[_beneficiary], convertFromEVMC(accounts[_addr].balance));

	// Record self destructs. Clearing will be done in newTransactionFrame().
	return MockedHost::selfdestruct(_addr, _beneficiary);
}

void EVMHost::recordCalls(evmc_message const& _message) noexcept
{
	if (recorded_calls.size() < max_recorded_calls)
		recorded_calls.emplace_back(_message);
}

// NOTE: this is used for both internal and external calls.
// External calls are triggered from ExecutionFramework and contain only EVMC_CREATE or EVMC_CALL.
evmc::Result EVMHost::call(evmc_message const& _message) noexcept
{
	recordCalls(_message);
	if (_message.recipient == 0x0000000000000000000000000000000000000001_address)
		return precompileECRecover(_message);
	else if (_message.recipient == 0x0000000000000000000000000000000000000002_address)
		return precompileSha256(_message);
	else if (_message.recipient == 0x0000000000000000000000000000000000000003_address)
		return precompileRipeMD160(_message);
	else if (_message.recipient == 0x0000000000000000000000000000000000000004_address)
		return precompileIdentity(_message);
	else if (_message.recipient == 0x0000000000000000000000000000000000000005_address && m_evmVersion >= langutil::EVMVersion::byzantium())
		return precompileModExp(_message);
	else if (_message.recipient == 0x0000000000000000000000000000000000000006_address && m_evmVersion >= langutil::EVMVersion::byzantium())
	{
		if (m_evmVersion <= langutil::EVMVersion::istanbul())
			return precompileALTBN128G1Add<EVMC_ISTANBUL>(_message);
		else
			return precompileALTBN128G1Add<EVMC_LONDON>(_message);
	}
	else if (_message.recipient == 0x0000000000000000000000000000000000000007_address && m_evmVersion >= langutil::EVMVersion::byzantium())
	{
		if (m_evmVersion <= langutil::EVMVersion::istanbul())
			return precompileALTBN128G1Mul<EVMC_ISTANBUL>(_message);
		else
			return precompileALTBN128G1Mul<EVMC_LONDON>(_message);
	}
	else if (_message.recipient == 0x0000000000000000000000000000000000000008_address && m_evmVersion >= langutil::EVMVersion::byzantium())
	{
		if (m_evmVersion <= langutil::EVMVersion::istanbul())
			return precompileALTBN128PairingProduct<EVMC_ISTANBUL>(_message);
		else
			return precompileALTBN128PairingProduct<EVMC_LONDON>(_message);
	}
	else if (_message.recipient == 0x0000000000000000000000000000000000000009_address && m_evmVersion >= langutil::EVMVersion::istanbul())
		return precompileBlake2f(_message);

	auto const stateBackup = accounts;

	u256 value{convertFromEVMC(_message.value)};
	auto& sender = accounts[_message.sender];

	evmc::bytes code;

	evmc_message message = _message;
	if (message.depth == 0)
	{
		message.gas -= message.kind == EVMC_CREATE ? evmasm::GasCosts::txCreateGas : evmasm::GasCosts::txGas;
		for (size_t i = 0; i < message.input_size; ++i)
			message.gas -= message.input_data[i] == 0 ? evmasm::GasCosts::txDataZeroGas : evmasm::GasCosts::txDataNonZeroGas(m_evmVersion);
		if (message.gas < 0)
		{
			evmc::Result result;
			result.status_code = EVMC_OUT_OF_GAS;
			accounts = stateBackup;
			return result;
		}
	}

	if (message.kind == EVMC_CREATE)
	{
		// TODO is the nonce incremented on failure, too?
		// NOTE: nonce for creation from contracts starts at 1
		// TODO: check if sender is an EOA and do not pre-increment
		sender.nonce++;

		auto encodeRlpInteger = [](int value) -> bytes {
			if (value == 0) {
				return bytes{128};
			} else if (value <= 127) {
				return bytes{static_cast<uint8_t>(value)};
			} else if (value <= 0xff) {
				return bytes{128 + 1, static_cast<uint8_t>(value)};
			} else if (value <= 0xffff) {
				return bytes{128 + 55 + 2, static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value)};
			} else {
				assertThrow(false, Exception, "Can only encode RLP numbers <= 0xffff");
			}
		};

		bytes encodedNonce = encodeRlpInteger(sender.nonce);

		h160 createAddress(keccak256(
			bytes{static_cast<uint8_t>(0xc0 + 21 + encodedNonce.size())} +
			bytes{0x94} +
			bytes(begin(message.sender.bytes), end(message.sender.bytes)) +
			encodedNonce
		), h160::AlignRight);

		message.recipient = convertToEVMC(createAddress);
		assertThrow(accounts.count(message.recipient) == 0, Exception, "Account cannot exist");

		code = evmc::bytes(message.input_data, message.input_data + message.input_size);
	}
	else if (message.kind == EVMC_CREATE2)
	{
		h160 createAddress(keccak256(
			bytes{0xff} +
			bytes(begin(message.sender.bytes), end(message.sender.bytes)) +
			bytes(begin(message.create2_salt.bytes), end(message.create2_salt.bytes)) +
			keccak256(bytes(message.input_data, message.input_data + message.input_size)).asBytes()
		), h160::AlignRight);

		message.recipient = convertToEVMC(createAddress);
		if (accounts.count(message.recipient) && (
			accounts[message.recipient].nonce > 0 ||
			!accounts[message.recipient].code.empty()
		))
		{
			evmc::Result result;
			result.status_code = EVMC_OUT_OF_GAS;
			accounts = stateBackup;
			return result;
		}

		code = evmc::bytes(message.input_data, message.input_data + message.input_size);
	}
	else
		code = accounts[message.code_address].code;

	auto& destination = accounts[message.recipient];

	if (value != 0 && message.kind != EVMC_DELEGATECALL && message.kind != EVMC_CALLCODE)
	{
		if (value > convertFromEVMC(sender.balance))
		{
			evmc::Result result;
			result.status_code = EVMC_INSUFFICIENT_BALANCE;
			accounts = stateBackup;
			return result;
		}
		transfer(sender, destination, value);
	}

	// Populate the access access list (enabled since Berlin).
	// Note, this will also properly touch the created address.
	// TODO: support a user supplied access list too
	if (m_evmRevision >= EVMC_BERLIN)
	{
		access_account(message.sender);
		access_account(message.recipient);

		// EIP-3651 rule
		if (m_evmRevision >= EVMC_SHANGHAI)
			access_account(tx_context.block_coinbase);
	}

	if (message.kind == EVMC_CREATE || message.kind == EVMC_CREATE2)
	{
		message.input_data = nullptr;
		message.input_size = 0;
	}
	evmc::Result result = m_vm.execute(*this, m_evmRevision, message, code.data(), code.size());

	if (message.kind == EVMC_CREATE || message.kind == EVMC_CREATE2)
	{
		result.gas_left -= static_cast<int64_t>(evmasm::GasCosts::createDataGas * result.output_size);
		if (result.gas_left < 0)
		{
			result.gas_left = 0;
			result.status_code = EVMC_OUT_OF_GAS;
			// TODO clear some fields?
		}
		else
		{
			result.create_address = message.recipient;
			destination.code = evmc::bytes(result.output_data, result.output_data + result.output_size);
			destination.codehash = convertToEVMC(keccak256({result.output_data, result.output_size}));
		}
	}

	if (result.status_code != EVMC_SUCCESS)
		accounts = stateBackup;

	return result;
}

evmc::bytes32 EVMHost::get_block_hash(int64_t _number) const noexcept
{
	return convertToEVMC(u256("0x3737373737373737373737373737373737373737373737373737373737373737") + _number);
}

h160 EVMHost::convertFromEVMC(evmc::address const& _addr)
{
	return h160(bytes(begin(_addr.bytes), end(_addr.bytes)));
}

evmc::address EVMHost::convertToEVMC(h160 const& _addr)
{
	evmc::address a;
	for (unsigned i = 0; i < 20; ++i)
		a.bytes[i] = _addr[i];
	return a;
}

h256 EVMHost::convertFromEVMC(evmc::bytes32 const& _data)
{
	return h256(bytes(begin(_data.bytes), end(_data.bytes)));
}

evmc::bytes32 EVMHost::convertToEVMC(h256 const& _data)
{
	evmc::bytes32 d;
	for (unsigned i = 0; i < 32; ++i)
		d.bytes[i] = _data[i];
	return d;
}

evmc::Result EVMHost::precompileECRecover(evmc_message const& _message) noexcept
{
	// NOTE this is a partial implementation for some inputs.

	// Fixed cost of 3000 gas.
	constexpr int64_t gas_cost = 3000;

	static map<bytes, EVMPrecompileOutput> const inputOutput{
		{
			fromHex(
				"18c547e4f7b0f325ad1e56f57e26c745b09a3e503d86e00e5255ff7f715d3d1c"
				"000000000000000000000000000000000000000000000000000000000000001c"
				"73b1693892219d736caba55bdb67216e485557ea6b6af75f37096c9aa6a5a75f"
				"eeb940b1d03b21e36b0e47e79769f095fe2ab855bd91e3a38756b7d75a9c4549"
			),
			{
				fromHex("000000000000000000000000a94f5374fce5edbc8e2a8697c15331677e6ebf0b"),
				gas_cost
			}
		},
		{
			fromHex(
				"47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad"
				"000000000000000000000000000000000000000000000000000000000000001c"
				"debaaa0cddb321b2dcaaf846d39605de7b97e77ba6106587855b9106cb104215"
				"61a22d94fa8b8a687ff9c911c844d1c016d1a685a9166858f9c7c1bc85128aca"
			),
			{
				fromHex("0000000000000000000000008743523d96a1b2cbe0c6909653a56da18ed484af"),
				gas_cost
			}
		}
	};
	evmc::Result result = precompileGeneric(_message, inputOutput);
	// ECRecover will return success with empty response in case of failure
	if (result.status_code != EVMC_SUCCESS && result.status_code != EVMC_OUT_OF_GAS)
		return resultWithGas(_message.gas, gas_cost, {});
	return result;
}

evmc::Result EVMHost::precompileSha256(evmc_message const& _message) noexcept
{
	// static data so that we do not need a release routine...
	bytes static hash;
	hash = picosha2::hash256(bytes(
		_message.input_data,
		_message.input_data + _message.input_size
	));

	// Base 60 gas + 12 gas / word.
	int64_t gas_cost = 60 + 12 * ((static_cast<int64_t>(_message.input_size) + 31) / 32);

	return resultWithGas(_message.gas, gas_cost, hash);
}

evmc::Result EVMHost::precompileRipeMD160(evmc_message const& _message) noexcept
{
	// NOTE this is a partial implementation for some inputs.

	// Base 600 gas + 120 gas / word.
	constexpr auto calc_cost = [](int64_t size) -> int64_t {
		return 600 + 120 * ((size + 31) / 32);
	};

	static map<bytes, EVMPrecompileOutput> const inputOutput{
		{
			bytes{},
			{
				fromHex("0000000000000000000000009c1185a5c5e9fc54612808977ee8f548b2258d31"),
				calc_cost(0)
			}
		},
		{
			fromHex("0000000000000000000000000000000000000000000000000000000000000004"),
			{
				fromHex("0000000000000000000000001b0f3c404d12075c68c938f9f60ebea4f74941a0"),
				calc_cost(32)
			}
		},
		{
			fromHex("0000000000000000000000000000000000000000000000000000000000000005"),
			{
				fromHex("000000000000000000000000ee54aa84fc32d8fed5a5fe160442ae84626829d9"),
				calc_cost(32)
			}
		},
		{
			fromHex("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"),
			{
				fromHex("0000000000000000000000001cf4e77f5966e13e109703cd8a0df7ceda7f3dc3"),
				calc_cost(32)
			}
		},
		{
			fromHex("0000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("000000000000000000000000f93175303eba2a7b372174fc9330237f5ad202fc"),
				calc_cost(32)
			}
		},
		{
			fromHex(
				"0800000000000000000000000000000000000000000000000000000000000000"
				"0401000000000000000000000000000000000000000000000000000000000000"
				"0000000400000000000000000000000000000000000000000000000000000000"
				"00000100"
			),
			{
				fromHex("000000000000000000000000f93175303eba2a7b372174fc9330237f5ad202fc"),
				calc_cost(100)
			}
		},
		{
			fromHex(
				"0800000000000000000000000000000000000000000000000000000000000000"
				"0501000000000000000000000000000000000000000000000000000000000000"
				"0000000500000000000000000000000000000000000000000000000000000000"
				"00000100"
			),
			{
				fromHex("0000000000000000000000004f4fc112e2bfbe0d38f896a46629e08e2fcfad5"),
				calc_cost(100)
			}
		},
		{
			fromHex(
				"08ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
				"ff010000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
				"ffffffff00000000000000000000000000000000000000000000000000000000"
				"00000100"
			),
			{
				fromHex("000000000000000000000000c0a2e4b1f3ff766a9a0089e7a410391730872495"),
				calc_cost(100)
			}
		},
		{
			fromHex(
				"6162636465666768696a6b6c6d6e6f707172737475767778797a414243444546"
				"4748494a4b4c4d4e4f505152535455565758595a303132333435363738393f21"
			),
			{
				fromHex("00000000000000000000000036c6b90a49e17d4c1e1b0e634ec74124d9b207da"),
				calc_cost(64)
			}
		},
		{
			fromHex("6162636465666768696a6b6c6d6e6f707172737475767778797a414243444546"),
			{
				fromHex("000000000000000000000000ac5ab22e07b0fb80c69b6207902f725e2507e546"),
				calc_cost(32)
			}
		}
	};
	return precompileGeneric(_message, inputOutput);
}

evmc::Result EVMHost::precompileIdentity(evmc_message const& _message) noexcept
{
	// static data so that we do not need a release routine...
	bytes static data;
	data = bytes(_message.input_data, _message.input_data + _message.input_size);

	// Base 15 gas + 3 gas / word.
	int64_t gas_cost = 15 + 3 * ((static_cast<int64_t>(_message.input_size) + 31) / 32);

	return resultWithGas(_message.gas, gas_cost, data);
}

evmc::Result EVMHost::precompileModExp(evmc_message const&) noexcept
{
	// TODO implement
	return resultWithFailure();
}

template <evmc_revision Revision>
evmc::Result EVMHost::precompileALTBN128G1Add(evmc_message const& _message) noexcept
{
	// NOTE this is a partial implementation for some inputs.

	// Fixed 500 or 150 gas.
	int64_t gas_cost = (Revision < EVMC_ISTANBUL) ? 500 : 150;

	static map<bytes, EVMPrecompileOutput> const inputOutput{
		{
			fromHex(
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"1385281136ff5b2c326807ff0a824b6ca4f21fcc7c8764e9801bc4ad497d5012"
				"02254594be8473dcf018a2aa66ea301e38fc865823acf75a9901721d1fc6bf4c"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"1385281136ff5b2c326807ff0a824b6ca4f21fcc7c8764e9801bc4ad497d5012"
					"02254594be8473dcf018a2aa66ea301e38fc865823acf75a9901721d1fc6bf4c"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"0000000000000000000000000000000000000000000000000000000000000001"
				"0000000000000000000000000000000000000000000000000000000000000002"
				"0000000000000000000000000000000000000000000000000000000000000001"
				"0000000000000000000000000000000000000000000000000000000000000002"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"030644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd3"
					"15ed738c0e0a7c92e7845f96b2ae9c0a68a6a449e3538fc7ff3ebf7a5a18a2c4"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"0000000000000000000000000000000000000000000000000000000000000001"
				"0000000000000000000000000000000000000000000000000000000000000002"
				"0000000000000000000000000000000000000000000000000000000000000001"
				"30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd45"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"0000000000000000000000000000000000000000000000000000000000000000"
					"0000000000000000000000000000000000000000000000000000000000000000"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"10b4876441e14a6be92a7fe66550848c01c676a12ac31d7cc13b21f49c4307c8"
				"09f5528bdb0ef9354837a0f4b4c9da973bd5b805d359976f719ab0b74e0a7368"
				"28d3c57516712e7843a5b3cfa7d7274a037943f5bd57c227620ad207728e4283"
				"2795fa9df21d4b8b329a45bae120f1fd9df9049ecacaa9dd1eca18bc6a55cd2f"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"16aed5ed486df6b2fb38015ded41400009ed4f34bef65b87b1f90f47052f8d94"
					"16dabf21b3f25b9665269d98dc17b1da6118251dc0b403ae50e96dfe91239375"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"1385281136ff5b2c326807ff0a824b6ca4f21fcc7c8764e9801bc4ad497d5012"
				"02254594be8473dcf018a2aa66ea301e38fc865823acf75a9901721d1fc6bf4c"
				"1644e84fef7b7fdc98254f0654580173307a3bc44db990581e7ab55a22446dcf"
				"28c2916b7e875692b195831945805438fcd30d2693d8a80cf8c88ec6ef4c315d"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"1e018816fc9bbd91313301ae9c254bb7d64d6cd54f3b49b92925e43e256b5faa"
					"1d1f2259c715327bedb42c095af6c0267e4e1be836b4e04b3f0502552f93cca9"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"16aed5ed486df6b2fb38015ded41400009ed4f34bef65b87b1f90f47052f8d94"
				"16dabf21b3f25b9665269d98dc17b1da6118251dc0b403ae50e96dfe91239375"
				"25ff95a3abccf32adc6a4c3c8caddca67723d8ada802e9b9f612e3ddb40b2005"
				"0d82b09bb4ec927bbf182bdc402790429322b7e2f285f2aad8ea135cbf7143d8"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"29d160febeef9770d47a32ee3b763850eb0594844fa57dd31b8ed02c78fdb797"
					"2c7cdf62c2498486fd52646e577a06723ce97737b3c958262d78c4a413661e8a"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"18014701594179c6b9ccae848e3d15c1f76f8a68b8092578296520e46c9bae0c"
				"1b5ed0e9e8f3ff35589ea81a45cf63887d4a92c099a3be1d97b26f0db96323dd"
				"16a1d378d1a98cf5383cdc512011234287ca43b6a078d1842d5c58c5b1f475cc"
				"1309377a7026d08ca1529eab74381a7e0d3a4b79d80bacec207cd52fc8e3769c"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"2583ed10e418133e44619c336f1be5ddae9e20d634a7683d9661401c750d7df4"
					"0185fbba22de9e698262925665735dbc4d6e8288bc3fc39fae10ca58e16e77f7"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"1c76476f4def4bb94541d57ebba1193381ffa7aa76ada664dd31c16024c43f59"
				"3034dd2920f673e204fee2811c678745fc819b55d3e9d294e45c9b03a76aef41"
				"0f25929bcb43d5a57391564615c9e70a992b10eafa4db109709649cf48c50dd2"
				"16da2f5cb6be7a0aa72c440c53c9bbdfec6c36c7d515536431b3a865468acbba"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"1496064626ba8bffeb7805f0d16143a65649bb0850333ea512c03fcdaf31e254"
					"07b4f210ab542533f1ee5633ae4406cd16c63494b537ce3f1cf4afff6f76a48f"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"1e018816fc9bbd91313301ae9c254bb7d64d6cd54f3b49b92925e43e256b5faa"
				"1d1f2259c715327bedb42c095af6c0267e4e1be836b4e04b3f0502552f93cca9"
				"2364294faf6b89fedeede9986aa777c4f6c2f5c4a4559ee93dfec9b7b94ef80b"
				"05aeae62655ea23865ae6661ae371a55c12098703d0f2301f4223e708c92efc6"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"2801b21090cbc48409e352647f3857134d373f81741f9d5e3d432f336d76f517"
					"13cf106acf943c2a331de21c7d5e3351354e7412f2dba2918483a6593a6828d4"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"2583ed10e418133e44619c336f1be5ddae9e20d634a7683d9661401c750d7df4"
				"0185fbba22de9e698262925665735dbc4d6e8288bc3fc39fae10ca58e16e77f7"
				"258f1faa356e470cca19c928afa5ceed6215c756912af5725b8db5777cc8f3b6"
				"175ced8a58d0c132c2b95ba14c16dde93e7f7789214116ff69da6f44daa966e6"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"10b4876441e14a6be92a7fe66550848c01c676a12ac31d7cc13b21f49c4307c8"
					"09f5528bdb0ef9354837a0f4b4c9da973bd5b805d359976f719ab0b74e0a7368"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"26dcfbc2e0bc9d82efb4acd73cb3e99730e27e10177fcfb78b6399a4bfcdf391"
				"27c440dbd5053253a3a692f9bf89b9b6e9612127cf97db1e11ffa9679acc933b"
				"1496064626ba8bffeb7805f0d16143a65649bb0850333ea512c03fcdaf31e254"
				"07b4f210ab542533f1ee5633ae4406cd16c63494b537ce3f1cf4afff6f76a48f"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"186bac5188a98c45e6016873d107f5cd131f3a3e339d0375e58bd6219347b008"
					"1e396bc242de0214898b0f68035f53ad5a6f96c6c8390ac56ed6ec9561d23159"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"26dcfbc2e0bc9d82efb4acd73cb3e99730e27e10177fcfb78b6399a4bfcdf391"
				"27c440dbd5053253a3a692f9bf89b9b6e9612127cf97db1e11ffa9679acc933b"
				"1c76476f4def4bb94541d57ebba1193381ffa7aa76ada664dd31c16024c43f59"
				"3034dd2920f673e204fee2811c678745fc819b55d3e9d294e45c9b03a76aef41"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"20a754d2071d4d53903e3b31a7e98ad6882d58aec240ef981fdf0a9d22c5926a"
					"29c853fcea789887315916bbeb89ca37edb355b4f980c9a12a94f30deeed3021"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"27231d5cdd0011259ff75678cf5a8f7840c22cb71d52b25e21e071205e8d9bc4"
				"26dd3d225c9a71476db0cf834232eba84020f3073c6d20c519963e0b98f235e1"
				"2174f0221490cd9c15b0387f3251ec3d49517a51c37a8076eac12afb4a95a707"
				"1d1c3fcd3161e2a417b4df0955f02db1fffa9005210fb30c5aa3755307e9d1f5"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"18014701594179c6b9ccae848e3d15c1f76f8a68b8092578296520e46c9bae0c"
					"1b5ed0e9e8f3ff35589ea81a45cf63887d4a92c099a3be1d97b26f0db96323dd"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"2801b21090cbc48409e352647f3857134d373f81741f9d5e3d432f336d76f517"
				"13cf106acf943c2a331de21c7d5e3351354e7412f2dba2918483a6593a6828d4"
				"2a49621e12910cd90f3e731083d454255bf1c533d6e15b8699156778d0f27f5d"
				"2590ee31824548d159aa2d22296bf149d564c0872f41b89b7dc5c6e6e3cd1c4d"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"27231d5cdd0011259ff75678cf5a8f7840c22cb71d52b25e21e071205e8d9bc4"
					"26dd3d225c9a71476db0cf834232eba84020f3073c6d20c519963e0b98f235e1"
				),
				gas_cost
			}
		},
		{
			fromHex(
				"29d160febeef9770d47a32ee3b763850eb0594844fa57dd31b8ed02c78fdb797"
				"2c7cdf62c2498486fd52646e577a06723ce97737b3c958262d78c4a413661e8a"
				"0aee46a7ea6e80a3675026dfa84019deee2a2dedb1bbe11d7fe124cb3efb4b5a"
				"044747b6e9176e13ede3a4dfd0d33ccca6321b9acd23bf3683a60adc0366ebaf"
				"0000000000000000000000000000000000000000000000000000000000000000"
				"0000000000000000000000000000000000000000000000000000000000000000"
			),
			{
				fromHex(
					"26dcfbc2e0bc9d82efb4acd73cb3e99730e27e10177fcfb78b6399a4bfcdf391"
					"27c440dbd5053253a3a692f9bf89b9b6e9612127cf97db1e11ffa9679acc933b"
				),
				gas_cost
			}
		}
	};
	return precompileGeneric(_message, inputOutput);
}

template <evmc_revision Revision>
evmc::Result EVMHost::precompileALTBN128G1Mul(evmc_message const& _message) noexcept
{
	// NOTE this is a partial implementation for some inputs.

	// Fixed 40000 or 6000 gas.
	int64_t gas_cost = (Revision < EVMC_ISTANBUL) ? 40000 : 6000;

	static map<bytes, EVMPrecompileOutput> const inputOutput{
		{
			fromHex("0000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("030644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd315ed738c0e0a7c92e7845f96b2ae9c0a68a6a449e3538fc7ff3ebf7a5a18a2c4"),
				gas_cost
			}
		},
		{
			fromHex("0000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000050000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("17c139df0efee0f766bc0204762b774362e4ded88953a39ce849a8a7fa163fa901e0559bacb160664764a357af8a9fe70baa9258e0b959273ffc5718c6d4cc7c"),
				gas_cost
			}
		},
		{
			fromHex("09b54f111d3b2d1b2fe1ae9669b3db3d7bf93b70f00647e65c849275de6dc7fe18b2e77c63a3e400d6d1f1fbc6e1a1167bbca603d34d03edea231eb0ab7b14b4030f7b0c405c888aff922307ea2cd1c70f64664bab76899500341f4260a209290000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("16a1d378d1a98cf5383cdc512011234287ca43b6a078d1842d5c58c5b1f475cc1309377a7026d08ca1529eab74381a7e0d3a4b79d80bacec207cd52fc8e3769c"),
				gas_cost
			}
		},
		{
			fromHex("0a6de0e2240aa253f46ce0da883b61976e3588146e01c9d8976548c145fe6e4a04fbaa3a4aed4bb77f30ebb07a3ec1c7d77a7f2edd75636babfeff97b1ea686e1551dcd4965285ef049512d2d30cbfc1a91acd5baad4a6e19e22e93176197f170000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("28d3c57516712e7843a5b3cfa7d7274a037943f5bd57c227620ad207728e42832795fa9df21d4b8b329a45bae120f1fd9df9049ecacaa9dd1eca18bc6a55cd2f"),
				gas_cost
			}
		},
		{
			fromHex("0c54b42137b67cc268cbb53ac62b00ecead23984092b494a88befe58445a244a18e3723d37fae9262d58b548a0575f59d9c3266db7afb4d5739555837f6b8b3e0c692b41f1acc961f6ea83bae2c3a1a55c54f766c63ba76989f52c149c17b5e70000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("258f1faa356e470cca19c928afa5ceed6215c756912af5725b8db5777cc8f3b6175ced8a58d0c132c2b95ba14c16dde93e7f7789214116ff69da6f44daa966e6"),
				gas_cost
			}
		},
		{
			fromHex("0f103f14a584d4203c27c26155b2c955f8dfa816980b24ba824e1972d6486a5d0c4165133b9f5be17c804203af781bcf168da7386620479f9b885ecbcd27b17b0ea71d0abb524cac7cfff5323e1d0b14ab705842426c978f96753ccce258ed930000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("2a49621e12910cd90f3e731083d454255bf1c533d6e15b8699156778d0f27f5d2590ee31824548d159aa2d22296bf149d564c0872f41b89b7dc5c6e6e3cd1c4d"),
				gas_cost
			}
		},
		{
			fromHex("111e2e2a5f8828f80ddad08f9f74db56dac1cc16c1cb278036f79a84cf7a116f1d7d62e192b219b9808faa906c5ced871788f6339e8d91b83ac1343e20a16b3000000000000000000000000000000000000000e40800000000000000008cdcbc0000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("25ff95a3abccf32adc6a4c3c8caddca67723d8ada802e9b9f612e3ddb40b20050d82b09bb4ec927bbf182bdc402790429322b7e2f285f2aad8ea135cbf7143d8"),
				gas_cost
			}
		},
		{
			fromHex("17d5d09b4146424bff7e6fb01487c477bbfcd0cdbbc92d5d6457aae0b6717cc502b5636903efbf46db9235bbe74045d21c138897fda32e079040db1a16c1a7a11887420878c0c8e37605291c626585eabbec8d8b97a848fe8d58a37b004583510000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("2364294faf6b89fedeede9986aa777c4f6c2f5c4a4559ee93dfec9b7b94ef80b05aeae62655ea23865ae6661ae371a55c12098703d0f2301f4223e708c92efc6"),
				gas_cost
			}
		},
		{
			fromHex("1c36e713d4d54e3a9644dffca1fc524be4868f66572516025a61ca542539d43f042dcc4525b82dfb242b09cb21909d5c22643dcdbe98c4d082cc2877e96b24db016086cc934d5cab679c6991a4efcedbab26d7e4fb23b6a1ad4e6b5c2fb59ce50000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("1644e84fef7b7fdc98254f0654580173307a3bc44db990581e7ab55a22446dcf28c2916b7e875692b195831945805438fcd30d2693d8a80cf8c88ec6ef4c315d"),
				gas_cost
			}
		},
		{
			fromHex("1e39e9f0f91fa7ff8047ffd90de08785777fe61c0e3434e728fce4cf35047ddc2e0b64d75ebfa86d7f8f8e08abbe2e7ae6e0a1c0b34d028f19fa56e9450527cb1eec35a0e955cad4bee5846ae0f1d0b742d8636b278450c534e38e06a60509f90000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("1385281136ff5b2c326807ff0a824b6ca4f21fcc7c8764e9801bc4ad497d501202254594be8473dcf018a2aa66ea301e38fc865823acf75a9901721d1fc6bf4c"),
				gas_cost
			}
		},
		{
			fromHex("232063b584fb76c8d07995bee3a38fa7565405f3549c6a918ddaa90ab971e7f82ac9b135a81d96425c92d02296322ad56ffb16299633233e4880f95aafa7fda70689c3dc4311426ee11707866b2cbdf9751dacd07245bf99d2113d3f5a8cac470000000000000000000000000000000000000000000000000000000000000000"),
			{
				fromHex("2174f0221490cd9c15b0387f3251ec3d49517a51c37a8076eac12afb4a95a7071d1c3fcd3161e2a417b4df0955f02db1fffa9005210fb30c5aa3755307e9d1f5"),
				gas_cost
			}
		}
	};
	return precompileGeneric(_message, inputOutput);
}

template <evmc_revision Revision>
evmc::Result EVMHost::precompileALTBN128PairingProduct(evmc_message const& _message) noexcept
{
	// Base + per pairing gas.
	constexpr auto calc_cost = [](unsigned points) -> int64_t {
		// Number of 192-byte points.
		return (Revision < EVMC_ISTANBUL) ?
			(100000 + 80000 * points):
			(45000 + 34000 * points);
	};

	// NOTE this is a partial implementation for some inputs.
	static map<bytes, EVMPrecompileOutput> const inputOutput{
		{
			fromHex(
				"17c139df0efee0f766bc0204762b774362e4ded88953a39ce849a8a7fa163fa9"
				"01e0559bacb160664764a357af8a9fe70baa9258e0b959273ffc5718c6d4cc7c"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
				"0000000000000000000000000000000000000000000000000000000000000001"
				"30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd45"
				"0a09ccf561b55fd99d1c1208dee1162457b57ac5af3759d50671e510e428b2a1"
				"2e539c423b302d13f4e5773c603948eaf5db5df8ae8a9a9113708390a06410d8"
				"19b763513924a736e4eebd0d78c91c1bc1d657fee4214057d21414011cfcc763"
				"2f8d9f9ab83727c77a2fec063cb7b6e5eb23044ccf535ad49d46d394fb6f6bf6"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(2)
			}
		},
		{
			fromHex(
				"0000000000000000000000000000000000000000000000000000000000000001"
				"0000000000000000000000000000000000000000000000000000000000000002"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
				"0000000000000000000000000000000000000000000000000000000000000001"
				"30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd45"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(2)
			}
		},
		{
			fromHex(
				"1c76476f4def4bb94541d57ebba1193381ffa7aa76ada664dd31c16024c43f59"
				"3034dd2920f673e204fee2811c678745fc819b55d3e9d294e45c9b03a76aef41"
				"209dd15ebff5d46c4bd888e51a93cf99a7329636c63514396b4a452003a35bf7"
				"04bf11ca01483bfa8b34b43561848d28905960114c8ac04049af4b6315a41678"
				"2bb8324af6cfc93537a2ad1a445cfd0ca2a71acd7ac41fadbf933c2a51be344d"
				"120a2a4cf30c1bf9845f20c6fe39e07ea2cce61f0c9bb048165fe5e4de877550"
				"111e129f1cf1097710d41c4ac70fcdfa5ba2023c6ff1cbeac322de49d1b6df7c"
				"2032c61a830e3c17286de9462bf242fca2883585b93870a73853face6a6bf411"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(2)
			}
		},
		{
			fromHex(
				"2eca0c7238bf16e83e7a1e6c5d49540685ff51380f309842a98561558019fc02"
				"03d3260361bb8451de5ff5ecd17f010ff22f5c31cdf184e9020b06fa5997db84"
				"1213d2149b006137fcfb23036606f848d638d576a120ca981b5b1a5f9300b3ee"
				"2276cf730cf493cd95d64677bbb75fc42db72513a4c1e387b476d056f80aa75f"
				"21ee6226d31426322afcda621464d0611d226783262e21bb3bc86b537e986237"
				"096df1f82dff337dd5972e32a8ad43e28a78a96a823ef1cd4debe12b6552ea5f"
				"06967a1237ebfeca9aaae0d6d0bab8e28c198c5a339ef8a2407e31cdac516db9"
				"22160fa257a5fd5b280642ff47b65eca77e626cb685c84fa6d3b6882a283ddd1"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(2)
			}
		},
		{
			fromHex(
				"0f25929bcb43d5a57391564615c9e70a992b10eafa4db109709649cf48c50dd2"
				"16da2f5cb6be7a0aa72c440c53c9bbdfec6c36c7d515536431b3a865468acbba"
				"2e89718ad33c8bed92e210e81d1853435399a271913a6520736a4729cf0d51eb"
				"01a9e2ffa2e92599b68e44de5bcf354fa2642bd4f26b259daa6f7ce3ed57aeb3"
				"14a9a87b789a58af499b314e13c3d65bede56c07ea2d418d6874857b70763713"
				"178fb49a2d6cd347dc58973ff49613a20757d0fcc22079f9abd10c3baee24590"
				"1b9e027bd5cfc2cb5db82d4dc9677ac795ec500ecd47deee3b5da006d6d049b8"
				"11d7511c78158de484232fc68daf8a45cf217d1c2fae693ff5871e8752d73b21"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(2)
			}
		},
		{
			fromHex(
				"2f2ea0b3da1e8ef11914acf8b2e1b32d99df51f5f4f206fc6b947eae860eddb6"
				"068134ddb33dc888ef446b648d72338684d678d2eb2371c61a50734d78da4b72"
				"25f83c8b6ab9de74e7da488ef02645c5a16a6652c3c71a15dc37fe3a5dcb7cb1"
				"22acdedd6308e3bb230d226d16a105295f523a8a02bfc5e8bd2da135ac4c245d"
				"065bbad92e7c4e31bf3757f1fe7362a63fbfee50e7dc68da116e67d600d9bf68"
				"06d302580dc0661002994e7cd3a7f224e7ddc27802777486bf80f40e4ca3cfdb"
				"186bac5188a98c45e6016873d107f5cd131f3a3e339d0375e58bd6219347b008"
				"122ae2b09e539e152ec5364e7e2204b03d11d3caa038bfc7cd499f8176aacbee"
				"1f39e4e4afc4bc74790a4a028aff2c3d2538731fb755edefd8cb48d6ea589b5e"
				"283f150794b6736f670d6a1033f9b46c6f5204f50813eb85c8dc4b59db1c5d39"
				"140d97ee4d2b36d99bc49974d18ecca3e7ad51011956051b464d9e27d46cc25e"
				"0764bb98575bd466d32db7b15f582b2d5c452b36aa394b789366e5e3ca5aabd4"
				"15794ab061441e51d01e94640b7e3084a07e02c78cf3103c542bc5b298669f21"
				"1b88da1679b0b64a63b7e0e7bfe52aae524f73a55be7fe70c7e9bfc94b4cf0da"
				"1213d2149b006137fcfb23036606f848d638d576a120ca981b5b1a5f9300b3ee"
				"2276cf730cf493cd95d64677bbb75fc42db72513a4c1e387b476d056f80aa75f"
				"21ee6226d31426322afcda621464d0611d226783262e21bb3bc86b537e986237"
				"096df1f82dff337dd5972e32a8ad43e28a78a96a823ef1cd4debe12b6552ea5f"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(3)
			}
		},
		{
			fromHex(
				"20a754d2071d4d53903e3b31a7e98ad6882d58aec240ef981fdf0a9d22c5926a"
				"29c853fcea789887315916bbeb89ca37edb355b4f980c9a12a94f30deeed3021"
				"1213d2149b006137fcfb23036606f848d638d576a120ca981b5b1a5f9300b3ee"
				"2276cf730cf493cd95d64677bbb75fc42db72513a4c1e387b476d056f80aa75f"
				"21ee6226d31426322afcda621464d0611d226783262e21bb3bc86b537e986237"
				"096df1f82dff337dd5972e32a8ad43e28a78a96a823ef1cd4debe12b6552ea5f"
				"1abb4a25eb9379ae96c84fff9f0540abcfc0a0d11aeda02d4f37e4baf74cb0c1"
				"1073b3ff2cdbb38755f8691ea59e9606696b3ff278acfc098fa8226470d03869"
				"217cee0a9ad79a4493b5253e2e4e3a39fc2df38419f230d341f60cb064a0ac29"
				"0a3d76f140db8418ba512272381446eb73958670f00cf46f1d9e64cba057b53c"
				"26f64a8ec70387a13e41430ed3ee4a7db2059cc5fc13c067194bcc0cb49a9855"
				"2fd72bd9edb657346127da132e5b82ab908f5816c826acb499e22f2412d1a2d7"
				"0f25929bcb43d5a57391564615c9e70a992b10eafa4db109709649cf48c50dd2"
				"198a1f162a73261f112401aa2db79c7dab1533c9935c77290a6ce3b191f2318d"
				"198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2"
				"1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed"
				"090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b"
				"12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa"
			),
			{
				fromHex("0000000000000000000000000000000000000000000000000000000000000001"),
				calc_cost(3)
			}
		}
	};
	return precompileGeneric(_message, inputOutput);
}

evmc::Result EVMHost::precompileBlake2f(evmc_message const&) noexcept
{
	// TODO implement
	return resultWithFailure();
}

evmc::Result EVMHost::precompileGeneric(
	evmc_message const& _message,
	map<bytes, EVMPrecompileOutput> const& _inOut) noexcept
{
	bytes input(_message.input_data, _message.input_data + _message.input_size);
	if (_inOut.count(input))
	{
		auto const& ret = _inOut.at(input);
		return resultWithGas(_message.gas, ret.gas_used, ret.output);
	}
	else
		return resultWithFailure();
}

evmc::Result EVMHost::resultWithFailure() noexcept
{
	evmc::Result result;
	result.status_code = EVMC_FAILURE;
	return result;
}

evmc::Result EVMHost::resultWithGas(
	int64_t gas_limit,
	int64_t gas_required,
	bytes const& _data
) noexcept
{
	evmc::Result result;
	if (gas_limit < gas_required)
	{
		result.status_code = EVMC_OUT_OF_GAS;
		result.gas_left = 0;
	}
	else
	{
		result.status_code = EVMC_SUCCESS;
		result.gas_left = gas_limit - gas_required;
	}
	result.output_data = _data.empty() ? nullptr : _data.data();
	result.output_size = _data.size();
	return result;
}

StorageMap const& EVMHost::get_address_storage(evmc::address const& _addr)
{
	assertThrow(account_exists(_addr), Exception, "Account does not exist.");
	return accounts[_addr].storage;
}

string EVMHostPrinter::state()
{
	// Print state and execution trace.
	if (m_host.account_exists(m_account))
	{
		storage();
		balance();
	}
	else
		selfdestructRecords();

	callRecords();
	return m_stateStream.str();
}

void EVMHostPrinter::storage()
{
	for (auto const& [slot, value]: m_host.get_address_storage(m_account))
		if (m_host.get_storage(m_account, slot))
			m_stateStream << "  "
				<< m_host.convertFromEVMC(slot)
				<< ": "
				<< m_host.convertFromEVMC(value.current)
				<< endl;
}

void EVMHostPrinter::balance()
{
	m_stateStream << "BALANCE "
		<< m_host.convertFromEVMC(m_host.get_balance(m_account))
		<< endl;
}

void EVMHostPrinter::selfdestructRecords()
{
	for (auto const& record: m_host.recorded_selfdestructs)
		for (auto const& beneficiary: record.second)
			m_stateStream << "SELFDESTRUCT"
				<< " BENEFICIARY "
				<< m_host.convertFromEVMC(beneficiary)
				<< endl;
}

void EVMHostPrinter::callRecords()
{
	static auto constexpr callKind = [](evmc_call_kind _kind) -> string
	{
		switch (_kind)
		{
			case evmc_call_kind::EVMC_CALL:
				return "CALL";
			case evmc_call_kind::EVMC_DELEGATECALL:
				return "DELEGATECALL";
			case evmc_call_kind::EVMC_CALLCODE:
				return "CALLCODE";
			case evmc_call_kind::EVMC_CREATE:
				return "CREATE";
			case evmc_call_kind::EVMC_CREATE2:
				return "CREATE2";
			default:
				assertThrow(false, Exception, "Invalid call kind.");
		}
	};

	for (auto const& record: m_host.recorded_calls)
		m_stateStream << callKind(record.kind)
			<< " VALUE "
			<< m_host.convertFromEVMC(record.value)
			<< endl;
}
