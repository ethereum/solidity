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

#include <libyul/backends/evm/AsmCodeGen.h>

#include <libyul/AsmData.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/EVMCodeTransform.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/Instruction.h>

#include <liblangutil/SourceLocation.h>

#include <libdevcore/FixedHash.h>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;

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

void EthAssemblyAdapter::appendInstruction(dev::eth::Instruction _instruction)
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
	appendInstruction(dev::eth::Instruction::JUMP);
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
	appendInstruction(dev::eth::Instruction::JUMPI);
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

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> EthAssemblyAdapter::createSubAssembly()
{
	shared_ptr<eth::Assembly> assembly{make_shared<eth::Assembly>()};
	auto sub = m_assembly.newSub(assembly);
	return {make_shared<EthAssemblyAdapter>(*assembly), size_t(sub.data())};
}

void EthAssemblyAdapter::appendDataOffset(AbstractAssembly::SubID _sub)
{
	auto it = m_dataHashBySubId.find(_sub);
	if (it == m_dataHashBySubId.end())
		m_assembly.pushSubroutineOffset(size_t(_sub));
	else
		m_assembly << eth::AssemblyItem(eth::PushData, it->second);
}

void EthAssemblyAdapter::appendDataSize(AbstractAssembly::SubID _sub)
{
	auto it = m_dataHashBySubId.find(_sub);
	if (it == m_dataHashBySubId.end())
		m_assembly.pushSubroutineSize(size_t(_sub));
	else
		m_assembly << u256(m_assembly.data(h256(it->second)).size());
}

AbstractAssembly::SubID EthAssemblyAdapter::appendData(bytes const& _data)
{
	eth::AssemblyItem pushData = m_assembly.newData(_data);
	SubID subID = m_nextDataCounter++;
	m_dataHashBySubId[subID] = pushData.data();
	return subID;
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
	langutil::EVMVersion _evmVersion,
	ExternalIdentifierAccess const& _identifierAccess,
	bool _useNamedLabelsForFunctions,
	bool _optimizeStackAllocation
)
{
	EthAssemblyAdapter assemblyAdapter(_assembly);
	BuiltinContext builtinContext;
	CodeTransform transform(
		assemblyAdapter,
		_analysisInfo,
		_parsedData,
		EVMDialect::strictAssemblyForEVM(_evmVersion),
		builtinContext,
		_optimizeStackAllocation,
		false,
		_identifierAccess,
		_useNamedLabelsForFunctions
	);
	try
	{
		transform(_parsedData);
	}
	catch (StackTooDeepError const& _e)
	{
		BOOST_THROW_EXCEPTION(
			InternalCompilerError() << errinfo_comment(
				"Stack too deep when compiling inline assembly" +
				(_e.comment() ? ": " + *_e.comment() : ".")
			));
	}
	solAssert(transform.stackErrors().empty(), "Stack errors present but not thrown.");
}
