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
#include <libevmasm/Instruction.h>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;

bool EVMVersion::hasOpcode(Instruction _opcode) const
{
	switch (_opcode)
	{
	case Instruction::CLZ:
		return hasCLZ();
	case Instruction::CHAINID:
		return hasChainID();
	case Instruction::SELFBALANCE:
		return hasSelfBalance();
	case Instruction::BASEFEE:
		return hasBaseFee();
	case Instruction::BLOBHASH:
		return hasBlobHash();
	case Instruction::BLOBBASEFEE:
		return hasBlobBaseFee();
	case Instruction::SLOTNUM:
		return hasSlotNum();
	case Instruction::MCOPY:
		return hasMcopy();
	case Instruction::TSTORE:
	case Instruction::TLOAD:
		return supportsTransientStorage();
	default:
		return true;
	}
}
