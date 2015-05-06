/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file GasMeter.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#include "GasMeter.h"
#include <libevmcore/Params.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

GasMeter::GasConsumption& GasMeter::GasConsumption::operator+=(GasConsumption const& _other)
{
	isInfinite = isInfinite || _other.isInfinite;
	if (isInfinite)
		return *this;
	bigint v = bigint(value) + _other.value;
	if (v > std::numeric_limits<u256>::max())
		isInfinite = true;
	else
		value = u256(v);
	return *this;
}

GasMeter::GasConsumption GasMeter::estimateMax(AssemblyItem const& _item)
{
	switch (_item.type()) {
	case Push:
	case PushTag:
		return runGas(Instruction::PUSH1);
	case Tag:
		return runGas(Instruction::JUMPDEST);
	case Operation:
	{
		GasConsumption gas = runGas(_item.instruction());
		switch (_item.instruction())
		{
		case Instruction::SSTORE:
			// @todo logic can be improved
			gas += c_sstoreSetGas;
			break;
		case Instruction::SLOAD:
			gas += c_sloadGas;
			break;
		case Instruction::MSTORE:
		case Instruction::MSTORE8:
		case Instruction::MLOAD:
		case Instruction::RETURN:
		case Instruction::SHA3:
		case Instruction::CALLDATACOPY:
		case Instruction::CODECOPY:
		case Instruction::EXTCODECOPY:
		case Instruction::LOG0:
		case Instruction::LOG1:
		case Instruction::LOG2:
		case Instruction::LOG3:
		case Instruction::LOG4:
		case Instruction::CALL:
		case Instruction::CALLCODE:
		case Instruction::CREATE:
		case Instruction::EXP:
			// @todo logic can be improved
			gas = GasConsumption::infinite();
			break;
		default:
			break;
		}
		return gas;
		break;
	}
	default:
		break;
	}

	return GasConsumption::infinite();
}

GasMeter::GasConsumption GasMeter::runGas(Instruction _instruction)
{
	if (_instruction == Instruction::JUMPDEST)
		return GasConsumption(1);

	int tier = instructionInfo(_instruction).gasPriceTier;
	return tier == InvalidTier ? GasConsumption::infinite() : c_tierStepGas[tier];
}


