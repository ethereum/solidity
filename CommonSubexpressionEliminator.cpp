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
 * @file CommonSubexpressionEliminator.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Optimizer step for common subexpression elimination and stack reorganisation.
 */

#include <functional>
#include <libevmcore/CommonSubexpressionEliminator.h>
#include <libevmcore/Assembly.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

vector<AssemblyItem> CommonSubexpressionEliminator::getOptimizedItems() const
{
	auto streamEquivalenceClass = [this](ostream& _out, EquivalenceClass _id)
	{
		auto const& eqClass = m_equivalenceClasses[_id];
		_out << "  " << _id << ": " << *eqClass.first;
		_out << "(";
		for (EquivalenceClass arg: eqClass.second)
			_out << dec << arg << ",";
		_out << ")" << endl;
	};

	cout << dec;
	cout << "Optimizer results:" << endl;
	cout << "Final stack height: " << m_stackHeight << endl;
	cout << "Stack elements: " << endl;
	for (auto const& it: m_stackElements)
	{
		cout
			<< "  " << dec << it.first.first << "(" << it.first.second << ") = ";
		streamEquivalenceClass(cout, it.second);
	}
	cout << "Equivalence classes: " << endl;
	for (EquivalenceClass eqClass = 0; eqClass < m_equivalenceClasses.size(); ++eqClass)
		streamEquivalenceClass(cout, eqClass);
	cout << "----------------------------" << endl;

	if (m_stackElements.size() == 0)
	{

	}
	int stackHeight;
//	m_stackElements
	// for all stack elements from most neg to most pos:
	//
	return vector<AssemblyItem>();
}

bool CommonSubexpressionEliminator::breaksBasicBlock(AssemblyItem const& _item)
{
	switch (_item.type())
	{
	case UndefinedItem:
	case Tag:
		return true;
	case Push:
	case PushString:
	case PushTag:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushData:
		return false;
	case Operation:
		return instructionInfo(_item.instruction()).sideEffects;
	}
}

void CommonSubexpressionEliminator::feedItem(AssemblyItem const& _item)
{
	cout << _item << endl;
	if (_item.type() != Operation)
	{
		if (_item.deposit() != 1)
			BOOST_THROW_EXCEPTION(InvalidDeposit());
		setStackElement(++m_stackHeight, getClass(_item, {}));
	}
	else
	{
		Instruction instruction = _item.instruction();
		InstructionInfo info = instructionInfo(instruction);
		if (Instruction::DUP1 <= instruction && instruction <= Instruction::DUP16)
			setStackElement(
				m_stackHeight + 1,
				getStackElement(m_stackHeight - int(instruction) + int(Instruction::DUP1))
			);
		else if (Instruction::SWAP1 <= instruction && instruction <= Instruction::SWAP16)
			swapStackElements(
				m_stackHeight,
				m_stackHeight - 1 - int(instruction) + int(Instruction::SWAP1)
			);
		else if (instruction != Instruction::POP)
		{
			vector<EquivalenceClass> arguments(info.args);
			for (int i = 0; i < info.args; ++i)
				arguments[i] = getStackElement(m_stackHeight - i);
			setStackElement(m_stackHeight + info.ret - info.args, getClass(_item, arguments));
		}
		m_stackHeight += info.ret - info.args;
	}
}

void CommonSubexpressionEliminator::setStackElement(int _stackHeight, EquivalenceClass _class)
{
	unsigned nextSequence = getNextStackElementSequence(_stackHeight);
	m_stackElements[make_pair(_stackHeight, nextSequence)] = _class;
}

void CommonSubexpressionEliminator::swapStackElements(int _stackHeightA, int _stackHeightB)
{
	if (_stackHeightA == _stackHeightB)
		BOOST_THROW_EXCEPTION(OptimizerException() << errinfo_comment("Swap on same stack elements."));
	EquivalenceClass classA = getStackElement(_stackHeightA);
	EquivalenceClass classB = getStackElement(_stackHeightB);

	unsigned nextSequenceA = getNextStackElementSequence(_stackHeightA);
	unsigned nextSequenceB = getNextStackElementSequence(_stackHeightB);
	m_stackElements[make_pair(_stackHeightA, nextSequenceA)] = classB;
	m_stackElements[make_pair(_stackHeightB, nextSequenceB)] = classA;
}

CommonSubexpressionEliminator::EquivalenceClass CommonSubexpressionEliminator::getStackElement(int _stackHeight)
{
	// retrieve class by last sequence number
	unsigned nextSequence = getNextStackElementSequence(_stackHeight);
	if (nextSequence > 0)
		return m_stackElements[make_pair(_stackHeight, nextSequence - 1)];

	// Stack element not found (not assigned yet), create new equivalence class.
	if (_stackHeight > 0)
		BOOST_THROW_EXCEPTION(OptimizerException() << errinfo_comment("Stack element accessed before assignment."));
	if (_stackHeight <= -16)
		BOOST_THROW_EXCEPTION(OptimizerException() << errinfo_comment("Stack too deep."));
	// This is a special assembly item that refers to elements pre-existing on the initial stack.
	m_spareAssemblyItem.push_back(make_shared<AssemblyItem>(dupInstruction(1 - _stackHeight)));
	m_equivalenceClasses.push_back(make_pair(m_spareAssemblyItem.back().get(), EquivalenceClasses()));
	return m_stackElements[make_pair(_stackHeight, nextSequence)] = EquivalenceClass(m_equivalenceClasses.size() - 1);
}

CommonSubexpressionEliminator::EquivalenceClass CommonSubexpressionEliminator::getClass(
	const AssemblyItem& _item,
	EquivalenceClasses const& _arguments
)
{
	// do a clever search, i.e.
	// - check for the presence of constants in the argument classes and do arithmetic
	// - check whether the two items are equal for a SUB instruction
	// - check whether 0 or 1 is in one of the classes for a MUL
	// - for commutative opcodes, sort the arguments before searching
	for (EquivalenceClass c = 0; c < m_equivalenceClasses.size(); ++c)
	{
		AssemblyItem const& classItem = *m_equivalenceClasses[c].first;
		if (classItem != _item)
			continue;
		if (_arguments.size() != m_equivalenceClasses[c].second.size())
			BOOST_THROW_EXCEPTION(
				OptimizerException() <<
				errinfo_comment("Equal assembly items with different number of arguments.")
			);
		if (equal(_arguments.begin(), _arguments.end(), m_equivalenceClasses[c].second.begin()))
			return c;
	}
	if (_item.type() == Operation && _arguments.size() == 2 && all_of(
				_arguments.begin(),
				_arguments.end(),
				[this](EquivalenceClass eqc) { return m_equivalenceClasses[eqc].first->match(Push); }))
	{
		map<Instruction, function<u256(u256, u256)>> const arithmetics =
		{
				//@todo these are not correct (e.g. for div by zero)
			{ Instruction::SUB, [](u256 a, u256 b)->u256{return a - b;} },
			{ Instruction::DIV, [](u256 a, u256 b)->u256{return a / b;} },
			{ Instruction::SDIV, [](u256 a, u256 b)->u256{return s2u(u2s(a) / u2s(b));} },
			{ Instruction::MOD, [](u256 a, u256 b)->u256{return a % b;} },
			{ Instruction::SMOD, [](u256 a, u256 b)->u256{return s2u(u2s(a) % u2s(b));} },
			{ Instruction::EXP, [](u256 a, u256 b)->u256{return (u256)boost::multiprecision::powm((bigint)a, (bigint)b, bigint(1) << 256);} },
			//{ Instruction::SIGNEXTEND, signextend },
			{ Instruction::LT, [](u256 a, u256 b)->u256{return a < b ? 1 : 0;} },
			{ Instruction::GT, [](u256 a, u256 b)->u256{return a > b ? 1 : 0;} },
			{ Instruction::SLT, [](u256 a, u256 b)->u256{return u2s(a) < u2s(b) ? 1 : 0;} },
			{ Instruction::SGT, [](u256 a, u256 b)->u256{return u2s(a) > u2s(b) ? 1 : 0;} },
			{ Instruction::EQ, [](u256 a, u256 b)->u256{return a == b ? 1 : 0;} },
			{ Instruction::ADD, [](u256 a, u256 b)->u256{return a + b;} },
			{ Instruction::MUL, [](u256 a, u256 b)->u256{return a * b;} },
			{ Instruction::AND, [](u256 a, u256 b)->u256{return a & b;} },
			{ Instruction::OR, [](u256 a, u256 b)->u256{return a | b;} },
			{ Instruction::XOR, [](u256 a, u256 b)->u256{return a ^ b;} },
		};
		if (arithmetics.count(_item.instruction()))
		{
			u256 result = arithmetics.at(_item.instruction())(
				m_equivalenceClasses[_arguments[0]].first->data(),
				m_equivalenceClasses[_arguments[1]].first->data()
			);
			m_spareAssemblyItem.push_back(make_shared<AssemblyItem>(result));
			return getClass(*m_spareAssemblyItem.back());
		}
	}
	m_equivalenceClasses.push_back(make_pair(&_item, _arguments));
	return m_equivalenceClasses.size() - 1;
}

unsigned CommonSubexpressionEliminator::getNextStackElementSequence(int _stackHeight)
{
	auto it = m_stackElements.upper_bound(make_pair(_stackHeight, unsigned(-1)));
	if (it == m_stackElements.begin())
		return 0;
	--it;
	if (it->first.first == _stackHeight)
		return it->first.second + 1;
	else
		return 0;
}
