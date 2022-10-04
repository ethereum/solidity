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
 * EVM versioning.
 */

#include <liblangutil/EVMVersion.h>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;

bool EVMVersion::hasInstruction(InternalInstruction _instruction) const
{
	switch (_instruction)
	{
	case InternalInstruction::RETURNDATACOPY:
	case InternalInstruction::RETURNDATASIZE:
		return supportsReturndata();
	case InternalInstruction::STATICCALL:
		return hasStaticCall();
	case InternalInstruction::SHL:
	case InternalInstruction::SHR:
	case InternalInstruction::SAR:
		return hasBitwiseShifting();
	case InternalInstruction::CREATE2:
		return hasCreate2();
	case InternalInstruction::EXTCODEHASH:
		return hasExtCodeHash();
	case InternalInstruction::CHAINID:
		return hasChainID();
	case InternalInstruction::SELFBALANCE:
		return hasSelfBalance();
	case InternalInstruction::BASEFEE:
		return hasBaseFee();
	case InternalInstruction::DIFFICULTY:
		return hasDifficulty();
	case InternalInstruction::PREVRANDAO:
		return hasPrevRandao();
	default:
		return true;
	}
}
