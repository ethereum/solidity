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

#include <libevmasm/GasMeter.h>

#include <libevmasm/KnownState.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;

GasMeter::GasConsumption& GasMeter::GasConsumption::operator+=(GasConsumption const& _other)
{
	if (_other.isInfinite && !isInfinite)
		*this = infinite();
	if (isInfinite)
		return *this;
	bigint v = bigint(value) + _other.value;
	if (v > numeric_limits<u256>::max())
		*this = infinite();
	else
		value = u256(v);
	return *this;
}

GasMeter::GasConsumption GasMeter::estimateMax(AssemblyItem const& _item, bool _includeExternalCosts)
{
	GasConsumption gas;
	switch (_item.type())
	{
	case Push:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
	case PushDeployTimeAddress:
		gas = runGas(InternalInstruction::PUSH1);
		break;
	case Tag:
		gas = runGas(InternalInstruction::JUMPDEST);
		break;
	case Operation:
	{
		ExpressionClasses& classes = m_state->expressionClasses();
		switch (_item.instruction())
		{
		case InternalInstruction::SSTORE:
		{
			ExpressionClasses::Id slot = m_state->relativeStackElement(0);
			ExpressionClasses::Id value = m_state->relativeStackElement(-1);
			if (classes.knownZero(value) || (
				m_state->storageContent().count(slot) &&
				classes.knownNonZero(m_state->storageContent().at(slot))
			))
				gas = GasCosts::totalSstoreResetGas(m_evmVersion); //@todo take refunds into account
			else
				gas = GasCosts::totalSstoreSetGas(m_evmVersion);
			break;
		}
		case InternalInstruction::SLOAD:
			gas = GasCosts::sloadGas(m_evmVersion);
			break;
		case InternalInstruction::RETURN:
		case InternalInstruction::REVERT:
			gas = runGas(_item.instruction());
			gas += memoryGas(0, -1);
			break;
		case InternalInstruction::MLOAD:
		case InternalInstruction::MSTORE:
			gas = runGas(_item.instruction());
			gas += memoryGas(classes.find(InternalInstruction::ADD, {
				m_state->relativeStackElement(0),
				classes.find(AssemblyItem(32))
			}));
			break;
		case InternalInstruction::MSTORE8:
			gas = runGas(_item.instruction());
			gas += memoryGas(classes.find(InternalInstruction::ADD, {
				m_state->relativeStackElement(0),
				classes.find(AssemblyItem(1))
			}));
			break;
		case InternalInstruction::KECCAK256:
			gas = GasCosts::keccak256Gas;
			gas += memoryGas(0, -1);
			gas += wordGas(GasCosts::keccak256WordGas, m_state->relativeStackElement(-1));
			break;
		case InternalInstruction::CALLDATACOPY:
		case InternalInstruction::CODECOPY:
		case InternalInstruction::RETURNDATACOPY:
			gas = runGas(_item.instruction());
			gas += memoryGas(0, -2);
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-2));
			break;
		case InternalInstruction::EXTCODESIZE:
			gas = GasCosts::extCodeGas(m_evmVersion);
			break;
		case InternalInstruction::EXTCODEHASH:
			gas = GasCosts::balanceGas(m_evmVersion);
			break;
		case InternalInstruction::EXTCODECOPY:
			gas = GasCosts::extCodeGas(m_evmVersion);
			gas += memoryGas(-1, -3);
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-3));
			break;
		case InternalInstruction::LOG0:
		case InternalInstruction::LOG1:
		case InternalInstruction::LOG2:
		case InternalInstruction::LOG3:
		case InternalInstruction::LOG4:
		{
			gas = GasCosts::logGas + GasCosts::logTopicGas * getLogNumber(_item.instruction());
			gas += memoryGas(0, -1);
			if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(-1)))
				gas += GasCosts::logDataGas * (*value);
			else
				gas = GasConsumption::infinite();
			break;
		}
		case InternalInstruction::CALL:
		case InternalInstruction::CALLCODE:
		case InternalInstruction::DELEGATECALL:
		case InternalInstruction::STATICCALL:
		{
			if (_includeExternalCosts)
				// We assume that we do not know the target contract and thus, the consumption is infinite.
				gas = GasConsumption::infinite();
			else
			{
				gas = GasCosts::callGas(m_evmVersion);
				if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(0)))
					gas += (*value);
				else
					gas = GasConsumption::infinite();
				if (_item.instruction() == InternalInstruction::CALL)
					gas += GasCosts::callNewAccountGas; // We very rarely know whether the address exists.
				int valueSize = 1;
				if (_item.instruction() == InternalInstruction::DELEGATECALL || _item.instruction() == InternalInstruction::STATICCALL)
					valueSize = 0;
				else if (!classes.knownZero(m_state->relativeStackElement(-1 - valueSize)))
					gas += GasCosts::callValueTransferGas;
				gas += memoryGas(-2 - valueSize, -3 - valueSize);
				gas += memoryGas(-4 - valueSize, -5 - valueSize);
			}
			break;
		}
		case InternalInstruction::SELFDESTRUCT:
			gas = GasCosts::selfdestructGas(m_evmVersion);
			gas += GasCosts::callNewAccountGas; // We very rarely know whether the address exists.
			break;
		case InternalInstruction::CREATE:
		case InternalInstruction::CREATE2:
			if (_includeExternalCosts)
				// We assume that we do not know the target contract and thus, the consumption is infinite.
				gas = GasConsumption::infinite();
			else
			{
				gas = GasCosts::createGas;
				gas += memoryGas(-1, -2);
			}
			break;
		case InternalInstruction::EXP:
			gas = GasCosts::expGas;
			if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(-1)))
			{
				if (*value)
				{
					// Note: msb() counts from 0 and throws on 0 as input.
					unsigned const significantByteCount  = (static_cast<unsigned>(boost::multiprecision::msb(*value)) + 1u + 7u) / 8u;
					gas += GasCosts::expByteGas(m_evmVersion) * significantByteCount;
				}
			}
			else
				gas += GasCosts::expByteGas(m_evmVersion) * 32;
			break;
		case InternalInstruction::BALANCE:
			gas = GasCosts::balanceGas(m_evmVersion);
			break;
		case InternalInstruction::CHAINID:
			gas = runGas(InternalInstruction::CHAINID);
			break;
		case InternalInstruction::SELFBALANCE:
			gas = runGas(InternalInstruction::SELFBALANCE);
			break;
		default:
			gas = runGas(_item.instruction());
			break;
		}
		break;
	}
	default:
		gas = GasConsumption::infinite();
		break;
	}

	m_state->feedItem(_item);
	return gas;
}

GasMeter::GasConsumption GasMeter::wordGas(u256 const& _multiplier, ExpressionClasses::Id _value)
{
	u256 const* value = m_state->expressionClasses().knownConstant(_value);
	if (!value)
		return GasConsumption::infinite();
	return GasConsumption(_multiplier * ((*value + 31) / 32));
}

GasMeter::GasConsumption GasMeter::memoryGas(ExpressionClasses::Id _position)
{
	u256 const* value = m_state->expressionClasses().knownConstant(_position);
	if (!value)
		return GasConsumption::infinite();
	if (*value < m_largestMemoryAccess)
		return GasConsumption(0);
	u256 previous = m_largestMemoryAccess;
	m_largestMemoryAccess = *value;
	auto memGas = [=](u256 const& pos) -> u256
	{
		u256 size = (pos + 31) / 32;
		return GasCosts::memoryGas * size + size * size / GasCosts::quadCoeffDiv;
	};
	return memGas(*value) - memGas(previous);
}

GasMeter::GasConsumption GasMeter::memoryGas(int _stackPosOffset, int _stackPosSize)
{
	ExpressionClasses& classes = m_state->expressionClasses();
	if (classes.knownZero(m_state->relativeStackElement(_stackPosSize)))
		return GasConsumption(0);
	else
		return memoryGas(classes.find(InternalInstruction::ADD, {
			m_state->relativeStackElement(_stackPosOffset),
			m_state->relativeStackElement(_stackPosSize)
		}));
}

unsigned GasMeter::runGas(InternalInstruction _instruction)
{
	if (_instruction == InternalInstruction::JUMPDEST)
		return 1;

	switch (instructionInfo(_instruction).gasPriceTier)
	{
	case Tier::Zero:    return GasCosts::tier0Gas;
	case Tier::Base:    return GasCosts::tier1Gas;
	case Tier::VeryLow: return GasCosts::tier2Gas;
	case Tier::Low:     return GasCosts::tier3Gas;
	case Tier::Mid:     return GasCosts::tier4Gas;
	case Tier::High:    return GasCosts::tier5Gas;
	case Tier::Ext:     return GasCosts::tier6Gas;
	default: break;
	}
	assertThrow(false, OptimizerException, "Invalid gas tier for instruction " + instructionInfo(_instruction).name);
	return 0;
}

u256 GasMeter::dataGas(bytes const& _data, bool _inCreation, langutil::EVMVersion _evmVersion)
{
	bigint gas = 0;
	if (_inCreation)
	{
		for (auto b: _data)
			gas += (b != 0) ? GasCosts::txDataNonZeroGas(_evmVersion) : GasCosts::txDataZeroGas;
	}
	else
		gas = bigint(GasCosts::createDataGas) * _data.size();
	assertThrow(gas < bigint(u256(-1)), OptimizerException, "Gas cost exceeds 256 bits.");
	return u256(gas);
}


u256 GasMeter::dataGas(uint64_t _length, bool _inCreation, langutil::EVMVersion _evmVersion)
{
	bigint gas = bigint(_length) * (_inCreation ? GasCosts::txDataNonZeroGas(_evmVersion) : GasCosts::createDataGas);
	assertThrow(gas < bigint(u256(-1)), OptimizerException, "Gas cost exceeds 256 bits.");
	return u256(gas);
}
