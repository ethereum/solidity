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
/** @file Instruction.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Instruction.h"

#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Log.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

const std::map<std::string, Instruction> dev::eth::c_instructions =
{
	{ "STOP", Instruction::STOP },
	{ "ADD", Instruction::ADD },
	{ "SUB", Instruction::SUB },
	{ "MUL", Instruction::MUL },
	{ "DIV", Instruction::DIV },
	{ "SDIV", Instruction::SDIV },
	{ "MOD", Instruction::MOD },
	{ "SMOD", Instruction::SMOD },
	{ "EXP", Instruction::EXP },
	{ "BNOT", Instruction::NOT },
	{ "LT", Instruction::LT },
	{ "GT", Instruction::GT },
	{ "SLT", Instruction::SLT },
	{ "SGT", Instruction::SGT },
	{ "EQ", Instruction::EQ },
	{ "NOT", Instruction::ISZERO },
	{ "AND", Instruction::AND },
	{ "OR", Instruction::OR },
	{ "XOR", Instruction::XOR },
	{ "BYTE", Instruction::BYTE },
	{ "ADDMOD", Instruction::ADDMOD },
	{ "MULMOD", Instruction::MULMOD },
	{ "SIGNEXTEND", Instruction::SIGNEXTEND },
	{ "SHA3", Instruction::SHA3 },
	{ "ADDRESS", Instruction::ADDRESS },
	{ "BALANCE", Instruction::BALANCE },
	{ "ORIGIN", Instruction::ORIGIN },
	{ "CALLER", Instruction::CALLER },
	{ "CALLVALUE", Instruction::CALLVALUE },
	{ "CALLDATALOAD", Instruction::CALLDATALOAD },
	{ "CALLDATASIZE", Instruction::CALLDATASIZE },
	{ "CALLDATACOPY", Instruction::CALLDATACOPY },
	{ "CODESIZE", Instruction::CODESIZE },
	{ "CODECOPY", Instruction::CODECOPY },
	{ "GASPRICE", Instruction::GASPRICE },
	{ "EXTCODESIZE", Instruction::EXTCODESIZE },
	{ "EXTCODECOPY", Instruction::EXTCODECOPY },
	{ "BLOCKHASH", Instruction::BLOCKHASH },
	{ "COINBASE", Instruction::COINBASE },
	{ "TIMESTAMP", Instruction::TIMESTAMP },
	{ "NUMBER", Instruction::NUMBER },
	{ "DIFFICULTY", Instruction::DIFFICULTY },
	{ "GASLIMIT", Instruction::GASLIMIT },
	{ "POP", Instruction::POP },
	{ "MLOAD", Instruction::MLOAD },
	{ "MSTORE", Instruction::MSTORE },
	{ "MSTORE8", Instruction::MSTORE8 },
	{ "SLOAD", Instruction::SLOAD },
	{ "SSTORE", Instruction::SSTORE },
	{ "JUMP", Instruction::JUMP },
	{ "JUMPI", Instruction::JUMPI },
	{ "PC", Instruction::PC },
	{ "MSIZE", Instruction::MSIZE },
	{ "GAS", Instruction::GAS },
	{ "JUMPDEST", Instruction::JUMPDEST },
	{ "PUSH1", Instruction::PUSH1 },
	{ "PUSH2", Instruction::PUSH2 },
	{ "PUSH3", Instruction::PUSH3 },
	{ "PUSH4", Instruction::PUSH4 },
	{ "PUSH5", Instruction::PUSH5 },
	{ "PUSH6", Instruction::PUSH6 },
	{ "PUSH7", Instruction::PUSH7 },
	{ "PUSH8", Instruction::PUSH8 },
	{ "PUSH9", Instruction::PUSH9 },
	{ "PUSH10", Instruction::PUSH10 },
	{ "PUSH11", Instruction::PUSH11 },
	{ "PUSH12", Instruction::PUSH12 },
	{ "PUSH13", Instruction::PUSH13 },
	{ "PUSH14", Instruction::PUSH14 },
	{ "PUSH15", Instruction::PUSH15 },
	{ "PUSH16", Instruction::PUSH16 },
	{ "PUSH17", Instruction::PUSH17 },
	{ "PUSH18", Instruction::PUSH18 },
	{ "PUSH19", Instruction::PUSH19 },
	{ "PUSH20", Instruction::PUSH20 },
	{ "PUSH21", Instruction::PUSH21 },
	{ "PUSH22", Instruction::PUSH22 },
	{ "PUSH23", Instruction::PUSH23 },
	{ "PUSH24", Instruction::PUSH24 },
	{ "PUSH25", Instruction::PUSH25 },
	{ "PUSH26", Instruction::PUSH26 },
	{ "PUSH27", Instruction::PUSH27 },
	{ "PUSH28", Instruction::PUSH28 },
	{ "PUSH29", Instruction::PUSH29 },
	{ "PUSH30", Instruction::PUSH30 },
	{ "PUSH31", Instruction::PUSH31 },
	{ "PUSH32", Instruction::PUSH32 },
	{ "DUP1", Instruction::DUP1 },
	{ "DUP2", Instruction::DUP2 },
	{ "DUP3", Instruction::DUP3 },
	{ "DUP4", Instruction::DUP4 },
	{ "DUP5", Instruction::DUP5 },
	{ "DUP6", Instruction::DUP6 },
	{ "DUP7", Instruction::DUP7 },
	{ "DUP8", Instruction::DUP8 },
	{ "DUP9", Instruction::DUP9 },
	{ "DUP10", Instruction::DUP10 },
	{ "DUP11", Instruction::DUP11 },
	{ "DUP12", Instruction::DUP12 },
	{ "DUP13", Instruction::DUP13 },
	{ "DUP14", Instruction::DUP14 },
	{ "DUP15", Instruction::DUP15 },
	{ "DUP16", Instruction::DUP16 },
	{ "SWAP1", Instruction::SWAP1 },
	{ "SWAP2", Instruction::SWAP2 },
	{ "SWAP3", Instruction::SWAP3 },
	{ "SWAP4", Instruction::SWAP4 },
	{ "SWAP5", Instruction::SWAP5 },
	{ "SWAP6", Instruction::SWAP6 },
	{ "SWAP7", Instruction::SWAP7 },
	{ "SWAP8", Instruction::SWAP8 },
	{ "SWAP9", Instruction::SWAP9 },
	{ "SWAP10", Instruction::SWAP10 },
	{ "SWAP11", Instruction::SWAP11 },
	{ "SWAP12", Instruction::SWAP12 },
	{ "SWAP13", Instruction::SWAP13 },
	{ "SWAP14", Instruction::SWAP14 },
	{ "SWAP15", Instruction::SWAP15 },
	{ "SWAP16", Instruction::SWAP16 },
	{ "LOG0", Instruction::LOG0 },
	{ "LOG1", Instruction::LOG1 },
	{ "LOG2", Instruction::LOG2 },
	{ "LOG3", Instruction::LOG3 },
	{ "LOG4", Instruction::LOG4 },
	{ "CREATE", Instruction::CREATE },
	{ "CALL", Instruction::CALL },
	{ "CALLCODE", Instruction::CALLCODE },
	{ "RETURN", Instruction::RETURN },
	{ "SUICIDE", Instruction::SUICIDE }
};

static const std::map<Instruction, InstructionInfo> c_instructionInfo =
{ //												Add, Args, Ret, SideEffects
	{ Instruction::STOP,		{ "STOP",			0, 0, 0, true } },
	{ Instruction::ADD,			{ "ADD",			0, 2, 1, false } },
	{ Instruction::SUB,			{ "SUB",			0, 2, 1, false } },
	{ Instruction::MUL,			{ "MUL",			0, 2, 1, false } },
	{ Instruction::DIV,			{ "DIV",			0, 2, 1, false } },
	{ Instruction::SDIV,		{ "SDIV",			0, 2, 1, false } },
	{ Instruction::MOD,			{ "MOD",			0, 2, 1, false } },
	{ Instruction::SMOD,		{ "SMOD",			0, 2, 1, false } },
	{ Instruction::EXP,			{ "EXP",			0, 2, 1, false } },
	{ Instruction::NOT,			{ "NOT",			0, 1, 1, false } },
	{ Instruction::LT,			{ "LT",				0, 2, 1, false } },
	{ Instruction::GT,			{ "GT",				0, 2, 1, false } },
	{ Instruction::SLT,			{ "SLT",			0, 2, 1, false } },
	{ Instruction::SGT,			{ "SGT",			0, 2, 1, false } },
	{ Instruction::EQ,			{ "EQ",				0, 2, 1, false } },
	{ Instruction::ISZERO,		{ "ISZERO",			0, 1, 1, false } },
	{ Instruction::AND,			{ "AND",			0, 2, 1, false } },
	{ Instruction::OR,			{ "OR",				0, 2, 1, false } },
	{ Instruction::XOR,			{ "XOR",			0, 2, 1, false } },
	{ Instruction::BYTE,		{ "BYTE",			0, 2, 1, false } },
	{ Instruction::ADDMOD,		{ "ADDMOD",			0, 3, 1, false } },
	{ Instruction::MULMOD,		{ "MULMOD",			0, 3, 1, false } },
	{ Instruction::SIGNEXTEND,	{ "SIGNEXTEND",		0, 2, 1, false } },
	{ Instruction::SHA3,		{ "SHA3",			0, 2, 1, false } },
	{ Instruction::ADDRESS,		{ "ADDRESS",		0, 0, 1, false } },
	{ Instruction::BALANCE,		{ "BALANCE",		0, 1, 1, false } },
	{ Instruction::ORIGIN,		{ "ORIGIN",			0, 0, 1, false } },
	{ Instruction::CALLER,		{ "CALLER",			0, 0, 1, false } },
	{ Instruction::CALLVALUE,	{ "CALLVALUE",		0, 0, 1, false } },
	{ Instruction::CALLDATALOAD,{ "CALLDATALOAD",	0, 1, 1, false } },
	{ Instruction::CALLDATASIZE,{ "CALLDATASIZE",	0, 0, 1, false } },
	{ Instruction::CALLDATACOPY,{ "CALLDATACOPY",	0, 3, 0, true } },
	{ Instruction::CODESIZE,	{ "CODESIZE",		0, 0, 1, false } },
	{ Instruction::CODECOPY,	{ "CODECOPY",		0, 3, 0, true } },
	{ Instruction::GASPRICE,	{ "GASPRICE",		0, 0, 1, false } },
	{ Instruction::EXTCODESIZE,	{ "EXTCODESIZE",	0, 1, 1, false } },
	{ Instruction::EXTCODECOPY,	{ "EXTCODECOPY",	0, 4, 0, true } },
	{ Instruction::BLOCKHASH,	{ "BLOCKHASH",		0, 1, 1, false } },
	{ Instruction::COINBASE,	{ "COINBASE",		0, 0, 1, false } },
	{ Instruction::TIMESTAMP,	{ "TIMESTAMP",		0, 0, 1, false } },
	{ Instruction::NUMBER,		{ "NUMBER",			0, 0, 1, false } },
	{ Instruction::DIFFICULTY,	{ "DIFFICULTY",		0, 0, 1, false } },
	{ Instruction::GASLIMIT,	{ "GASLIMIT",		0, 0, 1, false } },
	{ Instruction::POP,			{ "POP",			0, 1, 0, false } },
	{ Instruction::MLOAD,		{ "MLOAD",			0, 1, 1, false } },
	{ Instruction::MSTORE,		{ "MSTORE",			0, 2, 0, true } },
	{ Instruction::MSTORE8,		{ "MSTORE8",		0, 2, 0, true } },
	{ Instruction::SLOAD,		{ "SLOAD",			0, 1, 1, false } },
	{ Instruction::SSTORE,		{ "SSTORE",			0, 2, 0, true } },
	{ Instruction::JUMP,		{ "JUMP",			0, 1, 0, true } },
	{ Instruction::JUMPI,		{ "JUMPI",			0, 2, 0, true } },
	{ Instruction::PC,			{ "PC",				0, 0, 1, false } },
	{ Instruction::MSIZE,		{ "MSIZE",			0, 0, 1, false } },
	{ Instruction::GAS,			{ "GAS",			0, 0, 1, false } },
	{ Instruction::JUMPDEST,	{ "JUMPDEST",		0, 1, 0, true } },
	{ Instruction::PUSH1,		{ "PUSH1",			1, 0, 1, false } },
	{ Instruction::PUSH2,		{ "PUSH2",			2, 0, 1, false } },
	{ Instruction::PUSH3,		{ "PUSH3",			3, 0, 1, false } },
	{ Instruction::PUSH4,		{ "PUSH4",			4, 0, 1, false } },
	{ Instruction::PUSH5,		{ "PUSH5",			5, 0, 1, false } },
	{ Instruction::PUSH6,		{ "PUSH6",			6, 0, 1, false } },
	{ Instruction::PUSH7,		{ "PUSH7",			7, 0, 1, false } },
	{ Instruction::PUSH8,		{ "PUSH8",			8, 0, 1, false } },
	{ Instruction::PUSH9,		{ "PUSH9",			9, 0, 1, false } },
	{ Instruction::PUSH10,		{ "PUSH10",			10, 0, 1, false } },
	{ Instruction::PUSH11,		{ "PUSH11",			11, 0, 1, false } },
	{ Instruction::PUSH12,		{ "PUSH12",			12, 0, 1, false } },
	{ Instruction::PUSH13,		{ "PUSH13",			13, 0, 1, false } },
	{ Instruction::PUSH14,		{ "PUSH14",			14, 0, 1, false } },
	{ Instruction::PUSH15,		{ "PUSH15",			15, 0, 1, false } },
	{ Instruction::PUSH16,		{ "PUSH16",			16, 0, 1, false } },
	{ Instruction::PUSH17,		{ "PUSH17",			17, 0, 1, false } },
	{ Instruction::PUSH18,		{ "PUSH18",			18, 0, 1, false } },
	{ Instruction::PUSH19,		{ "PUSH19",			19, 0, 1, false } },
	{ Instruction::PUSH20,		{ "PUSH20",			20, 0, 1, false } },
	{ Instruction::PUSH21,		{ "PUSH21",			21, 0, 1, false } },
	{ Instruction::PUSH22,		{ "PUSH22",			22, 0, 1, false } },
	{ Instruction::PUSH23,		{ "PUSH23",			23, 0, 1, false } },
	{ Instruction::PUSH24,		{ "PUSH24",			24, 0, 1, false } },
	{ Instruction::PUSH25,		{ "PUSH25",			25, 0, 1, false } },
	{ Instruction::PUSH26,		{ "PUSH26",			26, 0, 1, false } },
	{ Instruction::PUSH27,		{ "PUSH27",			27, 0, 1, false } },
	{ Instruction::PUSH28,		{ "PUSH28",			28, 0, 1, false } },
	{ Instruction::PUSH29,		{ "PUSH29",			29, 0, 1, false } },
	{ Instruction::PUSH30,		{ "PUSH30",			30, 0, 1, false } },
	{ Instruction::PUSH31,		{ "PUSH31",			31, 0, 1, false } },
	{ Instruction::PUSH32,		{ "PUSH32",			32, 0, 1, false } },
	{ Instruction::DUP1,		{ "DUP1",			0, 1, 2, false } },
	{ Instruction::DUP2,		{ "DUP2",			0, 2, 3, false } },
	{ Instruction::DUP3,		{ "DUP3",			0, 3, 4, false } },
	{ Instruction::DUP4,		{ "DUP4",			0, 4, 5, false } },
	{ Instruction::DUP5,		{ "DUP5",			0, 5, 6, false } },
	{ Instruction::DUP6,		{ "DUP6",			0, 6, 7, false } },
	{ Instruction::DUP7,		{ "DUP7",			0, 7, 8, false } },
	{ Instruction::DUP8,		{ "DUP8",			0, 8, 9, false } },
	{ Instruction::DUP9,		{ "DUP9",			0, 9, 10, false } },
	{ Instruction::DUP10,		{ "DUP10",			0, 10, 11, false } },
	{ Instruction::DUP11,		{ "DUP11",			0, 11, 12, false } },
	{ Instruction::DUP12,		{ "DUP12",			0, 12, 13, false } },
	{ Instruction::DUP13,		{ "DUP13",			0, 13, 14, false } },
	{ Instruction::DUP14,		{ "DUP14",			0, 14, 15, false } },
	{ Instruction::DUP15,		{ "DUP15",			0, 15, 16, false } },
	{ Instruction::DUP16,		{ "DUP16",			0, 16, 17, false } },
	{ Instruction::SWAP1,		{ "SWAP1",			0, 2, 2, false } },
	{ Instruction::SWAP2,		{ "SWAP2",			0, 3, 3, false } },
	{ Instruction::SWAP3,		{ "SWAP3",			0, 4, 4, false } },
	{ Instruction::SWAP4,		{ "SWAP4",			0, 5, 5, false } },
	{ Instruction::SWAP5,		{ "SWAP5",			0, 6, 6, false } },
	{ Instruction::SWAP6,		{ "SWAP6",			0, 7, 7, false } },
	{ Instruction::SWAP7,		{ "SWAP7",			0, 8, 8, false } },
	{ Instruction::SWAP8,		{ "SWAP8",			0, 9, 9, false } },
	{ Instruction::SWAP9,		{ "SWAP9",			0, 10, 10, false } },
	{ Instruction::SWAP10,		{ "SWAP10",			0, 11, 11, false } },
	{ Instruction::SWAP11,		{ "SWAP11",			0, 12, 12, false } },
	{ Instruction::SWAP12,		{ "SWAP12",			0, 13, 13, false } },
	{ Instruction::SWAP13,		{ "SWAP13",			0, 14, 14, false } },
	{ Instruction::SWAP14,		{ "SWAP14",			0, 15, 15, false } },
	{ Instruction::SWAP15,		{ "SWAP15",			0, 16, 16, false } },
	{ Instruction::SWAP16,		{ "SWAP16",			0, 17, 17, false } },
	{ Instruction::LOG0,		{ "LOG0",			0, 2, 0, true } },
	{ Instruction::LOG1,		{ "LOG1",			0, 3, 0, true } },
	{ Instruction::LOG2,		{ "LOG2",			0, 4, 0, true } },
	{ Instruction::LOG3,		{ "LOG3",			0, 5, 0, true } },
	{ Instruction::LOG4,		{ "LOG4",			0, 6, 0, true } },
	{ Instruction::CREATE,		{ "CREATE",			0, 3, 1, true } },
	{ Instruction::CALL,		{ "CALL",			0, 7, 1, true } },
	{ Instruction::CALLCODE,	{ "CALLCODE",		0, 7, 1, true } },
	{ Instruction::RETURN,		{ "RETURN",			0, 2, 0, true } },
	{ Instruction::SUICIDE,		{ "SUICIDE",		0, 1, 0, true } }
};

string dev::eth::disassemble(bytes const& _mem)
{
	stringstream ret;
	unsigned numerics = 0;
	for (auto it = _mem.begin(); it != _mem.end(); ++it)
	{
		byte n = *it;
		auto iit = c_instructionInfo.find((Instruction)n);
		if (numerics || iit == c_instructionInfo.end() || (byte)iit->first != n)	// not an instruction or expecting an argument...
		{
			if (numerics)
				numerics--;
			ret << "0x" << hex << (int)n << " ";
		}
		else
		{
			auto const& ii = iit->second;
			ret << ii.name << " ";
			numerics = ii.additional;
		}
	}
	return ret.str();
}

InstructionInfo dev::eth::instructionInfo(Instruction _inst)
{
	try
	{
		return c_instructionInfo.at(_inst);
	}
	catch (...)
	{
#ifndef BOOST_NO_EXCEPTIONS
		cwarn << "<INVALID_INSTRUCTION: " << toString((unsigned)_inst) << ">\n" << boost::current_exception_diagnostic_information();
#else
		cwarn << "<INVALID_INSTRUCTION: " << toString((unsigned)_inst) << ">\n";
#endif
		return InstructionInfo({"<INVALID_INSTRUCTION: " + toString((unsigned)_inst) + ">", 0, 0, 0, false});
	}
}

bool dev::eth::isValidInstruction(Instruction _inst)
{
	return !!c_instructionInfo.count(_inst);
}
