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
/// As the simplification can remove instructions, care has to be taken if multiple
/// non-constant expressions are used. The simplifications should not change the
/// order of operations, though.
template <class Pattern>
std::vector<std::pair<Pattern, std::function<Pattern()>>> simplificationRuleList(
	Pattern A,
	Pattern B,
	Pattern C,
	Pattern X,
	Pattern Y
)
{
	std::vector<std::pair<Pattern, std::function<Pattern()>>> rules;
	rules += std::vector<std::pair<Pattern, std::function<Pattern()>>>{
		// arithmetics on constants
		{{Instruction::ADD, {A, B}}, [=]{ return A.d() +	 B.d(); }},
		{{Instruction::MUL, {A, B}}, [=]{ return A.d() * B.d(); }},
		{{Instruction::SUB, {A, B}}, [=]{ return A.d() - B.d(); }},
		{{Instruction::DIV, {A, B}}, [=]{ return B.d() == 0 ? 0 : divWorkaround(A.d(), B.d()); }},
		{{Instruction::SDIV, {A, B}}, [=]{ return B.d() == 0 ? 0 : s2u(divWorkaround(u2s(A.d()), u2s(B.d()))); }},
		{{Instruction::MOD, {A, B}}, [=]{ return B.d() == 0 ? 0 : modWorkaround(A.d(), B.d()); }},
		{{Instruction::SMOD, {A, B}}, [=]{ return B.d() == 0 ? 0 : s2u(modWorkaround(u2s(A.d()), u2s(B.d()))); }},
		{{Instruction::EXP, {A, B}}, [=]{ return u256(boost::multiprecision::powm(bigint(A.d()), bigint(B.d()), bigint(1) << 256)); }},
		{{Instruction::NOT, {A}}, [=]{ return ~A.d(); }},
		{{Instruction::LT, {A, B}}, [=]() -> u256 { return A.d() < B.d() ? 1 : 0; }},
		{{Instruction::GT, {A, B}}, [=]() -> u256 { return A.d() > B.d() ? 1 : 0; }},
		{{Instruction::SLT, {A, B}}, [=]() -> u256 { return u2s(A.d()) < u2s(B.d()) ? 1 : 0; }},
		{{Instruction::SGT, {A, B}}, [=]() -> u256 { return u2s(A.d()) > u2s(B.d()) ? 1 : 0; }},
		{{Instruction::EQ, {A, B}}, [=]() -> u256 { return A.d() == B.d() ? 1 : 0; }},
		{{Instruction::ISZERO, {A}}, [=]() -> u256 { return A.d() == 0 ? 1 : 0; }},
		{{Instruction::AND, {A, B}}, [=]{ return A.d() & B.d(); }},
		{{Instruction::OR, {A, B}}, [=]{ return A.d() | B.d(); }},
		{{Instruction::XOR, {A, B}}, [=]{ return A.d() ^ B.d(); }},
		{{Instruction::BYTE, {A, B}}, [=]{ return A.d() >= 32 ? 0 : (B.d() >> unsigned(8 * (31 - A.d()))) & 0xff; }},
		{{Instruction::ADDMOD, {A, B, C}}, [=]{ return C.d() == 0 ? 0 : u256((bigint(A.d()) + bigint(B.d())) % C.d()); }},
		{{Instruction::MULMOD, {A, B, C}}, [=]{ return C.d() == 0 ? 0 : u256((bigint(A.d()) * bigint(B.d())) % C.d()); }},
		{{Instruction::MULMOD, {A, B, C}}, [=]{ return A.d() * B.d(); }},
		{{Instruction::SIGNEXTEND, {A, B}}, [=]() -> u256 {
			if (A.d() >= 31)
				return B.d();
			unsigned testBit = unsigned(A.d()) * 8 + 7;
			u256 mask = (u256(1) << testBit) - 1;
			return u256(boost::multiprecision::bit_test(B.d(), testBit) ? B.d() | ~mask : B.d() & mask);
		}},

		// invariants involving known constants (commutative instructions will be checked with swapped operants too)
		{{Instruction::ADD, {X, 0}}, [=]{ return X; }},
		{{Instruction::SUB, {X, 0}}, [=]{ return X; }},
		{{Instruction::MUL, {X, 0}}, [=]{ return u256(0); }},
		{{Instruction::MUL, {X, 1}}, [=]{ return X; }},
		{{Instruction::DIV, {X, 0}}, [=]{ return u256(0); }},
		{{Instruction::DIV, {0, X}}, [=]{ return u256(0); }},
		{{Instruction::DIV, {X, 1}}, [=]{ return X; }},
		{{Instruction::SDIV, {X, 0}}, [=]{ return u256(0); }},
		{{Instruction::SDIV, {0, X}}, [=]{ return u256(0); }},
		{{Instruction::SDIV, {X, 1}}, [=]{ return X; }},
		{{Instruction::AND, {X, ~u256(0)}}, [=]{ return X; }},
		{{Instruction::AND, {X, 0}}, [=]{ return u256(0); }},
		{{Instruction::OR, {X, 0}}, [=]{ return X; }},
		{{Instruction::OR, {X, ~u256(0)}}, [=]{ return ~u256(0); }},
		{{Instruction::XOR, {X, 0}}, [=]{ return X; }},
		{{Instruction::MOD, {X, 0}}, [=]{ return u256(0); }},
		{{Instruction::MOD, {0, X}}, [=]{ return u256(0); }},
		{{Instruction::EQ, {X, 0}}, [=]() -> Pattern { return {Instruction::ISZERO, {X}}; } },

		// operations involving an expression and itself
		{{Instruction::AND, {X, X}}, [=]{ return X; }},
		{{Instruction::OR, {X, X}}, [=]{ return X; }},
		{{Instruction::XOR, {X, X}}, [=]{ return u256(0); }},
		{{Instruction::SUB, {X, X}}, [=]{ return u256(0); }},
		{{Instruction::EQ, {X, X}}, [=]{ return u256(1); }},
		{{Instruction::LT, {X, X}}, [=]{ return u256(0); }},
		{{Instruction::SLT, {X, X}}, [=]{ return u256(0); }},
		{{Instruction::GT, {X, X}}, [=]{ return u256(0); }},
		{{Instruction::SGT, {X, X}}, [=]{ return u256(0); }},
		{{Instruction::MOD, {X, X}}, [=]{ return u256(0); }},

		// logical instruction combinations
		{{Instruction::NOT, {{Instruction::NOT, {X}}}}, [=]{ return X; }},
		{{Instruction::XOR, {{{X}, {Instruction::XOR, {X, Y}}}}}, [=]{ return Y; }},
		{{Instruction::OR, {{{X}, {Instruction::AND, {X, Y}}}}}, [=]{ return X; }},
		{{Instruction::AND, {{{X}, {Instruction::OR, {X, Y}}}}}, [=]{ return X; }},
		{{Instruction::AND, {{{X}, {Instruction::NOT, {X}}}}}, [=]{ return u256(0); }},
		{{Instruction::OR, {{{X}, {Instruction::NOT, {X}}}}}, [=]{ return ~u256(0); }},
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
			[=]() -> Pattern { return {op, {X, Y}}; }
		});

	rules.push_back({
		{Instruction::ISZERO, {{Instruction::ISZERO, {{Instruction::ISZERO, {X}}}}}},
		[=]() -> Pattern { return {Instruction::ISZERO, {X}}; }
	});

	rules.push_back({
		{Instruction::ISZERO, {{Instruction::XOR, {X, Y}}}},
		[=]() -> Pattern { return { Instruction::EQ, {X, Y} }; }
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
		rules += std::vector<std::pair<Pattern, std::function<Pattern()>>>{{
			{op, {{op, {X, A}}, B}},
			[=]() -> Pattern { return {op, {X, fun(A.d(), B.d())}}; }
		}, {
		// X+(Y+A) -> (X+Y)+A
			{op, {{op, {X, A}}, Y}},
			[=]() -> Pattern { return {op, {{op, {X, Y}}, A}}; }
		}, {
		// For now, we still need explicit commutativity for the inner pattern
			{op, {{op, {A, X}}, B}},
			[=]() -> Pattern { return {op, {X, fun(A.d(), B.d())}}; }
		}, {
			{op, {{op, {A, X}}, Y}},
			[=]() -> Pattern { return {op, {{op, {X, Y}}, A}}; }
		}};
	}

	// move constants across subtractions
	rules += std::vector<std::pair<Pattern, std::function<Pattern()>>>{
		{
			// X - A -> X + (-A)
			{Instruction::SUB, {X, A}},
			[=]() -> Pattern { return {Instruction::ADD, {X, 0 - A.d()}}; }
		}, {
			// (X + A) - Y -> (X - Y) + A
			{Instruction::SUB, {{Instruction::ADD, {X, A}}, Y}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, A}}; }
		}, {
			// (A + X) - Y -> (X - Y) + A
			{Instruction::SUB, {{Instruction::ADD, {A, X}}, Y}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, A}}; }
		}, {
			// X - (Y + A) -> (X - Y) + (-A)
			{Instruction::SUB, {X, {Instruction::ADD, {Y, A}}}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, 0 - A.d()}}; }
		}, {
			// X - (A + Y) -> (X - Y) + (-A)
			{Instruction::SUB, {X, {Instruction::ADD, {A, Y}}}},
			[=]() -> Pattern { return {Instruction::ADD, {{Instruction::SUB, {X, Y}}, 0 - A.d()}}; }
		}
	};
	return rules;
}

}
}
