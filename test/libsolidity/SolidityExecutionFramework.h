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
 * @date 2014
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#pragma once

#include <functional>

#include <test/ExecutionFramework.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/DebugSettings.h>

#include <libyul/YulStack.h>

namespace solidity::frontend::test
{

class SolidityExecutionFramework: public solidity::test::ExecutionFramework
{

public:
	SolidityExecutionFramework(): m_showMetadata(solidity::test::CommonOptions::get().showMetadata) {}
	explicit SolidityExecutionFramework(
		langutil::EVMVersion _evmVersion,
		std::optional<uint8_t> _eofVersion,
		std::vector<boost::filesystem::path> const& _vmPaths,
		bool _appendCBORMetadata = true
	):
		ExecutionFramework(_evmVersion, _vmPaths),
		m_eofVersion(_eofVersion),
		m_showMetadata(solidity::test::CommonOptions::get().showMetadata),
		m_appendCBORMetadata(_appendCBORMetadata)
	{}

	bytes const& compileAndRunWithoutCheck(
		std::map<std::string, std::string> const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = {},
		std::map<std::string, solidity::test::Address> const& _libraryAddresses = {},
		std::optional<std::string> const& _sourceName = std::nullopt
	) override
	{
		bytes bytecode = multiSourceCompileContract(_sourceCode, _sourceName, _contractName, _libraryAddresses);

		// Note: Temporary solution that adds arguments size in the EOF data_section_size header to validate evmone implementation.
		// TODO: Validate header and version.
		if (bytecode.size() > 3)
		{
			if (bytecode[0] == uint8_t(0xef) && bytecode[1] == uint8_t(00)) // is EOF
			{
				const auto firstSectionOffset = 3; // magic + version
				bool stop = false;
				for (size_t i = firstSectionOffset; i < bytecode.size(); ++i)
				{
					switch (bytecode[i])
					{
					case uint8_t(0x00): // terminator
						stop = true;
						break;
					case uint8_t(0x01): // type section
						i += 2;			// skip type size section
						break;
					case uint8_t(0x02): // code section
					{
						bytesRef numCodeSectionsRef(&bytecode[i + 1], 2);
						size_t numCodeSections = fromBigEndian<size_t>(numCodeSectionsRef);
						i += numCodeSections * 2 + 2; // skip code section header
						break;
					}
					case uint8_t(0x03): // data section
					{
						auto dataSizeOffset = i + 1;
						bytesRef dataSizeRef(&bytecode[dataSizeOffset], 2);
						size_t dataSize = fromBigEndian<size_t>(dataSizeRef);
						toBigEndian(dataSize + _arguments.size(), dataSizeRef);
						i += 2; // skip data size section
						break;
					}
					}
					if (stop)
						break;
				}
			}
		}

		sendMessage(bytecode + _arguments, true, _value);
		return m_output;
	}

	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName = "",
		std::map<std::string, solidity::test::Address> const& _libraryAddresses = {}
	);

	bytes multiSourceCompileContract(
		std::map<std::string, std::string> const& _sources,
		std::optional<std::string> const& _mainSourceName = std::nullopt,
		std::string const& _contractName = "",
		std::map<std::string, solidity::test::Address> const& _libraryAddresses = {}
	);

	/// Returns @param _sourceCode prefixed with the version pragma and the abi coder v1 pragma,
	/// the latter only if it is forced.
	static std::string addPreamble(std::string const& _sourceCode);
protected:
	using CompilerStack = solidity::frontend::CompilerStack;
	std::optional<uint8_t> m_eofVersion;
	CompilerStack m_compiler;
	bool m_compileViaYul = false;
	bool m_compileToEwasm = false;
	bool m_showMetadata = false;
	bool m_appendCBORMetadata = true;
	CompilerStack::MetadataHash m_metadataHash = CompilerStack::MetadataHash::IPFS;
	RevertStrings m_revertStrings = RevertStrings::Default;
};

} // end namespaces
