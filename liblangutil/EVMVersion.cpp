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
 * EVM versioning.
 */

#include <liblangutil/EVMVersion.h>

using namespace langutil;
using namespace dev::eth;

bool EVMVersion::hasOpcode(Instruction _opcode) const
{
	switch (_opcode)
	{
	case Instruction::RETURNDATACOPY:
	case Instruction::RETURNDATASIZE:
		return supportsReturndata();
	case Instruction::STATICCALL:
		return hasStaticCall();
	case Instruction::SHL:
	case Instruction::SHR:
	case Instruction::SAR:
		return hasBitwiseShifting();
	case Instruction::CREATE2:
		return hasCreate2();
	case Instruction::EXTCODEHASH:
		return hasExtCodeHash();
	case Instruction::CHAINID:
		return hasChainID();
	case Instruction::SELFBALANCE:
		return hasSelfBalance();
	default:
		return true;
	}
}

