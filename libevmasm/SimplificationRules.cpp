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
 * @file ExpressionClasses.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Container for equivalence classes of expressions for use in common subexpression elimination.
 */

#include <libevmasm/ExpressionClasses.h>
#include <utility>
#include <tuple>
#include <functional>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/noncopyable.hpp>
#include <libevmasm/Assembly.h>
#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/SimplificationRules.h>

using namespace std;
using namespace dev;
using namespace dev::eth;


pair<Pattern, function<Pattern()> > const* Rules::findFirstMatch(
	Expression const& _expr,
	ExpressionClasses const& _classes
)
{
	resetMatchGroups();

	assertThrow(_expr.item, OptimizerException, "");
	for (auto const& rule: m_rules[byte(_expr.item->instruction())])
	{
		if (rule.first.matches(_expr, _classes))
			return &rule;
		resetMatchGroups();
	}
	return nullptr;
}

void Rules::addRules(std::vector<std::pair<Pattern, std::function<Pattern ()> > > const& _rules)
{
	for (auto const& r: _rules)
		addRule(r);
}

void Rules::addRule(std::pair<Pattern, std::function<Pattern()> > const& _rule)
{
	m_rules[byte(_rule.first.instruction())].push_back(_rule);
}

template <class S> S divWorkaround(S const& _a, S const& _b)
{
	return (S)(bigint(_a) / bigint(_b));
}

template <class S> S modWorkaround(S const& _a, S const& _b)
{
	return (S)(bigint(_a) % bigint(_b));
}

Rules::Rules()
{
	// Multiple occurences of one of these inside one rule must match the same equivalence class.
	// Constants.
	Pattern A(Push);
	Pattern B(Push);
	Pattern C(Push);
	// Anything.
	Pattern X;
	Pattern Y;
	Pattern Z;
	A.setMatchGroup(1, m_matchGroups);
	B.setMatchGroup(2, m_matchGroups);
	C.setMatchGroup(3, m_matchGroups);
	X.setMatchGroup(4, m_matchGroups);
	Y.setMatchGroup(5, m_matchGroups);
	Z.setMatchGroup(6, m_matchGroups);

	addRules(vector<pair<Pattern, function<Pattern()>>>{
		// arithmetics on constants
		{{Instruction::ADD, {A, B}}, [=]{ return A.d() + B.d(); }},
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
	});

	// Double negation of opcodes with binary result
	for (auto const& op: vector<Instruction>{
		Instruction::EQ,
		Instruction::LT,
		Instruction::SLT,
		Instruction::GT,
		Instruction::SGT
	})
		addRule({
			{Instruction::ISZERO, {{Instruction::ISZERO, {{op, {X, Y}}}}}},
			[=]() -> Pattern { return {op, {X, Y}}; }
		});

	addRule({
		{Instruction::ISZERO, {{Instruction::ISZERO, {{Instruction::ISZERO, {X}}}}}},
		[=]() -> Pattern { return {Instruction::ISZERO, {X}}; }
	});

	addRule({
		{Instruction::ISZERO, {{Instruction::XOR, {X, Y}}}},
		[=]() -> Pattern { return { Instruction::EQ, {X, Y} }; }
	});

	// Associative operations
	for (auto const& opFun: vector<pair<Instruction,function<u256(u256 const&,u256 const&)>>>{
		{Instruction::ADD, plus<u256>()},
		{Instruction::MUL, multiplies<u256>()},
		{Instruction::AND, bit_and<u256>()},
		{Instruction::OR, bit_or<u256>()},
		{Instruction::XOR, bit_xor<u256>()}
	})
	{
		auto op = opFun.first;
		auto fun = opFun.second;
		// Moving constants to the outside, order matters here!
		// we need actions that return expressions (or patterns?) here, and we need also reversed rules
		// (X+A)+B -> X+(A+B)
		addRules(vector<pair<Pattern, function<Pattern()>>>{{
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
		}});
	}

	// move constants across subtractions
	addRules(vector<pair<Pattern, function<Pattern()>>>{
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
	});
}

Pattern::Pattern(Instruction _instruction, std::vector<Pattern> const& _arguments):
	m_type(Operation),
	m_instruction(_instruction),
	m_arguments(_arguments)
{
}

void Pattern::setMatchGroup(unsigned _group, map<unsigned, Expression const*>& _matchGroups)
{
	m_matchGroup = _group;
	m_matchGroups = &_matchGroups;
}

bool Pattern::matches(Expression const& _expr, ExpressionClasses const& _classes) const
{
	if (!matchesBaseItem(_expr.item))
		return false;
	if (m_matchGroup)
	{
		if (!m_matchGroups->count(m_matchGroup))
			(*m_matchGroups)[m_matchGroup] = &_expr;
		else if ((*m_matchGroups)[m_matchGroup]->id != _expr.id)
			return false;
	}
	assertThrow(m_arguments.size() == 0 || _expr.arguments.size() == m_arguments.size(), OptimizerException, "");
	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i].matches(_classes.representative(_expr.arguments[i]), _classes))
			return false;
	return true;
}

AssemblyItem Pattern::toAssemblyItem(SourceLocation const& _location) const
{
	if (m_type == Operation)
		return AssemblyItem(m_instruction, _location);
	else
		return AssemblyItem(m_type, data(), _location);
}

string Pattern::toString() const
{
	stringstream s;
	switch (m_type)
	{
	case Operation:
		s << instructionInfo(m_instruction).name;
		break;
	case Push:
		if (m_data)
			s << "PUSH " << hex << data();
		else
			s << "PUSH ";
		break;
	case UndefinedItem:
		s << "ANY";
		break;
	default:
		if (m_data)
			s << "t=" << dec << m_type << " d=" << hex << data();
		else
			s << "t=" << dec << m_type << " d: nullptr";
		break;
	}
	if (!m_requireDataMatch)
		s << " ~";
	if (m_matchGroup)
		s << "[" << dec << m_matchGroup << "]";
	s << "(";
	for (Pattern const& p: m_arguments)
		s << p.toString() << ", ";
	s << ")";
	return s.str();
}

bool Pattern::matchesBaseItem(AssemblyItem const* _item) const
{
	if (m_type == UndefinedItem)
		return true;
	if (!_item)
		return false;
	if (m_type != _item->type())
		return false;
	else if (m_type == Operation)
		return m_instruction == _item->instruction();
	else if (m_requireDataMatch)
		return data() == _item->data();
	return true;
}

Pattern::Expression const& Pattern::matchGroupValue() const
{
	assertThrow(m_matchGroup > 0, OptimizerException, "");
	assertThrow(!!m_matchGroups, OptimizerException, "");
	assertThrow((*m_matchGroups)[m_matchGroup], OptimizerException, "");
	return *(*m_matchGroups)[m_matchGroup];
}

u256 const& Pattern::data() const
{
	assertThrow(m_data, OptimizerException, "");
	return *m_data;
}

ExpressionTemplate::ExpressionTemplate(Pattern const& _pattern, SourceLocation const& _location)
{
	if (_pattern.matchGroup())
	{
		hasId = true;
		id = _pattern.id();
	}
	else
	{
		hasId = false;
		item = _pattern.toAssemblyItem(_location);
	}
	for (auto const& arg: _pattern.arguments())
		arguments.push_back(ExpressionTemplate(arg, _location));
}

string ExpressionTemplate::toString() const
{
	stringstream s;
	if (hasId)
		s << id;
	else
		s << item;
	s << "(";
	for (auto const& arg: arguments)
		s << arg.toString();
	s << ")";
	return s.str();
}
