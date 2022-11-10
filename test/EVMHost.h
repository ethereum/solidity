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

#pragma once

#include <test/evmc/mocked_host.hpp>
#include <test/evmc/evmc.hpp>
#include <test/evmc/evmc.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>

#include <boost/filesystem.hpp>

namespace solidity::test
{
using Address = util::h160;
using StorageMap = std::map<evmc::bytes32, evmc::StorageValue>;

struct EVMPrecompileOutput {
	bytes const output;
	int64_t gas_used;
};

class EVMHost: public evmc::MockedHost
{
public:
	// Verbatim features of MockedHost.
	using MockedHost::account_exists;
	using MockedHost::get_storage;
	using MockedHost::set_storage;
	using MockedHost::get_balance;
	using MockedHost::get_code_size;
	using MockedHost::get_code_hash;
	using MockedHost::copy_code;
	using MockedHost::get_tx_context;
	using MockedHost::emit_log;
	using MockedHost::access_account;
	using MockedHost::access_storage;

	// Modified features of MockedHost.
	bool selfdestruct(evmc::address const& _addr, evmc::address const& _beneficiary) noexcept final;
	evmc::Result call(evmc_message const& _message) noexcept final;
	evmc::bytes32 get_block_hash(int64_t number) const noexcept final;

	// Solidity testing specific features.

	/// Tries to dynamically load an evmc vm supporting evm1 or ewasm and caches the loaded VM.
	/// @returns vmc::VM(nullptr) on failure.
	static evmc::VM& getVM(std::string const& _path = {});

	/// Tries to load all defined evmc vm shared libraries.
	/// @param _vmPaths paths to multiple evmc shared libraries.
	/// @throw Exception if multiple evm1 or multiple ewasm evmc vms where loaded.
	/// @returns A pair of booleans, the first element being true, if an evmc vm supporting evm1 was loaded properly,
	///          the second being true, if an evmc vm supporting ewasm was loaded properly.
	static std::tuple<bool, bool> checkVmPaths(std::vector<boost::filesystem::path> const& _vmPaths);

	explicit EVMHost(langutil::EVMVersion _evmVersion, evmc::VM& _vm);

	/// Reset entire state (including accounts).
	void reset();

	/// Start new block.
	void newBlock()
	{
		tx_context.block_number++;
		tx_context.block_timestamp += 15;
		recorded_logs.clear();
		newTransactionFrame();
	}

	/// @returns contents of storage at @param _addr.
	StorageMap const& get_address_storage(evmc::address const& _addr);

	static Address convertFromEVMC(evmc::address const& _addr);
	static evmc::address convertToEVMC(Address const& _addr);
	static util::h256 convertFromEVMC(evmc::bytes32 const& _data);
	static evmc::bytes32 convertToEVMC(util::h256 const& _data);
private:
	/// Transfer value between accounts. Checks for sufficient balance.
	void transfer(evmc::MockedAccount& _sender, evmc::MockedAccount& _recipient, u256 const& _value) noexcept;

	/// Start a new transaction frame.
	/// This will perform selfdestructs, apply storage status changes across all accounts,
	/// and clear account/storage access indicator for EIP-2929.
	void newTransactionFrame();

	/// Records calls made via @param _message.
	void recordCalls(evmc_message const& _message) noexcept;

	static evmc::Result precompileECRecover(evmc_message const& _message) noexcept;
	static evmc::Result precompileSha256(evmc_message const& _message) noexcept;
	static evmc::Result precompileRipeMD160(evmc_message const& _message) noexcept;
	static evmc::Result precompileIdentity(evmc_message const& _message) noexcept;
	static evmc::Result precompileModExp(evmc_message const& _message) noexcept;
	template <evmc_revision Revision>
	static evmc::Result precompileALTBN128G1Add(evmc_message const& _message) noexcept;
	template <evmc_revision Revision>
	static evmc::Result precompileALTBN128G1Mul(evmc_message const& _message) noexcept;
	template <evmc_revision Revision>
	static evmc::Result precompileALTBN128PairingProduct(evmc_message const& _message) noexcept;
	static evmc::Result precompileGeneric(evmc_message const& _message, std::map<bytes, EVMPrecompileOutput> const& _inOut) noexcept;
	/// @returns a result object with gas usage and result data taken from @a _data.
	/// The outcome will be a failure if the limit < required.
	/// @note The return value is only valid as long as @a _data is alive!
	static evmc::Result resultWithGas(int64_t gas_limit, int64_t gas_required, bytes const& _data) noexcept;
	static evmc::Result resultWithFailure() noexcept;

	evmc::VM& m_vm;
	/// EVM version requested by the testing tool
	langutil::EVMVersion m_evmVersion;
	/// EVM version requested from EVMC (matches the above)
	evmc_revision m_evmRevision;
};

class EVMHostPrinter
{
public:
	/// Constructs a host printer object for state at @param _address.
	explicit EVMHostPrinter(EVMHost& _host, evmc::address _address):
		m_host(_host),
		m_account(_address)
	{}
	/// @returns state at account maintained by host.
	std::string state();
private:
	/// Outputs storage at account to stateStream.
	void storage();
	/// Outputs call records for account to stateStream.
	void callRecords();
	/// Outputs balance of account to stateStream.
	void balance();
	/// Outputs self-destruct record for account to stateStream.
	void selfdestructRecords();

	std::ostringstream m_stateStream;
	EVMHost& m_host;
	evmc::address m_account;
};

}
