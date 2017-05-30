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
#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <libdevcore/SHA3.h>
#include <libdevcore/CommonData.h>

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

	buildDispatcher(_contract);

	return false;
}

void IRGenerate::buildDispatcher(ContractDefinition const& _contract)
{
	appendFunction(R"(
	{
		// Revert if value was received.
		function ensureNoValueTransfer()
		{
			switch callvalue()
			case 0 {}
			default { revert(0, 0) }
		}

		// Extract 32 bit method identifier
		function extractCallSignature() -> sig
		{
			// FIXME: replace with constant
			sig := div(calldataload(0), exp(2, 224))
		}
	}
	)");

	assembly::Switch _switch;
	_switch.expression = make_shared<assembly::Statement>(createFunctionCall("extractCallSignature"));

	for (auto const& function: _contract.definedFunctions())
	{
		if (!function->isPartOfExternalInterface())
			continue;

		assembly::Literal literal;
		literal.kind = assembly::LiteralKind::Number;
		literal.value = toHex(FixedHash<4>::Arith(FixedHash<4>(dev::keccak256(function->externalSignature()))), HexPrefix::Add);

		assembly::Block body;
		if (!function->isPayable())
			body.statements.emplace_back(createFunctionCall("ensureNoValueTransfer"));
		body.statements.emplace_back(createFunctionCall("_" + function->name()));

		assembly::Case _case;
		_case.value = make_shared<assembly::Literal>(literal);
		_case.body = std::move(body);

		_switch.cases.emplace_back(_case);
	}

	assembly::Case defaultCase;
	if (auto const& fallbackFunction = _contract.fallbackFunction())
	{
		assembly::Block body;
		if (!fallbackFunction->isPayable())
			body.statements.emplace_back(createFunctionCall("ensureNoValueTransfer"));
		body.statements.emplace_back(createFunctionCall("fallback"));
		defaultCase.body = std::move(body);
	}
	else
		defaultCase.body = wrapInBlock(createFunctionCall("revert"));
	_switch.cases.emplace_back(defaultCase);

	m_body.statements.emplace_back(_switch);
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
		funDef.name = "_" + _function.name();
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

	assembly::FunctionCall funCall;
	funCall.functionName.name = "revert";
	funCall.arguments.push_back(zero);
	funCall.arguments.push_back(zero);
	funCall.location = _throw.location();
	m_currentFunction.body.statements.emplace_back(funCall);
	return false;
}

bool IRGenerate::visit(InlineAssembly const& _inlineAssembly)
{
	m_currentFunction.body.statements.emplace_back(_inlineAssembly.operations());
	return false;
}

void IRGenerate::appendFunction(string const& _function)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto scanner = make_shared<Scanner>(CharStream(_function), "<irgenerated>");
	auto result = assembly::Parser(errorReporter).parse(scanner);
	solAssert(result, "");
	solAssert(errors.empty(), "");

	auto statements = result->statements;
	for (size_t i = 0; i < statements.size(); ++i)
		m_body.statements.emplace_back(std::move(statements[i]));
}

assembly::FunctionCall IRGenerate::createFunctionCall(string const& _function)
{
	assembly::FunctionCall funCall;
	funCall.functionName.name = _function;
	return funCall;
}

assembly::Block IRGenerate::wrapInBlock(assembly::Statement const& _statement)
{
	assembly::Block block;
	block.statements.push_back(_statement);
	return block;
}
