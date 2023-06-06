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

#include <libsolidity/ast/AST.h>
#include <libsolidity/experimental/ast/TypeSystem.h>

#include <algorithm>
#include <string>

namespace solidity::frontend::experimental
{

struct IRNames
{
	static std::string function(TypeEnvironment const& _env, FunctionDefinition const& _function, Type _type);
	static std::string function(VariableDeclaration const& _varDecl);
	static std::string creationObject(ContractDefinition const& _contract);
	static std::string deployedObject(ContractDefinition const& _contract);
	static std::string constructor(ContractDefinition const& _contract);
	static std::string localVariable(VariableDeclaration const& _declaration);
	static std::string localVariable(Expression const& _expression);
};

}
