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
		{{Pattern::Builtins::ADD, {A, B}}, [=]{ return A.d() + B.d(); }, false},
		{{Pattern::Builtins::MUL, {A, B}}, [=]{ return A.d() * B.d(); }, false},
		{{Pattern::Builtins::SUB, {A, B}}, [=]{ return A.d() - B.d(); }, false},
		{{Pattern::Builtins::DIV, {A, B}}, [=]{ return B.d() == 0 ? 0 : divWorkaround(A.d(), B.d()); }, false},
		{{Pattern::Builtins::SDIV, {A, B}}, [=]{ return B.d() == 0 ? 0 : s2u(divWorkaround(u2s(A.d()), u2s(B.d()))); }, false},
		{{Pattern::Builtins::MOD, {A, B}}, [=]{ return B.d() == 0 ? 0 : modWorkaround(A.d(), B.d()); }, false},
		{{Pattern::Builtins::SMOD, {A, B}}, [=]{ return B.d() == 0 ? 0 : s2u(modWorkaround(u2s(A.d()), u2s(B.d()))); }, false},
		{{Pattern::Builtins::EXP, {A, B}}, [=]{ return u256(boost::multiprecision::powm(bigint(A.d()), bigint(B.d()), bigint(1) << 256)); }, false},
		{{Pattern::Builtins::NOT, {A}}, [=]{ return ~A.d(); }, false},
		{{Pattern::Builtins::LT, {A, B}}, [=]() -> u256 { return A.d() < B.d() ? 1 : 0; }, false},
		{{Pattern::Builtins::GT, {A, B}}, [=]() -> u256 { return A.d() > B.d() ? 1 : 0; }, false},
		{{Pattern::Builtins::SLT, {A, B}}, [=]() -> u256 { return u2s(A.d()) < u2s(B.d()) ? 1 : 0; }, false},
		{{Pattern::Builtins::SGT, {A, B}}, [=]() -> u256 { return u2s(A.d()) > u2s(B.d()) ? 1 : 0; }, false},
		{{Pattern::Builtins::EQ, {A, B}}, [=]() -> u256 { return A.d() == B.d() ? 1 : 0; }, false},
		{{Pattern::Builtins::ISZERO, {A}}, [=]() -> u256 { return A.d() == 0 ? 1 : 0; }, false},
		{{Pattern::Builtins::AND, {A, B}}, [=]{ return A.d() & B.d(); }, false},
		{{Pattern::Builtins::OR, {A, B}}, [=]{ return A.d() | B.d(); }, false},
		{{Pattern::Builtins::XOR, {A, B}}, [=]{ return A.d() ^ B.d(); }, false},
		{{Pattern::Builtins::BYTE, {A, B}}, [=]{ return A.d() >= 32 ? 0 : (B.d() >> unsigned(8 * (31 - A.d()))) & 0xff; }, false},
		{{Pattern::Builtins::ADDMOD, {A, B, C}}, [=]{ return C.d() == 0 ? 0 : u256((bigint(A.d()) + bigint(B.d())) % C.d()); }, false},
		{{Pattern::Builtins::MULMOD, {A, B, C}}, [=]{ return C.d() == 0 ? 0 : u256((bigint(A.d()) * bigint(B.d())) % C.d()); }, false},
		{{Pattern::Builtins::SIGNEXTEND, {A, B}}, [=]() -> u256 {
			if (A.d() >= 31)
				return B.d();
			unsigned testBit = unsigned(A.d()) * 8 + 7;
			u256 mask = (u256(1) << testBit) - 1;
			return boost::multiprecision::bit_test(B.d(), testBit) ? B.d() | ~mask : B.d() & mask;
		}, false},
		{{Pattern::Builtins::SHL, {A, B}}, [=]{
			if (A.d() > 255)
				return u256(0);
			return shlWorkaround(B.d(), unsigned(A.d()));
		}, false},
		{{Pattern::Builtins::SHR, {A, B}}, [=]{
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
		{{Pattern::Builtins::ADD, {X, 0}}, [=]{ return X; }, false},
		{{Pattern::Builtins::ADD, {0, X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::SUB, {X, 0}}, [=]{ return X; }, false},
		{{Pattern::Builtins::SUB, {~u256(0), X}}, [=]() -> Pattern { return {Pattern::Builtins::NOT, {X}}; }, false},
		{{Pattern::Builtins::MUL, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::MUL, {0, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::MUL, {X, 1}}, [=]{ return X; }, false},
		{{Pattern::Builtins::MUL, {1, X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::MUL, {X, u256(-1)}}, [=]() -> Pattern { return {Pattern::Builtins::SUB, {0, X}}; }, false},
		{{Pattern::Builtins::MUL, {u256(-1), X}}, [=]() -> Pattern { return {Pattern::Builtins::SUB, {0, X}}; }, false},
		{{Pattern::Builtins::DIV, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::DIV, {0, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::DIV, {X, 1}}, [=]{ return X; }, false},
		{{Pattern::Builtins::SDIV, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::SDIV, {0, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::SDIV, {X, 1}}, [=]{ return X; }, false},
		{{Pattern::Builtins::AND, {X, ~u256(0)}}, [=]{ return X; }, false},
		{{Pattern::Builtins::AND, {~u256(0), X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::AND, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::AND, {0, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::OR, {X, 0}}, [=]{ return X; }, false},
		{{Pattern::Builtins::OR, {0, X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::OR, {X, ~u256(0)}}, [=]{ return ~u256(0); }, true},
		{{Pattern::Builtins::OR, {~u256(0), X}}, [=]{ return ~u256(0); }, true},
		{{Pattern::Builtins::XOR, {X, 0}}, [=]{ return X; }, false},
		{{Pattern::Builtins::XOR, {0, X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::MOD, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::MOD, {0, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::EQ, {X, 0}}, [=]() -> Pattern { return {Pattern::Builtins::ISZERO, {X}}; }, false },
		{{Pattern::Builtins::EQ, {0, X}}, [=]() -> Pattern { return {Pattern::Builtins::ISZERO, {X}}; }, false },
		{{Pattern::Builtins::SHL, {0, X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::SHR, {0, X}}, [=]{ return X; }, false},
		{{Pattern::Builtins::SHL, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::SHR, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::GT, {X, 0}}, [=]() -> Pattern { return {Pattern::Builtins::ISZERO, {{Pattern::Builtins::ISZERO, {X}}}}; }, false},
		{{Pattern::Builtins::LT, {0, X}}, [=]() -> Pattern { return {Pattern::Builtins::ISZERO, {{Pattern::Builtins::ISZERO, {X}}}}; }, false},
		{{Pattern::Builtins::GT, {X, ~u256(0)}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::LT, {~u256(0), X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::GT, {0, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::LT, {X, 0}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::AND, {{Pattern::Builtins::BYTE, {X, Y}}, {u256(0xff)}}}, [=]() -> Pattern { return {Pattern::Builtins::BYTE, {X, Y}}; }, false},
		{{Pattern::Builtins::BYTE, {31, X}}, [=]() -> Pattern { return {Pattern::Builtins::AND, {X, u256(0xff)}}; }, false}
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
		{{Pattern::Builtins::AND, {X, X}}, [=]{ return X; }, true},
		{{Pattern::Builtins::OR, {X, X}}, [=]{ return X; }, true},
		{{Pattern::Builtins::XOR, {X, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::SUB, {X, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::EQ, {X, X}}, [=]{ return u256(1); }, true},
		{{Pattern::Builtins::LT, {X, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::SLT, {X, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::GT, {X, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::SGT, {X, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::MOD, {X, X}}, [=]{ return u256(0); }, true}
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
		{{Pattern::Builtins::NOT, {{Pattern::Builtins::NOT, {X}}}}, [=]{ return X; }, false},
		{{Pattern::Builtins::XOR, {X, {Pattern::Builtins::XOR, {X, Y}}}}, [=]{ return Y; }, true},
		{{Pattern::Builtins::XOR, {X, {Pattern::Builtins::XOR, {Y, X}}}}, [=]{ return Y; }, true},
		{{Pattern::Builtins::XOR, {{Pattern::Builtins::XOR, {X, Y}}, X}}, [=]{ return Y; }, true},
		{{Pattern::Builtins::XOR, {{Pattern::Builtins::XOR, {Y, X}}, X}}, [=]{ return Y; }, true},
		{{Pattern::Builtins::OR, {X, {Pattern::Builtins::AND, {X, Y}}}}, [=]{ return X; }, true},
		{{Pattern::Builtins::OR, {X, {Pattern::Builtins::AND, {Y, X}}}}, [=]{ return X; }, true},
		{{Pattern::Builtins::OR, {{Pattern::Builtins::AND, {X, Y}}, X}}, [=]{ return X; }, true},
		{{Pattern::Builtins::OR, {{Pattern::Builtins::AND, {Y, X}}, X}}, [=]{ return X; }, true},
		{{Pattern::Builtins::AND, {X, {Pattern::Builtins::OR, {X, Y}}}}, [=]{ return X; }, true},
		{{Pattern::Builtins::AND, {X, {Pattern::Builtins::OR, {Y, X}}}}, [=]{ return X; }, true},
		{{Pattern::Builtins::AND, {{Pattern::Builtins::OR, {X, Y}}, X}}, [=]{ return X; }, true},
		{{Pattern::Builtins::AND, {{Pattern::Builtins::OR, {Y, X}}, X}}, [=]{ return X; }, true},
		{{Pattern::Builtins::AND, {X, {Pattern::Builtins::NOT, {X}}}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::AND, {{Pattern::Builtins::NOT, {X}}, X}}, [=]{ return u256(0); }, true},
		{{Pattern::Builtins::OR, {X, {Pattern::Builtins::NOT, {X}}}}, [=]{ return ~u256(0); }, true},
		{{Pattern::Builtins::OR, {{Pattern::Builtins::NOT, {X}}, X}}, [=]{ return ~u256(0); }, true},
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
			{Pattern::Builtins::MOD, {X, value}},
			[=]() -> Pattern { return {Pattern::Builtins::AND, {X, value - 1}}; },
			false
		});
	}

	// Replace SHL >=256, X with 0
	rules.push_back({
		{Pattern::Builtins::SHL, {A, X}},
		[=]() -> Pattern { return u256(0); },
		true,
		[=]() { return A.d() >= 256; }
	});

	// Replace SHR >=256, X with 0
	rules.push_back({
		{Pattern::Builtins::SHR, {A, X}},
		[=]() -> Pattern { return u256(0); },
		true,
		[=]() { return A.d() >= 256; }
	});

	// Replace BYTE(A, X), A >= 32 with 0
	rules.push_back({
		{Pattern::Builtins::BYTE, {A, X}},
		[=]() -> Pattern { return u256(0); },
		true,
		[=]() { return A.d() >= 32; }
	});

	for (auto const& op: std::vector<Instruction>{
		Pattern::Builtins::ADDRESS,
		Pattern::Builtins::CALLER,
		Pattern::Builtins::ORIGIN,
		Pattern::Builtins::COINBASE
	})
	{
		u256 const mask = (u256(1) << 160) - 1;
		rules.push_back({
			{Pattern::Builtins::AND, {{op, mask}}},
			[=]() -> Pattern { return op; },
			false
		});
		rules.push_back({
			{Pattern::Builtins::AND, {{mask, op}}},
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
		Pattern::Builtins::EQ,
		Pattern::Builtins::LT,
		Pattern::Builtins::SLT,
		Pattern::Builtins::GT,
		Pattern::Builtins::SGT
	})
		rules.push_back({
			{Pattern::Builtins::ISZERO, {{Pattern::Builtins::ISZERO, {{op, {X, Y}}}}}},
			[=]() -> Pattern { return {op, {X, Y}}; },
			false
		});

	rules.push_back({
		{Pattern::Builtins::ISZERO, {{Pattern::Builtins::ISZERO, {{Pattern::Builtins::ISZERO, {X}}}}}},
		[=]() -> Pattern { return {Pattern::Builtins::ISZERO, {X}}; },
		false
	});

	rules.push_back({
		{Pattern::Builtins::ISZERO, {{Pattern::Builtins::XOR, {X, Y}}}},
		[=]() -> Pattern { return { Pattern::Builtins::EQ, {X, Y} }; },
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
		{Pattern::Builtins::ADD, std::plus<u256>()},
		{Pattern::Builtins::MUL, std::multiplies<u256>()},
		{Pattern::Builtins::AND, std::bit_and<u256>()},
		{Pattern::Builtins::OR, std::bit_or<u256>()},
		{Pattern::Builtins::XOR, std::bit_xor<u256>()}
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
		{Pattern::Builtins::SHL, {{B}, {Pattern::Builtins::SHL, {{A}, {X}}}}},
		[=]() -> Pattern {
			bigint sum = bigint(A.d()) + B.d();
			if (sum >= 256)
				return {Pattern::Builtins::AND, {X, u256(0)}};
			else
				return {Pattern::Builtins::SHL, {u256(sum), X}};
		},
		false
	});

	// Combine two SHR by constant
	rules.push_back({
		// SHR(B, SHR(A, X)) -> SHR(min(A+B, 256), X)
		{Pattern::Builtins::SHR, {{B}, {Pattern::Builtins::SHR, {{A}, {X}}}}},
		[=]() -> Pattern {
			bigint sum = bigint(A.d()) + B.d();
			if (sum >= 256)
				return {Pattern::Builtins::AND, {X, u256(0)}};
			else
				return {Pattern::Builtins::SHR, {u256(sum), X}};
		},
		false
	});

	// Combine SHL-SHR by constant
	rules.push_back({
		// SHR(B, SHL(A, X)) -> AND(SH[L/R]([B - A / A - B], X), Mask)
		{Pattern::Builtins::SHR, {{B}, {Pattern::Builtins::SHL, {{A}, {X}}}}},
		[=]() -> Pattern {
			u256 mask = shlWorkaround(u256(-1), unsigned(A.d())) >> unsigned(B.d());

			if (A.d() > B.d())
				return {Pattern::Builtins::AND, {{Pattern::Builtins::SHL, {A.d() - B.d(), X}}, mask}};
			else if (B.d() > A.d())
				return {Pattern::Builtins::AND, {{Pattern::Builtins::SHR, {B.d() - A.d(), X}}, mask}};
			else
				return {Pattern::Builtins::AND, {X, mask}};
		},
		false,
		[=] { return A.d() < 256 && B.d() < 256; }
	});

	// Combine SHR-SHL by constant
	rules.push_back({
		// SHL(B, SHR(A, X)) -> AND(SH[L/R]([B - A / A - B], X), Mask)
		{Pattern::Builtins::SHL, {{B}, {Pattern::Builtins::SHR, {{A}, {X}}}}},
		[=]() -> Pattern {
			u256 mask = shlWorkaround(u256(-1) >> unsigned(A.d()), unsigned(B.d()));

			if (A.d() > B.d())
				return {Pattern::Builtins::AND, {{Pattern::Builtins::SHR, {A.d() - B.d(), X}}, mask}};
			else if (B.d() > A.d())
				return {Pattern::Builtins::AND, {{Pattern::Builtins::SHL, {B.d() - A.d(), X}}, mask}};
			else
				return {Pattern::Builtins::AND, {X, mask}};
		},
		false,
		[=] { return A.d() < 256 && B.d() < 256; }
	});

	// Move AND with constant across SHL and SHR by constant
	for (auto shiftOp: {Pattern::Builtins::SHL, Pattern::Builtins::SHR})
	{
		auto replacement = [=]() -> Pattern {
			u256 mask =
				shiftOp == Pattern::Builtins::SHL ?
				shlWorkaround(A.d(), unsigned(B.d())) :
				A.d() >> unsigned(B.d());
			return {Pattern::Builtins::AND, {{shiftOp, {B.d(), X}}, std::move(mask)}};
		};
		rules.push_back({
			// SH[L/R](B, AND(X, A)) -> AND(SH[L/R](B, X), [ A << B / A >> B ])
			{shiftOp, {{B}, {Pattern::Builtins::AND, {{X}, {A}}}}},
			replacement,
			false,
			[=] { return B.d() < 256; }
		});
		rules.push_back({
			// SH[L/R](B, AND(A, X)) -> AND(SH[L/R](B, X), [ A << B / A >> B ])
			{shiftOp, {{B}, {Pattern::Builtins::AND, {{A}, {X}}}}},
			replacement,
			false,
			[=] { return B.d() < 256; }
		});
	}

	rules.push_back({
		// MUL(X, SHL(Y, 1)) -> SHL(Y, X)
		{Pattern::Builtins::MUL, {X, {Pattern::Builtins::SHL, {Y, u256(1)}}}},
		[=]() -> Pattern {
			return {Pattern::Builtins::SHL, {Y, X}};
		},
		// Actually only changes the order, does not remove.
		true
	});
	rules.push_back({
		// MUL(SHL(X, 1), Y) -> SHL(X, Y)
		{Pattern::Builtins::MUL, {{Pattern::Builtins::SHL, {X, u256(1)}}, Y}},
		[=]() -> Pattern {
			return {Pattern::Builtins::SHL, {X, Y}};
		},
		false
	});

	rules.push_back({
		// DIV(X, SHL(Y, 1)) -> SHR(Y, X)
		{Pattern::Builtins::DIV, {X, {Pattern::Builtins::SHL, {Y, u256(1)}}}},
		[=]() -> Pattern {
			return {Pattern::Builtins::SHR, {Y, X}};
		},
		// Actually only changes the order, does not remove.
		true
	});

	std::function<bool()> feasibilityFunction = [=]() {
		if (B.d() > 256)
			return false;
		unsigned bAsUint = static_cast<unsigned>(B.d());
		return (A.d() & (u256(-1) >> bAsUint)) == (u256(-1) >> bAsUint);
	};

	rules.push_back({
		// AND(A, SHR(B, X)) -> A & ((2^256-1) >> B) == ((2^256-1) >> B)
		{Pattern::Builtins::AND, {A, {Pattern::Builtins::SHR, {B, X}}}},
		[=]() -> Pattern { return {Pattern::Builtins::SHR, {B, X}}; },
		false,
		feasibilityFunction
	});

	rules.push_back({
		// AND(SHR(B, X), A) -> ((2^256-1) >> B) & A == ((2^256-1) >> B)
		{Pattern::Builtins::AND, {{Pattern::Builtins::SHR, {B, X}}, A}},
		[=]() -> Pattern { return {Pattern::Builtins::SHR, {B, X}}; },
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
			{Pattern::Builtins::SUB, {X, A}},
			[=]() -> Pattern { return {Pattern::Builtins::ADD, {X, 0 - A.d()}}; },
			false
		}, {
			// (X + A) - Y -> (X - Y) + A
			{Pattern::Builtins::SUB, {{Pattern::Builtins::ADD, {X, A}}, Y}},
			[=]() -> Pattern { return {Pattern::Builtins::ADD, {{Pattern::Builtins::SUB, {X, Y}}, A}}; },
			false
		}, {
			// (A + X) - Y -> (X - Y) + A
			{Pattern::Builtins::SUB, {{Pattern::Builtins::ADD, {A, X}}, Y}},
			[=]() -> Pattern { return {Pattern::Builtins::ADD, {{Pattern::Builtins::SUB, {X, Y}}, A}}; },
			false
		}, {
			// X - (Y + A) -> (X - Y) + (-A)
			{Pattern::Builtins::SUB, {X, {Pattern::Builtins::ADD, {Y, A}}}},
			[=]() -> Pattern { return {Pattern::Builtins::ADD, {{Pattern::Builtins::SUB, {X, Y}}, 0 - A.d()}}; },
			false
		}, {
			// X - (A + Y) -> (X - Y) + (-A)
			{Pattern::Builtins::SUB, {X, {Pattern::Builtins::ADD, {A, Y}}}},
			[=]() -> Pattern { return {Pattern::Builtins::ADD, {{Pattern::Builtins::SUB, {X, Y}}, 0 - A.d()}}; },
			false
		}
	};
	return rules;
}

template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleListPart9(
	Pattern,
	Pattern,
	Pattern,
	Pattern W,
	Pattern X,
	Pattern Y,
	Pattern Z
)
{
	std::vector<SimplificationRule<Pattern>> rules;

	u256 const mask = (u256(1) << 160) - 1;
	// CREATE
	rules.push_back({
		{Pattern::Builtins::AND, {{Pattern::Builtins::CREATE, {W, X, Y}}, mask}},
		[=]() -> Pattern { return {Pattern::Builtins::CREATE, {W, X, Y}}; },
		false
	});
	rules.push_back({
		{Pattern::Builtins::AND, {{mask, {Pattern::Builtins::CREATE, {W, X, Y}}}}},
		[=]() -> Pattern { return {Pattern::Builtins::CREATE, {W, X, Y}}; },
		false
	});
	// CREATE2
	rules.push_back({
		{Pattern::Builtins::AND, {{Pattern::Builtins::CREATE2, {W, X, Y, Z}}, mask}},
		[=]() -> Pattern { return {Pattern::Builtins::CREATE2, {W, X, Y, Z}}; },
		false
	});
	rules.push_back({
		{Pattern::Builtins::AND, {{mask, {Pattern::Builtins::CREATE2, {W, X, Y, Z}}}}},
		[=]() -> Pattern { return {Pattern::Builtins::CREATE2, {W, X, Y, Z}}; },
		false
	});

	return rules;
}

/// @returns a list of simplification rules given certain match placeholders.
/// A, B and C should represent constants, W, X, Y, and Z arbitrary expressions.
/// The simplifications should never change the order of evaluation of
/// arbitrary operations.
template <class Pattern>
std::vector<SimplificationRule<Pattern>> simplificationRuleList(
	Pattern A,
	Pattern B,
	Pattern C,
	Pattern W,
	Pattern X,
	Pattern Y,
	Pattern Z
)
{
	std::vector<SimplificationRule<Pattern>> rules;
	rules += simplificationRuleListPart1(A, B, C, W, X);
	rules += simplificationRuleListPart2(A, B, C, W, X);
	rules += simplificationRuleListPart3(A, B, C, W, X);
	rules += simplificationRuleListPart4(A, B, C, W, X);
	rules += simplificationRuleListPart5(A, B, C, W, X);
	rules += simplificationRuleListPart6(A, B, C, W, X);
	rules += simplificationRuleListPart7(A, B, C, W, X);
	rules += simplificationRuleListPart8(A, B, C, W, X);
	rules += simplificationRuleListPart9(A, B, C, W, X, Y, Z);
	return rules;
}

}
}
