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
#include <libdevcore/CommonData.h>
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

template <typename Pattern>
struct EVMBuiltins
{
	using InstrType = Instruction;

	template<Instruction inst>
	struct PatternGenerator
	{
		template<typename... Args> constexpr Pattern operator()(Args&&... _args) const
		{
			return {inst, {std::forward<Args>(_args)...}};
		};
	};

	struct PatternGeneratorInstance
	{
		Instruction instruction;
		template<typename... Args> constexpr Pattern operator()(Args&&... _args) const
		{
			return {instruction, {std::forward<Args>(_args)...}};
		};
	};


	static auto constexpr STOP = PatternGenerator<Instruction::STOP>{};
	static auto constexpr ADD = PatternGenerator<Instruction::ADD>{};
	static auto constexpr SUB = PatternGenerator<Instruction::SUB>{};
	static auto constexpr MUL = PatternGenerator<Instruction::MUL>{};
	static auto constexpr DIV = PatternGenerator<Instruction::DIV>{};
	static auto constexpr SDIV = PatternGenerator<Instruction::SDIV>{};
	static auto constexpr MOD = PatternGenerator<Instruction::MOD>{};
	static auto constexpr SMOD = PatternGenerator<Instruction::SMOD>{};
	static auto constexpr EXP = PatternGenerator<Instruction::EXP>{};
	static auto constexpr NOT = PatternGenerator<Instruction::NOT>{};
	static auto constexpr LT = PatternGenerator<Instruction::LT>{};
	static auto constexpr GT = PatternGenerator<Instruction::GT>{};
	static auto constexpr SLT = PatternGenerator<Instruction::SLT>{};
	static auto constexpr SGT = PatternGenerator<Instruction::SGT>{};
	static auto constexpr EQ = PatternGenerator<Instruction::EQ>{};
	static auto constexpr ISZERO = PatternGenerator<Instruction::ISZERO>{};
	static auto constexpr AND = PatternGenerator<Instruction::AND>{};
	static auto constexpr OR = PatternGenerator<Instruction::OR>{};
	static auto constexpr XOR = PatternGenerator<Instruction::XOR>{};
	static auto constexpr BYTE = PatternGenerator<Instruction::BYTE>{};
	static auto constexpr SHL = PatternGenerator<Instruction::SHL>{};
	static auto constexpr SHR = PatternGenerator<Instruction::SHR>{};
	static auto constexpr SAR = PatternGenerator<Instruction::SAR>{};
	static auto constexpr ADDMOD = PatternGenerator<Instruction::ADDMOD>{};
	static auto constexpr MULMOD = PatternGenerator<Instruction::MULMOD>{};
	static auto constexpr SIGNEXTEND = PatternGenerator<Instruction::SIGNEXTEND>{};
	static auto constexpr KECCAK256 = PatternGenerator<Instruction::KECCAK256>{};
	static auto constexpr ADDRESS = PatternGenerator<Instruction::ADDRESS>{};
	static auto constexpr BALANCE = PatternGenerator<Instruction::BALANCE>{};
	static auto constexpr ORIGIN = PatternGenerator<Instruction::ORIGIN>{};
	static auto constexpr CALLER = PatternGenerator<Instruction::CALLER>{};
	static auto constexpr CALLVALUE = PatternGenerator<Instruction::CALLVALUE>{};
	static auto constexpr CALLDATALOAD = PatternGenerator<Instruction::CALLDATALOAD>{};
	static auto constexpr CALLDATASIZE = PatternGenerator<Instruction::CALLDATASIZE>{};
	static auto constexpr CALLDATACOPY = PatternGenerator<Instruction::CALLDATACOPY>{};
	static auto constexpr CODESIZE = PatternGenerator<Instruction::CODESIZE>{};
	static auto constexpr CODECOPY = PatternGenerator<Instruction::CODECOPY>{};
	static auto constexpr GASPRICE = PatternGenerator<Instruction::GASPRICE>{};
	static auto constexpr EXTCODESIZE = PatternGenerator<Instruction::EXTCODESIZE>{};
	static auto constexpr EXTCODECOPY = PatternGenerator<Instruction::EXTCODECOPY>{};
	static auto constexpr RETURNDATASIZE = PatternGenerator<Instruction::RETURNDATASIZE>{};
	static auto constexpr RETURNDATACOPY = PatternGenerator<Instruction::RETURNDATACOPY>{};
	static auto constexpr EXTCODEHASH = PatternGenerator<Instruction::EXTCODEHASH>{};
	static auto constexpr BLOCKHASH = PatternGenerator<Instruction::BLOCKHASH>{};
	static auto constexpr COINBASE = PatternGenerator<Instruction::COINBASE>{};
	static auto constexpr TIMESTAMP = PatternGenerator<Instruction::TIMESTAMP>{};
	static auto constexpr NUMBER = PatternGenerator<Instruction::NUMBER>{};
	static auto constexpr DIFFICULTY = PatternGenerator<Instruction::DIFFICULTY>{};
	static auto constexpr GASLIMIT = PatternGenerator<Instruction::GASLIMIT>{};
	static auto constexpr CHAINID = PatternGenerator<Instruction::CHAINID>{};
	static auto constexpr SELFBALANCE = PatternGenerator<Instruction::SELFBALANCE>{};
	static auto constexpr POP = PatternGenerator<Instruction::POP>{};
	static auto constexpr MLOAD = PatternGenerator<Instruction::MLOAD>{};
	static auto constexpr MSTORE = PatternGenerator<Instruction::MSTORE>{};
	static auto constexpr MSTORE8 = PatternGenerator<Instruction::MSTORE8>{};
	static auto constexpr SLOAD = PatternGenerator<Instruction::SLOAD>{};
	static auto constexpr SSTORE = PatternGenerator<Instruction::SSTORE>{};
	static auto constexpr PC = PatternGenerator<Instruction::PC>{};
	static auto constexpr MSIZE = PatternGenerator<Instruction::MSIZE>{};
	static auto constexpr GAS = PatternGenerator<Instruction::GAS>{};
	static auto constexpr LOG0 = PatternGenerator<Instruction::LOG0>{};
	static auto constexpr LOG1 = PatternGenerator<Instruction::LOG1>{};
	static auto constexpr LOG2 = PatternGenerator<Instruction::LOG2>{};
	static auto constexpr LOG3 = PatternGenerator<Instruction::LOG3>{};
	static auto constexpr LOG4 = PatternGenerator<Instruction::LOG4>{};
	static auto constexpr CREATE = PatternGenerator<Instruction::CREATE>{};
	static auto constexpr CALL = PatternGenerator<Instruction::CALL>{};
	static auto constexpr CALLCODE = PatternGenerator<Instruction::CALLCODE>{};
	static auto constexpr STATICCALL = PatternGenerator<Instruction::STATICCALL>{};
	static auto constexpr RETURN = PatternGenerator<Instruction::RETURN>{};
	static auto constexpr DELEGATECALL = PatternGenerator<Instruction::DELEGATECALL>{};
	static auto constexpr CREATE2 = PatternGenerator<Instruction::CREATE2>{};
	static auto constexpr REVERT = PatternGenerator<Instruction::REVERT>{};
	static auto constexpr INVALID = PatternGenerator<Instruction::INVALID>{};
	static auto constexpr SELFDESTRUCT = PatternGenerator<Instruction::SELFDESTRUCT>{};
};

}
}
