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

#include <libsolidity/experimental/codegen/IRGenerationContext.h>
#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/CallGraph.h>
#include <libsolidity/experimental/ast/TypeSystem.h>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/EVMVersion.h>

#include <libsolutil/JSON.h>

#include <string>

namespace solidity::frontend::experimental
{

class Analysis;

class IRGenerator
{
public:
	IRGenerator(
		langutil::EVMVersion _evmVersion,
		std::optional<uint8_t> _eofVersion,
		RevertStrings /*_revertStrings*/,
		std::map<std::string, unsigned> /*_sourceIndices*/,
		langutil::DebugInfoSelection const& /*_debugInfoSelection*/,
		langutil::CharStreamProvider const* /*_soliditySourceProvider*/,
		Analysis const& _analysis
	);

	std::string run(
		ContractDefinition const& _contract,
		bytes const& _cborMetadata,
		std::map<ContractDefinition const*, std::string_view const> const& _otherYulSources
	);

	std::string generate(ContractDefinition const& _contract);
	std::string generate(FunctionDefinition const& _function, Type _type);
private:
	langutil::EVMVersion const m_evmVersion;
	std::optional<uint8_t> const m_eofVersion;
	OptimiserSettings const m_optimiserSettings;
	//langutil::DebugInfoSelection m_debugInfoSelection = {};
	//langutil::CharStreamProvider const* m_soliditySourceProvider = nullptr;
	TypeEnvironment m_env;
	IRGenerationContext m_context;
};

}
