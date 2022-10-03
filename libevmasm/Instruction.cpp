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
/** @file InternalInstruction.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <iterator>
#include <libevmasm/Instruction.h>
#include <liblangutil/EVMVersion.h>
#include <libsolutil/SetOnce.h>

#include <array>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;

/// Caches the opcode to internal mapping upon first usage.
std::multimap<InstructionOpCode, InternalInstruction> opcode2InternalCache;

std::map<std::string, InternalInstruction> const solidity::evmasm::c_instructions =
{
	{ "STOP", InternalInstruction::STOP },
	{ "ADD", InternalInstruction::ADD },
	{ "SUB", InternalInstruction::SUB },
	{ "MUL", InternalInstruction::MUL },
	{ "DIV", InternalInstruction::DIV },
	{ "SDIV", InternalInstruction::SDIV },
	{ "MOD", InternalInstruction::MOD },
	{ "SMOD", InternalInstruction::SMOD },
	{ "EXP", InternalInstruction::EXP },
	{ "NOT", InternalInstruction::NOT },
	{ "LT", InternalInstruction::LT },
	{ "GT", InternalInstruction::GT },
	{ "SLT", InternalInstruction::SLT },
	{ "SGT", InternalInstruction::SGT },
	{ "EQ", InternalInstruction::EQ },
	{ "ISZERO", InternalInstruction::ISZERO },
	{ "AND", InternalInstruction::AND },
	{ "OR", InternalInstruction::OR },
	{ "XOR", InternalInstruction::XOR },
	{ "BYTE", InternalInstruction::BYTE },
	{ "SHL", InternalInstruction::SHL },
	{ "SHR", InternalInstruction::SHR },
	{ "SAR", InternalInstruction::SAR },
	{ "ADDMOD", InternalInstruction::ADDMOD },
	{ "MULMOD", InternalInstruction::MULMOD },
	{ "SIGNEXTEND", InternalInstruction::SIGNEXTEND },
	{ "KECCAK256", InternalInstruction::KECCAK256 },
	{ "ADDRESS", InternalInstruction::ADDRESS },
	{ "BALANCE", InternalInstruction::BALANCE },
	{ "ORIGIN", InternalInstruction::ORIGIN },
	{ "CALLER", InternalInstruction::CALLER },
	{ "CALLVALUE", InternalInstruction::CALLVALUE },
	{ "CALLDATALOAD", InternalInstruction::CALLDATALOAD },
	{ "CALLDATASIZE", InternalInstruction::CALLDATASIZE },
	{ "CALLDATACOPY", InternalInstruction::CALLDATACOPY },
	{ "CODESIZE", InternalInstruction::CODESIZE },
	{ "CODECOPY", InternalInstruction::CODECOPY },
	{ "GASPRICE", InternalInstruction::GASPRICE },
	{ "EXTCODESIZE", InternalInstruction::EXTCODESIZE },
	{ "EXTCODECOPY", InternalInstruction::EXTCODECOPY },
	{ "RETURNDATASIZE", InternalInstruction::RETURNDATASIZE },
	{ "RETURNDATACOPY", InternalInstruction::RETURNDATACOPY },
	{ "EXTCODEHASH", InternalInstruction::EXTCODEHASH },
	{ "BLOCKHASH", InternalInstruction::BLOCKHASH },
	{ "COINBASE", InternalInstruction::COINBASE },
	{ "TIMESTAMP", InternalInstruction::TIMESTAMP },
	{ "NUMBER", InternalInstruction::NUMBER },
	{ "DIFFICULTY", InternalInstruction::DIFFICULTY },
	{ "PREVRANDAO", InternalInstruction::PREVRANDAO },
	{ "GASLIMIT", InternalInstruction::GASLIMIT },
	{ "CHAINID", InternalInstruction::CHAINID },
	{ "SELFBALANCE", InternalInstruction::SELFBALANCE },
	{ "BASEFEE", InternalInstruction::BASEFEE },
	{ "POP", InternalInstruction::POP },
	{ "MLOAD", InternalInstruction::MLOAD },
	{ "MSTORE", InternalInstruction::MSTORE },
	{ "MSTORE8", InternalInstruction::MSTORE8 },
	{ "SLOAD", InternalInstruction::SLOAD },
	{ "SSTORE", InternalInstruction::SSTORE },
	{ "JUMP", InternalInstruction::JUMP },
	{ "JUMPI", InternalInstruction::JUMPI },
	{ "PC", InternalInstruction::PC },
	{ "MSIZE", InternalInstruction::MSIZE },
	{ "GAS", InternalInstruction::GAS },
	{ "JUMPDEST", InternalInstruction::JUMPDEST },
	{ "PUSH1", InternalInstruction::PUSH1 },
	{ "PUSH2", InternalInstruction::PUSH2 },
	{ "PUSH3", InternalInstruction::PUSH3 },
	{ "PUSH4", InternalInstruction::PUSH4 },
	{ "PUSH5", InternalInstruction::PUSH5 },
	{ "PUSH6", InternalInstruction::PUSH6 },
	{ "PUSH7", InternalInstruction::PUSH7 },
	{ "PUSH8", InternalInstruction::PUSH8 },
	{ "PUSH9", InternalInstruction::PUSH9 },
	{ "PUSH10", InternalInstruction::PUSH10 },
	{ "PUSH11", InternalInstruction::PUSH11 },
	{ "PUSH12", InternalInstruction::PUSH12 },
	{ "PUSH13", InternalInstruction::PUSH13 },
	{ "PUSH14", InternalInstruction::PUSH14 },
	{ "PUSH15", InternalInstruction::PUSH15 },
	{ "PUSH16", InternalInstruction::PUSH16 },
	{ "PUSH17", InternalInstruction::PUSH17 },
	{ "PUSH18", InternalInstruction::PUSH18 },
	{ "PUSH19", InternalInstruction::PUSH19 },
	{ "PUSH20", InternalInstruction::PUSH20 },
	{ "PUSH21", InternalInstruction::PUSH21 },
	{ "PUSH22", InternalInstruction::PUSH22 },
	{ "PUSH23", InternalInstruction::PUSH23 },
	{ "PUSH24", InternalInstruction::PUSH24 },
	{ "PUSH25", InternalInstruction::PUSH25 },
	{ "PUSH26", InternalInstruction::PUSH26 },
	{ "PUSH27", InternalInstruction::PUSH27 },
	{ "PUSH28", InternalInstruction::PUSH28 },
	{ "PUSH29", InternalInstruction::PUSH29 },
	{ "PUSH30", InternalInstruction::PUSH30 },
	{ "PUSH31", InternalInstruction::PUSH31 },
	{ "PUSH32", InternalInstruction::PUSH32 },
	{ "DUP1", InternalInstruction::DUP1 },
	{ "DUP2", InternalInstruction::DUP2 },
	{ "DUP3", InternalInstruction::DUP3 },
	{ "DUP4", InternalInstruction::DUP4 },
	{ "DUP5", InternalInstruction::DUP5 },
	{ "DUP6", InternalInstruction::DUP6 },
	{ "DUP7", InternalInstruction::DUP7 },
	{ "DUP8", InternalInstruction::DUP8 },
	{ "DUP9", InternalInstruction::DUP9 },
	{ "DUP10", InternalInstruction::DUP10 },
	{ "DUP11", InternalInstruction::DUP11 },
	{ "DUP12", InternalInstruction::DUP12 },
	{ "DUP13", InternalInstruction::DUP13 },
	{ "DUP14", InternalInstruction::DUP14 },
	{ "DUP15", InternalInstruction::DUP15 },
	{ "DUP16", InternalInstruction::DUP16 },
	{ "SWAP1", InternalInstruction::SWAP1 },
	{ "SWAP2", InternalInstruction::SWAP2 },
	{ "SWAP3", InternalInstruction::SWAP3 },
	{ "SWAP4", InternalInstruction::SWAP4 },
	{ "SWAP5", InternalInstruction::SWAP5 },
	{ "SWAP6", InternalInstruction::SWAP6 },
	{ "SWAP7", InternalInstruction::SWAP7 },
	{ "SWAP8", InternalInstruction::SWAP8 },
	{ "SWAP9", InternalInstruction::SWAP9 },
	{ "SWAP10", InternalInstruction::SWAP10 },
	{ "SWAP11", InternalInstruction::SWAP11 },
	{ "SWAP12", InternalInstruction::SWAP12 },
	{ "SWAP13", InternalInstruction::SWAP13 },
	{ "SWAP14", InternalInstruction::SWAP14 },
	{ "SWAP15", InternalInstruction::SWAP15 },
	{ "SWAP16", InternalInstruction::SWAP16 },
	{ "LOG0", InternalInstruction::LOG0 },
	{ "LOG1", InternalInstruction::LOG1 },
	{ "LOG2", InternalInstruction::LOG2 },
	{ "LOG3", InternalInstruction::LOG3 },
	{ "LOG4", InternalInstruction::LOG4 },
	{ "CREATE", InternalInstruction::CREATE },
	{ "CALL", InternalInstruction::CALL },
	{ "CALLCODE", InternalInstruction::CALLCODE },
	{ "STATICCALL", InternalInstruction::STATICCALL },
	{ "RETURN", InternalInstruction::RETURN },
	{ "DELEGATECALL", InternalInstruction::DELEGATECALL },
	{ "CREATE2", InternalInstruction::CREATE2 },
	{ "REVERT", InternalInstruction::REVERT },
	{ "INVALID", InternalInstruction::INVALID },
	{ "SELFDESTRUCT", InternalInstruction::SELFDESTRUCT }
};

static std::array<InstructionInfo, static_cast<size_t>(InternalInstruction::MAX_INTERNAL_INSTRUCTION)> const c_instructionInfo =
{{ //												Add, Args, Ret, SideEffects, GasPriceTier
	{ "STOP", InstructionOpCode::STOP,					0, 0, 0, true,  Tier::Zero },
	{ "ADD", InstructionOpCode::ADD,						0, 2, 1, false, Tier::VeryLow },
	{ "SUB", InstructionOpCode::SUB,						0, 2, 1, false, Tier::VeryLow },
	{ "MUL", InstructionOpCode::MUL,						0, 2, 1, false, Tier::Low },
	{ "DIV", InstructionOpCode::DIV,						0, 2, 1, false, Tier::Low },
	{ "SDIV", InstructionOpCode::SDIV,					0, 2, 1, false, Tier::Low },
	{ "MOD", InstructionOpCode::MOD,						0, 2, 1, false, Tier::Low },
	{ "SMOD", InstructionOpCode::SMOD,					0, 2, 1, false, Tier::Low },
	{ "ADDMOD", InstructionOpCode::ADDMOD,					0, 3, 1, false, Tier::Mid },
	{ "MULMOD", InstructionOpCode::MULMOD,					0, 3, 1, false, Tier::Mid },
	{ "EXP", InstructionOpCode::EXP,						0, 2, 1, false, Tier::Special },
	{ "SIGNEXTEND", InstructionOpCode::SIGNEXTEND,			0, 2, 1, false, Tier::Low },

	{ "LT", InstructionOpCode::LT,							0, 2, 1, false, Tier::VeryLow },
	{ "GT", InstructionOpCode::GT,							0, 2, 1, false, Tier::VeryLow },
	{ "SLT", InstructionOpCode::SLT,						0, 2, 1, false, Tier::VeryLow },
	{ "SGT", InstructionOpCode::SGT,						0, 2, 1, false, Tier::VeryLow },
	{ "EQ", InstructionOpCode::EQ,							0, 2, 1, false, Tier::VeryLow },
	{ "ISZERO", InstructionOpCode::ISZERO,					0, 1, 1, false, Tier::VeryLow },
	{ "AND", InstructionOpCode::AND,						0, 2, 1, false, Tier::VeryLow },
	{ "OR", InstructionOpCode::OR,							0, 2, 1, false, Tier::VeryLow },
	{ "XOR", InstructionOpCode::XOR,						0, 2, 1, false, Tier::VeryLow },
	{ "NOT", InstructionOpCode::NOT,						0, 1, 1, false, Tier::VeryLow },
	{ "BYTE", InstructionOpCode::BYTE,					0, 2, 1, false, Tier::VeryLow },
	{ "SHL", InstructionOpCode::SHL,					0, 2, 1, false, Tier::VeryLow },
	{ "SHR", InstructionOpCode::SHR,					0, 2, 1, false, Tier::VeryLow },
	{ "SAR", InstructionOpCode::SAR,					0, 2, 1, false, Tier::VeryLow },

	{ "KECCAK256", InstructionOpCode::KECCAK256,				0, 2, 1, true, Tier::Special },

	{ "ADDRESS", InstructionOpCode::ADDRESS,				0, 0, 1, false, Tier::Base },
	{ "BALANCE", InstructionOpCode::BALANCE,				0, 1, 1, false, Tier::Balance },
	{ "ORIGIN", InstructionOpCode::ORIGIN,					0, 0, 1, false, Tier::Base },
	{ "CALLER", InstructionOpCode::CALLER,					0, 0, 1, false, Tier::Base },
	{ "CALLVALUE", InstructionOpCode::CALLVALUE,			0, 0, 1, false, Tier::Base },
	{ "CALLDATALOAD", InstructionOpCode::CALLDATALOAD,	0, 1, 1, false, Tier::VeryLow },
	{ "CALLDATASIZE", InstructionOpCode::CALLDATASIZE,	0, 0, 1, false, Tier::Base },
	{ "CALLDATACOPY", InstructionOpCode::CALLDATACOPY,	0, 3, 0, true, Tier::VeryLow },
	{ "CODESIZE", InstructionOpCode::CODESIZE,			0, 0, 1, false, Tier::Base },
	{ "CODECOPY", InstructionOpCode::CODECOPY,			0, 3, 0, true, Tier::VeryLow },
	{ "GASPRICE", InstructionOpCode::GASPRICE,			0, 0, 1, false, Tier::Base },
	{ "EXTCODESIZE", InstructionOpCode::EXTCODESIZE,		0, 1, 1, false, Tier::ExtCode },
	{ "EXTCODECOPY", InstructionOpCode::EXTCODECOPY,		0, 4, 0, true, Tier::ExtCode },
	{ "RETURNDATASIZE", InstructionOpCode::RETURNDATASIZE,		0, 0, 1, false, Tier::Base },
	{ "RETURNDATACOPY", InstructionOpCode::RETURNDATACOPY,		0, 3, 0, true, Tier::VeryLow },
	{ "EXTCODEHASH", InstructionOpCode::EXTCODEHASH,		0, 1, 1, false, Tier::Balance },

	{ "BLOCKHASH", InstructionOpCode::BLOCKHASH,			0, 1, 1, false, Tier::Ext },
	{ "COINBASE", InstructionOpCode::COINBASE,			0, 0, 1, false, Tier::Base },
	{ "TIMESTAMP", InstructionOpCode::TIMESTAMP,			0, 0, 1, false, Tier::Base },
	{ "NUMBER", InstructionOpCode::NUMBER,					0, 0, 1, false, Tier::Base },
	{ "DIFFICULTY", InstructionOpCode::DIFFICULTY,			0, 0, 1, false, Tier::Base },
	{ "PREVRANDAO", InstructionOpCode::PREVRANDAO,			0, 0, 1, false, Tier::Base },
	{ "GASLIMIT", InstructionOpCode::GASLIMIT,			0, 0, 1, false, Tier::Base },
	{ "CHAINID", InstructionOpCode::CHAINID,				0, 0, 1, false, Tier::Base },
	{ "SELFBALANCE", InstructionOpCode::SELFBALANCE,		0, 0, 1, false, Tier::Low },
	{ "BASEFEE", InstructionOpCode::BASEFEE,             0, 0, 1, false, Tier::Base },

	{ "POP", InstructionOpCode::POP,						0, 1, 0, false, Tier::Base },
	{ "MLOAD", InstructionOpCode::MLOAD,					0, 1, 1, true, Tier::VeryLow },
	{ "MSTORE", InstructionOpCode::MSTORE,					0, 2, 0, true, Tier::VeryLow },
	{ "MSTORE8", InstructionOpCode::MSTORE8,				0, 2, 0, true, Tier::VeryLow },
	{ "SLOAD", InstructionOpCode::SLOAD,					0, 1, 1, false, Tier::Special },
	{ "SSTORE", InstructionOpCode::SSTORE,					0, 2, 0, true, Tier::Special },
	{ "JUMP", InstructionOpCode::JUMP,					0, 1, 0, true, Tier::Mid },
	{ "JUMPI", InstructionOpCode::JUMPI,					0, 2, 0, true, Tier::High },
	{ "PC", InstructionOpCode::PC,							0, 0, 1, false, Tier::Base },
	{ "MSIZE", InstructionOpCode::MSIZE,					0, 0, 1, false, Tier::Base },
	{ "GAS", InstructionOpCode::GAS,						0, 0, 1, false, Tier::Base },
	{ "JUMPDEST", InstructionOpCode::JUMPDEST,			0, 0, 0, true, Tier::Special },

	{ "PUSH1", InstructionOpCode::PUSH1,					1, 0, 1, false, Tier::VeryLow },
	{ "PUSH2", InstructionOpCode::PUSH2,					2, 0, 1, false, Tier::VeryLow },
	{ "PUSH3", InstructionOpCode::PUSH3,					3, 0, 1, false, Tier::VeryLow },
	{ "PUSH4", InstructionOpCode::PUSH4,					4, 0, 1, false, Tier::VeryLow },
	{ "PUSH5", InstructionOpCode::PUSH5,					5, 0, 1, false, Tier::VeryLow },
	{ "PUSH6", InstructionOpCode::PUSH6,					6, 0, 1, false, Tier::VeryLow },
	{ "PUSH7", InstructionOpCode::PUSH7,					7, 0, 1, false, Tier::VeryLow },
	{ "PUSH8", InstructionOpCode::PUSH8,					8, 0, 1, false, Tier::VeryLow },
	{ "PUSH9", InstructionOpCode::PUSH9,					9, 0, 1, false, Tier::VeryLow },
	{ "PUSH10", InstructionOpCode::PUSH10,					10, 0, 1, false, Tier::VeryLow },
	{ "PUSH11", InstructionOpCode::PUSH11,					11, 0, 1, false, Tier::VeryLow },
	{ "PUSH12", InstructionOpCode::PUSH12,					12, 0, 1, false, Tier::VeryLow },
	{ "PUSH13", InstructionOpCode::PUSH13,					13, 0, 1, false, Tier::VeryLow },
	{ "PUSH14", InstructionOpCode::PUSH14,					14, 0, 1, false, Tier::VeryLow },
	{ "PUSH15", InstructionOpCode::PUSH15,					15, 0, 1, false, Tier::VeryLow },
	{ "PUSH16", InstructionOpCode::PUSH16,					16, 0, 1, false, Tier::VeryLow },
	{ "PUSH17", InstructionOpCode::PUSH17,					17, 0, 1, false, Tier::VeryLow },
	{ "PUSH18", InstructionOpCode::PUSH18,					18, 0, 1, false, Tier::VeryLow },
	{ "PUSH19", InstructionOpCode::PUSH19,					19, 0, 1, false, Tier::VeryLow },
	{ "PUSH20", InstructionOpCode::PUSH20,					20, 0, 1, false, Tier::VeryLow },
	{ "PUSH21", InstructionOpCode::PUSH21,					21, 0, 1, false, Tier::VeryLow },
	{ "PUSH22", InstructionOpCode::PUSH22,					22, 0, 1, false, Tier::VeryLow },
	{ "PUSH23", InstructionOpCode::PUSH23,					23, 0, 1, false, Tier::VeryLow },
	{ "PUSH24", InstructionOpCode::PUSH24,					24, 0, 1, false, Tier::VeryLow },
	{ "PUSH25", InstructionOpCode::PUSH25,					25, 0, 1, false, Tier::VeryLow },
	{ "PUSH26", InstructionOpCode::PUSH26,					26, 0, 1, false, Tier::VeryLow },
	{ "PUSH27", InstructionOpCode::PUSH27,					27, 0, 1, false, Tier::VeryLow },
	{ "PUSH28", InstructionOpCode::PUSH28,					28, 0, 1, false, Tier::VeryLow },
	{ "PUSH29", InstructionOpCode::PUSH29,					29, 0, 1, false, Tier::VeryLow },
	{ "PUSH30", InstructionOpCode::PUSH30,					30, 0, 1, false, Tier::VeryLow },
	{ "PUSH31", InstructionOpCode::PUSH31,					31, 0, 1, false, Tier::VeryLow },
	{ "PUSH32", InstructionOpCode::PUSH32,					32, 0, 1, false, Tier::VeryLow },

	{ "DUP1", InstructionOpCode::DUP1,					0, 1, 2, false, Tier::VeryLow },
	{ "DUP2", InstructionOpCode::DUP2,					0, 2, 3, false, Tier::VeryLow },
	{ "DUP3", InstructionOpCode::DUP3,					0, 3, 4, false, Tier::VeryLow },
	{ "DUP4", InstructionOpCode::DUP4,					0, 4, 5, false, Tier::VeryLow },
	{ "DUP5", InstructionOpCode::DUP5,					0, 5, 6, false, Tier::VeryLow },
	{ "DUP6", InstructionOpCode::DUP6,					0, 6, 7, false, Tier::VeryLow },
	{ "DUP7", InstructionOpCode::DUP7,					0, 7, 8, false, Tier::VeryLow },
	{ "DUP8", InstructionOpCode::DUP8,					0, 8, 9, false, Tier::VeryLow },
	{ "DUP9", InstructionOpCode::DUP9,					0, 9, 10, false, Tier::VeryLow },
	{ "DUP10", InstructionOpCode::DUP10,					0, 10, 11, false, Tier::VeryLow },
	{ "DUP11", InstructionOpCode::DUP11,					0, 11, 12, false, Tier::VeryLow },
	{ "DUP12", InstructionOpCode::DUP12,					0, 12, 13, false, Tier::VeryLow },
	{ "DUP13", InstructionOpCode::DUP13,					0, 13, 14, false, Tier::VeryLow },
	{ "DUP14", InstructionOpCode::DUP14,					0, 14, 15, false, Tier::VeryLow },
	{ "DUP15", InstructionOpCode::DUP15,					0, 15, 16, false, Tier::VeryLow },
	{ "DUP16", InstructionOpCode::DUP16,					0, 16, 17, false, Tier::VeryLow },

	{ "SWAP1", InstructionOpCode::SWAP1,					0, 2, 2, false, Tier::VeryLow },
	{ "SWAP2", InstructionOpCode::SWAP2,					0, 3, 3, false, Tier::VeryLow },
	{ "SWAP3", InstructionOpCode::SWAP3,					0, 4, 4, false, Tier::VeryLow },
	{ "SWAP4", InstructionOpCode::SWAP4,					0, 5, 5, false, Tier::VeryLow },
	{ "SWAP5", InstructionOpCode::SWAP5,					0, 6, 6, false, Tier::VeryLow },
	{ "SWAP6", InstructionOpCode::SWAP6,					0, 7, 7, false, Tier::VeryLow },
	{ "SWAP7", InstructionOpCode::SWAP7,					0, 8, 8, false, Tier::VeryLow },
	{ "SWAP8", InstructionOpCode::SWAP8,					0, 9, 9, false, Tier::VeryLow },
	{ "SWAP9", InstructionOpCode::SWAP9,					0, 10, 10, false, Tier::VeryLow },
	{ "SWAP10", InstructionOpCode::SWAP10,					0, 11, 11, false, Tier::VeryLow },
	{ "SWAP11", InstructionOpCode::SWAP11,					0, 12, 12, false, Tier::VeryLow },
	{ "SWAP12", InstructionOpCode::SWAP12,					0, 13, 13, false, Tier::VeryLow },
	{ "SWAP13", InstructionOpCode::SWAP13,					0, 14, 14, false, Tier::VeryLow },
	{ "SWAP14", InstructionOpCode::SWAP14,					0, 15, 15, false, Tier::VeryLow },
	{ "SWAP15", InstructionOpCode::SWAP15,					0, 16, 16, false, Tier::VeryLow },
	{ "SWAP16", InstructionOpCode::SWAP16,					0, 17, 17, false, Tier::VeryLow },

	{ "LOG0", InstructionOpCode::LOG0,					0, 2, 0, true, Tier::Special },
	{ "LOG1", InstructionOpCode::LOG1,					0, 3, 0, true, Tier::Special },
	{ "LOG2", InstructionOpCode::LOG2,					0, 4, 0, true, Tier::Special },
	{ "LOG3", InstructionOpCode::LOG3,					0, 5, 0, true, Tier::Special },
	{ "LOG4", InstructionOpCode::LOG4,					0, 6, 0, true, Tier::Special },

	{ "CREATE", InstructionOpCode::CREATE,					0, 3, 1, true, Tier::Special },
	{ "CALL", InstructionOpCode::CALL,					0, 7, 1, true, Tier::Special },
	{ "CALLCODE", InstructionOpCode::CALLCODE,			0, 7, 1, true, Tier::Special },
	{ "RETURN", InstructionOpCode::RETURN,					0, 2, 0, true, Tier::Zero },
	{ "DELEGATECALL", InstructionOpCode::DELEGATECALL,		0, 6, 1, true, Tier::Special },
	{ "CREATE2", InstructionOpCode::CREATE2,				0, 4, 1, true, Tier::Special },
	{ "STATICCALL", InstructionOpCode::STATICCALL,			0, 6, 1, true, Tier::Special },
	{ "REVERT", InstructionOpCode::REVERT,				0, 2, 0, true, Tier::Zero },
	{ "INVALID", InstructionOpCode::INVALID,				0, 0, 0, true, Tier::Zero },
	{ "SELFDESTRUCT", InstructionOpCode::SELFDESTRUCT,			0, 1, 0, true, Tier::Special }
}};

InstructionInfo const& solidity::evmasm::instructionInfo(InternalInstruction _inst)
{
	try
	{
		return c_instructionInfo.at(static_cast<size_t>(_inst));
	}
	catch (...)
	{
		static InstructionInfo invalidInstruction = InstructionInfo({
			"<INVALID_INTERNAL_INSTRUCTION: " + to_string(static_cast<unsigned>(_inst)) + ">",
			InstructionOpCode::INVALID,
			0,
			0,
			0,
			false,
			Tier::Invalid
		});
		return invalidInstruction;
	}
}

/// Get the Opcode of an instruction.
InstructionOpCode solidity::evmasm::instructionOpcode(InternalInstruction _inst)
{
	return instructionInfo(_inst).opCode;
}

InternalInstruction solidity::evmasm::internalInstruction(InstructionOpCode _opcode, langutil::EVMVersion _evmVersion)
{
	// Create cached table if not existing
	if (opcode2InternalCache.empty())
		for (size_t i = 0; i < c_instructionInfo.size(); i++)
			if (c_instructionInfo[i].opCode == _opcode)
				opcode2InternalCache.insert({_opcode, static_cast<InternalInstruction>(i)});

	SetOnce<InternalInstruction> validInstruction{};
	auto [start, end] = opcode2InternalCache.equal_range(_opcode);

	// Check which instructions work for the given evm version
	for (;start != end; start++)
		if (_evmVersion.hasInstruction(start->second))
			validInstruction = start->second;

	return *validInstruction;
}

bool solidity::evmasm::isValidInstruction(InternalInstruction _inst)
{
	return _inst < InternalInstruction::MAX_INTERNAL_INSTRUCTION;
}
