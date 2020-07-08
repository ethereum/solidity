// SPDX-License-Identifier: GPL-3.0
/**
 * EVM versioning.
 */

#include <liblangutil/EVMVersion.h>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;

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

