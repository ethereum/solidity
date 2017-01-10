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
 * @file CommonSubexpressionEliminator.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Optimizer step for common subexpression elimination and stack reorganisation.
 */

#include <functional>
#include <boost/range/adaptor/reversed.hpp>
#include <libdevcore/SHA3.h>
#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/AssemblyItem.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

vector<AssemblyItem> CommonSubexpressionEliminator::getOptimizedItems()
{
	optimizeBreakingItem();

	KnownState nextInitialState = m_state;
	if (m_breakingItem)
		nextInitialState.feedItem(*m_breakingItem);
	KnownState nextState = nextInitialState;

	ScopeGuard reset([&]()
	{
		m_breakingItem = nullptr;
		m_storeOperations.clear();
		m_initialState = move(nextInitialState);
		m_state = move(nextState);
	});

	map<int, Id> initialStackContents;
	map<int, Id> targetStackContents;
	int minHeight = m_state.stackHeight() + 1;
	if (!m_state.stackElements().empty())
		minHeight = min(minHeight, m_state.stackElements().begin()->first);
	for (int height = minHeight; height <= m_initialState.stackHeight(); ++height)
		initialStackContents[height] = m_initialState.stackElement(height, SourceLocation());
	for (int height = minHeight; height <= m_state.stackHeight(); ++height)
		targetStackContents[height] = m_state.stackElement(height, SourceLocation());

	AssemblyItems items = CSECodeGenerator(m_state.expressionClasses(), m_storeOperations).generateCode(
		m_initialState.sequenceNumber(),
		m_initialState.stackHeight(),
		initialStackContents,
		targetStackContents
	);
	if (m_breakingItem)
		items.push_back(*m_breakingItem);

	return items;
}

void CommonSubexpressionEliminator::feedItem(AssemblyItem const& _item, bool _copyItem)
{
	StoreOperation op = m_state.feedItem(_item, _copyItem);
	if (op.isValid())
		m_storeOperations.push_back(op);
}

void CommonSubexpressionEliminator::optimizeBreakingItem()
{
	if (!m_breakingItem)
		return;

	ExpressionClasses& classes = m_state.expressionClasses();
	SourceLocation const& itemLocation = m_breakingItem->location();
	if (*m_breakingItem == AssemblyItem(Instruction::JUMPI))
	{
		AssemblyItem::JumpType jumpType = m_breakingItem->getJumpType();

		Id condition = m_state.stackElement(m_state.stackHeight() - 1, itemLocation);
		if (classes.knownNonZero(condition))
		{
			feedItem(AssemblyItem(Instruction::SWAP1, itemLocation), true);
			feedItem(AssemblyItem(Instruction::POP, itemLocation), true);

			AssemblyItem item(Instruction::JUMP, itemLocation);
			item.setJumpType(jumpType);
			m_breakingItem = classes.storeItem(item);
		}
		else if (classes.knownZero(condition))
		{
			AssemblyItem it(Instruction::POP, itemLocation);
			feedItem(it, true);
			feedItem(it, true);
			m_breakingItem = nullptr;
		}
	}
	else if (*m_breakingItem == AssemblyItem(Instruction::RETURN))
	{
		Id size = m_state.stackElement(m_state.stackHeight() - 1, itemLocation);
		if (classes.knownZero(size))
		{
			feedItem(AssemblyItem(Instruction::POP, itemLocation), true);
			feedItem(AssemblyItem(Instruction::POP, itemLocation), true);
			AssemblyItem item(Instruction::STOP, itemLocation);
			m_breakingItem = classes.storeItem(item);
		}
	}
}

CSECodeGenerator::CSECodeGenerator(
	ExpressionClasses& _expressionClasses,
	vector<CSECodeGenerator::StoreOperation> const& _storeOperations
):
	m_expressionClasses(_expressionClasses)
{
	for (auto const& store: _storeOperations)
		m_storeOperations[make_pair(store.target, store.slot)].push_back(store);
}

AssemblyItems CSECodeGenerator::generateCode(
	unsigned _initialSequenceNumber,
	int _initialStackHeight,
	map<int, Id> const& _initialStack,
	map<int, Id> const& _targetStackContents
)
{
	m_stackHeight = _initialStackHeight;
	m_stack = _initialStack;
	m_targetStack = _targetStackContents;
	for (auto const& item: m_stack)
		m_classPositions[item.second].insert(item.first);

	// generate the dependency graph starting from final storage and memory writes and target stack contents
	for (auto const& p: m_storeOperations)
		addDependencies(p.second.back().expression);
	for (auto const& targetItem: m_targetStack)
	{
		m_finalClasses.insert(targetItem.second);
		addDependencies(targetItem.second);
	}

	// store all needed sequenced expressions
	set<pair<unsigned, Id>> sequencedExpressions;
	for (auto const& p: m_neededBy)
		for (auto id: {p.first, p.second})
			if (unsigned seqNr = m_expressionClasses.representative(id).sequenceNumber)
			{
				if (seqNr < _initialSequenceNumber)
					// Invalid sequenced operation.
					// @todo quick fix for now. Proper fix needs to choose representative with higher
					// sequence number during dependency analyis.
					BOOST_THROW_EXCEPTION(StackTooDeepException());
				sequencedExpressions.insert(make_pair(seqNr, id));
			}

	// Perform all operations on storage and memory in order, if they are needed.
	for (auto const& seqAndId: sequencedExpressions)
		if (!m_classPositions.count(seqAndId.second))
			generateClassElement(seqAndId.second, true);

	// generate the target stack elements
	for (auto const& targetItem: m_targetStack)
	{
		if (m_stack.count(targetItem.first) && m_stack.at(targetItem.first) == targetItem.second)
			continue; // already there
		generateClassElement(targetItem.second);
		assertThrow(!m_classPositions[targetItem.second].empty(), OptimizerException, "");
		if (m_classPositions[targetItem.second].count(targetItem.first))
			continue;
		SourceLocation sourceLocation;
		if (m_expressionClasses.representative(targetItem.second).item)
			sourceLocation = m_expressionClasses.representative(targetItem.second).item->location();
		int position = classElementPosition(targetItem.second);
		if (position < targetItem.first)
			// it is already at its target, we need another copy
			appendDup(position, sourceLocation);
		else
			appendOrRemoveSwap(position, sourceLocation);
		appendOrRemoveSwap(targetItem.first, sourceLocation);
	}

	// remove surplus elements
	while (removeStackTopIfPossible())
	{
		// no-op
	}

	// check validity
	int finalHeight = 0;
	if (!m_targetStack.empty())
		// have target stack, so its height should be the final height
		finalHeight = (--m_targetStack.end())->first;
	else if (!_initialStack.empty())
		// no target stack, only erase the initial stack
		finalHeight = _initialStack.begin()->first - 1;
	else
		// neither initial no target stack, no change in height
		finalHeight = _initialStackHeight;
	assertThrow(finalHeight == m_stackHeight, OptimizerException, "Incorrect final stack height.");

	return m_generatedItems;
}

void CSECodeGenerator::addDependencies(Id _c)
{
	if (m_classPositions.count(_c))
		return; // it is already on the stack
	if (m_neededBy.count(_c))
		return; // we already computed the dependencies for _c
	ExpressionClasses::Expression expr = m_expressionClasses.representative(_c);
	if (expr.item->type() == UndefinedItem)
		BOOST_THROW_EXCEPTION(
			// If this exception happens, we need to find a different way to generate the
			// compound expression.
			ItemNotAvailableException() << errinfo_comment("Undefined item requested but not available.")
		);
	for (Id argument: expr.arguments)
	{
		addDependencies(argument);
		m_neededBy.insert(make_pair(argument, _c));
	}
	if (expr.item && expr.item->type() == Operation && (
		expr.item->instruction() == Instruction::SLOAD ||
		expr.item->instruction() == Instruction::MLOAD ||
		expr.item->instruction() == Instruction::SHA3
	))
	{
		// this loads an unknown value from storage or memory and thus, in addition to its
		// arguments, depends on all store operations to addresses where we do not know that
		// they are different that occur before this load
		StoreOperation::Target target = expr.item->instruction() == Instruction::SLOAD ?
			StoreOperation::Storage : StoreOperation::Memory;
		Id slotToLoadFrom = expr.arguments.at(0);
		for (auto const& p: m_storeOperations)
		{
			if (p.first.first != target)
				continue;
			Id slot = p.first.second;
			StoreOperations const& storeOps = p.second;
			if (storeOps.front().sequenceNumber > expr.sequenceNumber)
				continue;
			bool knownToBeIndependent = false;
			switch (expr.item->instruction())
			{
			case Instruction::SLOAD:
				knownToBeIndependent = m_expressionClasses.knownToBeDifferent(slot, slotToLoadFrom);
				break;
			case Instruction::MLOAD:
				knownToBeIndependent = m_expressionClasses.knownToBeDifferentBy32(slot, slotToLoadFrom);
				break;
			case Instruction::SHA3:
			{
				Id length = expr.arguments.at(1);
				AssemblyItem offsetInstr(Instruction::SUB, expr.item->location());
				Id offsetToStart = m_expressionClasses.find(offsetInstr, {slot, slotToLoadFrom});
				u256 const* o = m_expressionClasses.knownConstant(offsetToStart);
				u256 const* l = m_expressionClasses.knownConstant(length);
				if (l && *l == 0)
					knownToBeIndependent = true;
				else if (o)
				{
					// We could get problems here if both *o and *l are larger than 2**254
					// but it is probably ok for the optimizer to produce wrong code for such cases
					// which cannot be executed anyway because of the non-payable price.
					if (u2s(*o) <= -32)
						knownToBeIndependent = true;
					else if (l && u2s(*o) >= 0 && *o >= *l)
						knownToBeIndependent = true;
				}
				break;
			}
			default:
				break;
			}
			if (knownToBeIndependent)
				continue;

			// note that store and load never have the same sequence number
			Id latestStore = storeOps.front().expression;
			for (auto it = ++storeOps.begin(); it != storeOps.end(); ++it)
				if (it->sequenceNumber < expr.sequenceNumber)
					latestStore = it->expression;
			addDependencies(latestStore);
			m_neededBy.insert(make_pair(latestStore, _c));
		}
	}
}

void CSECodeGenerator::generateClassElement(Id _c, bool _allowSequenced)
{
	for (auto it: m_classPositions)
		for (auto p: it.second)
			if (p > m_stackHeight)
			{
				assertThrow(false, OptimizerException, "");
			}
	// do some cleanup
	removeStackTopIfPossible();

	if (m_classPositions.count(_c))
	{
		assertThrow(
			!m_classPositions[_c].empty(),
			OptimizerException,
			"Element already removed but still needed."
		);
		return;
	}
	ExpressionClasses::Expression const& expr = m_expressionClasses.representative(_c);
	assertThrow(
		_allowSequenced || expr.sequenceNumber == 0,
		OptimizerException,
		"Sequence constrained operation requested out of sequence."
	);
	assertThrow(expr.item, OptimizerException, "Non-generated expression without item.");
	assertThrow(
		expr.item->type() != UndefinedItem,
		OptimizerException,
		"Undefined item requested but not available."
	);
	vector<Id> const& arguments = expr.arguments;
	for (Id arg: boost::adaptors::reverse(arguments))
		generateClassElement(arg);

	SourceLocation const& itemLocation = expr.item->location();
	// The arguments are somewhere on the stack now, so it remains to move them at the correct place.
	// This is quite difficult as sometimes, the values also have to removed in this process
	// (if canBeRemoved() returns true) and the two arguments can be equal. For now, this is
	// implemented for every single case for combinations of up to two arguments manually.
	if (arguments.size() == 1)
	{
		if (canBeRemoved(arguments[0], _c))
			appendOrRemoveSwap(classElementPosition(arguments[0]), itemLocation);
		else
			appendDup(classElementPosition(arguments[0]), itemLocation);
	}
	else if (arguments.size() == 2)
	{
		if (canBeRemoved(arguments[1], _c))
		{
			appendOrRemoveSwap(classElementPosition(arguments[1]), itemLocation);
			if (arguments[0] == arguments[1])
				appendDup(m_stackHeight, itemLocation);
			else if (canBeRemoved(arguments[0], _c))
			{
				appendOrRemoveSwap(m_stackHeight - 1, itemLocation);
				appendOrRemoveSwap(classElementPosition(arguments[0]), itemLocation);
			}
			else
				appendDup(classElementPosition(arguments[0]), itemLocation);
		}
		else
		{
			if (arguments[0] == arguments[1])
			{
				appendDup(classElementPosition(arguments[0]), itemLocation);
				appendDup(m_stackHeight, itemLocation);
			}
			else if (canBeRemoved(arguments[0], _c))
			{
				appendOrRemoveSwap(classElementPosition(arguments[0]), itemLocation);
				appendDup(classElementPosition(arguments[1]), itemLocation);
				appendOrRemoveSwap(m_stackHeight - 1, itemLocation);
			}
			else
			{
				appendDup(classElementPosition(arguments[1]), itemLocation);
				appendDup(classElementPosition(arguments[0]), itemLocation);
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

	while (SemanticInformation::isCommutativeOperation(*expr.item) &&
			!m_generatedItems.empty() &&
			m_generatedItems.back() == AssemblyItem(Instruction::SWAP1))
		// this will not append a swap but remove the one that is already there
		appendOrRemoveSwap(m_stackHeight - 1, itemLocation);
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		m_classPositions[m_stack[m_stackHeight - i]].erase(m_stackHeight - i);
		m_stack.erase(m_stackHeight - i);
	}
	appendItem(*expr.item);
	if (expr.item->type() != Operation || instructionInfo(expr.item->instruction()).ret == 1)
	{
		m_stack[m_stackHeight] = _c;
		m_classPositions[_c].insert(m_stackHeight);
	}
	else
	{
		assertThrow(
			instructionInfo(expr.item->instruction()).ret == 0,
			OptimizerException,
			"Invalid number of return values."
		);
		m_classPositions[_c]; // ensure it is created to mark the expression as generated
	}
}

int CSECodeGenerator::classElementPosition(Id _id) const
{
	assertThrow(
		m_classPositions.count(_id) && !m_classPositions.at(_id).empty(),
		OptimizerException,
		"Element requested but is not present."
	);
	return *max_element(m_classPositions.at(_id).begin(), m_classPositions.at(_id).end());
}

bool CSECodeGenerator::canBeRemoved(Id _element, Id _result, int _fromPosition)
{
	// Default for _fromPosition is the canonical position of the element.
	if (_fromPosition == c_invalidPosition)
		_fromPosition = classElementPosition(_element);

	bool haveCopy = m_classPositions.at(_element).size() > 1;
	if (m_finalClasses.count(_element))
		// It is part of the target stack. It can be removed if it is a copy that is not in the target position.
		return haveCopy && (!m_targetStack.count(_fromPosition) || m_targetStack[_fromPosition] != _element);
	else if (!haveCopy)
	{
		// Can be removed unless it is needed by a class that has not been computed yet.
		// Note that m_classPositions also includes classes that were deleted in the meantime.
		auto range = m_neededBy.equal_range(_element);
		for (auto it = range.first; it != range.second; ++it)
			if (it->second != _result && !m_classPositions.count(it->second))
				return false;
	}
	return true;
}

bool CSECodeGenerator::removeStackTopIfPossible()
{
	if (m_stack.empty())
		return false;
	assertThrow(m_stack.count(m_stackHeight) > 0, OptimizerException, "");
	Id top = m_stack[m_stackHeight];
	if (!canBeRemoved(top, Id(-1), m_stackHeight))
		return false;
	m_classPositions[m_stack[m_stackHeight]].erase(m_stackHeight);
	m_stack.erase(m_stackHeight);
	appendItem(AssemblyItem(Instruction::POP));
	return true;
}

void CSECodeGenerator::appendDup(int _fromPosition, SourceLocation const& _location)
{
	assertThrow(_fromPosition != c_invalidPosition, OptimizerException, "");
	int instructionNum = 1 + m_stackHeight - _fromPosition;
	assertThrow(instructionNum <= 16, StackTooDeepException, "Stack too deep, try removing local variables.");
	assertThrow(1 <= instructionNum, OptimizerException, "Invalid stack access.");
	appendItem(AssemblyItem(dupInstruction(instructionNum), _location));
	m_stack[m_stackHeight] = m_stack[_fromPosition];
	m_classPositions[m_stack[m_stackHeight]].insert(m_stackHeight);
}

void CSECodeGenerator::appendOrRemoveSwap(int _fromPosition, SourceLocation const& _location)
{
	assertThrow(_fromPosition != c_invalidPosition, OptimizerException, "");
	if (_fromPosition == m_stackHeight)
		return;
	int instructionNum = m_stackHeight - _fromPosition;
	assertThrow(instructionNum <= 16, StackTooDeepException, "Stack too deep, try removing local variables.");
	assertThrow(1 <= instructionNum, OptimizerException, "Invalid stack access.");
	appendItem(AssemblyItem(swapInstruction(instructionNum), _location));

	if (m_stack[m_stackHeight] != m_stack[_fromPosition])
	{
		m_classPositions[m_stack[m_stackHeight]].erase(m_stackHeight);
		m_classPositions[m_stack[m_stackHeight]].insert(_fromPosition);
		m_classPositions[m_stack[_fromPosition]].erase(_fromPosition);
		m_classPositions[m_stack[_fromPosition]].insert(m_stackHeight);
		swap(m_stack[m_stackHeight], m_stack[_fromPosition]);
	}
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
