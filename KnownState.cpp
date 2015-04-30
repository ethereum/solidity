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
 * @file KnownState.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Contains knowledge about the state of the virtual machine at a specific instruction.
 */

#include "KnownState.h"
#include <functional>
#include <libdevcrypto/SHA3.h>
#include <libevmasm/AssemblyItem.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

ostream& KnownState::stream(
	ostream& _out,
	map<int, Id> _initialStack,
	map<int, Id> _targetStack
) const
{
	auto streamExpressionClass = [this](ostream& _out, Id _id)
	{
		auto const& expr = m_expressionClasses->representative(_id);
		_out << "  " << dec << _id << ": " << *expr.item;
		if (expr.sequenceNumber)
			_out << "@" << dec << expr.sequenceNumber;
		_out << "(";
		for (Id arg: expr.arguments)
			_out << dec << arg << ",";
		_out << ")" << endl;
	};

	_out << "Optimizer analysis:" << endl;
	_out << "Final stack height: " << dec << m_stackHeight << endl;
	_out << "Equivalence classes: " << endl;
	for (Id eqClass = 0; eqClass < m_expressionClasses->size(); ++eqClass)
		streamExpressionClass(_out, eqClass);

	_out << "Initial stack: " << endl;
	for (auto const& it: _initialStack)
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

KnownState::StoreOperation KnownState::feedItem(AssemblyItem const& _item, bool _copyItem)
{
	StoreOperation op;
	if (_item.type() != Operation)
	{
		assertThrow(_item.deposit() == 1, InvalidDeposit, "");
		setStackElement(++m_stackHeight, m_expressionClasses->find(_item, {}, _copyItem));
	}
	else
	{
		Instruction instruction = _item.instruction();
		InstructionInfo info = instructionInfo(instruction);
		if (SemanticInformation::isDupInstruction(_item))
			setStackElement(
				m_stackHeight + 1,
				stackElement(
					m_stackHeight - int(instruction) + int(Instruction::DUP1),
					_item.getLocation()
				)
			);
		else if (SemanticInformation::isSwapInstruction(_item))
			swapStackElements(
				m_stackHeight,
				m_stackHeight - 1 - int(instruction) + int(Instruction::SWAP1),
				_item.getLocation()
			);
		else if (instruction != Instruction::POP)
		{
			vector<Id> arguments(info.args);
			for (int i = 0; i < info.args; ++i)
				arguments[i] = stackElement(m_stackHeight - i, _item.getLocation());
			if (_item.instruction() == Instruction::SSTORE)
				op = storeInStorage(arguments[0], arguments[1], _item.getLocation());
			else if (_item.instruction() == Instruction::SLOAD)
				setStackElement(
					m_stackHeight + _item.deposit(),
					loadFromStorage(arguments[0], _item.getLocation())
				);
			else if (_item.instruction() == Instruction::MSTORE)
				op = storeInMemory(arguments[0], arguments[1], _item.getLocation());
			else if (_item.instruction() == Instruction::MLOAD)
				setStackElement(
					m_stackHeight + _item.deposit(),
					loadFromMemory(arguments[0], _item.getLocation())
				);
			else if (_item.instruction() == Instruction::SHA3)
				setStackElement(
					m_stackHeight + _item.deposit(),
					applySha3(arguments.at(0), arguments.at(1), _item.getLocation())
				);
			else
				setStackElement(
					m_stackHeight + _item.deposit(),
					m_expressionClasses->find(_item, arguments, _copyItem)
				);
		}
		m_stackHeight += _item.deposit();
	}
	return op;
}

ExpressionClasses::Id KnownState::stackElement(int _stackHeight, SourceLocation const& _location)
{
	if (m_stackElements.count(_stackHeight))
		return m_stackElements.at(_stackHeight);
	// Stack element not found (not assigned yet), create new equivalence class.
	return m_stackElements[_stackHeight] = m_expressionClasses->newId();
}

ExpressionClasses::Id KnownState::initialStackElement(
	int _stackHeight,
	SourceLocation const& _location
)
{
	assertThrow(_stackHeight <= 0, OptimizerException, "Initial stack element of positive height requested.");
	assertThrow(_stackHeight > -16, StackTooDeepException, "");
	// This is a special assembly item that refers to elements pre-existing on the initial stack.
	return m_expressionClasses->find(AssemblyItem(dupInstruction(1 - _stackHeight), _location));
}

void KnownState::setStackElement(int _stackHeight, Id _class)
{
	m_stackElements[_stackHeight] = _class;
}

void KnownState::swapStackElements(
	int _stackHeightA,
	int _stackHeightB,
	SourceLocation const& _location
)
{
	assertThrow(_stackHeightA != _stackHeightB, OptimizerException, "Swap on same stack elements.");
	// ensure they are created
	stackElement(_stackHeightA, _location);
	stackElement(_stackHeightB, _location);

	swap(m_stackElements[_stackHeightA], m_stackElements[_stackHeightB]);
}

KnownState::StoreOperation KnownState::storeInStorage(
	Id _slot,
	Id _value,
	SourceLocation const& _location)
{
	if (m_storageContent.count(_slot) && m_storageContent[_slot] == _value)
		// do not execute the storage if we know that the value is already there
		return StoreOperation();
	m_sequenceNumber++;
	decltype(m_storageContent) storageContents;
	// Copy over all values (i.e. retain knowledge about them) where we know that this store
	// operation will not destroy the knowledge. Specifically, we copy storage locations we know
	// are different from _slot or locations where we know that the stored value is equal to _value.
	for (auto const& storageItem: m_storageContent)
		if (m_expressionClasses->knownToBeDifferent(storageItem.first, _slot) || storageItem.second == _value)
			storageContents.insert(storageItem);
	m_storageContent = move(storageContents);

	AssemblyItem item(Instruction::SSTORE, _location);
	Id id = m_expressionClasses->find(item, {_slot, _value}, true, m_sequenceNumber);
	StoreOperation operation(StoreOperation::Storage, _slot, m_sequenceNumber, id);
	m_storageContent[_slot] = _value;
	// increment a second time so that we get unique sequence numbers for writes
	m_sequenceNumber++;

	return operation;
}

ExpressionClasses::Id KnownState::loadFromStorage(Id _slot, SourceLocation const& _location)
{
	if (m_storageContent.count(_slot))
		return m_storageContent.at(_slot);

	AssemblyItem item(Instruction::SLOAD, _location);
	return m_storageContent[_slot] = m_expressionClasses->find(item, {_slot}, true, m_sequenceNumber);
}

KnownState::StoreOperation KnownState::storeInMemory(Id _slot, Id _value, SourceLocation const& _location)
{
	if (m_memoryContent.count(_slot) && m_memoryContent[_slot] == _value)
		// do not execute the store if we know that the value is already there
		return StoreOperation();
	m_sequenceNumber++;
	decltype(m_memoryContent) memoryContents;
	// copy over values at points where we know that they are different from _slot by at least 32
	for (auto const& memoryItem: m_memoryContent)
		if (m_expressionClasses->knownToBeDifferentBy32(memoryItem.first, _slot))
			memoryContents.insert(memoryItem);
	m_memoryContent = move(memoryContents);

	AssemblyItem item(Instruction::MSTORE, _location);
	Id id = m_expressionClasses->find(item, {_slot, _value}, true, m_sequenceNumber);
	StoreOperation operation(StoreOperation(StoreOperation::Memory, _slot, m_sequenceNumber, id));
	m_memoryContent[_slot] = _value;
	// increment a second time so that we get unique sequence numbers for writes
	m_sequenceNumber++;
	return operation;
}

ExpressionClasses::Id KnownState::loadFromMemory(Id _slot, SourceLocation const& _location)
{
	if (m_memoryContent.count(_slot))
		return m_memoryContent.at(_slot);

	AssemblyItem item(Instruction::MLOAD, _location);
	return m_memoryContent[_slot] = m_expressionClasses->find(item, {_slot}, true, m_sequenceNumber);
}

KnownState::Id KnownState::applySha3(
	Id _start,
	Id _length,
	SourceLocation const& _location
)
{
	AssemblyItem sha3Item(Instruction::SHA3, _location);
	// Special logic if length is a short constant, otherwise we cannot tell.
	u256 const* l = m_expressionClasses->knownConstant(_length);
	// unknown or too large length
	if (!l || *l > 128)
		return m_expressionClasses->find(sha3Item, {_start, _length}, true, m_sequenceNumber);

	vector<Id> arguments;
	for (u256 i = 0; i < *l; i += 32)
	{
		Id slot = m_expressionClasses->find(
			AssemblyItem(Instruction::ADD, _location),
			{_start, m_expressionClasses->find(i)}
		);
		arguments.push_back(loadFromMemory(slot, _location));
	}
	if (m_knownSha3Hashes.count(arguments))
		return m_knownSha3Hashes.at(arguments);
	Id v;
	// If all arguments are known constants, compute the sha3 here
	if (all_of(arguments.begin(), arguments.end(), [this](Id _a) { return !!m_expressionClasses->knownConstant(_a); }))
	{
		bytes data;
		for (Id a: arguments)
			data += toBigEndian(*m_expressionClasses->knownConstant(a));
		data.resize(size_t(*l));
		v = m_expressionClasses->find(AssemblyItem(u256(sha3(data)), _location));
	}
	else
		v = m_expressionClasses->find(sha3Item, {_start, _length}, true, m_sequenceNumber);
	return m_knownSha3Hashes[arguments] = v;
}

