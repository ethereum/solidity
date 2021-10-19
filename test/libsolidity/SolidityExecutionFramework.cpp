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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <boost/test/framework.hpp>

#include <cstdlib>
#include <iostream>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::langutil;
using namespace solidity::test;
using namespace std;

bytes SolidityExecutionFramework::multiSourceCompileContract(
	map<string, string> const& _sourceCode,
	optional<string> const& _mainSourceName,
	string const& _contractName,
	map<string, Address> const& _libraryAddresses
)
{
	if (_mainSourceName.has_value())
		solAssert(_sourceCode.find(_mainSourceName.value()) != _sourceCode.end(), "");
	map<string, string> sourcesWithPreamble = _sourceCode;
	for (auto& entry: sourcesWithPreamble)
		entry.second = addPreamble(entry.second);

	m_compiler.reset();
	m_compiler.enableEwasmGeneration(m_compileToEwasm);
	m_compiler.setSources(sourcesWithPreamble);
	m_compiler.setLibraries(_libraryAddresses);
	m_compiler.setRevertStringBehaviour(m_revertStrings);
	m_compiler.setEVMVersion(m_evmVersion);
	m_compiler.setOptimiserSettings(m_optimiserSettings);
	m_compiler.enableEvmBytecodeGeneration(!m_compileViaYul);
	m_compiler.enableIRGeneration(m_compileViaYul);
	m_compiler.setRevertStringBehaviour(m_revertStrings);
	if (!m_compiler.compile())
	{
		// The testing framework expects an exception for
		// "unimplemented" yul IR generation.
		if (m_compileViaYul)
			for (auto const& error: m_compiler.errors())
				if (error->type() == langutil::Error::Type::CodeGenerationError)
					BOOST_THROW_EXCEPTION(*error);
		langutil::SourceReferenceFormatter{std::cerr, m_compiler, true, false}
			.printErrorInformation(m_compiler.errors());
		BOOST_ERROR("Compiling contract failed");
	}
	string contractName(_contractName.empty() ? m_compiler.lastContractName(_mainSourceName) : _contractName);
	evmasm::LinkerObject obj;
	if (m_compileViaYul)
	{
		if (m_compileToEwasm)
			obj = m_compiler.ewasmObject(contractName);
		else
		{
			// Try compiling twice: If the first run fails due to stack errors, forcefully enable
			// the optimizer.
			for (bool forceEnableOptimizer: {false, true})
			{
				OptimiserSettings optimiserSettings = m_optimiserSettings;
				if (!forceEnableOptimizer && !optimiserSettings.runYulOptimiser)
				{
					// Enable some optimizations on the first run
					optimiserSettings.runYulOptimiser = true;
					optimiserSettings.yulOptimiserSteps = "uljmul jmul";
				}
				else if (forceEnableOptimizer)
					optimiserSettings = OptimiserSettings::full();

				yul::AssemblyStack asmStack(
					m_evmVersion,
					yul::AssemblyStack::Language::StrictAssembly,
					optimiserSettings,
					DebugInfoSelection::All()
				);
				bool analysisSuccessful = asmStack.parseAndAnalyze("", m_compiler.yulIROptimized(contractName));
				solAssert(analysisSuccessful, "Code that passed analysis in CompilerStack can't have errors");

				try
				{
					asmStack.optimize();
					obj = move(*asmStack.assemble(yul::AssemblyStack::Machine::EVM).bytecode);
					obj.link(_libraryAddresses);
					break;
				}
				catch (...)
				{
					if (forceEnableOptimizer || optimiserSettings == OptimiserSettings::full())
						throw;
				}
			}
		}
	}
	else
		obj = m_compiler.object(contractName);
	BOOST_REQUIRE(obj.linkReferences.empty());
	if (m_showMetadata)
		cout << "metadata: " << m_compiler.metadata(contractName) << endl;
	return obj.bytecode;
}

bytes SolidityExecutionFramework::compileContract(
	string const& _sourceCode,
	string const& _contractName,
	map<string, Address> const& _libraryAddresses
)
{
	return multiSourceCompileContract(
		{{"", _sourceCode}},
		nullopt,
		_contractName,
		_libraryAddresses
	);
}

string SolidityExecutionFramework::addPreamble(string const& _sourceCode)
{
	// Silence compiler version warning
	string preamble = "pragma solidity >=0.0;\n";
	if (_sourceCode.find("// SPDX-License-Identifier:") == string::npos)
		preamble += "// SPDX-License-Identifier: unlicensed\n";
	if (
		solidity::test::CommonOptions::get().useABIEncoderV1 &&
		_sourceCode.find("pragma experimental ABIEncoderV2;") == string::npos &&
		_sourceCode.find("pragma abicoder") == string::npos
	)
		preamble += "pragma abicoder v1;\n";
	return preamble + _sourceCode;
}
