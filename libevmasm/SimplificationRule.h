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
 * Expression simplification pattern.
 */

#pragma once

#include <libevmasm/Instruction.h>
#include <libsolutil/CommonData.h>
#include <functional>

namespace solidity::evmasm
{

/**
 * Rule that contains a pattern, an action that can be applied
 * after the pattern has matched and optional condition to check if the
 * action should be applied.
 */
template <class Pattern>
struct SimplificationRule
{
	SimplificationRule(
		Pattern _pattern,
		std::function<Pattern()> _action,
		std::function<bool()> _feasible = {}
	):
		pattern(std::move(_pattern)),
		action(std::move(_action)),
		feasible(std::move(_feasible))
	{}

	Pattern pattern;
	std::function<Pattern()> action;
	std::function<bool()> feasible;
};

template <typename Pattern>
struct EVMBuiltins
{
	using InstrType = InternalInstruction;

	template<InternalInstruction inst>
	struct PatternGenerator
	{
		template<typename... Args> constexpr Pattern operator()(Args&&... _args) const
		{
			return {inst, {std::forward<Args>(_args)...}};
		}
	};

	struct PatternGeneratorInstance
	{
		InternalInstruction instruction;
		template<typename... Args> constexpr Pattern operator()(Args&&... _args) const
		{
			return {instruction, {std::forward<Args>(_args)...}};
		}
	};


	static auto constexpr STOP = PatternGenerator<InternalInstruction::STOP>{};
	static auto constexpr ADD = PatternGenerator<InternalInstruction::ADD>{};
	static auto constexpr SUB = PatternGenerator<InternalInstruction::SUB>{};
	static auto constexpr MUL = PatternGenerator<InternalInstruction::MUL>{};
	static auto constexpr DIV = PatternGenerator<InternalInstruction::DIV>{};
	static auto constexpr SDIV = PatternGenerator<InternalInstruction::SDIV>{};
	static auto constexpr MOD = PatternGenerator<InternalInstruction::MOD>{};
	static auto constexpr SMOD = PatternGenerator<InternalInstruction::SMOD>{};
	static auto constexpr EXP = PatternGenerator<InternalInstruction::EXP>{};
	static auto constexpr NOT = PatternGenerator<InternalInstruction::NOT>{};
	static auto constexpr LT = PatternGenerator<InternalInstruction::LT>{};
	static auto constexpr GT = PatternGenerator<InternalInstruction::GT>{};
	static auto constexpr SLT = PatternGenerator<InternalInstruction::SLT>{};
	static auto constexpr SGT = PatternGenerator<InternalInstruction::SGT>{};
	static auto constexpr EQ = PatternGenerator<InternalInstruction::EQ>{};
	static auto constexpr ISZERO = PatternGenerator<InternalInstruction::ISZERO>{};
	static auto constexpr AND = PatternGenerator<InternalInstruction::AND>{};
	static auto constexpr OR = PatternGenerator<InternalInstruction::OR>{};
	static auto constexpr XOR = PatternGenerator<InternalInstruction::XOR>{};
	static auto constexpr BYTE = PatternGenerator<InternalInstruction::BYTE>{};
	static auto constexpr SHL = PatternGenerator<InternalInstruction::SHL>{};
	static auto constexpr SHR = PatternGenerator<InternalInstruction::SHR>{};
	static auto constexpr SAR = PatternGenerator<InternalInstruction::SAR>{};
	static auto constexpr ADDMOD = PatternGenerator<InternalInstruction::ADDMOD>{};
	static auto constexpr MULMOD = PatternGenerator<InternalInstruction::MULMOD>{};
	static auto constexpr SIGNEXTEND = PatternGenerator<InternalInstruction::SIGNEXTEND>{};
	static auto constexpr KECCAK256 = PatternGenerator<InternalInstruction::KECCAK256>{};
	static auto constexpr ADDRESS = PatternGenerator<InternalInstruction::ADDRESS>{};
	static auto constexpr BALANCE = PatternGenerator<InternalInstruction::BALANCE>{};
	static auto constexpr ORIGIN = PatternGenerator<InternalInstruction::ORIGIN>{};
	static auto constexpr CALLER = PatternGenerator<InternalInstruction::CALLER>{};
	static auto constexpr CALLVALUE = PatternGenerator<InternalInstruction::CALLVALUE>{};
	static auto constexpr CALLDATALOAD = PatternGenerator<InternalInstruction::CALLDATALOAD>{};
	static auto constexpr CALLDATASIZE = PatternGenerator<InternalInstruction::CALLDATASIZE>{};
	static auto constexpr CALLDATACOPY = PatternGenerator<InternalInstruction::CALLDATACOPY>{};
	static auto constexpr CODESIZE = PatternGenerator<InternalInstruction::CODESIZE>{};
	static auto constexpr CODECOPY = PatternGenerator<InternalInstruction::CODECOPY>{};
	static auto constexpr GASPRICE = PatternGenerator<InternalInstruction::GASPRICE>{};
	static auto constexpr EXTCODESIZE = PatternGenerator<InternalInstruction::EXTCODESIZE>{};
	static auto constexpr EXTCODECOPY = PatternGenerator<InternalInstruction::EXTCODECOPY>{};
	static auto constexpr RETURNDATASIZE = PatternGenerator<InternalInstruction::RETURNDATASIZE>{};
	static auto constexpr RETURNDATACOPY = PatternGenerator<InternalInstruction::RETURNDATACOPY>{};
	static auto constexpr EXTCODEHASH = PatternGenerator<InternalInstruction::EXTCODEHASH>{};
	static auto constexpr BLOCKHASH = PatternGenerator<InternalInstruction::BLOCKHASH>{};
	static auto constexpr COINBASE = PatternGenerator<InternalInstruction::COINBASE>{};
	static auto constexpr TIMESTAMP = PatternGenerator<InternalInstruction::TIMESTAMP>{};
	static auto constexpr NUMBER = PatternGenerator<InternalInstruction::NUMBER>{};
	static auto constexpr DIFFICULTY = PatternGenerator<InternalInstruction::DIFFICULTY>{};
	static auto constexpr PREVRANDAO = PatternGenerator<InternalInstruction::PREVRANDAO>{};
	static auto constexpr GASLIMIT = PatternGenerator<InternalInstruction::GASLIMIT>{};
	static auto constexpr CHAINID = PatternGenerator<InternalInstruction::CHAINID>{};
	static auto constexpr SELFBALANCE = PatternGenerator<InternalInstruction::SELFBALANCE>{};
	static auto constexpr BASEFEE = PatternGenerator<InternalInstruction::BASEFEE>{};
	static auto constexpr POP = PatternGenerator<InternalInstruction::POP>{};
	static auto constexpr MLOAD = PatternGenerator<InternalInstruction::MLOAD>{};
	static auto constexpr MSTORE = PatternGenerator<InternalInstruction::MSTORE>{};
	static auto constexpr MSTORE8 = PatternGenerator<InternalInstruction::MSTORE8>{};
	static auto constexpr SLOAD = PatternGenerator<InternalInstruction::SLOAD>{};
	static auto constexpr SSTORE = PatternGenerator<InternalInstruction::SSTORE>{};
	static auto constexpr PC = PatternGenerator<InternalInstruction::PC>{};
	static auto constexpr MSIZE = PatternGenerator<InternalInstruction::MSIZE>{};
	static auto constexpr GAS = PatternGenerator<InternalInstruction::GAS>{};
	static auto constexpr LOG0 = PatternGenerator<InternalInstruction::LOG0>{};
	static auto constexpr LOG1 = PatternGenerator<InternalInstruction::LOG1>{};
	static auto constexpr LOG2 = PatternGenerator<InternalInstruction::LOG2>{};
	static auto constexpr LOG3 = PatternGenerator<InternalInstruction::LOG3>{};
	static auto constexpr LOG4 = PatternGenerator<InternalInstruction::LOG4>{};
	static auto constexpr CREATE = PatternGenerator<InternalInstruction::CREATE>{};
	static auto constexpr CALL = PatternGenerator<InternalInstruction::CALL>{};
	static auto constexpr CALLCODE = PatternGenerator<InternalInstruction::CALLCODE>{};
	static auto constexpr STATICCALL = PatternGenerator<InternalInstruction::STATICCALL>{};
	static auto constexpr RETURN = PatternGenerator<InternalInstruction::RETURN>{};
	static auto constexpr DELEGATECALL = PatternGenerator<InternalInstruction::DELEGATECALL>{};
	static auto constexpr CREATE2 = PatternGenerator<InternalInstruction::CREATE2>{};
	static auto constexpr REVERT = PatternGenerator<InternalInstruction::REVERT>{};
	static auto constexpr INVALID = PatternGenerator<InternalInstruction::INVALID>{};
	static auto constexpr SELFDESTRUCT = PatternGenerator<InternalInstruction::SELFDESTRUCT>{};
};

}
