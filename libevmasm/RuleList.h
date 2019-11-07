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
template <class S> S shlWorkaround(S const& _x, unsigned _amount)
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;
	return std::vector<SimplificationRule<Pattern>> {
		// arithmetic on constants
		{Builtins::ADD(A, B), [=]{ return A.d() + B.d(); }, false},
		{Builtins::MUL(A, B), [=]{ return A.d() * B.d(); }, false},
		{Builtins::SUB(A, B), [=]{ return A.d() - B.d(); }, false},
		{Builtins::DIV(A, B), [=]{ return B.d() == 0 ? 0 : divWorkaround(A.d(), B.d()); }, false},
		{Builtins::SDIV(A, B), [=]{ return B.d() == 0 ? 0 : s2u(divWorkaround(u2s(A.d()), u2s(B.d()))); }, false},
		{Builtins::MOD(A, B), [=]{ return B.d() == 0 ? 0 : modWorkaround(A.d(), B.d()); }, false},
		{Builtins::SMOD(A, B), [=]{ return B.d() == 0 ? 0 : s2u(modWorkaround(u2s(A.d()), u2s(B.d()))); }, false},
		{Builtins::EXP(A, B), [=]{ return Word(boost::multiprecision::powm(bigint(A.d()), bigint(B.d()), bigint(1) << Pattern::WordSize)); }, false},
		{Builtins::NOT(A), [=]{ return ~A.d(); }, false},
		{Builtins::LT(A, B), [=]() -> Word { return A.d() < B.d() ? 1 : 0; }, false},
		{Builtins::GT(A, B), [=]() -> Word { return A.d() > B.d() ? 1 : 0; }, false},
		{Builtins::SLT(A, B), [=]() -> Word { return u2s(A.d()) < u2s(B.d()) ? 1 : 0; }, false},
		{Builtins::SGT(A, B), [=]() -> Word { return u2s(A.d()) > u2s(B.d()) ? 1 : 0; }, false},
		{Builtins::EQ(A, B), [=]() -> Word { return A.d() == B.d() ? 1 : 0; }, false},
		{Builtins::ISZERO(A), [=]() -> Word { return A.d() == 0 ? 1 : 0; }, false},
		{Builtins::AND(A, B), [=]{ return A.d() & B.d(); }, false},
		{Builtins::OR(A, B), [=]{ return A.d() | B.d(); }, false},
		{Builtins::XOR(A, B), [=]{ return A.d() ^ B.d(); }, false},
		{Builtins::BYTE(A, B), [=]{
			return
				A.d() >= Pattern::WordSize / 8 ?
				0 :
				(B.d() >> unsigned(8 * (Pattern::WordSize / 8 - 1 - A.d()))) & 0xff;
		}, false},
		{Builtins::ADDMOD(A, B, C), [=]{ return C.d() == 0 ? 0 : Word((bigint(A.d()) + bigint(B.d())) % C.d()); }, false},
		{Builtins::MULMOD(A, B, C), [=]{ return C.d() == 0 ? 0 : Word((bigint(A.d()) * bigint(B.d())) % C.d()); }, false},
		{Builtins::SIGNEXTEND(A, B), [=]() -> Word {
			if (A.d() >= Pattern::WordSize / 8 - 1)
				return B.d();
			unsigned testBit = unsigned(A.d()) * 8 + 7;
			Word mask = (Word(1) << testBit) - 1;
			return boost::multiprecision::bit_test(B.d(), testBit) ? B.d() | ~mask : B.d() & mask;
		}, false},
		{Builtins::SHL(A, B), [=]{
			if (A.d() >= Pattern::WordSize)
				return Word(0);
			return shlWorkaround(B.d(), unsigned(A.d()));
		}, false},
		{Builtins::SHR(A, B), [=]{
			if (A.d() >= Pattern::WordSize)
				return Word(0);
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;
	return std::vector<SimplificationRule<Pattern>> {
		// invariants involving known constants
		{Builtins::ADD(X, 0), [=]{ return X; }, false},
		{Builtins::ADD(0, X), [=]{ return X; }, false},
		{Builtins::SUB(X, 0), [=]{ return X; }, false},
		{Builtins::SUB(~Word(0), X), [=]() -> Pattern { return Builtins::NOT(X); }, false},
		{Builtins::MUL(X, 0), [=]{ return Word(0); }, true},
		{Builtins::MUL(0, X), [=]{ return Word(0); }, true},
		{Builtins::MUL(X, 1), [=]{ return X; }, false},
		{Builtins::MUL(1, X), [=]{ return X; }, false},
		{Builtins::MUL(X, Word(-1)), [=]() -> Pattern { return Builtins::SUB(0, X); }, false},
		{Builtins::MUL(Word(-1), X), [=]() -> Pattern { return Builtins::SUB(0, X); }, false},
		{Builtins::DIV(X, 0), [=]{ return Word(0); }, true},
		{Builtins::DIV(0, X), [=]{ return Word(0); }, true},
		{Builtins::DIV(X, 1), [=]{ return X; }, false},
		{Builtins::SDIV(X, 0), [=]{ return Word(0); }, true},
		{Builtins::SDIV(0, X), [=]{ return Word(0); }, true},
		{Builtins::SDIV(X, 1), [=]{ return X; }, false},
		{Builtins::AND(X, ~Word(0)), [=]{ return X; }, false},
		{Builtins::AND(~Word(0), X), [=]{ return X; }, false},
		{Builtins::AND(X, 0), [=]{ return Word(0); }, true},
		{Builtins::AND(0, X), [=]{ return Word(0); }, true},
		{Builtins::OR(X, 0), [=]{ return X; }, false},
		{Builtins::OR(0, X), [=]{ return X; }, false},
		{Builtins::OR(X, ~Word(0)), [=]{ return ~Word(0); }, true},
		{Builtins::OR(~Word(0), X), [=]{ return ~Word(0); }, true},
		{Builtins::XOR(X, 0), [=]{ return X; }, false},
		{Builtins::XOR(0, X), [=]{ return X; }, false},
		{Builtins::MOD(X, 0), [=]{ return Word(0); }, true},
		{Builtins::MOD(0, X), [=]{ return Word(0); }, true},
		{Builtins::EQ(X, 0), [=]() -> Pattern { return Builtins::ISZERO(X); }, false },
		{Builtins::EQ(0, X), [=]() -> Pattern { return Builtins::ISZERO(X); }, false },
		{Builtins::SHL(0, X), [=]{ return X; }, false},
		{Builtins::SHR(0, X), [=]{ return X; }, false},
		{Builtins::SHL(X, 0), [=]{ return Word(0); }, true},
		{Builtins::SHR(X, 0), [=]{ return Word(0); }, true},
		{Builtins::GT(X, 0), [=]() -> Pattern { return Builtins::ISZERO(Builtins::ISZERO(X)); }, false},
		{Builtins::LT(0, X), [=]() -> Pattern { return Builtins::ISZERO(Builtins::ISZERO(X)); }, false},
		{Builtins::GT(X, ~Word(0)), [=]{ return Word(0); }, true},
		{Builtins::LT(~Word(0), X), [=]{ return Word(0); }, true},
		{Builtins::GT(0, X), [=]{ return Word(0); }, true},
		{Builtins::LT(X, 0), [=]{ return Word(0); }, true},
		{Builtins::AND(Builtins::BYTE(X, Y), Word(0xff)), [=]() -> Pattern { return Builtins::BYTE(X, Y); }, false},
		{Builtins::BYTE(Word(Pattern::WordSize / 8 - 1), X), [=]() -> Pattern { return Builtins::AND(X, Word(0xff)); }, false}
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;
	return std::vector<SimplificationRule<Pattern>> {
		// operations involving an expression and itself
		{Builtins::AND(X, X), [=]{ return X; }, true},
		{Builtins::OR(X, X), [=]{ return X; }, true},
		{Builtins::XOR(X, X), [=]{ return Word(0); }, true},
		{Builtins::SUB(X, X), [=]{ return Word(0); }, true},
		{Builtins::EQ(X, X), [=]{ return Word(1); }, true},
		{Builtins::LT(X, X), [=]{ return Word(0); }, true},
		{Builtins::SLT(X, X), [=]{ return Word(0); }, true},
		{Builtins::GT(X, X), [=]{ return Word(0); }, true},
		{Builtins::SGT(X, X), [=]{ return Word(0); }, true},
		{Builtins::MOD(X, X), [=]{ return Word(0); }, true}
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;
	return std::vector<SimplificationRule<Pattern>> {
		// logical instruction combinations
		{Builtins::NOT(Builtins::NOT(X)), [=]{ return X; }, false},
		{Builtins::XOR(X, Builtins::XOR(X, Y)), [=]{ return Y; }, true},
		{Builtins::XOR(X, Builtins::XOR(Y, X)), [=]{ return Y; }, true},
		{Builtins::XOR(Builtins::XOR(X, Y), X), [=]{ return Y; }, true},
		{Builtins::XOR(Builtins::XOR(Y, X), X), [=]{ return Y; }, true},
		{Builtins::OR(X, Builtins::AND(X, Y)), [=]{ return X; }, true},
		{Builtins::OR(X, Builtins::AND(Y, X)), [=]{ return X; }, true},
		{Builtins::OR(Builtins::AND(X, Y), X), [=]{ return X; }, true},
		{Builtins::OR(Builtins::AND(Y, X), X), [=]{ return X; }, true},
		{Builtins::AND(X, Builtins::OR(X, Y)), [=]{ return X; }, true},
		{Builtins::AND(X, Builtins::OR(Y, X)), [=]{ return X; }, true},
		{Builtins::AND(Builtins::OR(X, Y), X), [=]{ return X; }, true},
		{Builtins::AND(Builtins::OR(Y, X), X), [=]{ return X; }, true},
		{Builtins::AND(X, Builtins::NOT(X)), [=]{ return Word(0); }, true},
		{Builtins::AND(Builtins::NOT(X), X), [=]{ return Word(0); }, true},
		{Builtins::OR(X, Builtins::NOT(X)), [=]{ return ~Word(0); }, true},
		{Builtins::OR(Builtins::NOT(X), X), [=]{ return ~Word(0); }, true},
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;

	std::vector<SimplificationRule<Pattern>> rules;

	// Replace MOD X, <power-of-two> with AND X, <power-of-two> - 1
	for (size_t i = 0; i < Pattern::WordSize; ++i)
	{
		Word value = Word(1) << i;
		rules.push_back({
			Builtins::MOD(X, value),
			[=]() -> Pattern { return Builtins::AND(X, value - 1); },
			false
		});
	}

	// Replace SHL >=256, X with 0
	rules.push_back({
		Builtins::SHL(A, X),
		[=]() -> Pattern { return Word(0); },
		true,
		[=]() { return A.d() >= Pattern::WordSize; }
	});

	// Replace SHR >=256, X with 0
	rules.push_back({
		Builtins::SHR(A, X),
		[=]() -> Pattern { return Word(0); },
		true,
		[=]() { return A.d() >= Pattern::WordSize; }
	});

	// Replace BYTE(A, X), A >= 32 with 0
	rules.push_back({
		Builtins::BYTE(A, X),
		[=]() -> Pattern { return Word(0); },
		true,
		[=]() { return A.d() >= Pattern::WordSize / 8; }
	});

	for (auto instr: {
		Instruction::ADDRESS,
		Instruction::CALLER,
		Instruction::ORIGIN,
		Instruction::COINBASE
	})
	{
		assertThrow(Pattern::WordSize > 160, OptimizerException, "");
		Word const mask = (Word(1) << 160) - 1;
		rules.push_back({
			Builtins::AND(Pattern{instr}, mask),
			[=]() -> Pattern { return {instr}; },
			false
		});
		rules.push_back({
			Builtins::AND(mask, Pattern{instr}),
			[=]() -> Pattern { return {instr}; },
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
	using Builtins = typename Pattern::Builtins;

	std::vector<SimplificationRule<Pattern>> rules;
	// Double negation of opcodes with boolean result
	for (auto instr: {
		Instruction::EQ,
		Instruction::LT,
		Instruction::SLT,
		Instruction::GT,
		Instruction::SGT
	})
	{
		typename Builtins::PatternGeneratorInstance op{instr};
		rules.push_back({
			Builtins::ISZERO(Builtins::ISZERO(op(X, Y))),
			[=]() -> Pattern { return op(X, Y); },
			false
		});
	}

	rules.push_back({
		Builtins::ISZERO(Builtins::ISZERO(Builtins::ISZERO(X))),
		[=]() -> Pattern { return Builtins::ISZERO(X); },
		false
	});

	rules.push_back({
		Builtins::ISZERO(Builtins::XOR(X, Y)),
		[=]() -> Pattern { return Builtins::EQ(X, Y); },
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;

	std::vector<SimplificationRule<Pattern>> rules;
	// Associative operations
	for (auto&& instrAndFunc: std::vector<std::pair<Instruction, std::function<Word(Word, Word)>>>{
		{Instruction::ADD, std::plus<Word>()},
		{Instruction::MUL, std::multiplies<Word>()},
		{Instruction::AND, std::bit_and<Word>()},
		{Instruction::OR, std::bit_or<Word>()},
		{Instruction::XOR, std::bit_xor<Word>()}
	})
	{
		typename Builtins::PatternGeneratorInstance op{instrAndFunc.first};
		std::function<Word(Word, Word)> fun = instrAndFunc.second;
		// Moving constants to the outside, order matters here - we first add rules
		// for constants and then for non-constants.
		// xa can be (X, A) or (A, X)
		for (auto const& opXA: {op(X, A), op(A, X)})
		{
			rules += std::vector<SimplificationRule<Pattern>>{{
				// (X+A)+B -> X+(A+B)
				op(opXA, B),
				[=]() -> Pattern { return op(X, fun(A.d(), B.d())); },
				false
			}, {
				// (X+A)+Y -> (X+Y)+A
				op(opXA, Y),
				[=]() -> Pattern { return op(op(X, Y), A); },
				false
			}, {
				// B+(X+A) -> X+(A+B)
				op(B, opXA),
				[=]() -> Pattern { return op(X, fun(A.d(), B.d())); },
				false
			}, {
				// Y+(X+A) -> (Y+X)+A
				op(Y, opXA),
				[=]() -> Pattern { return op(op(Y, X), A); },
				false
			}};
		}
	}

	// Combine two SHL by constant
	rules.push_back({
		// SHL(B, SHL(A, X)) -> SHL(min(A+B, 256), X)
		Builtins::SHL(B, Builtins::SHL(A, X)),
		[=]() -> Pattern {
			bigint sum = bigint(A.d()) + B.d();
			if (sum >= Pattern::WordSize)
				return Builtins::AND(X, Word(0));
			else
				return Builtins::SHL(Word(sum), X);
		},
		false
	});

	// Combine two SHR by constant
	rules.push_back({
		// SHR(B, SHR(A, X)) -> SHR(min(A+B, 256), X)
		Builtins::SHR(B, Builtins::SHR(A, X)),
		[=]() -> Pattern {
			bigint sum = bigint(A.d()) + B.d();
			if (sum >= Pattern::WordSize)
				return Builtins::AND(X, Word(0));
			else
				return Builtins::SHR(Word(sum), X);
		},
		false
	});

	// Combine SHL-SHR by constant
	rules.push_back({
		// SHR(B, SHL(A, X)) -> AND(SH[L/R]([B - A / A - B], X), Mask)
		Builtins::SHR(B, Builtins::SHL(A, X)),
		[=]() -> Pattern {
			Word mask = shlWorkaround(~Word(0), unsigned(A.d())) >> unsigned(B.d());

			if (A.d() > B.d())
				return Builtins::AND(Builtins::SHL(A.d() - B.d(), X), mask);
			else if (B.d() > A.d())
				return Builtins::AND(Builtins::SHR(B.d() - A.d(), X), mask);
			else
				return Builtins::AND(X, mask);
		},
		false,
		[=] { return A.d() < Pattern::WordSize && B.d() < Pattern::WordSize; }
	});

	// Combine SHR-SHL by constant
	rules.push_back({
		// SHL(B, SHR(A, X)) -> AND(SH[L/R]([B - A / A - B], X), Mask)
		Builtins::SHL(B, Builtins::SHR(A, X)),
		[=]() -> Pattern {
			Word mask = shlWorkaround((~Word(0)) >> unsigned(A.d()), unsigned(B.d()));

			if (A.d() > B.d())
				return Builtins::AND(Builtins::SHR(A.d() - B.d(), X), mask);
			else if (B.d() > A.d())
				return Builtins::AND(Builtins::SHL(B.d() - A.d(), X), mask);
			else
				return Builtins::AND(X, mask);
		},
		false,
		[=] { return A.d() < Pattern::WordSize && B.d() < Pattern::WordSize; }
	});

	// Move AND with constant across SHL and SHR by constant
	for (auto instr: {Instruction::SHL, Instruction::SHR})
	{
		typename Builtins::PatternGeneratorInstance shiftOp{instr};
		auto replacement = [=]() -> Pattern {
			Word mask =
				instr == Instruction::SHL ?
				shlWorkaround(A.d(), unsigned(B.d())) :
				A.d() >> unsigned(B.d());
			return Builtins::AND(shiftOp(B.d(), X), std::move(mask));
		};
		rules.push_back({
			// SH[L/R](B, AND(X, A)) -> AND(SH[L/R](B, X), [ A << B / A >> B ])
			shiftOp(B, Builtins::AND(X, A)),
			replacement,
			false,
			[=] { return B.d() < Pattern::WordSize; }
		});
		rules.push_back({
			// SH[L/R](B, AND(A, X)) -> AND(SH[L/R](B, X), [ A << B / A >> B ])
			shiftOp(B, Builtins::AND(A, X)),
			replacement,
			false,
			[=] { return B.d() < Pattern::WordSize; }
		});
	}

	rules.push_back({
		// MUL(X, SHL(Y, 1)) -> SHL(Y, X)
		Builtins::MUL(X, Builtins::SHL(Y, Word(1))),
		[=]() -> Pattern {
			return Builtins::SHL(Y, X);
		},
		// Actually only changes the order, does not remove.
		true
	});
	rules.push_back({
		// MUL(SHL(X, 1), Y) -> SHL(X, Y)
		Builtins::MUL(Builtins::SHL(X, Word(1)), Y),
		[=]() -> Pattern {
			return Builtins::SHL(X, Y);
		},
		false
	});

	rules.push_back({
		// DIV(X, SHL(Y, 1)) -> SHR(Y, X)
		Builtins::DIV(X, Builtins::SHL(Y, Word(1))),
		[=]() -> Pattern {
			return Builtins::SHR(Y, X);
		},
		// Actually only changes the order, does not remove.
		true
	});

	std::function<bool()> feasibilityFunction = [=]() {
		if (B.d() > Pattern::WordSize)
			return false;
		unsigned bAsUint = static_cast<unsigned>(B.d());
		return (A.d() & ((~Word(0)) >> bAsUint)) == ((~Word(0)) >> bAsUint);
	};

	rules.push_back({
		// AND(A, SHR(B, X)) -> A & ((2^256-1) >> B) == ((2^256-1) >> B)
		Builtins::AND(A, Builtins::SHR(B, X)),
		[=]() -> Pattern { return Builtins::SHR(B, X); },
		false,
		feasibilityFunction
	});

	rules.push_back({
		// AND(SHR(B, X), A) -> ((2^256-1) >> B) & A == ((2^256-1) >> B)
		Builtins::AND(Builtins::SHR(B, X), A),
		[=]() -> Pattern { return Builtins::SHR(B, X); },
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
	using Builtins = typename Pattern::Builtins;
	std::vector<SimplificationRule<Pattern>> rules;

	// move constants across subtractions
	rules += std::vector<SimplificationRule<Pattern>>{
		{
			// X - A -> X + (-A)
			Builtins::SUB(X, A),
			[=]() -> Pattern { return Builtins::ADD(X, 0 - A.d()); },
			false
		}, {
			// (X + A) - Y -> (X - Y) + A
			Builtins::SUB(Builtins::ADD(X, A), Y),
			[=]() -> Pattern { return Builtins::ADD(Builtins::SUB(X, Y), A); },
			false
		}, {
			// (A + X) - Y -> (X - Y) + A
			Builtins::SUB(Builtins::ADD(A, X), Y),
			[=]() -> Pattern { return Builtins::ADD(Builtins::SUB(X, Y), A); },
			false
		}, {
			// X - (Y + A) -> (X - Y) + (-A)
			Builtins::SUB(X, Builtins::ADD(Y, A)),
			[=]() -> Pattern { return Builtins::ADD(Builtins::SUB(X, Y), 0 - A.d()); },
			false
		}, {
			// X - (A + Y) -> (X - Y) + (-A)
			Builtins::SUB(X, Builtins::ADD(A, Y)),
			[=]() -> Pattern { return Builtins::ADD(Builtins::SUB(X, Y), 0 - A.d()); },
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
	using Word = typename Pattern::Word;
	using Builtins = typename Pattern::Builtins;
	std::vector<SimplificationRule<Pattern>> rules;

	assertThrow(Pattern::WordSize > 160, OptimizerException, "");
	Word const mask = (Word(1) << 160) - 1;
	// CREATE
	rules.push_back({
		Builtins::AND(Builtins::CREATE(W, X, Y), mask),
		[=]() -> Pattern { return Builtins::CREATE(W, X, Y); },
		false
	});
	rules.push_back({
		Builtins::AND(mask, Builtins::CREATE(W, X, Y)),
		[=]() -> Pattern { return Builtins::CREATE(W, X, Y); },
		false
	});
	// CREATE2
	rules.push_back({
		Builtins::AND(Builtins::CREATE2(W, X, Y, Z), mask),
		[=]() -> Pattern { return Builtins::CREATE2(W, X, Y, Z); },
		false
	});
	rules.push_back({
		Builtins::AND(mask, Builtins::CREATE2(W, X, Y, Z)),
		[=]() -> Pattern { return Builtins::CREATE2(W, X, Y, Z); },
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
	using Word = typename Pattern::Word;
	// Some sanity checks
	assertThrow(Pattern::WordSize % 8 == 0, OptimizerException, "");
	assertThrow(Pattern::WordSize >= 8, OptimizerException, "");
	assertThrow(Pattern::WordSize <= 256, OptimizerException, "");
	assertThrow(Word(-1) == ~Word(0), OptimizerException, "");
	assertThrow(Word(-1) + 1 == Word(0), OptimizerException, "");

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
