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


#include <libevmasm/Instruction.h>
#include <libevmasm/SimplificationRule.h>

#include <libdevcore/CommonData.h>

#include <boost/multiprecision/detail/min_max.hpp>

#include <vector>
#include <functional>

namespace dev
{
namespace eth
{

template <class S> S divWorkaround(S const& _a, S const& _b)
{
	return (S)(bigint(_a) / bigint(_b));
}

template <class S> S modWorkaround(S const& _a, S const& _b)
{
	return (S)(bigint(_a) % bigint(_b));
}

// This works around a bug fixed with Boost 1.64.
// https://www.boost.org/doc/libs/1_68_0/libs/multiprecision/doc/html/boost_multiprecision/map/hist.html#boost_multiprecision.map.hist.multiprecision_2_3_1_boost_1_64
inline u256 shlWorkaround(u256 const& _x, unsigned _amount)
{
	return u256((bigint(_x) << _amount) & u256(-1));
}

// simplificationRuleList below was split up into parts to prevent
// stack overflows in the JavaScript optimizer for emscripten builds
// that affected certain browser versions.
template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart1(
	Pattern A,
	Pattern B,
	Pattern C,
	Pattern,
	Pattern
)
{
	return std::vector<SimplificationRule<Pattern>> {
		// arithmetic on constants
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
			return boost::multiprecision::bit_test(B.d(), testBit) ? B.d() | ~mask : B.d() & mask;
		}, false},
		{{Instruction::SHL, {A, B}}, [=]{
			if (A.d() > 255)
				return u256(0);
			return shlWorkaround(B.d(), unsigned(A.d()));
		}, false},
		{{Instruction::SHR, {A, B}}, [=]{
			if (A.d() > 255)
				return u256(0);
			return B.d() >> unsigned(A.d());
		}, false}
	};
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart2(
	Pattern,
	Pattern,
	Pattern,
	Pattern X,
	Pattern Y
)
{
	return std::vector<SimplificationRule<Pattern>> {
		// invariants involving known constants
		{{Instruction::ADD, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::ADD, {0, X}}, [=]{ return X; }, false},
		{{Instruction::SUB, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::MUL, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::MUL, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::MUL, {X, 1}}, [=]{ return X; }, false},
		{{Instruction::MUL, {1, X}}, [=]{ return X; }, false},
		{{Instruction::MUL, {X, u256(-1)}}, [=]() -> Pattern { return {Instruction::SUB, {0, X}}; }, false},
		{{Instruction::MUL, {u256(-1), X}}, [=]() -> Pattern { return {Instruction::SUB, {0, X}}; }, false},
		{{Instruction::DIV, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::DIV, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::DIV, {X, 1}}, [=]{ return X; }, false},
		{{Instruction::SDIV, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::SDIV, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::SDIV, {X, 1}}, [=]{ return X; }, false},
		{{Instruction::AND, {X, ~u256(0)}}, [=]{ return X; }, false},
		{{Instruction::AND, {~u256(0), X}}, [=]{ return X; }, false},
		{{Instruction::AND, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::AND, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::OR, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::OR, {0, X}}, [=]{ return X; }, false},
		{{Instruction::OR, {X, ~u256(0)}}, [=]{ return ~u256(0); }, true},
		{{Instruction::OR, {~u256(0), X}}, [=]{ return ~u256(0); }, true},
		{{Instruction::XOR, {X, 0}}, [=]{ return X; }, false},
		{{Instruction::XOR, {0, X}}, [=]{ return X; }, false},
		{{Instruction::MOD, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::MOD, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::EQ, {X, 0}}, [=]() -> Pattern { return {Instruction::ISZERO, {X}}; }, false },
		{{Instruction::EQ, {0, X}}, [=]() -> Pattern { return {Instruction::ISZERO, {X}}; }, false },
		{{Instruction::SHL, {0, X}}, [=]{ return X; }, false},
		{{Instruction::SHR, {0, X}}, [=]{ return X; }, false},
		{{Instruction::SHL, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::SHR, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::GT, {X, 0}}, [=]() -> Pattern { return {Instruction::ISZERO, {{Instruction::ISZERO, {X}}}}; }, false},
		{{Instruction::LT, {0, X}}, [=]() -> Pattern { return {Instruction::ISZERO, {{Instruction::ISZERO, {X}}}}; }, false},
		{{Instruction::GT, {X, ~u256(0)}}, [=]{ return u256(0); }, true},
		{{Instruction::LT, {~u256(0), X}}, [=]{ return u256(0); }, true},
		{{Instruction::GT, {0, X}}, [=]{ return u256(0); }, true},
		{{Instruction::LT, {X, 0}}, [=]{ return u256(0); }, true},
		{{Instruction::AND, {{Instruction::BYTE, {X, Y}}, {u256(0xff)}}}, [=]() -> Pattern { return {Instruction::BYTE, {X, Y}}; }, false},
		{{Instruction::BYTE, {31, X}}, [=]() -> Pattern { return {Instruction::AND, {X, u256(0xff)}}; }, false}
	};
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart3(
	Pattern,
	Pattern,
	Pattern,
	Pattern X,
	Pattern
)
{
	return std::vector<SimplificationRule<Pattern>> {
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
		{{Instruction::MOD, {X, X}}, [=]{ return u256(0); }, true}
	};
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart4(
	Pattern,
	Pattern,
	Pattern,
	Pattern X,
	Pattern Y
)
{
	return std::vector<SimplificationRule<Pattern>> {
		// logical instruction combinations
		{{Instruction::NOT, {{Instruction::NOT, {X}}}}, [=]{ return X; }, false},
		{{Instruction::XOR, {X, {Instruction::XOR, {X, Y}}}}, [=]{ return Y; }, true},
		{{Instruction::XOR, {X, {Instruction::XOR, {Y, X}}}}, [=]{ return Y; }, true},
		{{Instruction::XOR, {{Instruction::XOR, {X, Y}}, X}}, [=]{ return Y; }, true},
		{{Instruction::XOR, {{Instruction::XOR, {Y, X}}, X}}, [=]{ return Y; }, true},
		{{Instruction::OR, {X, {Instruction::AND, {X, Y}}}}, [=]{ return X; }, true},
		{{Instruction::OR, {X, {Instruction::AND, {Y, X}}}}, [=]{ return X; }, true},
		{{Instruction::OR, {{Instruction::AND, {X, Y}}, X}}, [=]{ return X; }, true},
		{{Instruction::OR, {{Instruction::AND, {Y, X}}, X}}, [=]{ return X; }, true},
		{{Instruction::AND, {X, {Instruction::OR, {X, Y}}}}, [=]{ return X; }, true},
		{{Instruction::AND, {X, {Instruction::OR, {Y, X}}}}, [=]{ return X; }, true},
		{{Instruction::AND, {{Instruction::OR, {X, Y}}, X}}, [=]{ return X; }, true},
		{{Instruction::AND, {{Instruction::OR, {Y, X}}, X}}, [=]{ return X; }, true},
		{{Instruction::AND, {X, {Instruction::NOT, {X}}}}, [=]{ return u256(0); }, true},
		{{Instruction::AND, {{Instruction::NOT, {X}}, X}}, [=]{ return u256(0); }, true},
		{{Instruction::OR, {X, {Instruction::NOT, {X}}}}, [=]{ return ~u256(0); }, true},
		{{Instruction::OR, {{Instruction::NOT, {X}}, X}}, [=]{ return ~u256(0); }, true},
	};
}


template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart5(
	Pattern A,
	Pattern,
	Pattern,
	Pattern X,
	Pattern
)
{
	std::vector<SimplificationRule<Pattern>> rules;

	// Replace MOD X, <power-of-two> with AND X, <power-of-two> - 1
	for (size_t i = 0; i < 256; ++i)
	{
		u256 value = u256(1) << i;
		rules.push_back({
			{Instruction::MOD, {X, value}},
			[=]() -> Pattern { return {Instruction::AND, {X, value - 1}}; },
			false
		});
	}

	// Replace SHL >=256, X with 0
	rules.push_back({
		{Instruction::SHL, {A, X}},
		[=]() -> Pattern { return u256(0); },
		true,
		[=]() { return A.d() >= 256; }
	});

	// Replace SHR >=256, X with 0
	rules.push_back({
		{Instruction::SHR, {A, X}},
		[=]() -> Pattern { return u256(0); },
		true,
		[=]() { return A.d() >= 256; }
	});

	for (auto const& op: std::vector<Instruction>{
		Instruction::ADDRESS,
		Instruction::CALLER,
		Instruction::ORIGIN,
		Instruction::COINBASE,
		Instruction::CREATE,
		Instruction::CREATE2
	})
	{
		u256 const mask = (u256(1) << 160) - 1;
		rules.push_back({
			{Instruction::AND, {{op, mask}}},
			[=]() -> Pattern { return op; },
			false
		});
		rules.push_back({
			{Instruction::AND, {{mask, op}}},
			[=]() -> Pattern { return op; },
			false
		});
	}
	return rules;
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart6(
	Pattern,
	Pattern,
	Pattern,
	Pattern X,
	Pattern Y
)
{
	std::vector<SimplificationRule<Pattern>> rules;
	// Double negation of opcodes with boolean result
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

	return rules;
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart7(
	Pattern A,
	Pattern B,
	Pattern,
	Pattern X,
	Pattern Y
)
{
	std::vector<SimplificationRule<Pattern>> rules;
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
		// Moving constants to the outside, order matters here - we first add rules
		// for constants and then for non-constants.
		// xa can be (X, A) or (A, X)
		for (auto xa: {std::vector<Pattern>{X, A}, std::vector<Pattern>{A, X}})
		{
			rules += std::vector<SimplificationRule<Pattern>>{{
				// (X+A)+B -> X+(A+B)
				{op, {{op, xa}, B}},
				[=]() -> Pattern { return {op, {X, fun(A.d(), B.d())}}; },
				false
			}, {
				// (X+A)+Y -> (X+Y)+A
				{op, {{op, xa}, Y}},
				[=]() -> Pattern { return {op, {{op, {X, Y}}, A}}; },
				false
			}, {
				// B+(X+A) -> X+(A+B)
				{op, {B, {op, xa}}},
				[=]() -> Pattern { return {op, {X, fun(A.d(), B.d())}}; },
				false
			}, {
				// Y+(X+A) -> (Y+X)+A
				{op, {Y, {op, xa}}},
				[=]() -> Pattern { return {op, {{op, {Y, X}}, A}}; },
				false
			}};
		}
	}

	// Combine two SHL by constant
	rules.push_back({
		// SHL(B, SHL(A, X)) -> SHL(min(A+B, 256), X)
		{Instruction::SHL, {{B}, {Instruction::SHL, {{A}, {X}}}}},
		[=]() -> Pattern {
			bigint sum = bigint(A.d()) + B.d();
			if (sum >= 256)
				return {Instruction::AND, {X, u256(0)}};
			else
				return {Instruction::SHL, {u256(sum), X}};
		},
		false
	});

	// Combine two SHR by constant
	rules.push_back({
		// SHR(B, SHR(A, X)) -> SHR(min(A+B, 256), X)
		{Instruction::SHR, {{B}, {Instruction::SHR, {{A}, {X}}}}},
		[=]() -> Pattern {
			bigint sum = bigint(A.d()) + B.d();
			if (sum >= 256)
				return {Instruction::AND, {X, u256(0)}};
			else
				return {Instruction::SHR, {u256(sum), X}};
		},
		false
	});

	// Combine SHL-SHR by constant
	rules.push_back({
		// SHR(B, SHL(A, X)) -> AND(SH[L/R]([B - A / A - B], X), Mask)
		{Instruction::SHR, {{B}, {Instruction::SHL, {{A}, {X}}}}},
		[=]() -> Pattern {
			u256 mask = shlWorkaround(u256(-1), unsigned(A.d())) >> unsigned(B.d());

			if (A.d() > B.d())
				return {Instruction::AND, {{Instruction::SHL, {A.d() - B.d(), X}}, mask}};
			else if (B.d() > A.d())
				return {Instruction::AND, {{Instruction::SHR, {B.d() - A.d(), X}}, mask}};
			else
				return {Instruction::AND, {X, mask}};
		},
		false,
		[=] { return A.d() < 256 && B.d() < 256; }
	});

	// Combine SHR-SHL by constant
	rules.push_back({
		// SHL(B, SHR(A, X)) -> AND(SH[L/R]([B - A / A - B], X), Mask)
		{Instruction::SHL, {{B}, {Instruction::SHR, {{A}, {X}}}}},
		[=]() -> Pattern {
			u256 mask = shlWorkaround(u256(-1) >> unsigned(A.d()), unsigned(B.d()));

			if (A.d() > B.d())
				return {Instruction::AND, {{Instruction::SHR, {A.d() - B.d(), X}}, mask}};
			else if (B.d() > A.d())
				return {Instruction::AND, {{Instruction::SHL, {B.d() - A.d(), X}}, mask}};
			else
				return {Instruction::AND, {X, mask}};
		},
		false,
		[=] { return A.d() < 256 && B.d() < 256; }
	});

	// Move AND with constant across SHL and SHR by constant
	for (auto shiftOp: {Instruction::SHL, Instruction::SHR})
	{
		auto replacement = [=]() -> Pattern {
			u256 mask =
				shiftOp == Instruction::SHL ?
				shlWorkaround(A.d(), unsigned(B.d())) :
				A.d() >> unsigned(B.d());
			return {Instruction::AND, {{shiftOp, {B.d(), X}}, std::move(mask)}};
		};
		rules.push_back({
			// SH[L/R](B, AND(X, A)) -> AND(SH[L/R](B, X), [ A << B / A >> B ])
			{shiftOp, {{B}, {Instruction::AND, {{X}, {A}}}}},
			replacement,
			false,
			[=] { return B.d() < 256; }
		});
		rules.push_back({
			// SH[L/R](B, AND(A, X)) -> AND(SH[L/R](B, X), [ A << B / A >> B ])
			{shiftOp, {{B}, {Instruction::AND, {{A}, {X}}}}},
			replacement,
			false,
			[=] { return B.d() < 256; }
		});
	}

	rules.push_back({
		// MUL(X, SHL(Y, 1)) -> SHL(Y, X)
		{Instruction::MUL, {X, {Instruction::SHL, {Y, u256(1)}}}},
		[=]() -> Pattern {
			return {Instruction::SHL, {Y, X}};
		},
		false
	});
	rules.push_back({
		// MUL(SHL(X, 1), Y) -> SHL(X, Y)
		{Instruction::MUL, {{Instruction::SHL, {X, u256(1)}}, Y}},
		[=]() -> Pattern {
			return {Instruction::SHL, {X, Y}};
		},
		false
	});

	rules.push_back({
		// DIV(X, SHL(Y, 1)) -> SHR(Y, X)
		{Instruction::DIV, {X, {Instruction::SHL, {Y, u256(1)}}}},
		[=]() -> Pattern {
			return {Instruction::SHR, {Y, X}};
		},
		false
	});

	std::function<bool()> feasibilityFunction = [=]() {
		if (B.d() > 256)
			return false;
		unsigned bAsUint = static_cast<unsigned>(B.d());
		return (A.d() & (u256(-1) >> bAsUint)) == (u256(-1) >> bAsUint);
	};

	rules.push_back({
		// AND(A, SHR(B, X)) -> A & ((2^256-1) >> B) == ((2^256-1) >> B)
		{Instruction::AND, {A, {Instruction::SHR, {B, X}}}},
		[=]() -> Pattern { return {Instruction::SHR, {B, X}}; },
		false,
		feasibilityFunction
	});

	rules.push_back({
		// AND(SHR(B, X), A) -> ((2^256-1) >> B) & A == ((2^256-1) >> B)
		{Instruction::AND, {{Instruction::SHR, {B, X}}, A}},
		[=]() -> Pattern { return {Instruction::SHR, {B, X}}; },
		false,
		feasibilityFunction
	});

	return rules;
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart8(
	Pattern A,
	Pattern,
	Pattern,
	Pattern X,
	Pattern Y
)
{
	std::vector<SimplificationRule<Pattern>> rules;

	// move constants across subtractions
	rules += std::vector<SimplificationRule<Pattern>>{
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

/// @returns a list of simplification rules given certain match placeholders.
/// A, B and C should represent constants, X and Y arbitrary expressions.
/// The simplifications should never change the order of evaluation of
/// arbitrary operations.
template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleList(
	Pattern A,
	Pattern B,
	Pattern C,
	Pattern X,
	Pattern Y
)
{
	std::vector<SimplificationRule<Pattern>> rules;
	rules += simplificationRuleListPart1(A, B, C, X, Y);
	rules += simplificationRuleListPart2(A, B, C, X, Y);
	rules += simplificationRuleListPart3(A, B, C, X, Y);
	rules += simplificationRuleListPart4(A, B, C, X, Y);
	rules += simplificationRuleListPart5(A, B, C, X, Y);
	rules += simplificationRuleListPart6(A, B, C, X, Y);
	rules += simplificationRuleListPart7(A, B, C, X, Y);
	rules += simplificationRuleListPart8(A, B, C, X, Y);
	return rules;
}

}
}
