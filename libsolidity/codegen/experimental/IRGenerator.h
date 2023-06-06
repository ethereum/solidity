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

#pragma once

#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/CallGraph.h>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/EVMVersion.h>

#include <json/json.h>

#include <string>

namespace solidity::frontend::experimental
{

class SourceUnit;

class IRGenerator
{
public:
	IRGenerator(
		langutil::EVMVersion _evmVersion,
		std::optional<uint8_t> _eofVersion,
		RevertStrings /*_revertStrings*/,
		std::map<std::string, unsigned> /*_sourceIndices*/,
		langutil::DebugInfoSelection const& _debugInfoSelection,
		langutil::CharStreamProvider const* _soliditySourceProvider
	):
		m_evmVersion(_evmVersion),
		m_eofVersion(_eofVersion),
		m_debugInfoSelection(_debugInfoSelection),
		m_soliditySourceProvider(_soliditySourceProvider)
	{}

	std::string run(
		ContractDefinition const& _contract,
		bytes const& _cborMetadata,
		std::map<ContractDefinition const*, std::string_view const> const& _otherYulSources
	) const;

	std::string generate(ContractDefinition const& _contract) const;
	std::string generate(FunctionDefinition const& _function) const;
	std::string generate(InlineAssembly const& _assembly) const;
private:
	langutil::EVMVersion const m_evmVersion;
	std::optional<uint8_t> const m_eofVersion;
	OptimiserSettings const m_optimiserSettings;
	langutil::DebugInfoSelection m_debugInfoSelection = {};
	langutil::CharStreamProvider const* m_soliditySourceProvider = nullptr;
};

}
