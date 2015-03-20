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
/**
 * @file ExpressionClasses.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Container for equivalence classes of expressions for use in common subexpression elimination.
 */

#include <libevmcore/ExpressionClasses.h>
#include <utility>
#include <tuple>
#include <functional>
#include <boost/range/adaptor/reversed.hpp>
#include <libevmcore/Assembly.h>
#include <libevmcore/CommonSubexpressionEliminator.h>

using namespace std;
using namespace dev;
using namespace dev::eth;


bool ExpressionClasses::Expression::operator<(const ExpressionClasses::Expression& _other) const
{
	auto type = item->type();
	auto otherType = _other.item->type();
	return std::tie(type, item->data(), arguments) <
		std::tie(otherType, _other.item->data(), _other.arguments);
}

ExpressionClasses::Id ExpressionClasses::find(AssemblyItem const& _item, Ids const& _arguments)
{
	Expression exp;
	exp.item = &_item;
	exp.arguments = _arguments;

	if (SemanticInformation::isCommutativeOperation(_item))
		sort(exp.arguments.begin(), exp.arguments.end());

	//@todo store all class members (not only the representatives) in an efficient data structure to search here
	for (Expression const& e: m_representatives)
		if (!(e < exp || exp < e))
			return e.id;

	if (SemanticInformation::isDupInstruction(_item))
	{
		// Special item that refers to values pre-existing on the stack
		m_spareAssemblyItem.push_back(make_shared<AssemblyItem>(_item));
		exp.item = m_spareAssemblyItem.back().get();
	}

	ExpressionClasses::Id id = tryToSimplify(exp);
	if (id < m_representatives.size())
		return id;

	exp.id = m_representatives.size();
	m_representatives.push_back(exp);
	return exp.id;
}

ExpressionClasses::Id ExpressionClasses::tryToSimplify(Expression const& _expr, bool _secondRun)
{
	if (_expr.item->type() != Operation)
		return -1;

	// @todo:
	// ISZERO ISZERO
	// associative operations (as done in Assembly.cpp)
	// 2 * x == x + x

	Id arg1;
	Id arg2;
	Id arg3;
	u256 data1;
	u256 data2;
	u256 data3;
	switch (_expr.arguments.size())
	{
	default:
		arg3 = _expr.arguments.at(2);
		data3 = representative(arg3).item->data();
	case 2:
		arg2 = _expr.arguments.at(1);
		data2 = representative(arg2).item->data();
	case 1:
		arg1 = _expr.arguments.at(0);
		data1 = representative(arg1).item->data();
	case 0:
		break;
	}

	/**
	 * Simplification rule. If _strict is false, Push or a constant matches any constant,
	 * otherwise Push matches "0" and a constant matches itself.
	 * "UndefinedItem" matches any expression, but all of them must be equal inside one rule.
	 */
	struct Rule
	{
		Rule(AssemblyItems const& _pattern, bool _strict, function<AssemblyItem()> const& _action):
			pattern(_pattern),
			assemblyItemAction(_action),
			strict(_strict)
		{}
		Rule(AssemblyItems const& _pattern, function<AssemblyItem()> _action):
			Rule(_pattern, false, _action)
		{}
		Rule(AssemblyItems const& _pattern, bool _strict, function<Id()> const& _action):
			pattern(_pattern),
			idAction(_action),
			strict(_strict)
		{}
		Rule(AssemblyItems const& _pattern, function<Id()> _action):
			Rule(_pattern, false, _action)
		{}
		bool matches(ExpressionClasses const& _classes, Expression const& _expr) const
		{
			if (!_expr.item->match(pattern.front()))
				return false;
			assertThrow(_expr.arguments.size() == pattern.size() - 1, OptimizerException, "");
			Id argRequiredToBeEqual(-1);
			for (size_t i = 1; i < pattern.size(); ++i)
			{
				Id arg = _expr.arguments[i - 1];
				if (pattern[i].type() == UndefinedItem)
				{
					if (argRequiredToBeEqual == Id(-1))
						argRequiredToBeEqual = arg;
					else if (argRequiredToBeEqual != arg)
						return false;
				}
				else
				{
					AssemblyItem const& argItem = *_classes.representative(arg).item;
					if (strict && argItem != pattern[i])
						return false;
					else if (!strict && !argItem.match(pattern[i]))
						return false;
				}
			}
			return true;
		}

		AssemblyItems pattern;
		function<AssemblyItem()> assemblyItemAction;
		function<Id()> idAction;
		bool strict;
	};

	vector<Rule> c_singleLevel{
		// arithmetics on constants involving only stack variables
		{{Instruction::ADD, Push, Push}, [&]{ return data1 + data2; }},
		{{Instruction::MUL, Push, Push}, [&]{ return data1 * data2; }},
		{{Instruction::SUB, Push, Push}, [&]{ return data1 - data2; }},
		{{Instruction::DIV, Push, Push}, [&]{ return data2 == 0 ? 0 : data1 / data2; }},
		{{Instruction::SDIV, Push, Push}, [&]{ return data2 == 0 ? 0 : s2u(u2s(data1) / u2s(data2)); }},
		{{Instruction::MOD, Push, Push}, [&]{ return data2 == 0 ? 0 : data1 % data2; }},
		{{Instruction::SMOD, Push, Push}, [&]{ return data2 == 0 ? 0 : s2u(u2s(data1) % u2s(data2)); }},
		{{Instruction::EXP, Push, Push}, [&]{ return u256(boost::multiprecision::powm(bigint(data1), bigint(data2), bigint(1) << 256)); }},
		{{Instruction::NOT, Push}, [&]{ return ~data1; }},
		{{Instruction::LT, Push, Push}, [&]() -> u256 { return data1 < data2 ? 1 : 0; }},
		{{Instruction::GT, Push, Push}, [&]() -> u256 { return data1 > data2 ? 1 : 0; }},
		{{Instruction::SLT, Push, Push}, [&]() -> u256 { return u2s(data1) < u2s( data2) ? 1 : 0; }},
		{{Instruction::SGT, Push, Push}, [&]() -> u256 { return u2s(data1) > u2s( data2) ? 1 : 0; }},
		{{Instruction::EQ, Push, Push}, [&]() -> u256 { return data1 == data2 ? 1 : 0; }},
		{{Instruction::ISZERO, Push}, [&]() -> u256 { return data1 == 0 ? 1 : 0; }},
		{{Instruction::AND, Push, Push}, [&]{ return data1 & data2; }},
		{{Instruction::OR, Push, Push}, [&]{ return data1 | data2; }},
		{{Instruction::XOR, Push, Push}, [&]{ return data1 ^ data2; }},
		{{Instruction::BYTE, Push, Push}, [&]{ return data1 >= 32 ? 0 : (data2 >> unsigned(8 * (31 - data1))) & 0xff; }},
		{{Instruction::ADDMOD, Push, Push, Push}, [&]{ return data3 == 0 ? 0 : u256((bigint(data1) + bigint(data2)) % data3); }},
		{{Instruction::MULMOD, Push, Push, Push}, [&]{ return data3 == 0 ? 0 : u256((bigint(data1) * bigint(data2)) % data3); }},
		{{Instruction::MULMOD, Push, Push, Push}, [&]{ return data1 * data2; }},
		{{Instruction::SIGNEXTEND, Push, Push}, [&]{
			if (data1 >= 31)
				return data2;
			unsigned testBit = unsigned(data1) * 8 + 7;
			u256 mask = (u256(1) << testBit) - 1;
			return u256(boost::multiprecision::bit_test(data2, testBit) ? data2 | ~mask : data2 & mask);
		}},
		{{Instruction::ADD, UndefinedItem, u256(0)}, true, [&]{ return arg1; }},
		{{Instruction::MUL, UndefinedItem, u256(1)}, true, [&]{ return arg1; }},
		{{Instruction::OR, UndefinedItem, u256(0)}, true, [&]{ return arg1; }},
		{{Instruction::XOR, UndefinedItem, u256(0)}, true, [&]{ return arg1; }},
		{{Instruction::AND, UndefinedItem, ~u256(0)}, true, [&]{ return arg1; }},
		{{Instruction::MUL, UndefinedItem, u256(0)}, true, [&]{ return u256(0); }},
		{{Instruction::DIV, UndefinedItem, u256(0)}, true, [&]{ return u256(0); }},
		{{Instruction::MOD, UndefinedItem, u256(0)}, true, [&]{ return u256(0); }},
		{{Instruction::MOD, u256(0), UndefinedItem}, true, [&]{ return u256(0); }},
		{{Instruction::AND, UndefinedItem, u256(0)}, true, [&]{ return u256(0); }},
		{{Instruction::OR, UndefinedItem, ~u256(0)}, true, [&]{ return ~u256(0); }},
		{{Instruction::AND, UndefinedItem, UndefinedItem}, true, [&]{ return arg1; }},
		{{Instruction::OR, UndefinedItem, UndefinedItem}, true, [&]{ return arg1; }},
		{{Instruction::SUB, UndefinedItem, UndefinedItem}, true, [&]{ return u256(0); }},
		{{Instruction::EQ, UndefinedItem, UndefinedItem}, true, [&]{ return u256(1); }},
		{{Instruction::LT, UndefinedItem, UndefinedItem}, true, [&]{ return u256(0); }},
		{{Instruction::SLT, UndefinedItem, UndefinedItem}, true, [&]{ return u256(0); }},
		{{Instruction::GT, UndefinedItem, UndefinedItem}, true, [&]{ return u256(0); }},
		{{Instruction::SGT, UndefinedItem, UndefinedItem}, true, [&]{ return u256(0); }},
		{{Instruction::MOD, UndefinedItem, UndefinedItem}, true, [&]{ return u256(0); }},
	};

	for (auto const& rule: c_singleLevel)
		if (rule.matches(*this, _expr))
		{
			if (rule.idAction)
				return rule.idAction();
			else
			{
				m_spareAssemblyItem.push_back(make_shared<AssemblyItem>(rule.assemblyItemAction()));
				return find(*m_spareAssemblyItem.back());
			}
		}

	if (!_secondRun && _expr.arguments.size() == 2 && SemanticInformation::isCommutativeOperation(*_expr.item))
	{
		Expression expr = _expr;
		swap(expr.arguments[0], expr.arguments[1]);
		return tryToSimplify(expr, true);
	}

	return -1;
}
