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
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into Yul.
 */

#pragma once

#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <liblangutil/EVMVersion.h>
#include <string>

namespace dev
{
namespace solidity
{

class SourceUnit;

class IRGenerator
{
public:
	IRGenerator(langutil::EVMVersion _evmVersion, OptimiserSettings _optimiserSettings):
		m_evmVersion(_evmVersion),
		m_optimiserSettings(_optimiserSettings),
		m_context(_evmVersion, std::move(_optimiserSettings)),
		m_utils(_evmVersion, m_context.functionCollector())
	{}

	/// Generates and returns the IR code, in unoptimized and optimized form
	/// (or just pretty-printed, depending on the optimizer settings).
	std::pair<std::string, std::string> run(ContractDefinition const& _contract);

private:
	std::string generateIR(ContractDefinition const& _contract);
	std::string generateIRFunction(FunctionDefinition const& _function);

	std::string constructorCode(FunctionDefinition const& _constructor);
	std::string deployCode(ContractDefinition const& _contract);
	std::string callValueCheck();

	std::string creationObjectName(ContractDefinition const& _contract);
	std::string runtimeObjectName(ContractDefinition const& _contract);

	std::string dispatchRoutine(ContractDefinition const& _contract);

	std::string memoryInit();

	void resetContext();

	langutil::EVMVersion const m_evmVersion;
	OptimiserSettings const m_optimiserSettings;

	IRGenerationContext m_context;
	YulUtilFunctions m_utils;
};

}
}
