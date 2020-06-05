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
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#include <cstdlib>
#include <iostream>
#include <boost/test/framework.hpp>
#include <test/libsolidity/SolidityExecutionFramework.h>

using namespace solidity;
using namespace solidity::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;

bytes SolidityExecutionFramework::compileContract(
	string const& _sourceCode,
	string const& _contractName,
	map<string, Address> const& _libraryAddresses
)
{
	// Silence compiler version warning
	std::string sourceCode = "pragma solidity >=0.0;\n";
	if (
		solidity::test::CommonOptions::get().useABIEncoderV2 &&
		_sourceCode.find("pragma experimental ABIEncoderV2;") == std::string::npos
	)
		sourceCode += "pragma experimental ABIEncoderV2;\n";
	sourceCode += _sourceCode;
	m_compiler.reset();
	m_compiler.enableEwasmGeneration(m_compileToEwasm);
	m_compiler.setSources({{"", sourceCode}});
	m_compiler.setLibraries(_libraryAddresses);
	m_compiler.setRevertStringBehaviour(m_revertStrings);
	m_compiler.setEVMVersion(m_evmVersion);
	m_compiler.setOptimiserSettings(m_optimiserSettings);
	m_compiler.enableIRGeneration(m_compileViaYul);
	m_compiler.setRevertStringBehaviour(m_revertStrings);
	if (!m_compiler.compile())
	{
		langutil::SourceReferenceFormatter formatter(std::cerr);

		for (auto const& error: m_compiler.errors())
			formatter.printErrorInformation(*error);
		BOOST_ERROR("Compiling contract failed");
	}
	std::string contractName(_contractName.empty() ? m_compiler.lastContractName() : _contractName);
	evmasm::LinkerObject obj;
	if (m_compileViaYul)
	{
		if (m_compileToEwasm)
			obj = m_compiler.ewasmObject(contractName);
		else
		{
			yul::AssemblyStack asmStack(
				m_evmVersion,
				yul::AssemblyStack::Language::StrictAssembly,
				// Ignore optimiser settings here because we need Yul optimisation to
				// get code that does not exhaust the stack.
				OptimiserSettings::full());
			bool analysisSuccessful = asmStack.parseAndAnalyze("", m_compiler.yulIROptimized(contractName));
			solAssert(analysisSuccessful, "Code that passed analysis in CompilerStack can't have errors");
			asmStack.optimize();
			obj = std::move(*asmStack.assemble(yul::AssemblyStack::Machine::EVM).bytecode);
		}
	}
	else
		obj = m_compiler.object(contractName);
	BOOST_REQUIRE(obj.linkReferences.empty());
	if (m_showMetadata)
		cout << "metadata: " << m_compiler.metadata(contractName) << endl;
	return obj.bytecode;
}
