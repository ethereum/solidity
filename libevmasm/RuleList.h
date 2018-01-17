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
 * @date 2018
 * Templatized list of simplification rules.
 */

#pragma once

#include <vector>
#include <functional>

#include <libevmasm/Instruction.h>

#include <libdevcore/CommonData.h>

namespace dev
{
namespace solidity
{

template <class S> S divWorkaround(S const& _a, S const& _b)
{
	return (S)(bigint(_a) / bigint(_b));
}

template <class S> S modWorkaround(S const& _a, S const& _b)
{
	return (S)(bigint(_a) % bigint(_b));
}

/// @returns a list of simplification rules given certain match placeholders.
/// A, B and C should represent constants, X and Y arbitrary expressions.
/// The third element in the tuple is a boolean flag that indicates whether
/// any non-constant elements in the pattern are removed by applying it.
/// The simplifications should neven change the order of evaluation of
/// arbitrary operations, though.
template <class Pattern>
std::vector<std::tuple<Pattern, std::function<Pattern()>, bool>> simplificationRuleList(
	Pattern A,
	Pattern B,
	Pattern C,
	Pattern X,
	Pattern Y
)
{
	std::vector<std::tuple<Pattern, std::function<Pattern()>, bool>> rules;
	rules += std::vector<std::tuple<Pattern, std::function<Pattern()>, bool>>{
		// arithmetics on constants
		{{Instruction::ADD, {A, B}}, [=]{ return A.d() + B.d(); }, false},
		{{Instruction::MUL, {A, B}}, [=]{ return A.d() * B.d(); }, false},
		{{Instruction::SUB, {A, B}}, [=]{ return A.d() - B.d(); }, false},
		{{Instruction::DIV, {A, B}}, [=]{ return B.d() == 0 ? 0 : divWorkaround(A.d(), B.d()); }, false},
		{{Instruction::SDIV, {A, B}}, [=]{ return B.d() == 0 ? 0 : s2u(divWorkaround(u2s(A.d()), u2s(B.d()))); }, false},
		{{Instruction::MOD, {A, B}}, [=]{ return B.d() == 0 ? 0 : modWorkaround(A.d(), B.d()); }, false},
		{{Instruction::SMOD, {A, B}}, [=]{ return B.d() == 0 ? 0 : s2u(modWorkaround(u2s(A.d()), u2s(B.d()))); }, false},
		{{Instruction::EXP, {A, B}}, [=]{ return u256(boost::multiprecision::powm(bigint(A.d()), bigint(B.d()), bigint(1) << 256)); }, false},
		{{Instruction::NOT, {A}}, [=]{ return ~A.d(); }, false},
		{{Instruction::LT, {A, B}}, [=]() -> u256 { return A.d() < B.d() ? 1 : 0; }, false},
		{{Instruction::GT, {A, B}}, [=]() -> u256 { return A.d() > B.d() ? 1 : 0; }, false},
		{{Instruction::SLT, {A, B}}, [=]() -> u256 { return u2s(A.d()) < u2s(B.d()) ? 1 : 0; }, false},
		{{Instruction::SGT, {A, B}}, [=]() -> u256 { return u2s(A.d()) > u2s(B.d()) ? 1 : 0; }, false},
		{{Instruction::EQ, {A, B}}, [=]() -> u256 { return A.d() == B.d() ? 1 : 0; }, false},
		{{Instruction::ISZERO, {A}}, [=]() -> u256 { return A.d() == 0 ? 1 : 0; }, false},
		{{Instruction::AND, {A, B}}, [=]{ return A.d() & B.d(); }, false},
		{{Instruction::OR, {A, B}}, [=]{ return A.d() | B.d(); }, false},
		{{Instruction::XOR, {A, B}}, [=]{ return A.d() ^ B.d(); }, false},
		{{Instruction::BYTE, {A, B}}, [=]{ return A.d() >= 32 ? 0 : (B.d() >> unsigned(8 * (31 - A.d()))) & 0xff; }, false},
		{{Instruction::ADDMOD, {A, B, C}}, [=]{ return C.d() == 0 ? 0 : u256((bigint(A.d()) + bigint(B.d())) % C.d()); }, false},
		{{Instruction::MULMOD, {A, B, C}}, [=]{ return C.d() == 0 ? 0 : u256((bigint(A.d()) * bigint(B.d())) % C.d()); }, false},
		{{Instruction::MULMOD, {A, B, C}}, [=]{ return A.d() * B.d(); }, false},
		{{Instruction::SIGNEXTEND, {A, B}}, [=]() -> u256 {
			if (A.d() >= 31)
				return B.d();
			unsigned testBit = unsigned(A.d()) * 8 + 7;
			u256 mask = (u256(1) << testBit) - 1;
			return u256(boost::multiprecision::bit_test(B.d(), testBit) ? B.d() | ~mask : B.d() & mask);
		}, false},

		// invariants involving known constants (commutative instructions will be checked with swapped operants too)
		{{Instruction::ADD, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::SUB, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::MUL, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::MUL, {X, 1}}, [=]{ return X; }, false},
		{{Instruction::DIV, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::DIV, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::DIV, {X, 1}}, [=]{ return X; }, false},
		{{Instruction::SDIV, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::SDIV, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::SDIV, {X, 1}}, [=]{ return X; }, false},
		{{Instruction::AND, {X, ~u256(0)}}, [=]{ return X; }, false},
		{{Instruction::AND, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::OR, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::OR, {X, ~u256(0)}}, [=]{ return ~u256(0); }, true},
		{{Instruction::XOR, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::MOD, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::MOD, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::EQ, {X, 0}}, [=]() -> Pattern { return {Instruction::ISZERO, {X}}; }, false },

		// operations involving an expression and itself
		{{Instruction::AND, {X, X}}, [=]{ return X; }, true},
		{{Instruction::OR, {X, X}}, [=]{ return X; }, true},
		{{Instruction::XOR, {X, X}}, [=]{ return u256(0); }, true},
		{{Instruction::SUB, {X, X}}, [=]{ return u256(0); }, true},
		{{Instruction::EQ, {X, X}}, [=]{ return u256(1); }, true},
		{{Instruction::LT, {X, X}}, [=]{ return u256(0); }, true},
		{{Instruction::SLT, {X, X}}, [=]{ return u256(0); }, true},
		{{Instruction::GT, {X, X}}, [=]{ return u256(0); }, true},
		{{Instruction::SGT, {X, X}}, [=]{ return u256(0); }, true},
		{{Instruction::MOD, {X, X}}, [=]{ return u256(0); }, true},

		// logical instruction combinations
		{{Instruction::NOT, {{Instruction::NOT, {X}}}}, [=]{ return X; }, false},
		{{Instruction::XOR, {{{X}, {Instruction::XOR, {X, Y}}}}}, [=]{ return Y; }, true},
		{{Instruction::OR, {{{X}, {Instruction::AND, {X, Y}}}}}, [=]{ return X; }, true},
		{{Instruction::AND, {{{X}, {Instruction::OR, {X, Y}}}}}, [=]{ return X; }, true},
		{{Instruction::AND, {{{X}, {Instruction::NOT, {X}}}}}, [=]{ return u256(0); }, true},
		{{Instruction::OR, {{{X}, {Instruction::NOT, {X}}}}}, [=]{ return ~u256(0); }, true},
	};

	// Double negation of opcodes with binary result
	for (auto const& op: std::vector<Instruction>{
		Instruction::EQ,
		Instruction::LT,
		Instruction::SLT,
		Instruction::GT,
		Instruction::SGT
	})
		rules.push_back({
			{Instruction::ISZERO, {{Instruction::ISZERO, {{op, {X, Y}}}}}},
			[=]() -> Pattern { return {op, {X, Y}}; },
			false
		});

	rules.push_back({
		{Instruction::ISZERO, {{Instruction::ISZERO, {{Instruction::ISZERO, {X}}}}}},
		[=]() -> Pattern { return {Instruction::ISZERO, {X}}; },
		false
	});

	rules.push_back({
		{Instruction::ISZERO, {{Instruction::XOR, {X, Y}}}},
		[=]() -> Pattern { return { Instruction::EQ, {X, Y} }; },
		false
	});

	// Associative operations
	for (auto const& opFun: std::vector<std::pair<Instruction,std::function<u256(u256 const&,u256 const&)>>>{
		{Instruction::ADD, std::plus<u256>()},
		{Instruction::MUL, std::multiplies<u256>()},
		{Instruction::AND, std::bit_and<u256>()},
		{Instruction::OR, std::bit_or<u256>()},
		{Instruction::XOR, std::bit_xor<u256>()}
	})
	{
		auto op = opFun.first;
		auto fun = opFun.second;
		// Moving constants to the outside, order matters here!
		// we need actions that return expressions (or patterns?) here, and we need also reversed rules
		// (X+A)+B -> X+(A+B)
		rules += std::vector<std::tuple<Pattern, std::function<Pattern()>, bool>>{{
			{op, {{op, {X, A}}, B}},
			[=]() -> Pattern { return {op, {X, fun(A.d(), B.d())}}; },
			false
		}, {
		// X+(Y+A) -> (X+Y)+A
			{op, {{op, {X, A}}, Y}},
			[=]() -> Pattern { return {op, {{op, {X, Y}}, A}}; },
			false
		}, {
		// For now, we still need explicit commutativity for the inner pattern
			{op, {{op, {A, X}}, B}},
			[=]() -> Pattern { return {op, {X, fun(A.d(), B.d())}}; },
			false
		}, {
			{op, {{op, {A, X}}, Y}},
			[=]() -> Pattern { return {op, {{op, {X, Y}}, A}}; },
			false
		}};
	}

	// move constants across subtractions
	rules += std::vector<std::tuple<Pattern, std::function<Pattern()>, bool>>{
		{
			// X - A -> X + (-A)
			{Instruction::SUB, {X, A}},
			[=]() -> Pattern { return {Instruction::ADD, {X, 0 - A.d()}}; },
			false
		}, {
			// (X + A) - Y -> (X - Y) + A
			{Instruction::SUB, {{Instruction::ADD, {X, A}}, Y}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, A}}; },
			false
		}, {
			// (A + X) - Y -> (X - Y) + A
			{Instruction::SUB, {{Instruction::ADD, {A, X}}, Y}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, A}}; },
			false
		}, {
			// X - (Y + A) -> (X - Y) + (-A)
			{Instruction::SUB, {X, {Instruction::ADD, {Y, A}}}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, 0 - A.d()}}; },
			false
		}, {
			// X - (A + Y) -> (X - Y) + (-A)
			{Instruction::SUB, {X, {Instruction::ADD, {A, Y}}}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, 0 - A.d()}}; },
			false
		}
	};
	return rules;
}

}
}
