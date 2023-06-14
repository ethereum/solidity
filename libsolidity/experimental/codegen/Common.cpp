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

#include <libsolidity/experimental/codegen/Common.h>
#include <libsolidity/experimental/ast/TypeSystem.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <libsolutil/CommonIO.h>

#include <libyul/AsmPrinter.h>

using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::util;
using namespace solidity::yul;

namespace solidity::frontend::experimental
{

std::string IRNames::function(TypeEnvironment const& _env, FunctionDefinition const& _function, Type _type)
{
	if (_function.isConstructor())
		return constructor(*_function.annotation().contract);

	return "fun_" + _function.name() + "_" + std::to_string(_function.id()) + "$" + TypeEnvironmentHelpers{_env}.canonicalTypeName(_type) + "$";
}

std::string IRNames::function(VariableDeclaration const& _varDecl)
{
	return "getter_fun_" + _varDecl.name() + "_" + std::to_string(_varDecl.id());
}

std::string IRNames::creationObject(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + toString(_contract.id());
}

std::string IRNames::deployedObject(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + toString(_contract.id()) + "_deployed";
}

std::string IRNames::constructor(ContractDefinition const& _contract)
{
	return "constructor_" + _contract.name() + "_" + std::to_string(_contract.id());
}

std::string IRNames::localVariable(VariableDeclaration const& _declaration)
{
	return "var_" + _declaration.name() + '_' + std::to_string(_declaration.id());
}

std::string IRNames::localVariable(Expression const& _expression)
{
	return "expr_" + std::to_string(_expression.id());
}

}
