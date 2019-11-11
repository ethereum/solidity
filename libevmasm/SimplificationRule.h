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
 * Expression simplification pattern.
 */

#pragma once

#include <libevmasm/Instruction.h>
#include <functional>

namespace dev
{
namespace eth
{

/**
 * Rule that contains a pattern, an action that can be applied
 * after the pattern has matched and a bool that indicates
 * whether the action would remove something from the expression
 * than is not a constant literal.
 */
template <class Pattern>
struct SimplificationRule
{
	SimplificationRule(
		Pattern _pattern,
		std::function<Pattern()> _action,
		bool _removesNonConstants,
		std::function<bool()> _feasible = {}
	):
		pattern(std::move(_pattern)),
		action(std::move(_action)),
		removesNonConstants(_removesNonConstants),
		feasible(std::move(_feasible))
	{}

	Pattern pattern;
	std::function<Pattern()> action;
	bool removesNonConstants;
	std::function<bool()> feasible;
};

struct EVMBuiltins
{
	using InstrType = Instruction;
	static auto constexpr STOP = Instruction::STOP;
	static auto constexpr ADD = Instruction::ADD;
	static auto constexpr SUB = Instruction::SUB;
	static auto constexpr MUL = Instruction::MUL;
	static auto constexpr DIV = Instruction::DIV;
	static auto constexpr SDIV = Instruction::SDIV;
	static auto constexpr MOD = Instruction::MOD;
	static auto constexpr SMOD = Instruction::SMOD;
	static auto constexpr EXP = Instruction::EXP;
	static auto constexpr NOT = Instruction::NOT;
	static auto constexpr LT = Instruction::LT;
	static auto constexpr GT = Instruction::GT;
	static auto constexpr SLT = Instruction::SLT;
	static auto constexpr SGT = Instruction::SGT;
	static auto constexpr EQ = Instruction::EQ;
	static auto constexpr ISZERO = Instruction::ISZERO;
	static auto constexpr AND = Instruction::AND;
	static auto constexpr OR = Instruction::OR;
	static auto constexpr XOR = Instruction::XOR;
	static auto constexpr BYTE = Instruction::BYTE;
	static auto constexpr SHL = Instruction::SHL;
	static auto constexpr SHR = Instruction::SHR;
	static auto constexpr SAR = Instruction::SAR;
	static auto constexpr ADDMOD = Instruction::ADDMOD;
	static auto constexpr MULMOD = Instruction::MULMOD;
	static auto constexpr SIGNEXTEND = Instruction::SIGNEXTEND;
	static auto constexpr KECCAK256 = Instruction::KECCAK256;
	static auto constexpr ADDRESS = Instruction::ADDRESS;
	static auto constexpr BALANCE = Instruction::BALANCE;
	static auto constexpr ORIGIN = Instruction::ORIGIN;
	static auto constexpr CALLER = Instruction::CALLER;
	static auto constexpr CALLVALUE = Instruction::CALLVALUE;
	static auto constexpr CALLDATALOAD = Instruction::CALLDATALOAD;
	static auto constexpr CALLDATASIZE = Instruction::CALLDATASIZE;
	static auto constexpr CALLDATACOPY = Instruction::CALLDATACOPY;
	static auto constexpr CODESIZE = Instruction::CODESIZE;
	static auto constexpr CODECOPY = Instruction::CODECOPY;
	static auto constexpr GASPRICE = Instruction::GASPRICE;
	static auto constexpr EXTCODESIZE = Instruction::EXTCODESIZE;
	static auto constexpr EXTCODECOPY = Instruction::EXTCODECOPY;
	static auto constexpr RETURNDATASIZE = Instruction::RETURNDATASIZE;
	static auto constexpr RETURNDATACOPY = Instruction::RETURNDATACOPY;
	static auto constexpr EXTCODEHASH = Instruction::EXTCODEHASH;
	static auto constexpr BLOCKHASH = Instruction::BLOCKHASH;
	static auto constexpr COINBASE = Instruction::COINBASE;
	static auto constexpr TIMESTAMP = Instruction::TIMESTAMP;
	static auto constexpr NUMBER = Instruction::NUMBER;
	static auto constexpr DIFFICULTY = Instruction::DIFFICULTY;
	static auto constexpr GASLIMIT = Instruction::GASLIMIT;
	static auto constexpr CHAINID = Instruction::CHAINID;
	static auto constexpr SELFBALANCE = Instruction::SELFBALANCE;
	static auto constexpr POP = Instruction::POP;
	static auto constexpr MLOAD = Instruction::MLOAD;
	static auto constexpr MSTORE = Instruction::MSTORE;
	static auto constexpr MSTORE8 = Instruction::MSTORE8;
	static auto constexpr SLOAD = Instruction::SLOAD;
	static auto constexpr SSTORE = Instruction::SSTORE;
	static auto constexpr PC = Instruction::PC;
	static auto constexpr MSIZE = Instruction::MSIZE;
	static auto constexpr GAS = Instruction::GAS;
	static auto constexpr LOG0 = Instruction::LOG0;
	static auto constexpr LOG1 = Instruction::LOG1;
	static auto constexpr LOG2 = Instruction::LOG2;
	static auto constexpr LOG3 = Instruction::LOG3;
	static auto constexpr LOG4 = Instruction::LOG4;
	static auto constexpr CREATE = Instruction::CREATE;
	static auto constexpr CALL = Instruction::CALL;
	static auto constexpr CALLCODE = Instruction::CALLCODE;
	static auto constexpr STATICCALL = Instruction::STATICCALL;
	static auto constexpr RETURN = Instruction::RETURN;
	static auto constexpr DELEGATECALL = Instruction::DELEGATECALL;
	static auto constexpr CREATE2 = Instruction::CREATE2;
	static auto constexpr REVERT = Instruction::REVERT;
	static auto constexpr INVALID = Instruction::INVALID;
	static auto constexpr SELFDESTRUCT = Instruction::SELFDESTRUCT;
};

}
}
