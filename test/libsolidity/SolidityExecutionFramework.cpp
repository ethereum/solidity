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
#include <test/libsolidity/util/Common.h>

#include <liblangutil/DebugInfoSelection.h>
#include <libyul/Exceptions.h>
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

bytes SolidityExecutionFramework::multiSourceCompileContract(
	std::map<std::string, std::string> const& _sourceCode,
	std::optional<std::string> const& _mainSourceName,
	std::string const& _contractName,
	std::map<std::string, Address> const& _libraryAddresses
)
{
	if (_mainSourceName.has_value())
		solAssert(_sourceCode.find(_mainSourceName.value()) != _sourceCode.end(), "");

	m_compiler.reset();
	m_compiler.setSources(withPreamble(
		_sourceCode,
		solidity::test::CommonOptions::get().useABIEncoderV1 // _addAbicoderV1Pragma
	));
	m_compiler.setLibraries(_libraryAddresses);
	m_compiler.setRevertStringBehaviour(m_revertStrings);
	m_compiler.setEVMVersion(m_evmVersion);
	m_compiler.setEOFVersion(m_eofVersion);
	m_compiler.setOptimiserSettings(m_optimiserSettings);
	m_compiler.enableEvmBytecodeGeneration(true);
	m_compiler.setViaIR(m_compileViaYul);
	m_compiler.setRevertStringBehaviour(m_revertStrings);
	if (!m_appendCBORMetadata) {
		m_compiler.setMetadataFormat(CompilerStack::MetadataFormat::NoMetadata);
	}
	m_compiler.setMetadataHash(m_metadataHash);

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
	std::string contractName(_contractName.empty() ? m_compiler.lastContractName(_mainSourceName) : _contractName);
	evmasm::LinkerObject obj = m_compiler.object(contractName);
	BOOST_REQUIRE(obj.linkReferences.empty());
	if (m_showMetadata)
		std::cout << "metadata: " << m_compiler.metadata(contractName) << std::endl;
	return obj.bytecode;
}

bytes SolidityExecutionFramework::compileContract(
	std::string const& _sourceCode,
	std::string const& _contractName,
	std::map<std::string, Address> const& _libraryAddresses
)
{
	return multiSourceCompileContract(
		{{"", _sourceCode}},
		std::nullopt,
		_contractName,
		_libraryAddresses
	);
}
