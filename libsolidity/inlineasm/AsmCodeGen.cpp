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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Code-generating part of inline assembly.
 */

#include <libsolidity/inlineasm/AsmCodeGen.h>

#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmScope.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/SourceLocation.h>
#include <libevmasm/Instruction.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/EVMCodeTransform.h>

#include <libdevcore/CommonIO.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

class EthAssemblyAdapter: public julia::AbstractAssembly
{
public:
	explicit EthAssemblyAdapter(eth::Assembly& _assembly):
		m_assembly(_assembly)
	{
	}
	virtual void setSourceLocation(SourceLocation const& _location) override
	{
		m_assembly.setSourceLocation(_location);
	}
	virtual int stackHeight() const override { return m_assembly.deposit(); }
	virtual void appendInstruction(solidity::Instruction _instruction) override
	{
		m_assembly.append(_instruction);
	}
	virtual void appendConstant(u256 const& _constant) override
	{
		m_assembly.append(_constant);
	}
	/// Append a label.
	virtual void appendLabel(LabelID _labelId) override
	{
		m_assembly.append(eth::AssemblyItem(eth::Tag, _labelId));
	}
	/// Append a label reference.
	virtual void appendLabelReference(LabelID _labelId) override
	{
		m_assembly.append(eth::AssemblyItem(eth::PushTag, _labelId));
	}
	virtual size_t newLabelId() override
	{
		return assemblyTagToIdentifier(m_assembly.newTag());
	}
	virtual size_t namedLabel(std::string const& _name) override
	{
		return assemblyTagToIdentifier(m_assembly.namedTag(_name));
	}
	virtual void appendLinkerSymbol(std::string const& _linkerSymbol) override
	{
		m_assembly.appendLibraryAddress(_linkerSymbol);
	}
	virtual void appendJump(int _stackDiffAfter) override
	{
		appendInstruction(solidity::Instruction::JUMP);
		m_assembly.adjustDeposit(_stackDiffAfter);
	}
	virtual void appendJumpTo(LabelID _labelId, int _stackDiffAfter) override
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter);
	}
	virtual void appendJumpToIf(LabelID _labelId) override
	{
		appendLabelReference(_labelId);
		appendInstruction(solidity::Instruction::JUMPI);
	}
	virtual void appendBeginsub(LabelID, int) override
	{
		// TODO we could emulate that, though
		solAssert(false, "BEGINSUB not implemented for EVM 1.0");
	}
	/// Call a subroutine.
	virtual void appendJumpsub(LabelID, int, int) override
	{
		// TODO we could emulate that, though
		solAssert(false, "JUMPSUB not implemented for EVM 1.0");
	}

	/// Return from a subroutine.
	virtual void appendReturnsub(int, int) override
	{
		// TODO we could emulate that, though
		solAssert(false, "RETURNSUB not implemented for EVM 1.0");
	}

	virtual void appendAssemblySize() override
	{
		m_assembly.appendProgramSize();
	}

private:
	static LabelID assemblyTagToIdentifier(eth::AssemblyItem const& _tag)
	{
		u256 id = _tag.data();
		solAssert(id <= std::numeric_limits<LabelID>::max(), "Tag id too large.");
		return LabelID(id);
	}

	eth::Assembly& m_assembly;
};

void assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	eth::Assembly& _assembly,
	julia::ExternalIdentifierAccess const& _identifierAccess,
	bool _useNamedLabelsForFunctions
)
{
	EthAssemblyAdapter assemblyAdapter(_assembly);
	julia::CodeTransform(
		assemblyAdapter,
		_analysisInfo,
		false,
		false,
		_identifierAccess,
		_useNamedLabelsForFunctions
	)(_parsedData);
}
