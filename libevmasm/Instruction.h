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
/** @file Instruction.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libevmasm/Exceptions.h>
#include <libsolutil/Common.h>
#include <libsolutil/Assertions.h>

#include <boost/operators.hpp>

namespace solidity::langutil
{

/**
 * A version specifier of the EVM we want to compile to.
 * Defaults to the latest version deployed on Ethereum Mainnet at the time of compiler release.
 */
class EVMVersion;
}

namespace solidity::evmasm
{

/// Internal only enum of all instructions
enum class InternalInstruction: uint8_t
{
	STOP = 0,				///< halts execution
	ADD,				///< addition operation
	MUL,				///< multiplication operation
	SUB,				///< subtraction operation
	DIV,				///< integer division operation
	SDIV,				///< signed integer division operation
	MOD,				///< modulo remainder operation
	SMOD,				///< signed modulo remainder operation
	ADDMOD,				///< unsigned modular addition
	MULMOD,				///< unsigned modular multiplication
	EXP,				///< exponential operation
	SIGNEXTEND,			///< extend length of signed integer

	LT,					///< less-than comparison
	GT,					///< greater-than comparison
	SLT,				///< signed less-than comparison
	SGT,				///< signed greater-than comparison
	EQ,					///< equality comparison
	ISZERO,				///< simple not operator
	AND,				///< bitwise AND operation
	OR,					///< bitwise OR operation
	XOR,				///< bitwise XOR operation
	NOT,				///< bitwise NOT operation
	BYTE,				///< retrieve single byte from word
	SHL,				///< bitwise SHL operation
	SHR,				///< bitwise SHR operation
	SAR,				///< bitwise SAR operation

	KECCAK256,			///< compute KECCAK-256 hash

	ADDRESS,			///< get address of currently executing account
	BALANCE,			///< get balance of the given account
	ORIGIN,				///< get execution origination address
	CALLER,				///< get caller address
	CALLVALUE,			///< get deposited value by the instruction/transaction responsible for this execution
	CALLDATALOAD,		///< get input data of current environment
	CALLDATASIZE,		///< get size of input data in current environment
	CALLDATACOPY,		///< copy input data in current environment to memory
	CODESIZE,			///< get size of code running in current environment
	CODECOPY,			///< copy code running in current environment to memory
	GASPRICE,			///< get price of gas in current environment
	EXTCODESIZE,		///< get external code size (from another contract)
	EXTCODECOPY,		///< copy external code (from another contract)
	RETURNDATASIZE,		///< get size of return data buffer
	RETURNDATACOPY,		///< copy return data in current environment to memory
	EXTCODEHASH,		///< get external code hash (from another contract)

	BLOCKHASH,			///< get hash of most recent complete block
	COINBASE,			///< get the block's coinbase address
	TIMESTAMP,			///< get the block's timestamp
	NUMBER,				///< get the block's number
	DIFFICULTY,			///< get the block's difficulty
	PREVRANDAO,			///< get randomness provided by the beacon chain
	GASLIMIT,			///< get the block's gas limit
	CHAINID,			///< get the config's chainid param
	SELFBALANCE,		///< get balance of the current account
	BASEFEE,            ///< get the block's basefee

	POP,				///< remove item from stack
	MLOAD,				///< load word from memory
	MSTORE,				///< save word to memory
	MSTORE8,			///< save byte to memory
	SLOAD,				///< load word from storage
	SSTORE,				///< save word to storage
	JUMP,				///< alter the program counter
	JUMPI,				///< conditionally alter the program counter
	PC,					///< get the program counter
	MSIZE,				///< get the size of active memory
	GAS,				///< get the amount of available gas
	JUMPDEST,			///< set a potential jump destination

	PUSH1,				///< place 1 byte item on stack
	PUSH2,				///< place 2 byte item on stack
	PUSH3,				///< place 3 byte item on stack
	PUSH4,				///< place 4 byte item on stack
	PUSH5,				///< place 5 byte item on stack
	PUSH6,				///< place 6 byte item on stack
	PUSH7,				///< place 7 byte item on stack
	PUSH8,				///< place 8 byte item on stack
	PUSH9,				///< place 9 byte item on stack
	PUSH10,				///< place 10 byte item on stack
	PUSH11,				///< place 11 byte item on stack
	PUSH12,				///< place 12 byte item on stack
	PUSH13,				///< place 13 byte item on stack
	PUSH14,				///< place 14 byte item on stack
	PUSH15,				///< place 15 byte item on stack
	PUSH16,				///< place 16 byte item on stack
	PUSH17,				///< place 17 byte item on stack
	PUSH18,				///< place 18 byte item on stack
	PUSH19,				///< place 19 byte item on stack
	PUSH20,				///< place 20 byte item on stack
	PUSH21,				///< place 21 byte item on stack
	PUSH22,				///< place 22 byte item on stack
	PUSH23,				///< place 23 byte item on stack
	PUSH24,				///< place 24 byte item on stack
	PUSH25,				///< place 25 byte item on stack
	PUSH26,				///< place 26 byte item on stack
	PUSH27,				///< place 27 byte item on stack
	PUSH28,				///< place 28 byte item on stack
	PUSH29,				///< place 29 byte item on stack
	PUSH30,				///< place 30 byte item on stack
	PUSH31,				///< place 31 byte item on stack
	PUSH32,				///< place 32 byte item on stack

	DUP1,				///< copies the highest item in the stack to the top of the stack
	DUP2,				///< copies the second highest item in the stack to the top of the stack
	DUP3,				///< copies the third highest item in the stack to the top of the stack
	DUP4,				///< copies the 4th highest item in the stack to the top of the stack
	DUP5,				///< copies the 5th highest item in the stack to the top of the stack
	DUP6,				///< copies the 6th highest item in the stack to the top of the stack
	DUP7,				///< copies the 7th highest item in the stack to the top of the stack
	DUP8,				///< copies the 8th highest item in the stack to the top of the stack
	DUP9,				///< copies the 9th highest item in the stack to the top of the stack
	DUP10,				///< copies the 10th highest item in the stack to the top of the stack
	DUP11,				///< copies the 11th highest item in the stack to the top of the stack
	DUP12,				///< copies the 12th highest item in the stack to the top of the stack
	DUP13,				///< copies the 13th highest item in the stack to the top of the stack
	DUP14,				///< copies the 14th highest item in the stack to the top of the stack
	DUP15,				///< copies the 15th highest item in the stack to the top of the stack
	DUP16,				///< copies the 16th highest item in the stack to the top of the stack

	SWAP1,			///< swaps the highest and second highest value on the stack
	SWAP2,				///< swaps the highest and third highest value on the stack
	SWAP3,				///< swaps the highest and 4th highest value on the stack
	SWAP4,				///< swaps the highest and 5th highest value on the stack
	SWAP5,				///< swaps the highest and 6th highest value on the stack
	SWAP6,				///< swaps the highest and 7th highest value on the stack
	SWAP7,				///< swaps the highest and 8th highest value on the stack
	SWAP8,				///< swaps the highest and 9th highest value on the stack
	SWAP9,				///< swaps the highest and 10th highest value on the stack
	SWAP10,				///< swaps the highest and 11th highest value on the stack
	SWAP11,				///< swaps the highest and 12th highest value on the stack
	SWAP12,				///< swaps the highest and 13th highest value on the stack
	SWAP13,				///< swaps the highest and 14th highest value on the stack
	SWAP14,				///< swaps the highest and 15th highest value on the stack
	SWAP15,				///< swaps the highest and 16th highest value on the stack
	SWAP16,				///< swaps the highest and 17th highest value on the stack

	LOG0,				///< Makes a log entry; no topics.
	LOG1,				///< Makes a log entry; 1 topic.
	LOG2,				///< Makes a log entry; 2 topics.
	LOG3,				///< Makes a log entry; 3 topics.
	LOG4,				///< Makes a log entry; 4 topics.

	CREATE,				///< create a new account with associated code
	CALL,				///< message-call into an account
	CALLCODE,			///< message-call with another account's code only
	RETURN,				///< halt execution returning output data
	DELEGATECALL,		///< like CALLCODE but keeps caller's value and sender
	CREATE2,			///< create new account with associated code at address `sha3(0xff + sender + salt + init code) % 2**160`
	STATICCALL,			///< like CALL but disallow state modifications

	REVERT,				///< halt execution, revert state and return output data
	INVALID,			///< invalid instruction for expressing runtime errors (e.g., division-by-zero)
	SELFDESTRUCT,		///< halt execution and register account for later deletion
						///
	MAX_INTERNAL_INSTRUCTION ///< Marker for largest internal instruction value
};

/// Virtual machine bytecode instruction.
enum class InstructionOpCode: uint8_t
{
	STOP = 0x00,		///< halts execution
	ADD,				///< addition operation
	MUL,				///< multiplication operation
	SUB,				///< subtraction operation
	DIV,				///< integer division operation
	SDIV,				///< signed integer division operation
	MOD,				///< modulo remainder operation
	SMOD,				///< signed modulo remainder operation
	ADDMOD,				///< unsigned modular addition
	MULMOD,				///< unsigned modular multiplication
	EXP,				///< exponential operation
	SIGNEXTEND,			///< extend length of signed integer

	LT = 0x10,			///< less-than comparison
	GT,					///< greater-than comparison
	SLT,				///< signed less-than comparison
	SGT,				///< signed greater-than comparison
	EQ,					///< equality comparison
	ISZERO,				///< simple not operator
	AND,				///< bitwise AND operation
	OR,					///< bitwise OR operation
	XOR,				///< bitwise XOR operation
	NOT,				///< bitwise NOT operation
	BYTE,				///< retrieve single byte from word
	SHL,				///< bitwise SHL operation
	SHR,				///< bitwise SHR operation
	SAR,				///< bitwise SAR operation

	KECCAK256 = 0x20,		///< compute KECCAK-256 hash

	ADDRESS = 0x30,		///< get address of currently executing account
	BALANCE,			///< get balance of the given account
	ORIGIN,				///< get execution origination address
	CALLER,				///< get caller address
	CALLVALUE,			///< get deposited value by the instruction/transaction responsible for this execution
	CALLDATALOAD,		///< get input data of current environment
	CALLDATASIZE,		///< get size of input data in current environment
	CALLDATACOPY,		///< copy input data in current environment to memory
	CODESIZE,			///< get size of code running in current environment
	CODECOPY,			///< copy code running in current environment to memory
	GASPRICE,			///< get price of gas in current environment
	EXTCODESIZE,		///< get external code size (from another contract)
	EXTCODECOPY,		///< copy external code (from another contract)
	RETURNDATASIZE = 0x3d,	///< get size of return data buffer
	RETURNDATACOPY = 0x3e,	///< copy return data in current environment to memory
	EXTCODEHASH = 0x3f,	///< get external code hash (from another contract)

	BLOCKHASH = 0x40,	///< get hash of most recent complete block
	COINBASE,			///< get the block's coinbase address
	TIMESTAMP,			///< get the block's timestamp
	NUMBER,				///< get the block's number
	DIFFICULTY,			///< get the block's difficulty
	PREVRANDAO = DIFFICULTY,	///< get randomness provided by the beacon chain
	GASLIMIT = 0x45,	///< get the block's gas limit
	CHAINID,			///< get the config's chainid param
	SELFBALANCE,		///< get balance of the current account
	BASEFEE,            ///< get the block's basefee

	POP = 0x50,			///< remove item from stack
	MLOAD,				///< load word from memory
	MSTORE,				///< save word to memory
	MSTORE8,			///< save byte to memory
	SLOAD,				///< load word from storage
	SSTORE,				///< save word to storage
	JUMP,				///< alter the program counter
	JUMPI,				///< conditionally alter the program counter
	PC,					///< get the program counter
	MSIZE,				///< get the size of active memory
	GAS,				///< get the amount of available gas
	JUMPDEST,			///< set a potential jump destination

	PUSH1 = 0x60,		///< place 1 byte item on stack
	PUSH2,				///< place 2 byte item on stack
	PUSH3,				///< place 3 byte item on stack
	PUSH4,				///< place 4 byte item on stack
	PUSH5,				///< place 5 byte item on stack
	PUSH6,				///< place 6 byte item on stack
	PUSH7,				///< place 7 byte item on stack
	PUSH8,				///< place 8 byte item on stack
	PUSH9,				///< place 9 byte item on stack
	PUSH10,				///< place 10 byte item on stack
	PUSH11,				///< place 11 byte item on stack
	PUSH12,				///< place 12 byte item on stack
	PUSH13,				///< place 13 byte item on stack
	PUSH14,				///< place 14 byte item on stack
	PUSH15,				///< place 15 byte item on stack
	PUSH16,				///< place 16 byte item on stack
	PUSH17,				///< place 17 byte item on stack
	PUSH18,				///< place 18 byte item on stack
	PUSH19,				///< place 19 byte item on stack
	PUSH20,				///< place 20 byte item on stack
	PUSH21,				///< place 21 byte item on stack
	PUSH22,				///< place 22 byte item on stack
	PUSH23,				///< place 23 byte item on stack
	PUSH24,				///< place 24 byte item on stack
	PUSH25,				///< place 25 byte item on stack
	PUSH26,				///< place 26 byte item on stack
	PUSH27,				///< place 27 byte item on stack
	PUSH28,				///< place 28 byte item on stack
	PUSH29,				///< place 29 byte item on stack
	PUSH30,				///< place 30 byte item on stack
	PUSH31,				///< place 31 byte item on stack
	PUSH32,				///< place 32 byte item on stack

	DUP1 = 0x80,		///< copies the highest item in the stack to the top of the stack
	DUP2,				///< copies the second highest item in the stack to the top of the stack
	DUP3,				///< copies the third highest item in the stack to the top of the stack
	DUP4,				///< copies the 4th highest item in the stack to the top of the stack
	DUP5,				///< copies the 5th highest item in the stack to the top of the stack
	DUP6,				///< copies the 6th highest item in the stack to the top of the stack
	DUP7,				///< copies the 7th highest item in the stack to the top of the stack
	DUP8,				///< copies the 8th highest item in the stack to the top of the stack
	DUP9,				///< copies the 9th highest item in the stack to the top of the stack
	DUP10,				///< copies the 10th highest item in the stack to the top of the stack
	DUP11,				///< copies the 11th highest item in the stack to the top of the stack
	DUP12,				///< copies the 12th highest item in the stack to the top of the stack
	DUP13,				///< copies the 13th highest item in the stack to the top of the stack
	DUP14,				///< copies the 14th highest item in the stack to the top of the stack
	DUP15,				///< copies the 15th highest item in the stack to the top of the stack
	DUP16,				///< copies the 16th highest item in the stack to the top of the stack

	SWAP1 = 0x90,		///< swaps the highest and second highest value on the stack
	SWAP2,				///< swaps the highest and third highest value on the stack
	SWAP3,				///< swaps the highest and 4th highest value on the stack
	SWAP4,				///< swaps the highest and 5th highest value on the stack
	SWAP5,				///< swaps the highest and 6th highest value on the stack
	SWAP6,				///< swaps the highest and 7th highest value on the stack
	SWAP7,				///< swaps the highest and 8th highest value on the stack
	SWAP8,				///< swaps the highest and 9th highest value on the stack
	SWAP9,				///< swaps the highest and 10th highest value on the stack
	SWAP10,				///< swaps the highest and 11th highest value on the stack
	SWAP11,				///< swaps the highest and 12th highest value on the stack
	SWAP12,				///< swaps the highest and 13th highest value on the stack
	SWAP13,				///< swaps the highest and 14th highest value on the stack
	SWAP14,				///< swaps the highest and 15th highest value on the stack
	SWAP15,				///< swaps the highest and 16th highest value on the stack
	SWAP16,				///< swaps the highest and 17th highest value on the stack

	LOG0 = 0xa0,		///< Makes a log entry; no topics.
	LOG1,				///< Makes a log entry; 1 topic.
	LOG2,				///< Makes a log entry; 2 topics.
	LOG3,				///< Makes a log entry; 3 topics.
	LOG4,				///< Makes a log entry; 4 topics.

	CREATE = 0xf0,		///< create a new account with associated code
	CALL,				///< message-call into an account
	CALLCODE,			///< message-call with another account's code only
	RETURN,				///< halt execution returning output data
	DELEGATECALL,		///< like CALLCODE but keeps caller's value and sender
	CREATE2 = 0xf5,		///< create new account with associated code at address `sha3(0xff + sender + salt + init code) % 2**160`
	STATICCALL = 0xfa,	///< like CALL but disallow state modifications

	REVERT = 0xfd,		///< halt execution, revert state and return output data
	INVALID = 0xfe,		///< invalid instruction for expressing runtime errors (e.g., division-by-zero)
	SELFDESTRUCT = 0xff	///< halt execution and register account for later deletion
};

enum class Tier
{
	Zero = 0,	// 0, Zero
	Base,		// 2, Quick
	VeryLow,	// 3, Fastest
	Low,		// 5, Fast
	Mid,		// 8, Mid
	High,		// 10, Slow
	Ext,		// 20, Ext
	ExtCode,	// 700, Extcode
	Balance,	// 400, Balance
	Special,	// multiparam or otherwise special
	Invalid		// Invalid.
};

/// Information structure for a particular instruction.
struct InstructionInfo
{
	std::string name;	///< The name of the instruction.
	InstructionOpCode opCode; ///< InternalInstruction OpCode
	int additional;		///< Additional items required in memory for this instructions (only for PUSH).
	int args;			///< Number of items required on the stack for this instruction (and, for the purposes of ret, the number taken from the stack).
	int ret;			///< Number of items placed (back) on the stack by this instruction, assuming args items were removed.
	bool sideEffects;	///< false if the only effect on the execution environment (apart from gas usage) is a change to a topmost segment of the stack
	Tier gasPriceTier;	///< Tier for gas pricing.
};

/// Information on all the instructions.
InstructionInfo const& instructionInfo(InternalInstruction _inst);

/// Get the Opcode of an internal instruction.
InstructionOpCode instructionOpcode(InternalInstruction _inst);



/// Get internal instruction for a given opcode
InternalInstruction internalInstruction(InstructionOpCode _opcode, langutil::EVMVersion _evmVersion);

/// check whether instructions exists.
bool isValidInstruction(InternalInstruction _inst);

/// Convert from string mnemonic to InternalInstruction type.
extern const std::map<std::string, InternalInstruction> c_instructions;


/// @returns true if the instruction is a PUSH
inline bool isPushInstruction(InternalInstruction _inst)
{
	return InstructionOpCode::PUSH1 <= instructionOpcode(_inst) && instructionOpcode(_inst) <= InstructionOpCode::PUSH32;
}

/// @returns true if the instruction is a DUP
inline bool isDupInstruction(InternalInstruction _inst)
{
	return InstructionOpCode::DUP1 <= instructionOpcode(_inst) && instructionOpcode(_inst) <= InstructionOpCode::DUP16;
}

/// @returns true if the instruction is a SWAP
inline bool isSwapInstruction(InternalInstruction _inst)
{
	return InstructionOpCode::SWAP1 <= instructionOpcode(_inst) && instructionOpcode(_inst) <= InstructionOpCode::SWAP16;
}

/// @returns true if the instruction is a LOG
inline bool isLogInstruction(InternalInstruction _inst)
{
	return InstructionOpCode::LOG0 <= instructionOpcode(_inst) && instructionOpcode(_inst) <= InstructionOpCode::LOG4;
}

/// @returns the number of PUSH InternalInstruction _inst
inline unsigned getPushNumber(InternalInstruction _inst)
{
	return static_cast<uint8_t>(_inst) - unsigned(InstructionOpCode::PUSH1) + 1;
}

/// @returns the number of DUP InternalInstruction _inst
inline unsigned getDupNumber(InternalInstruction _inst)
{
	return static_cast<uint8_t>(_inst) - unsigned(InstructionOpCode::DUP1) + 1;
}

/// @returns the number of SWAP InternalInstruction _inst
inline unsigned getSwapNumber(InternalInstruction _inst)
{
	return static_cast<uint8_t>(_inst) - unsigned(InstructionOpCode::SWAP1) + 1;
}

/// @returns the number of LOG InternalInstruction _inst
inline unsigned getLogNumber(InternalInstruction _inst)
{
	return static_cast<uint8_t>(_inst) - unsigned(InstructionOpCode::LOG0);
}

/// @returns the PUSH<_number> instruction
inline InternalInstruction pushInstruction(unsigned _number)
{
	assertThrow(1 <= _number && _number <= 32, InvalidOpcode, std::string("Invalid PUSH instruction requested (") + std::to_string(_number) + ").");
	return InternalInstruction(unsigned(InternalInstruction::PUSH1) + _number - 1);
}

/// @returns the DUP<_number> instruction
inline InternalInstruction dupInstruction(unsigned _number)
{
	assertThrow(1 <= _number && _number <= 16, InvalidOpcode, std::string("Invalid DUP instruction requested (") + std::to_string(_number) + ").");
	return InternalInstruction(unsigned(InternalInstruction::DUP1) + _number - 1);
}

/// @returns the SWAP<_number> instruction
inline InternalInstruction swapInstruction(unsigned _number)
{
	assertThrow(1 <= _number && _number <= 16, InvalidOpcode, std::string("Invalid SWAP instruction requested (") + std::to_string(_number) + ").");
	return InternalInstruction(unsigned(InternalInstruction::SWAP1) + _number - 1);
}

/// @returns the LOG<_number> instruction
inline InternalInstruction logInstruction(unsigned _number)
{
	assertThrow(_number <= 4, InvalidOpcode, std::string("Invalid LOG instruction requested (") + std::to_string(_number) + ").");
	return InternalInstruction(unsigned(InternalInstruction::LOG0) + _number);
}


}
