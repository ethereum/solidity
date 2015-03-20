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
#include <boost/range/adaptor/reversed.hpp>
#include <libevmcore/Assembly.h>
#include <libevmcore/CommonSubexpressionEliminator.h>

using namespace std;
using namespace dev;
using namespace dev::eth;


ExpressionClasses::Id ExpressionClasses::find(AssemblyItem const& _item, Ids const& _arguments)
{
	// TODO: do a clever search, i.e.
	// - check for the presence of constants in the argument classes and do arithmetic
	// - check whether the two items are equal for a SUB instruction
	// - check whether 0 or 1 is in one of the classes for a MUL

	Expression exp;
	exp.item = &_item;
	exp.arguments = _arguments;
	if (SemanticInformation::isCommutativeOperation(_item))
		sort(exp.arguments.begin(), exp.arguments.end());

	//@todo use a data structure that allows better searches
	for (Expression const& e: m_representatives)
		if (std::tie(*e.item, e.arguments) == std::tie(*exp.item, exp.arguments))
			return e.id;

	if (SemanticInformation::isDupInstruction(_item))
	{
		// Special item that refers to values pre-existing on the stack
		m_spareAssemblyItem.push_back(make_shared<AssemblyItem>(_item));
		exp.item = m_spareAssemblyItem.back().get();
	}
	else if (_item.type() == Operation)
	{
		//@todo try to avoid having to do this multiple times by storing not only one representative of
		// an equivalence class

		// constant folding
		auto isConstant = [this](Id eqc) { return representative(eqc).item->match(Push); };
		if (exp.arguments.size() == 2 && all_of(exp.arguments.begin(), exp.arguments.end(), isConstant))
		{
			auto signextend = [](u256 a, u256 b) -> u256
			{
				if (a >= 31)
					return b;
				unsigned testBit = unsigned(a) * 8 + 7;
				u256 mask = (u256(1) << testBit) - 1;
				return boost::multiprecision::bit_test(b, testBit) ? b | ~mask : b & mask;
			};
			map<Instruction, function<u256(u256, u256)>> const arithmetics =
			{
				{ Instruction::SUB, [](u256 a, u256 b) -> u256 {return a - b; } },
				{ Instruction::DIV, [](u256 a, u256 b) -> u256 {return b == 0 ? 0 : a / b; } },
				{ Instruction::SDIV, [](u256 a, u256 b) -> u256 { return b == 0 ? 0 : s2u(u2s(a) / u2s(b)); } },
				{ Instruction::MOD, [](u256 a, u256 b) -> u256 { return b == 0 ? 0 : a % b; } },
				{ Instruction::SMOD, [](u256 a, u256 b) -> u256 { return b == 0 ? 0 : s2u(u2s(a) % u2s(b)); } },
				{ Instruction::EXP, [](u256 a, u256 b) -> u256 { return (u256)boost::multiprecision::powm(bigint(a), bigint(b), bigint(1) << 256); } },
				{ Instruction::SIGNEXTEND, signextend },
				{ Instruction::LT, [](u256 a, u256 b) -> u256 { return a < b ? 1 : 0; } },
				{ Instruction::GT, [](u256 a, u256 b) -> u256 { return a > b ? 1 : 0; } },
				{ Instruction::SLT, [](u256 a, u256 b) -> u256 { return u2s(a) < u2s(b) ? 1 : 0; } },
				{ Instruction::SGT, [](u256 a, u256 b) -> u256 { return u2s(a) > u2s(b) ? 1 : 0; } },
				{ Instruction::EQ, [](u256 a, u256 b) -> u256 { return a == b ? 1 : 0; } },
				{ Instruction::ADD, [](u256 a, u256 b) -> u256 { return a + b; } },
				{ Instruction::MUL, [](u256 a, u256 b) -> u256 { return a * b; } },
				{ Instruction::AND, [](u256 a, u256 b) -> u256 { return a & b; } },
				{ Instruction::OR, [](u256 a, u256 b) -> u256 { return a | b; } },
				{ Instruction::XOR, [](u256 a, u256 b) -> u256 { return a ^ b; } },
			};
			if (arithmetics.count(_item.instruction()))
			{
				u256 result = arithmetics.at(_item.instruction())(
					representative(exp.arguments[0]).item->data(),
					representative(exp.arguments[1]).item->data()
				);
				m_spareAssemblyItem.push_back(make_shared<AssemblyItem>(result));
				return find(*m_spareAssemblyItem.back());
			}
		}
	}
	exp.id = m_representatives.size();
	m_representatives.push_back(exp);
	return exp.id;
}

