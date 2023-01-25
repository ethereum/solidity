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
 * Adaptor between AbstractAssembly and libevmasm.
 */

#include <libyul/backends/evm/EthAssemblyAdapter.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/Exceptions.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/Instruction.h>

#include <liblangutil/SourceLocation.h>

#include <memory>
#include <functional>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

EthAssemblyAdapter::EthAssemblyAdapter(evmasm::Assembly& _assembly):
	m_assembly(_assembly)
{
}

void EthAssemblyAdapter::setSourceLocation(SourceLocation const& _location)
{
	m_assembly.setSourceLocation(_location);
}

int EthAssemblyAdapter::stackHeight() const
{
	return m_assembly.deposit();
}

void EthAssemblyAdapter::setStackHeight(int height)
{
	m_assembly.setDeposit(height);
}

void EthAssemblyAdapter::appendInstruction(evmasm::Instruction _instruction)
{
	m_assembly.append(_instruction);
}

void EthAssemblyAdapter::appendConstant(u256 const& _constant)
{
	m_assembly.append(_constant);
}

void EthAssemblyAdapter::appendLabel(LabelID _labelId)
{
	m_assembly.append(evmasm::AssemblyItem(evmasm::Tag, _labelId));
}

void EthAssemblyAdapter::appendLabelReference(LabelID _labelId)
{
	m_assembly.append(evmasm::AssemblyItem(evmasm::PushTag, _labelId));
}

size_t EthAssemblyAdapter::newLabelId()
{
	return assemblyTagToIdentifier(m_assembly.newTag());
}

size_t EthAssemblyAdapter::namedLabel(std::string const& _name, size_t _params, size_t _returns, std::optional<size_t> _sourceID)
{
	return assemblyTagToIdentifier(m_assembly.namedTag(_name, _params, _returns, _sourceID));
}

void EthAssemblyAdapter::appendLinkerSymbol(std::string const& _linkerSymbol)
{
	m_assembly.appendLibraryAddress(_linkerSymbol);
}

void EthAssemblyAdapter::appendVerbatim(bytes _data, size_t _arguments, size_t _returnVariables)
{
	m_assembly.appendVerbatim(std::move(_data), _arguments, _returnVariables);
}

void EthAssemblyAdapter::appendJump(int _stackDiffAfter, JumpType _jumpType)
{
	appendJumpInstruction(evmasm::Instruction::JUMP, _jumpType);
	m_assembly.adjustDeposit(_stackDiffAfter);
}

void EthAssemblyAdapter::appendJumpTo(LabelID _labelId, int _stackDiffAfter, JumpType _jumpType)
{
	appendLabelReference(_labelId);
	appendJump(_stackDiffAfter, _jumpType);
}

void EthAssemblyAdapter::appendJumpToIf(LabelID _labelId, JumpType _jumpType)
{
	appendLabelReference(_labelId);
	appendJumpInstruction(evmasm::Instruction::JUMPI, _jumpType);
}

void EthAssemblyAdapter::appendAssemblySize()
{
	m_assembly.appendProgramSize();
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> EthAssemblyAdapter::createSubAssembly(bool _creation, string _name)
{
	shared_ptr<evmasm::Assembly> assembly{make_shared<evmasm::Assembly>(m_assembly.evmVersion(), _creation, std::move(_name))};
	auto sub = m_assembly.newSub(assembly);
	return {make_shared<EthAssemblyAdapter>(*assembly), static_cast<size_t>(sub.data())};
}

void EthAssemblyAdapter::appendDataOffset(vector<AbstractAssembly::SubID> const& _subPath)
{
	if (auto it = m_dataHashBySubId.find(_subPath[0]); it != m_dataHashBySubId.end())
	{
		yulAssert(_subPath.size() == 1, "");
		m_assembly << evmasm::AssemblyItem(evmasm::PushData, it->second);
		return;
	}

	m_assembly.pushSubroutineOffset(m_assembly.encodeSubPath(_subPath));
}

void EthAssemblyAdapter::appendDataSize(vector<AbstractAssembly::SubID> const& _subPath)
{
	if (auto it = m_dataHashBySubId.find(_subPath[0]); it != m_dataHashBySubId.end())
	{
		yulAssert(_subPath.size() == 1, "");
		m_assembly << u256(m_assembly.data(h256(it->second)).size());
		return;
	}

	m_assembly.pushSubroutineSize(m_assembly.encodeSubPath(_subPath));
}

AbstractAssembly::SubID EthAssemblyAdapter::appendData(bytes const& _data)
{
	evmasm::AssemblyItem pushData = m_assembly.newData(_data);
	SubID subID = m_nextDataCounter++;
	m_dataHashBySubId[subID] = pushData.data();
	return subID;
}

void EthAssemblyAdapter::appendToAuxiliaryData(bytes const& _data)
{
	m_assembly.appendToAuxiliaryData(_data);
}

void EthAssemblyAdapter::appendImmutable(std::string const& _identifier)
{
	m_assembly.appendImmutable(_identifier);
}

void EthAssemblyAdapter::appendImmutableAssignment(std::string const& _identifier)
{
	m_assembly.appendImmutableAssignment(_identifier);
}

void EthAssemblyAdapter::markAsInvalid()
{
	m_assembly.markAsInvalid();
}

EthAssemblyAdapter::LabelID EthAssemblyAdapter::assemblyTagToIdentifier(evmasm::AssemblyItem const& _tag)
{
	u256 id = _tag.data();
	yulAssert(id <= std::numeric_limits<LabelID>::max(), "Tag id too large.");
	return LabelID(id);
}

void EthAssemblyAdapter::appendJumpInstruction(evmasm::Instruction _instruction, JumpType _jumpType)
{
	yulAssert(_instruction == evmasm::Instruction::JUMP || _instruction == evmasm::Instruction::JUMPI, "");
	evmasm::AssemblyItem jump(_instruction);
	switch (_jumpType)
	{
	case JumpType::Ordinary:
		yulAssert(jump.getJumpType() == evmasm::AssemblyItem::JumpType::Ordinary, "");
		break;
	case JumpType::IntoFunction:
		jump.setJumpType(evmasm::AssemblyItem::JumpType::IntoFunction);
		break;
	case JumpType::OutOfFunction:
		jump.setJumpType(evmasm::AssemblyItem::JumpType::OutOfFunction);
		break;
	}
	m_assembly.append(std::move(jump));
}
