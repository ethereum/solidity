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
 * Miscellaneous utilities for use in IR generator.
 */

#pragma once

#include <libsolidity/ast/AST.h>

#include <string>

namespace solidity::frontend
{

struct IRNames
{
	static std::string function(FunctionDefinition const& _function);
	static std::string function(VariableDeclaration const& _varDecl);
	static std::string creationObject(ContractDefinition const& _contract);
	static std::string runtimeObject(ContractDefinition const& _contract);
	static std::string implicitConstructor(ContractDefinition const& _contract);
	static std::string constantValueFunction(VariableDeclaration const& _constant);
	static std::string localVariable(VariableDeclaration const& _declaration);
	static std::string localVariable(Expression const& _expression);
	/// @returns the variable name that can be used to inspect the success or failure of an external
	/// function call that was invoked as part of the try statement.
	static std::string trySuccessConditionVariable(Expression const& _expression);
	static std::string tupleComponent(size_t _i);
	static std::string zeroValue(Type const& _type, std::string const& _variableName);
};

}
