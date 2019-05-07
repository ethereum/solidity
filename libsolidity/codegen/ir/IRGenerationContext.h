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
 * Class that contains contextual information during IR generation.
 */

#pragma once

#include <libsolidity/interface/OptimiserSettings.h>

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <liblangutil/EVMVersion.h>

#include <libdevcore/Common.h>

#include <string>
#include <memory>
#include <vector>

namespace dev
{
namespace solidity
{

class ContractDefinition;
class VariableDeclaration;
class FunctionDefinition;
class Expression;
class YulUtilFunctions;

/**
 * Class that contains contextual information during IR generation.
 */
class IRGenerationContext
{
public:
	IRGenerationContext(langutil::EVMVersion _evmVersion, OptimiserSettings _optimiserSettings):
		m_evmVersion(_evmVersion),
		m_optimiserSettings(std::move(_optimiserSettings)),
		m_functions(std::make_shared<MultiUseYulFunctionCollector>())
	{}

	std::shared_ptr<MultiUseYulFunctionCollector> functionCollector() const { return m_functions; }

	/// Sets the current inheritance hierarchy from derived to base.
	void setInheritanceHierarchy(std::vector<ContractDefinition const*> _hierarchy)
	{
		m_inheritanceHierarchy = std::move(_hierarchy);
	}


	std::string addLocalVariable(VariableDeclaration const& _varDecl);
	bool isLocalVariable(VariableDeclaration const& _varDecl) const { return m_localVariables.count(&_varDecl); }
	std::string localVariableName(VariableDeclaration const& _varDecl);

	void addStateVariable(VariableDeclaration const& _varDecl, u256 _storageOffset, unsigned _byteOffset);
	bool isStateVariable(VariableDeclaration const& _varDecl) const { return m_stateVariables.count(&_varDecl); }
	std::pair<u256, unsigned> storageLocationOfVariable(VariableDeclaration const& _varDecl) const
	{
		return m_stateVariables.at(&_varDecl);
	}

	std::string functionName(FunctionDefinition const& _function);
	FunctionDefinition const& virtualFunction(FunctionDefinition const& _functionDeclaration);
	std::string virtualFunctionName(FunctionDefinition const& _functionDeclaration);

	std::string newYulVariable();
	/// @returns the variable (or comma-separated list of variables) that contain
	/// the value of the given expression.
	std::string variable(Expression const& _expression);

	std::string internalDispatch(size_t _in, size_t _out);

	/// @returns a new copy of the utility function generator (but using the same function set).
	YulUtilFunctions utils();

	langutil::EVMVersion evmVersion() const { return m_evmVersion; };

private:
	langutil::EVMVersion m_evmVersion;
	OptimiserSettings m_optimiserSettings;
	std::vector<ContractDefinition const*> m_inheritanceHierarchy;
	std::map<VariableDeclaration const*, std::string> m_localVariables;
	/// Storage offsets of state variables
	std::map<VariableDeclaration const*, std::pair<u256, unsigned>> m_stateVariables;
	std::shared_ptr<MultiUseYulFunctionCollector> m_functions;
	size_t m_varCounter = 0;
};

}
}
