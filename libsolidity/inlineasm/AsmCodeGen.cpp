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

#include <libjulia/backends/evm/AbstractAssembly.h>
#include <libjulia/backends/evm/EVMCodeTransform.h>

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
	EthAssemblyAdapter(eth::Assembly& _assembly):
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
	virtual void appendLabel(size_t _labelId) override
	{
		m_assembly.append(eth::AssemblyItem(eth::Tag, _labelId));
	}
	/// Append a label reference.
	virtual void appendLabelReference(size_t _labelId) override
	{
		m_assembly.append(eth::AssemblyItem(eth::PushTag, _labelId));
	}
	virtual size_t newLabelId() override
	{
		return assemblyTagToIdentifier(m_assembly.newTag());
	}
	virtual void appendLinkerSymbol(std::string const& _linkerSymbol) override
	{
		m_assembly.appendLibraryAddress(_linkerSymbol);
	}

private:
	size_t assemblyTagToIdentifier(eth::AssemblyItem const& _tag) const
	{
		u256 id = _tag.data();
		solAssert(id <= std::numeric_limits<size_t>::max(), "Tag id too large.");
		return size_t(id);
	}

	eth::Assembly& m_assembly;
};

eth::Assembly assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	julia::ExternalIdentifierAccess const& _identifierAccess
)
{
	eth::Assembly assembly;
	EthAssemblyAdapter assemblyAdapter(assembly);
	julia::CodeTransform(m_errors, assemblyAdapter, _parsedData, _analysisInfo, _identifierAccess);
	return assembly;
}

void assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	eth::Assembly& _assembly,
	julia::ExternalIdentifierAccess const& _identifierAccess
)
{
	EthAssemblyAdapter assemblyAdapter(_assembly);
	julia::CodeTransform(m_errors, assemblyAdapter, _parsedData, _analysisInfo, _identifierAccess);
}
