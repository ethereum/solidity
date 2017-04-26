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
/** @file GasMeter.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#include "GasMeter.h"
#include <libevmasm/KnownState.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

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
	case PushString:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
		gas = runGas(Instruction::PUSH1);
		break;
	case Tag:
		gas = runGas(Instruction::JUMPDEST);
		break;
	case Operation:
	{
		ExpressionClasses& classes = m_state->expressionClasses();
		gas = runGas(_item.instruction());
		switch (_item.instruction())
		{
		case Instruction::SSTORE:
		{
			ExpressionClasses::Id slot = m_state->relativeStackElement(0);
			ExpressionClasses::Id value = m_state->relativeStackElement(-1);
			if (classes.knownZero(value) || (
				m_state->storageContent().count(slot) &&
				classes.knownNonZero(m_state->storageContent().at(slot))
			))
				gas += GasCosts::sstoreResetGas; //@todo take refunds into account
			else
				gas += GasCosts::sstoreSetGas;
			break;
		}
		case Instruction::SLOAD:
			gas += GasCosts::sloadGas;
			break;
		case Instruction::RETURN:
		case Instruction::REVERT:
			gas += memoryGas(0, -1);
			break;
		case Instruction::MLOAD:
		case Instruction::MSTORE:
			gas += memoryGas(classes.find(Instruction::ADD, {
				m_state->relativeStackElement(0),
				classes.find(AssemblyItem(32))
			}));
			break;
		case Instruction::MSTORE8:
			gas += memoryGas(classes.find(Instruction::ADD, {
				m_state->relativeStackElement(0),
				classes.find(AssemblyItem(1))
			}));
			break;
		case Instruction::SHA3:
			gas = GasCosts::sha3Gas;
			gas += wordGas(GasCosts::sha3WordGas, m_state->relativeStackElement(-1));
			gas += memoryGas(0, -1);
			break;
		case Instruction::CALLDATACOPY:
		case Instruction::CODECOPY:
			gas += memoryGas(0, -2);
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-2));
			break;
		case Instruction::EXTCODECOPY:
			gas += memoryGas(-1, -3);
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-3));
			break;
		case Instruction::LOG0:
		case Instruction::LOG1:
		case Instruction::LOG2:
		case Instruction::LOG3:
		case Instruction::LOG4:
		{
			unsigned n = unsigned(_item.instruction()) - unsigned(Instruction::LOG0);
			gas = GasCosts::logGas + GasCosts::logTopicGas * n;
			gas += memoryGas(0, -1);
			if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(-1)))
				gas += GasCosts::logDataGas * (*value);
			else
				gas = GasConsumption::infinite();
			break;
		}
		case Instruction::CALL:
		case Instruction::CALLCODE:
		case Instruction::DELEGATECALL:
		{
			if (_includeExternalCosts)
				// We assume that we do not know the target contract and thus, the consumption is infinite.
				gas = GasConsumption::infinite();
			else
			{
				gas = GasCosts::callGas;
				if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(0)))
					gas += (*value);
				else
					gas = GasConsumption::infinite();
				if (_item.instruction() == Instruction::CALL)
					gas += GasCosts::callNewAccountGas; // We very rarely know whether the address exists.
				int valueSize = _item.instruction() == Instruction::DELEGATECALL ? 0 : 1;
				if (!classes.knownZero(m_state->relativeStackElement(-1 - valueSize)))
					gas += GasCosts::callValueTransferGas;
				gas += memoryGas(-2 - valueSize, -3 - valueSize);
				gas += memoryGas(-4 - valueSize, -5 - valueSize);
			}
			break;
		}
		case Instruction::SELFDESTRUCT:
			gas = GasCosts::selfdestructGas;
			gas += GasCosts::callNewAccountGas; // We very rarely know whether the address exists.
			break;
		case Instruction::CREATE:
			if (_includeExternalCosts)
				// We assume that we do not know the target contract and thus, the consumption is infinite.
				gas = GasConsumption::infinite();
			else
			{
				gas = GasCosts::createGas;
				gas += memoryGas(-1, -2);
			}
			break;
		case Instruction::EXP:
			gas = GasCosts::expGas;
			if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(-1)))
				gas += GasCosts::expByteGas * (32 - (h256(*value).firstBitSet() / 8));
			else
				gas += GasCosts::expByteGas * 32;
			break;
		default:
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

GasMeter::GasConsumption GasMeter::wordGas(u256 const& _multiplier, ExpressionClasses::Id _position)
{
	u256 const* value = m_state->expressionClasses().knownConstant(_position);
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
		return GasConsumption(u256(0));
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
		return memoryGas(classes.find(Instruction::ADD, {
			m_state->relativeStackElement(_stackPosOffset),
			m_state->relativeStackElement(_stackPosSize)
		}));
}

unsigned GasMeter::runGas(Instruction _instruction)
{
	if (_instruction == Instruction::JUMPDEST)
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
	case Tier::Special: return GasCosts::tier7Gas;
	case Tier::ExtCode: return GasCosts::extCodeGas;
	case Tier::Balance: return GasCosts::balanceGas;
	default: break;
	}
	assertThrow(false, OptimizerException, "Invalid gas tier.");
	return 0;
}


