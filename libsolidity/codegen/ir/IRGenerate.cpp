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
 * Component that translates Solidity code into JULIA.
 */

#include <libsolidity/codegen/ir/IRGenerate.h>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

bool IRGenerate::visit(ContractDefinition const& _contract)
{
	solUnimplementedAssert(
		_contract.contractKind() == ContractDefinition::ContractKind::Contract,
		"Non-contracts (libraries, interfaces) are not supported yet."
	);
	solUnimplementedAssert(_contract.baseContracts().empty(), "Inheritance not supported yet.");
	solUnimplementedAssert(_contract.definedStructs().empty(), "User-defined types not supported yet.");
	solUnimplementedAssert(_contract.definedEnums().empty(), "User-defined types not supported yet.");
	solUnimplementedAssert(_contract.events().empty(), "Events not supported yet.");
	solUnimplementedAssert(_contract.functionModifiers().empty(), "Modifiers not supported yet.");

	m_body.location = _contract.location();

	ASTNode::listAccept(_contract.definedFunctions(), *this);

	return false;
}

bool IRGenerate::visit(FunctionDefinition const& _function)
{
	solUnimplementedAssert(_function.isImplemented(), "Unimplemented functions not supported yet.");
	solUnimplementedAssert(_function.modifiers().empty(), "Modifiers not supported yet.");
	solUnimplementedAssert(_function.parameters().empty(), "Parameters not supported yet.");
	solUnimplementedAssert(_function.returnParameters().empty(), "Return parameters not supported yet.");

	assembly::FunctionDefinition funDef;
	if (_function.name().empty())
		funDef.name = "fallback";
	else
		funDef.name = _function.name();
	funDef.location = _function.location();
	m_currentFunction = funDef;
	_function.body().accept(*this);
	return false;
}

void IRGenerate::endVisit(FunctionDefinition const&)
{
	// invalidate m_currentFunction
	m_body.statements.emplace_back(m_currentFunction);
}

bool IRGenerate::visit(Block const& _node)
{
	for (auto const& statement: _node.statements())
		statement->accept(*this);
	return false;
}

bool IRGenerate::visit(Throw const& _throw)
{
	assembly::Literal zero;
	zero.kind = assembly::LiteralKind::Number;
	zero.value = "0";
	zero.type = "u256";

	assembly::FunctionCall funCall;
	funCall.functionName.name = "revert";
	funCall.arguments.push_back(zero);
	funCall.arguments.push_back(zero);
	funCall.location = _throw.location();
	m_currentFunction.body.statements.emplace_back(funCall);
	return false;
}
