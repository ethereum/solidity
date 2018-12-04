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
 * Adaptor between the abstract assembly and eth assembly.
 */

#include <libsolidity/codegen/AsmCodeGen.h>

#include <libyul/AsmData.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/EVMCodeTransform.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/Instruction.h>

#include <liblangutil/SourceLocation.h>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;
using namespace dev::solidity;

EthAssemblyAdapter::EthAssemblyAdapter(eth::Assembly& _assembly):
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

void EthAssemblyAdapter::appendInstruction(solidity::Instruction _instruction)
{
	m_assembly.append(_instruction);
}

void EthAssemblyAdapter::appendConstant(u256 const& _constant)
{
	m_assembly.append(_constant);
}

void EthAssemblyAdapter::appendLabel(LabelID _labelId)
{
	m_assembly.append(eth::AssemblyItem(eth::Tag, _labelId));
}

void EthAssemblyAdapter::appendLabelReference(LabelID _labelId)
{
	m_assembly.append(eth::AssemblyItem(eth::PushTag, _labelId));
}

size_t EthAssemblyAdapter::newLabelId()
{
	return assemblyTagToIdentifier(m_assembly.newTag());
}

size_t EthAssemblyAdapter::namedLabel(std::string const& _name)
{
	return assemblyTagToIdentifier(m_assembly.namedTag(_name));
}

void EthAssemblyAdapter::appendLinkerSymbol(std::string const& _linkerSymbol)
{
	m_assembly.appendLibraryAddress(_linkerSymbol);
}

void EthAssemblyAdapter::appendJump(int _stackDiffAfter)
{
	appendInstruction(solidity::Instruction::JUMP);
	m_assembly.adjustDeposit(_stackDiffAfter);
}

void EthAssemblyAdapter::appendJumpTo(LabelID _labelId, int _stackDiffAfter)
{
	appendLabelReference(_labelId);
	appendJump(_stackDiffAfter);
}

void EthAssemblyAdapter::appendJumpToIf(LabelID _labelId)
{
	appendLabelReference(_labelId);
	appendInstruction(solidity::Instruction::JUMPI);
}

void EthAssemblyAdapter::appendBeginsub(LabelID, int)
{
	// TODO we could emulate that, though
	solAssert(false, "BEGINSUB not implemented for EVM 1.0");
}

void EthAssemblyAdapter::appendJumpsub(LabelID, int, int)
{
	// TODO we could emulate that, though
	solAssert(false, "JUMPSUB not implemented for EVM 1.0");
}

void EthAssemblyAdapter::appendReturnsub(int, int)
{
	// TODO we could emulate that, though
	solAssert(false, "RETURNSUB not implemented for EVM 1.0");
}

void EthAssemblyAdapter::appendAssemblySize()
{
	m_assembly.appendProgramSize();
}

EthAssemblyAdapter::LabelID EthAssemblyAdapter::assemblyTagToIdentifier(eth::AssemblyItem const& _tag)
{
	u256 id = _tag.data();
	solAssert(id <= std::numeric_limits<LabelID>::max(), "Tag id too large.");
	return LabelID(id);
}

void CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	eth::Assembly& _assembly,
	ExternalIdentifierAccess const& _identifierAccess,
	bool _useNamedLabelsForFunctions
)
{
	EthAssemblyAdapter assemblyAdapter(_assembly);
	CodeTransform(
		assemblyAdapter,
		_analysisInfo,
		false,
		false,
		_identifierAccess,
		_useNamedLabelsForFunctions
	)(_parsedData);
}
