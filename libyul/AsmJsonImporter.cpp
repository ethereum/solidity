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
/**
 * @author julius <djudju@protonmail.com>
 * @date 2019
 * Converts an inlineAssembly AST from JSON format to AsmData

 */

#include <libyul/AsmJsonImporter.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>

using namespace solidity::langutil;

namespace solidity::yul
{

using SourceLocation = langutil::SourceLocation;

SourceLocation const AsmJsonImporter::createSourceLocation(Json const& _node)
{
	yulAssert(member(_node, "src").is_string(), "'src' must be a string");

	return solidity::langutil::parseSourceLocation(_node["src"].get<std::string>(), m_sourceNames);
}

AST AsmJsonImporter::createAST(solidity::Json const& _node)
{
	YulNameRepository nameRepository(m_dialect);
	auto block = createBlock(_node, nameRepository);
	return {std::move(nameRepository), std::move(block)};
}

template <class T>
T AsmJsonImporter::createAsmNode(Json const& _node)
{
	T r;
	SourceLocation nativeLocation = createSourceLocation(_node);
	yulAssert(nativeLocation.hasText(), "Invalid source location in Asm AST");
	// TODO: We should add originLocation to the AST.
	// While it's not included, we'll use nativeLocation for it because we only support importing
	// inline assembly as a part of a Solidity AST and there these locations are always the same.
	r.debugData = DebugData::create(nativeLocation, nativeLocation);
	return r;
}

Json AsmJsonImporter::member(Json const& _node, std::string const& _name)
{
	if (!_node.contains(_name))
		return Json();
	return _node[_name];
}

TypedName AsmJsonImporter::createTypedName(Json const& _node, YulNameRepository& _nameRepository)
{
	auto typedName = createAsmNode<TypedName>(_node);
	typedName.type = _nameRepository.nameOfType(member(_node, "type").get<std::string>());
	typedName.name = _nameRepository.defineName(member(_node, "name").get<std::string>());
	return typedName;
}

Statement AsmJsonImporter::createStatement(Json const& _node, YulNameRepository& _nameRepository)
{
	Json jsonNodeType = member(_node, "nodeType");
	yulAssert(jsonNodeType.is_string(), "Expected \"nodeType\" to be of type string!");
	std::string nodeType = jsonNodeType.get<std::string>();

	yulAssert(nodeType.substr(0, 3) == "Yul", "Invalid nodeType prefix");
	nodeType = nodeType.substr(3);

	if (nodeType == "ExpressionStatement")
		return createExpressionStatement(_node, _nameRepository);
	else if (nodeType == "Assignment")
		return createAssignment(_node, _nameRepository);
	else if (nodeType == "VariableDeclaration")
		return createVariableDeclaration(_node, _nameRepository);
	else if (nodeType == "FunctionDefinition")
		return createFunctionDefinition(_node, _nameRepository);
	else if (nodeType == "If")
		return createIf(_node, _nameRepository);
	else if (nodeType == "Switch")
		return createSwitch(_node, _nameRepository);
	else if (nodeType == "ForLoop")
		return createForLoop(_node, _nameRepository);
	else if (nodeType == "Break")
		return createBreak(_node);
	else if (nodeType == "Continue")
		return createContinue(_node);
	else if (nodeType == "Leave")
		return createLeave(_node);
	else if (nodeType == "Block")
		return createBlock(_node, _nameRepository);
	else
		yulAssert(false, "Invalid nodeType as statement");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

Expression AsmJsonImporter::createExpression(Json const& _node, YulNameRepository& _nameRepository)
{
	Json jsonNodeType = member(_node, "nodeType");
	yulAssert(jsonNodeType.is_string(), "Expected \"nodeType\" to be of type string!");
	std::string nodeType = jsonNodeType.get<std::string>();

	yulAssert(nodeType.substr(0, 3) == "Yul", "Invalid nodeType prefix");
	nodeType = nodeType.substr(3);

	if (nodeType == "FunctionCall")
		return createFunctionCall(_node, _nameRepository);
	else if (nodeType == "Identifier")
		return createIdentifier(_node, _nameRepository);
	else if (nodeType == "Literal")
		return createLiteral(_node, _nameRepository);
	else
		yulAssert(false, "Invalid nodeType as expression");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

std::vector<Expression> AsmJsonImporter::createExpressionVector(Json const& _array, YulNameRepository& _nameRepository)
{
	std::vector<Expression> ret;
	for (auto& var: _array)
		ret.emplace_back(createExpression(var, _nameRepository));
	return ret;
}

std::vector<Statement> AsmJsonImporter::createStatementVector(Json const& _array, YulNameRepository& _nameRepository)
{
	std::vector<Statement> ret;
	for (auto& var: _array)
		ret.emplace_back(createStatement(var, _nameRepository));
	return ret;
}

Block AsmJsonImporter::createBlock(Json const& _node, YulNameRepository& _nameRepository)
{
	auto block = createAsmNode<Block>(_node);
	block.statements = createStatementVector(_node["statements"], _nameRepository);
	return block;
}

Literal AsmJsonImporter::createLiteral(Json const& _node, YulNameRepository& _nameRepository)
{
	auto lit = createAsmNode<Literal>(_node);
	std::string kind = member(_node, "kind").get<std::string>();

	solAssert(member(_node, "hexValue").is_string() || member(_node, "value").is_string(), "");
	std::string value;
	if (_node.contains("hexValue"))
		value = util::asString(util::fromHex(member(_node, "hexValue").get<std::string>()));
	else
		value = member(_node, "value").get<std::string>();
	lit.type = _nameRepository.nameOfType(member(_node, "type").get<std::string>());
	if (kind == "number")
	{
		langutil::CharStream charStream(value, "");
		langutil::Scanner scanner{charStream};
		lit.kind = LiteralKind::Number;
		yulAssert(
			scanner.currentToken() == Token::Number,
			"Expected number but got " + langutil::TokenTraits::friendlyName(scanner.currentToken()) + std::string(" while scanning ") + value
		);
	}
	else if (kind == "bool")
	{
		langutil::CharStream charStream(value, "");
		langutil::Scanner scanner{charStream};
		lit.kind = LiteralKind::Boolean;
		yulAssert(
			scanner.currentToken() == Token::TrueLiteral ||
			scanner.currentToken() == Token::FalseLiteral,
			"Expected true/false literal!"
		);
	}
	else if (kind == "string")
	{
		lit.kind = LiteralKind::String;
		yulAssert(
			value.size() <= 32,
			"String literal too long (" + std::to_string(value.size()) + " > 32)"
		);
	}
	else
		yulAssert(false, "unknown type of literal");

	// import only for inline assembly, no unlimited string literals there
	lit.value = valueOfLiteral(value, lit.kind, false /* _unlimitedLiteralArgument */);

	yulAssert(validLiteral(lit));
	return lit;
}

Leave AsmJsonImporter::createLeave(Json const& _node)
{
	return createAsmNode<Leave>(_node);
}

Identifier AsmJsonImporter::createIdentifier(Json const& _node, YulNameRepository& _nameRepository)
{
	auto identifier = createAsmNode<Identifier>(_node);
	identifier.name = _nameRepository.defineName(member(_node, "name").get<std::string>());
	return identifier;
}

Assignment AsmJsonImporter::createAssignment(Json const& _node, YulNameRepository& _nameRepository)
{
	auto assignment = createAsmNode<Assignment>(_node);

	if (_node.contains("variableNames"))
		for (auto const& var: member(_node, "variableNames"))
			assignment.variableNames.emplace_back(createIdentifier(var, _nameRepository));

	assignment.value = std::make_unique<Expression>(createExpression(member(_node, "value"), _nameRepository));
	return assignment;
}

FunctionCall AsmJsonImporter::createFunctionCall(Json const& _node, YulNameRepository& _nameRepository)
{
	auto functionCall = createAsmNode<FunctionCall>(_node);

	for (auto const& var: member(_node, "arguments"))
		functionCall.arguments.emplace_back(createExpression(var, _nameRepository));

	functionCall.functionName = createIdentifier(member(_node, "functionName"), _nameRepository);

	return functionCall;
}

ExpressionStatement AsmJsonImporter::createExpressionStatement(Json const& _node, YulNameRepository& _nameRepository)
{
	auto statement = createAsmNode<ExpressionStatement>(_node);
	statement.expression = createExpression(member(_node, "expression"), _nameRepository);
	return statement;
}

VariableDeclaration AsmJsonImporter::createVariableDeclaration(Json const& _node, YulNameRepository& _nameRepository)
{
	auto varDec = createAsmNode<VariableDeclaration>(_node);
	for (auto const& var: member(_node, "variables"))
		varDec.variables.emplace_back(createTypedName(var, _nameRepository));

	if (_node.contains("value"))
		varDec.value = std::make_unique<Expression>(createExpression(member(_node, "value"), _nameRepository));

	return varDec;
}

FunctionDefinition AsmJsonImporter::createFunctionDefinition(Json const& _node, YulNameRepository& _nameRepository)
{
	auto funcDef = createAsmNode<FunctionDefinition>(_node);
	funcDef.name = _nameRepository.defineName(member(_node, "name").get<std::string>());

	if (_node.contains("parameters"))
		for (auto const& var: member(_node, "parameters"))
			funcDef.parameters.emplace_back(createTypedName(var, _nameRepository));

	if (_node.contains("returnVariables"))
		for (auto const& var: member(_node, "returnVariables"))
			funcDef.returnVariables.emplace_back(createTypedName(var, _nameRepository));

	funcDef.body = createBlock(member(_node, "body"), _nameRepository);
	return funcDef;
}

If AsmJsonImporter::createIf(Json const& _node, YulNameRepository& _nameRepository)
{
	auto ifStatement = createAsmNode<If>(_node);
	ifStatement.condition = std::make_unique<Expression>(createExpression(member(_node, "condition"), _nameRepository));
	ifStatement.body = createBlock(member(_node, "body"), _nameRepository);
	return ifStatement;
}

Case AsmJsonImporter::createCase(Json const& _node, YulNameRepository& _nameRepository)
{
	auto caseStatement = createAsmNode<Case>(_node);
	auto const& value = member(_node, "value");
	if (value.is_string())
		yulAssert(value.get<std::string>() == "default", "Expected default case");
	else
		caseStatement.value = std::make_unique<Literal>(createLiteral(value, _nameRepository));
	caseStatement.body = createBlock(member(_node, "body"), _nameRepository);
	return caseStatement;
}

Switch AsmJsonImporter::createSwitch(Json const& _node, YulNameRepository& _nameRepository)
{
	auto switchStatement = createAsmNode<Switch>(_node);
	switchStatement.expression = std::make_unique<Expression>(createExpression(member(_node, "expression"), _nameRepository));
	for (auto const& var: member(_node, "cases"))
		switchStatement.cases.emplace_back(createCase(var, _nameRepository));
	return switchStatement;
}

ForLoop AsmJsonImporter::createForLoop(Json const& _node, YulNameRepository& _nameRepository)
{
	auto forLoop = createAsmNode<ForLoop>(_node);
	forLoop.pre = createBlock(member(_node, "pre"), _nameRepository);
	forLoop.condition = std::make_unique<Expression>(createExpression(member(_node, "condition"), _nameRepository));
	forLoop.post = createBlock(member(_node, "post"), _nameRepository);
	forLoop.body = createBlock(member(_node, "body"), _nameRepository);
	return forLoop;
}

Break AsmJsonImporter::createBreak(Json const& _node)
{
	return createAsmNode<Break>(_node);
}

Continue AsmJsonImporter::createContinue(Json const& _node)
{
	return createAsmNode<Continue>(_node);
}

}
