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

#include <test/evmc/mocked_host.hpp>
#include <test/evmc/evmc.hpp>
#include <test/evmc/evmc.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>

#include <boost/filesystem.hpp>

namespace solidity::test
{
using Address = util::h160;

class EVMHost: public evmc::MockedHost
{
public:
	using MockedHost::get_code_size;
	using MockedHost::get_balance;

	/// Tries to dynamically load libevmone. @returns nullptr on failure.
	/// The path has to be provided for the first successful run and will be ignored
	/// afterwards.
	static evmc::VM& getVM(std::string const& _path = {});

	static bool checkVmPaths(std::vector<boost::filesystem::path> const& _vmPaths);

	explicit EVMHost(langutil::EVMVersion _evmVersion, evmc::VM& _vm = getVM());

	void reset() { accounts.clear(); m_currentAddress = {}; }
	void newBlock()
	{
		tx_context.block_number++;
		tx_context.block_timestamp += 15;
		recorded_logs.clear();
	}

	bool account_exists(evmc::address const& _addr) const noexcept final
	{
		return evmc::MockedHost::account_exists(_addr);
	}

	void selfdestruct(evmc::address const& _addr, evmc::address const& _beneficiary) noexcept final;

	evmc::result call(evmc_message const& _message) noexcept final;

	evmc::bytes32 get_block_hash(int64_t number) const noexcept final;

	static Address convertFromEVMC(evmc::address const& _addr);
	static evmc::address convertToEVMC(Address const& _addr);
	static util::h256 convertFromEVMC(evmc::bytes32 const& _data);
	static evmc::bytes32 convertToEVMC(util::h256 const& _data);

	/// Checks if the VM has the given capability.
	bool hasCapability(evmc_capabilities capability) const noexcept
	{
		return m_vm.has_capability(capability);
	}

private:
	evmc::address m_currentAddress = {};

	static evmc::result precompileECRecover(evmc_message const& _message) noexcept;
	static evmc::result precompileSha256(evmc_message const& _message) noexcept;
	static evmc::result precompileRipeMD160(evmc_message const& _message) noexcept;
	static evmc::result precompileIdentity(evmc_message const& _message) noexcept;
	static evmc::result precompileModExp(evmc_message const& _message) noexcept;
	static evmc::result precompileALTBN128G1Add(evmc_message const& _message) noexcept;
	static evmc::result precompileALTBN128G1Mul(evmc_message const& _message) noexcept;
	static evmc::result precompileALTBN128PairingProduct(evmc_message const& _message) noexcept;
	static evmc::result precompileGeneric(evmc_message const& _message, std::map<bytes, bytes> const& _inOut) noexcept;
	/// @returns a result object with no gas usage and result data taken from @a _data.
	/// @note The return value is only valid as long as @a _data is alive!
	static evmc::result resultWithGas(evmc_message const& _message, bytes const& _data) noexcept;

	evmc::VM& m_vm;
	// EVM version requested by the testing tool
	langutil::EVMVersion m_evmVersion;
	// EVM version requested from EVMC (matches the above)
	evmc_revision m_evmRevision;
};


}
