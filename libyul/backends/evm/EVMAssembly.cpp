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
// SPDX-License-Identifier: GPL-3.0
/**
 * Assembly interface for EVM and EVM1.5.
 */

#include <libyul/backends/evm/EVMAssembly.h>
#include <libyul/Exceptions.h>

#include <libevmasm/Instruction.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

namespace
{
/// Size of labels in bytes. Four-byte labels are required by some EVM1.5 instructions.
size_t constexpr labelReferenceSize = 4;

size_t constexpr assemblySizeReferenceSize = 4;
}


void EVMAssembly::setSourceLocation(langutil::SourceLocation const&)
{
	// Ignored for now;
}

void EVMAssembly::appendInstruction(evmasm::Instruction _instr)
{
	m_bytecode.push_back(uint8_t(_instr));
	m_stackHeight += instructionInfo(_instr).ret - instructionInfo(_instr).args;
}

void EVMAssembly::appendConstant(u256 const& _constant)
{
	bytes data = toCompactBigEndian(_constant, 1);
	appendInstruction(evmasm::pushInstruction(static_cast<unsigned>(data.size())));
	m_bytecode += data;
}

void EVMAssembly::appendLabel(LabelID _labelId)
{
	setLabelToCurrentPosition(_labelId);
	appendInstruction(evmasm::Instruction::JUMPDEST);
}

void EVMAssembly::appendLabelReference(LabelID _labelId)
{
	// @TODO we now always use labelReferenceSize for all labels, it could be shortened
	// for some of them.
	appendInstruction(evmasm::pushInstruction(labelReferenceSize));
	m_labelReferences[m_bytecode.size()] = _labelId;
	m_bytecode += bytes(labelReferenceSize);
}

EVMAssembly::LabelID EVMAssembly::newLabelId()
{
	m_labelPositions[m_nextLabelId] = numeric_limits<size_t>::max();
	return m_nextLabelId++;
}

AbstractAssembly::LabelID EVMAssembly::namedLabel(string const& _name, size_t, size_t, std::optional<size_t>)
{
	yulAssert(!_name.empty(), "");
	if (!m_namedLabels.count(_name))
		m_namedLabels[_name] = newLabelId();
	return m_namedLabels[_name];
}

void EVMAssembly::appendLinkerSymbol(string const&)
{
	yulAssert(false, "Linker symbols not yet implemented.");
}

void EVMAssembly::appendJump(int _stackDiffAfter, JumpType)
{
	appendInstruction(evmasm::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void EVMAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter, JumpType _jumpType)
{
	appendLabelReference(_labelId);
	appendJump(_stackDiffAfter, _jumpType);
}

void EVMAssembly::appendJumpToIf(LabelID _labelId, JumpType)
{
	appendLabelReference(_labelId);
	appendInstruction(evmasm::Instruction::JUMPI);
}

evmasm::LinkerObject EVMAssembly::finalize()
{
	yulAssert(!m_invalid, "Attempted to finalize invalid assembly object.");
	size_t bytecodeSize = m_bytecode.size();
	for (auto const& ref: m_assemblySizePositions)
		updateReference(ref, assemblySizeReferenceSize, u256(bytecodeSize));

	for (auto const& ref: m_labelReferences)
	{
		size_t referencePos = ref.first;
		yulAssert(m_labelPositions.count(ref.second), "");
		size_t labelPos = m_labelPositions.at(ref.second);
		yulAssert(labelPos != numeric_limits<size_t>::max(), "Undefined but allocated label used.");
		updateReference(referencePos, labelReferenceSize, u256(labelPos));
	}

	evmasm::LinkerObject obj;
	obj.bytecode = m_bytecode;
	return obj;
}

void EVMAssembly::setLabelToCurrentPosition(LabelID _labelId)
{
	yulAssert(m_labelPositions.count(_labelId), "Label not found.");
	yulAssert(m_labelPositions[_labelId] == numeric_limits<size_t>::max(), "Label already set.");
	m_labelPositions[_labelId] = m_bytecode.size();
}

void EVMAssembly::appendLabelReferenceInternal(LabelID _labelId)
{
	m_labelReferences[m_bytecode.size()] = _labelId;
	m_bytecode += bytes(labelReferenceSize);
}

void EVMAssembly::appendAssemblySize()
{
	appendInstruction(evmasm::pushInstruction(assemblySizeReferenceSize));
	m_assemblySizePositions.push_back(m_bytecode.size());
	m_bytecode += bytes(assemblySizeReferenceSize);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> EVMAssembly::createSubAssembly(string)
{
	yulAssert(false, "Sub assemblies not implemented.");
	return {};
}

void EVMAssembly::appendDataOffset(vector<AbstractAssembly::SubID> const&)
{
	yulAssert(false, "Data not implemented.");
}

void EVMAssembly::appendDataSize(vector<AbstractAssembly::SubID> const&)
{
	yulAssert(false, "Data not implemented.");
}

AbstractAssembly::SubID EVMAssembly::appendData(bytes const&)
{
	yulAssert(false, "Data not implemented.");
}

void EVMAssembly::appendImmutable(std::string const&)
{
	yulAssert(false, "loadimmutable not implemented.");
}

void EVMAssembly::appendImmutableAssignment(std::string const&)
{
	yulAssert(false, "setimmutable not implemented.");
}

void EVMAssembly::updateReference(size_t pos, size_t size, u256 value)
{
	yulAssert(m_bytecode.size() >= size && pos <= m_bytecode.size() - size, "");
	yulAssert(value < (u256(1) << (8 * size)), "");
	for (size_t i = 0; i < size; i++)
		m_bytecode[pos + i] = uint8_t((value >> (8 * (size - i - 1))) & 0xff);
}
