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
/** @file GasMeter.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 *
 * Utilities for tracking gas costs.
 *
 * With respect to EIP-2929, we do not track warm accounts or storage slots and they are always
 * charged the worst-case, i.e., cold-access.
 */

#pragma once

#include <libevmasm/ExpressionClasses.h>
#include <libevmasm/AssemblyItem.h>

#include <liblangutil/EVMVersion.h>

#include <ostream>
#include <tuple>
#include <utility>

namespace solidity::evmasm
{

class KnownState;

namespace GasCosts
{
	/// NOTE: The GAS_... constants referenced by comments are defined for each EVM version in the Execution Specs:
	/// https://ethereum.github.io/execution-specs/autoapi/ethereum/<evm version>/vm/gas/index.html

	static unsigned const stackLimit = 1024;
	static unsigned const tier0Gas = 0;                       // GAS_ZERO (in Execution Specs)
	static unsigned const tier1Gas = 2;                       // GAS_BASE
	static unsigned const tier2Gas = 3;                       // GAS_VERY_LOW
	static unsigned const tier3Gas = 5;                       // GAS_LOW / GAS_FAST_STEP
	static unsigned const tier4Gas = 8;                       // GAS_MID
	static unsigned const tier5Gas = 10;                      // GAS_HIGH
	static unsigned const tier6Gas = 20;                      // GAS_BLOCK_HASH
	static unsigned const expGas = 10;                        // GAS_EXPONENTIATION
	inline unsigned expByteGas(langutil::EVMVersion _evmVersion)
	{
		return _evmVersion >= langutil::EVMVersion::spuriousDragon() ? 50 : 10; // GAS_EXPONENTIATION_PER_BYTE
	}
	static unsigned const keccak256Gas = 30;                  // GAS_KECCAK256
	static unsigned const keccak256WordGas = 6;               // GAS_KECCAK256_WORD
	/// Corresponds to ACCESS_LIST_ADDRESS_COST from EIP-2930
	static unsigned const accessListAddressCost = 2400;
	/// Corresponds to ACCESS_LIST_STORAGE_COST from EIP-2930
	static unsigned const accessListStorageKeyCost = 1900;
	/// Corresponds to COLD_SLOAD_COST from EIP-2929
	static unsigned const coldSloadCost = 2100;               // GAS_COLD_SLOAD
	/// Corresponds to COLD_ACCOUNT_ACCESS_COST from EIP-2929
	static unsigned const coldAccountAccessCost = 2600;       // GAS_COLD_ACCOUNT_ACCESS
	/// Corresponds to WARM_STORAGE_READ_COST from EIP-2929
	static unsigned const warmStorageReadCost = 100;          // GAS_WARM_ACCESS
	inline unsigned sloadGas(langutil::EVMVersion _evmVersion)
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return coldSloadCost;
		else if (_evmVersion >= langutil::EVMVersion::istanbul())
			return 800;
		else if (_evmVersion >= langutil::EVMVersion::tangerineWhistle())
			return 200;
		else
			return 50;
	}
	/// Corresponds to SSTORE_SET_GAS
	static unsigned const sstoreSetGas = 20000;               // GAS_STORAGE_SET
	/// Corresponds to SSTORE_RESET_GAS from EIP-2929
	static unsigned const sstoreResetGas = 5000 - coldSloadCost;
	/// Corresponds to SSTORE_CLEARS_SCHEDULE from EIP-2200
	inline static unsigned sstoreClearsSchedule(langutil::EVMVersion _evmVersion)
	{
		// Changes from EIP-3529
		if (_evmVersion >= langutil::EVMVersion::london())
			return sstoreResetGas + accessListStorageKeyCost;
		else
			return 15000;
	}
	inline static unsigned totalSstoreSetGas(langutil::EVMVersion _evmVersion)
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return sstoreSetGas + coldSloadCost;
		else
			return sstoreSetGas;
	}
	/// Corresponds to SSTORE_RESET_GAS from EIP-2929
	/// For Berlin, the maximum is SSTORE_RESET_GAS + COLD_SLOAD_COST = 5000
	/// For previous versions, it's a fixed 5000
	inline unsigned totalSstoreResetGas(langutil::EVMVersion _evmVersion)
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return sstoreResetGas + coldSloadCost;
		else
			return 5000;
	}
	inline unsigned extCodeGas(langutil::EVMVersion _evmVersion)
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return coldAccountAccessCost;
		else if (_evmVersion >= langutil::EVMVersion::tangerineWhistle())
			return 700;
		else
			return 20;
	}
	inline unsigned balanceGas(langutil::EVMVersion _evmVersion)
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return coldAccountAccessCost;
		else if (_evmVersion >= langutil::EVMVersion::istanbul())
			return 700;
		else if (_evmVersion >= langutil::EVMVersion::tangerineWhistle())
			return 400;
		else
			return 20;
	}
	static unsigned const jumpdestGas = 1;                    // GAS_JUMPDEST
	static unsigned const logGas = 375;                       // GAS_LOG
	static unsigned const logDataGas = 8;                     // GAS_LOG_DATA
	static unsigned const logTopicGas = 375;                  // GAS_LOG_TOPIC
	static unsigned const createGas = 32000;                  // GAS_CREATE
	inline unsigned callGas(langutil::EVMVersion _evmVersion)
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return coldAccountAccessCost;
		else if (_evmVersion >= langutil::EVMVersion::tangerineWhistle())
			return 700;
		else
			return 40;
	}
	static unsigned const callStipend = 2300;                  // GAS_CALL_STIPEND
	static unsigned const callValueTransferGas = 9000;         // GAS_CALL_VALUE
	static unsigned const callNewAccountGas = 25000;           // GAS_NEW_ACCOUNT / GAS_SELF_DESTRUCT_NEW_ACCOUNT
	inline unsigned selfdestructGas(langutil::EVMVersion _evmVersion) // GAS_SELF_DESTRUCT
	{
		if (_evmVersion >= langutil::EVMVersion::berlin())
			return coldAccountAccessCost;
		else if (_evmVersion >= langutil::EVMVersion::tangerineWhistle())
			return 5000;
		else
			return 0;
	}
	inline unsigned selfdestructRefundGas(langutil::EVMVersion _evmVersion)
	{
		// Changes from EIP-3529
		if (_evmVersion >= langutil::EVMVersion::london())
			return 0;
		else
			return 24000;
	}
	static unsigned const memoryGas = 3;                       // GAS_MEMORY / GAS_COPY / GAS_RETURN_DATA_COPY
	static unsigned const quadCoeffDiv = 512;
	static unsigned const createDataGas = 200;                 // GAS_CODE_DEPOSIT
	static unsigned const txGas = 21000;
	static unsigned const txCreateGas = 53000;
	static unsigned const txDataZeroGas = 4;
	inline unsigned txDataNonZeroGas(langutil::EVMVersion _evmVersion)
	{
		return _evmVersion >= langutil::EVMVersion::istanbul() ? 16 : 68;
	}
	static unsigned const copyGas = 3;
}

/**
 * Class that helps computing the maximum gas consumption for instructions.
 * Has to be initialized with a certain known state that will be automatically updated for
 * each call to estimateMax. These calls have to supply strictly subsequent AssemblyItems.
 * A new gas meter has to be constructed (with a new state) for control flow changes.
 */
class GasMeter
{
public:
	struct GasConsumption
	{
		GasConsumption(unsigned _value = 0, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		GasConsumption(u256 _value, bool _infinite = false): value(std::move(_value)), isInfinite(_infinite) {}
		static GasConsumption infinite() { return GasConsumption(0, true); }

		GasConsumption& operator+=(GasConsumption const& _other);
		GasConsumption operator+(GasConsumption const& _other) const
		{
			GasConsumption result = *this;
			result += _other;
			return result;
		}
		bool operator<(GasConsumption const& _other) const
		{
			return std::make_pair(isInfinite, value) < std::make_pair(_other.isInfinite, _other.value);
		}

		u256 value;
		bool isInfinite;
	};

	/// Constructs a new gas meter given the current state.
	GasMeter(std::shared_ptr<KnownState>  _state, langutil::EVMVersion _evmVersion, u256  _largestMemoryAccess = 0):
		m_state(std::move(_state)), m_evmVersion(_evmVersion), m_largestMemoryAccess(std::move(_largestMemoryAccess)) {}

	/// @returns an upper bound on the gas consumed by the given instruction and updates
	/// the state.
	/// @param _inculdeExternalCosts if true, include costs caused by other contracts in calls.
	GasConsumption estimateMax(AssemblyItem const& _item, bool _includeExternalCosts = true);

	u256 const& largestMemoryAccess() const { return m_largestMemoryAccess; }

	/// @returns gas costs for simple instructions with constant gas costs (that do not
	/// change with EVM versions)
	static unsigned runGas(Instruction _instruction, langutil::EVMVersion _evmVersion);

	/// @returns the gas cost of the supplied data, depending whether it is in creation code, or not.
	/// In case of @a _inCreation, the data is only sent as a transaction and is not stored, whereas
	/// otherwise code will be stored and have to pay "createDataGas" cost.
	static u256 dataGas(bytes const& _data, bool _inCreation, langutil::EVMVersion _evmVersion);

	/// @returns the gas cost of non-zero data of the supplied length, depending whether it is in creation code, or not.
	/// In case of @a _inCreation, the data is only sent as a transaction and is not stored, whereas
	/// otherwise code will be stored and have to pay "createDataGas" cost.
	static u256 dataGas(uint64_t _length, bool _inCreation, langutil::EVMVersion _evmVersion);

private:
	/// @returns _multiplier * (_value + 31) / 32, if _value is a known constant and infinite otherwise.
	GasConsumption wordGas(u256 const& _multiplier, ExpressionClasses::Id _value);
	/// @returns the gas needed to access the given memory position.
	/// @todo this assumes that memory was never accessed before and thus over-estimates gas usage.
	GasConsumption memoryGas(ExpressionClasses::Id _position);
	/// @returns the memory gas for accessing the memory at a specific offset for a number of bytes
	/// given as values on the stack at the given relative positions.
	GasConsumption memoryGas(int _stackPosOffset, int _stackPosSize);

	std::shared_ptr<KnownState> m_state;
	langutil::EVMVersion m_evmVersion;
	/// Largest point where memory was accessed since the creation of this object.
	u256 m_largestMemoryAccess;
};

inline std::ostream& operator<<(std::ostream& _str, GasMeter::GasConsumption const& _consumption)
{
	if (_consumption.isInfinite)
		return _str << "[???]";
	else
		return _str << std::dec << _consumption.value;
}

}
