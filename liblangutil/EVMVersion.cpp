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

bool EVMVersion::hasOpcode(Instruction _opcode, std::optional<uint8_t> _eofVersion) const
{
	// EOF version can be only defined since prague
	assert(!_eofVersion.has_value() || this->m_version >= prague());

	switch (_opcode)
	{
	case Instruction::RETURNDATACOPY:
	case Instruction::RETURNDATASIZE:
		return supportsReturndata();
	case Instruction::STATICCALL:
		return !_eofVersion.has_value() && hasStaticCall();
	case Instruction::SHL:
	case Instruction::SHR:
	case Instruction::SAR:
		return hasBitwiseShifting();
	case Instruction::CREATE2:
		return !_eofVersion.has_value() && hasCreate2();
	case Instruction::EXTCODEHASH:
		return !_eofVersion.has_value() && hasExtCodeHash();
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
	case Instruction::MCOPY:
		return hasMcopy();
	case Instruction::TSTORE:
	case Instruction::TLOAD:
		return supportsTransientStorage();
	// Instructions below are deprecated in EOF
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::SELFDESTRUCT:
	case Instruction::JUMP:
	case Instruction::JUMPI:
	case Instruction::PC:
	case Instruction::CREATE:
	case Instruction::CODESIZE:
	case Instruction::CODECOPY:
	case Instruction::EXTCODESIZE:
	case Instruction::EXTCODECOPY:
	case Instruction::GAS:
		return !_eofVersion.has_value();
	// Instructions below available only in EOF
	case Instruction::EOFCREATE:
	case Instruction::RETURNCONTRACT:
	case Instruction::DATALOADN:
		return _eofVersion.has_value();
	default:
		return true;
	}
}
