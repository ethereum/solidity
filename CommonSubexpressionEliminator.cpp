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
#include <boost/range/adaptor/reversed.hpp>
#include <libevmcore/CommonSubexpressionEliminator.h>
#include <libevmcore/Assembly.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

vector<AssemblyItem> CommonSubexpressionEliminator::getOptimizedItems()
{
	map<int, ExpressionClasses::Id> initialStackContents;
	map<int, ExpressionClasses::Id> targetStackContents;
	int minHeight = m_stackHeight + 1;
	if (!m_stackElements.empty())
		minHeight = min(minHeight, m_stackElements.begin()->first.first);
	for (int height = minHeight; height <= max(0, m_stackHeight); ++height)
	{
		// make sure it is created
		ExpressionClasses::Id c = getStackElement(height);
		if (height <= 0)
			initialStackContents[height] = m_expressionClasses.find(AssemblyItem(dupInstruction(1 - height)));
		if (height <= m_stackHeight)
			targetStackContents[height] = c;
	}

	// Debug info:
	//stream(cout, currentStackContents, targetStackContents);

	return CSECodeGenerator(m_expressionClasses).generateCode(initialStackContents, targetStackContents);
}

ostream& CommonSubexpressionEliminator::stream(
	ostream& _out,
	map<int, ExpressionClasses::Id> _currentStack,
	map<int, ExpressionClasses::Id> _targetStack
) const
{
	auto streamExpressionClass = [this](ostream& _out, ExpressionClasses::Id _id)
	{
		auto const& expr = m_expressionClasses.representative(_id);
		_out << "  " << _id << ": " << *expr.item;
		_out << "(";
		for (ExpressionClasses::Id arg: expr.arguments)
			_out << dec << arg << ",";
		_out << ")" << endl;
	};

	_out << "Optimizer analysis:" << endl;
	_out << "Final stack height: " << dec << m_stackHeight << endl;
	_out << "Stack elements: " << endl;
	for (auto const& it: m_stackElements)
	{
		_out << "  " << dec << it.first.first << "(" << it.first.second << ") = ";
		streamExpressionClass(_out, it.second);
	}
	_out << "Equivalence classes: " << endl;
	for (ExpressionClasses::Id eqClass = 0; eqClass < m_expressionClasses.size(); ++eqClass)
		streamExpressionClass(_out, eqClass);

	_out << "Current stack: " << endl;
	for (auto const& it: _currentStack)
	{
		_out << "  " << dec << it.first << ": ";
		streamExpressionClass(_out, it.second);
	}
	_out << "Target stack: " << endl;
	for (auto const& it: _targetStack)
	{
		_out << "  " << dec << it.first << ": ";
		streamExpressionClass(_out, it.second);
	}

	return _out;
}

void CommonSubexpressionEliminator::feedItem(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
	{
		if (_item.deposit() != 1)
			BOOST_THROW_EXCEPTION(InvalidDeposit());
		setStackElement(++m_stackHeight, m_expressionClasses.find(_item, {}));
	}
	else
	{
		Instruction instruction = _item.instruction();
		InstructionInfo info = instructionInfo(instruction);
		if (SemanticInformation::isDupInstruction(_item))
			setStackElement(
				m_stackHeight + 1,
				getStackElement(m_stackHeight - int(instruction) + int(Instruction::DUP1))
			);
		else if (SemanticInformation::isSwapInstruction(_item))
			swapStackElements(
				m_stackHeight,
				m_stackHeight - 1 - int(instruction) + int(Instruction::SWAP1)
			);
		else if (instruction != Instruction::POP)
		{
			vector<ExpressionClasses::Id> arguments(info.args);
			for (int i = 0; i < info.args; ++i)
				arguments[i] = getStackElement(m_stackHeight - i);
			setStackElement(m_stackHeight + _item.deposit(), m_expressionClasses.find(_item, arguments));
		}
		m_stackHeight += _item.deposit();
	}
}

void CommonSubexpressionEliminator::setStackElement(int _stackHeight, ExpressionClasses::Id _class)
{
	unsigned nextSequence = getNextStackElementSequence(_stackHeight);
	m_stackElements[make_pair(_stackHeight, nextSequence)] = _class;
}

void CommonSubexpressionEliminator::swapStackElements(int _stackHeightA, int _stackHeightB)
{
	if (_stackHeightA == _stackHeightB)
		BOOST_THROW_EXCEPTION(OptimizerException() << errinfo_comment("Swap on same stack elements."));
	ExpressionClasses::Id classA = getStackElement(_stackHeightA);
	ExpressionClasses::Id classB = getStackElement(_stackHeightB);

	unsigned nextSequenceA = getNextStackElementSequence(_stackHeightA);
	unsigned nextSequenceB = getNextStackElementSequence(_stackHeightB);
	m_stackElements[make_pair(_stackHeightA, nextSequenceA)] = classB;
	m_stackElements[make_pair(_stackHeightB, nextSequenceB)] = classA;
}

ExpressionClasses::Id CommonSubexpressionEliminator::getStackElement(int _stackHeight)
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
	return m_stackElements[make_pair(_stackHeight, nextSequence)] =
		m_expressionClasses.find(AssemblyItem(dupInstruction(1 - _stackHeight)));
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

bool SemanticInformation::breaksBasicBlock(AssemblyItem const& _item)
{
	switch (_item.type())
	{
	default:
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
	{
		if (isSwapInstruction(_item) || isDupInstruction(_item))
			return false;
		if (_item.instruction() == Instruction::GAS || _item.instruction() == Instruction::PC)
			return true; // GAS and PC assume a specific order of opcodes
		InstructionInfo info = instructionInfo(_item.instruction());
		// the second requirement will be lifted once it is implemented
		return info.sideEffects || info.args > 2;
	}
	}
}

bool SemanticInformation::isCommutativeOperation(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	switch (_item.instruction())
	{
	case Instruction::ADD:
	case Instruction::MUL:
	case Instruction::EQ:
	case Instruction::AND:
	case Instruction::OR:
	case Instruction::XOR:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::isDupInstruction(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	return Instruction::DUP1 <= _item.instruction() && _item.instruction() <= Instruction::DUP16;
}

bool SemanticInformation::isSwapInstruction(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	return Instruction::SWAP1 <= _item.instruction() && _item.instruction() <= Instruction::SWAP16;
}

AssemblyItems CSECodeGenerator::generateCode(
	map<int, ExpressionClasses::Id> const& _initialStack,
	map<int, ExpressionClasses::Id> const& _targetStackContents
)
{
	m_stack = _initialStack;
	for (auto const& item: m_stack)
		if (!m_classPositions.count(item.second))
			m_classPositions[item.second] = item.first;

	// @todo: provide information about the positions of copies of class elements

	// generate the dependency graph
	for (auto const& targetItem: _targetStackContents)
	{
		m_finalClasses.insert(targetItem.second);
		addDependencies(targetItem.second);
	}

	// generate the actual elements
	for (auto const& targetItem: _targetStackContents)
	{
		removeStackTopIfPossible();
		int position = generateClassElement(targetItem.second);
		if (position == targetItem.first)
			continue;
		if (position < targetItem.first)
			// it is already at its target, we need another copy
			appendDup(position);
		else
			appendSwapOrRemove(position);
		appendSwapOrRemove(targetItem.first);
	}

	// remove surplus elements
	while (removeStackTopIfPossible())
	{
		// no-op
	}

	// check validity
	int finalHeight = 0;
	if (!_targetStackContents.empty())
		// have target stack, so its height should be the final height
		finalHeight = (--_targetStackContents.end())->first;
	else if (!_initialStack.empty())
		// no target stack, only erase the initial stack
		finalHeight = _initialStack.begin()->first - 1;
	else
		// neither initial no target stack, no change in height
		finalHeight = 0;
	assertThrow(finalHeight == m_stackHeight, OptimizerException, "Incorrect final stack height.");

	return m_generatedItems;
}

void CSECodeGenerator::addDependencies(ExpressionClasses::Id _c)
{
	if (m_neededBy.count(_c))
		return;
	for (ExpressionClasses::Id argument: m_expressionClasses.representative(_c).arguments)
	{
		addDependencies(argument);
		m_neededBy.insert(make_pair(argument, _c));
	}
}

int CSECodeGenerator::generateClassElement(ExpressionClasses::Id _c)
{
	if (m_classPositions.count(_c))
	{
		assertThrow(
			m_classPositions[_c] != c_invalidPosition,
			OptimizerException,
			"Element already removed but still needed."
		);
		return m_classPositions[_c];
	}
	ExpressionClasses::Ids const& arguments = m_expressionClasses.representative(_c).arguments;
	for (ExpressionClasses::Id arg: boost::adaptors::reverse(arguments))
		generateClassElement(arg);

	// The arguments are somewhere on the stack now, so it remains to move them at the correct place.
	// This is quite difficult as sometimes, the values also have to removed in this process
	// (if canBeRemoved() returns true) and the two arguments can be equal. For now, this is
	// implemented for every single case for combinations of up to two arguments manually.
	if (arguments.size() == 1)
	{
		if (canBeRemoved(arguments[0], _c))
			appendSwapOrRemove(generateClassElement(arguments[0]));
		else
			appendDup(generateClassElement(arguments[0]));
	}
	else if (arguments.size() == 2)
	{
		if (canBeRemoved(arguments[1], _c))
		{
			appendSwapOrRemove(generateClassElement(arguments[1]));
			if (arguments[0] == arguments[1])
				appendDup(m_stackHeight);
			else if (canBeRemoved(arguments[0], _c))
			{
				appendSwapOrRemove(m_stackHeight - 1);
				appendSwapOrRemove(generateClassElement(arguments[0]));
			}
			else
				appendDup(generateClassElement(arguments[0]));
		}
		else
		{
			if (arguments[0] == arguments[1])
			{
				appendDup(generateClassElement(arguments[0]));
				appendDup(m_stackHeight);
			}
			else if (canBeRemoved(arguments[0], _c))
			{
				appendSwapOrRemove(generateClassElement(arguments[0]));
				appendDup(generateClassElement(arguments[1]));
				appendSwapOrRemove(m_stackHeight - 1);
			}
			else
			{
				appendDup(generateClassElement(arguments[1]));
				appendDup(generateClassElement(arguments[0]));
			}
		}
	}
	else
		assertThrow(
			arguments.size() <= 2,
			OptimizerException,
			"Opcodes with more than two arguments not implemented yet."
		);
	for (size_t i = 0; i < arguments.size(); ++i)
		assertThrow(m_stack[m_stackHeight - i] == arguments[i], OptimizerException, "Expected arguments not present." );

	AssemblyItem const& item = *m_expressionClasses.representative(_c).item;
	while (SemanticInformation::isCommutativeOperation(item) &&
			!m_generatedItems.empty() &&
			m_generatedItems.back() == AssemblyItem(Instruction::SWAP1))
		// this will not append a swap but remove the one that is already there
		appendSwapOrRemove(m_stackHeight - 1);
	for (auto arg: arguments)
		if (canBeRemoved(arg, _c))
			m_classPositions[arg] = c_invalidPosition;
	for (size_t i = 0; i < arguments.size(); ++i)
		m_stack.erase(m_stackHeight - i);
	appendItem(*m_expressionClasses.representative(_c).item);
	m_stack[m_stackHeight] = _c;
	return m_classPositions[_c] = m_stackHeight;
}

bool CSECodeGenerator::canBeRemoved(ExpressionClasses::Id _element, ExpressionClasses::Id _result)
{
	// Returns false if _element is finally needed or is needed by a class that has not been
	// computed yet. Note that m_classPositions also includes classes that were deleted in the meantime.
	if (m_finalClasses.count(_element))
		return false;

	auto range = m_neededBy.equal_range(_element);
	for (auto it = range.first; it != range.second; ++it)
		if (it->second != _result && !m_classPositions.count(it->second))
			return false;
	return true;
}

bool CSECodeGenerator::removeStackTopIfPossible()
{
	if (m_stack.empty())
		return false;
	assertThrow(m_stack.count(m_stackHeight), OptimizerException, "");
	ExpressionClasses::Id top = m_stack[m_stackHeight];
	if (!canBeRemoved(top))
		return false;
	m_generatedItems.push_back(AssemblyItem(Instruction::POP));
	m_stack.erase(m_stackHeight);
	m_stackHeight--;
	return true;
}

void CSECodeGenerator::appendDup(int _fromPosition)
{
	int nr = 1 + m_stackHeight - _fromPosition;
	assertThrow(1 <= nr && nr <= 16, OptimizerException, "Stack too deep.");
	m_generatedItems.push_back(AssemblyItem(dupInstruction(nr)));
	m_stackHeight++;
	m_stack[m_stackHeight] = m_stack[_fromPosition];
}

void CSECodeGenerator::appendSwapOrRemove(int _fromPosition)
{
	if (_fromPosition == m_stackHeight)
		return;
	int nr = m_stackHeight - _fromPosition;
	assertThrow(1 <= nr && nr <= 16, OptimizerException, "Stack too deep.");
	m_generatedItems.push_back(AssemblyItem(swapInstruction(nr)));
	// The value of a class can be present in multiple locations on the stack. We only update the
	// "canonical" one that is tracked by m_classPositions
	if (m_classPositions[m_stack[m_stackHeight]] == m_stackHeight)
		m_classPositions[m_stack[m_stackHeight]] = _fromPosition;
	if (m_classPositions[m_stack[_fromPosition]] == _fromPosition)
		m_classPositions[m_stack[_fromPosition]] = m_stackHeight;
	swap(m_stack[m_stackHeight], m_stack[_fromPosition]);
	if (m_generatedItems.size() >= 2 &&
		SemanticInformation::isSwapInstruction(m_generatedItems.back()) &&
		*(m_generatedItems.end() - 2) == m_generatedItems.back())
	{
		m_generatedItems.pop_back();
		m_generatedItems.pop_back();
	}
}

void CSECodeGenerator::appendItem(AssemblyItem const& _item)
{
	m_generatedItems.push_back(_item);
	m_stackHeight += _item.deposit();
}
