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

#include <libsolidity/codegen/ir/Common.h>

#include <libsolutil/CommonIO.h>

using namespace std;
using namespace solidity::util;
using namespace solidity::frontend;

string IRNames::function(FunctionDefinition const& _function)
{
	// @TODO previously, we had to distinguish creation context and runtime context,
	// but since we do not work with jump positions anymore, this should not be a problem, right?
	return "fun_" + _function.name() + "_" + to_string(_function.id());
}

string IRNames::function(VariableDeclaration const& _varDecl)
{
	return "getter_fun_" + _varDecl.name() + "_" + to_string(_varDecl.id());
}

string IRNames::creationObject(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + toString(_contract.id());
}

string IRNames::runtimeObject(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + toString(_contract.id()) + "_deployed";
}
