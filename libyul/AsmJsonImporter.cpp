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
	return AST(createBlock(_node));
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

NameWithDebugData AsmJsonImporter::createNameWithDebugData(Json const& _node)
{
	auto nameWithDebugData = createAsmNode<NameWithDebugData>(_node);
	nameWithDebugData.name = YulName{member(_node, "name").get<std::string>()};
	return nameWithDebugData;
}

Statement AsmJsonImporter::createStatement(Json const& _node)
{
	Json jsonNodeType = member(_node, "nodeType");
	yulAssert(jsonNodeType.is_string(), "Expected \"nodeType\" to be of type string!");
	std::string nodeType = jsonNodeType.get<std::string>();

	yulAssert(nodeType.substr(0, 3) == "Yul", "Invalid nodeType prefix");
	nodeType = nodeType.substr(3);

	if (nodeType == "ExpressionStatement")
		return createExpressionStatement(_node);
	else if (nodeType == "Assignment")
		return createAssignment(_node);
	else if (nodeType == "VariableDeclaration")
		return createVariableDeclaration(_node);
	else if (nodeType == "FunctionDefinition")
		return createFunctionDefinition(_node);
	else if (nodeType == "If")
		return createIf(_node);
	else if (nodeType == "Switch")
		return createSwitch(_node);
	else if (nodeType == "ForLoop")
		return createForLoop(_node);
	else if (nodeType == "Break")
		return createBreak(_node);
	else if (nodeType == "Continue")
		return createContinue(_node);
	else if (nodeType == "Leave")
		return createLeave(_node);
	else if (nodeType == "Block")
		return createBlock(_node);
	else
		yulAssert(false, "Invalid nodeType as statement");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

Expression AsmJsonImporter::createExpression(Json const& _node)
{
	Json jsonNodeType = member(_node, "nodeType");
	yulAssert(jsonNodeType.is_string(), "Expected \"nodeType\" to be of type string!");
	std::string nodeType = jsonNodeType.get<std::string>();

	yulAssert(nodeType.substr(0, 3) == "Yul", "Invalid nodeType prefix");
	nodeType = nodeType.substr(3);

	if (nodeType == "FunctionCall")
		return createFunctionCall(_node);
	else if (nodeType == "Identifier")
		return createIdentifier(_node);
	else if (nodeType == "Literal")
		return createLiteral(_node);
	else
		yulAssert(false, "Invalid nodeType as expression");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

std::vector<Expression> AsmJsonImporter::createExpressionVector(Json const& _array)
{
	std::vector<Expression> ret;
	for (auto& var: _array)
		ret.emplace_back(createExpression(var));
	return ret;
}

std::vector<Statement> AsmJsonImporter::createStatementVector(Json const& _array)
{
	std::vector<Statement> ret;
	for (auto& var: _array)
		ret.emplace_back(createStatement(var));
	return ret;
}

Block AsmJsonImporter::createBlock(Json const& _node)
{
	auto block = createAsmNode<Block>(_node);
	block.statements = createStatementVector(_node["statements"]);
	return block;
}

Literal AsmJsonImporter::createLiteral(Json const& _node)
{
	auto lit = createAsmNode<Literal>(_node);
	std::string kind = member(_node, "kind").get<std::string>();

	solAssert(member(_node, "hexValue").is_string() || member(_node, "value").is_string(), "");
	std::string value;
	if (_node.contains("hexValue"))
		value = util::asString(util::fromHex(member(_node, "hexValue").get<std::string>()));
	else
		value = member(_node, "value").get<std::string>();
	{
		auto const typeNode = member(_node, "type");
		yulAssert(typeNode.empty() || typeNode.get<std::string>().empty());
	}
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

Identifier AsmJsonImporter::createIdentifier(Json const& _node)
{
	auto identifier = createAsmNode<Identifier>(_node);
	identifier.name = YulName(member(_node, "name").get<std::string>());
	return identifier;
}

Assignment AsmJsonImporter::createAssignment(Json const& _node)
{
	auto assignment = createAsmNode<Assignment>(_node);

	if (_node.contains("variableNames"))
		for (auto const& var: member(_node, "variableNames"))
			assignment.variableNames.emplace_back(createIdentifier(var));

	assignment.value = std::make_unique<Expression>(createExpression(member(_node, "value")));
	return assignment;
}

FunctionCall AsmJsonImporter::createFunctionCall(Json const& _node)
{
	auto functionCall = createAsmNode<FunctionCall>(_node);

	for (auto const& var: member(_node, "arguments"))
		functionCall.arguments.emplace_back(createExpression(var));

	functionCall.functionName = createIdentifier(member(_node, "functionName"));

	return functionCall;
}

ExpressionStatement AsmJsonImporter::createExpressionStatement(Json const& _node)
{
	auto statement = createAsmNode<ExpressionStatement>(_node);
	statement.expression = createExpression(member(_node, "expression"));
	return statement;
}

VariableDeclaration AsmJsonImporter::createVariableDeclaration(Json const& _node)
{
	auto varDec = createAsmNode<VariableDeclaration>(_node);
	for (auto const& var: member(_node, "variables"))
		varDec.variables.emplace_back(createNameWithDebugData(var));

	if (_node.contains("value"))
		varDec.value = std::make_unique<Expression>(createExpression(member(_node, "value")));

	return varDec;
}

FunctionDefinition AsmJsonImporter::createFunctionDefinition(Json const& _node)
{
	auto funcDef = createAsmNode<FunctionDefinition>(_node);
	funcDef.name = YulName{member(_node, "name").get<std::string>()};

	if (_node.contains("parameters"))
		for (auto const& var: member(_node, "parameters"))
			funcDef.parameters.emplace_back(createNameWithDebugData(var));

	if (_node.contains("returnVariables"))
		for (auto const& var: member(_node, "returnVariables"))
			funcDef.returnVariables.emplace_back(createNameWithDebugData(var));

	funcDef.body = createBlock(member(_node, "body"));
	return funcDef;
}

If AsmJsonImporter::createIf(Json const& _node)
{
	auto ifStatement = createAsmNode<If>(_node);
	ifStatement.condition = std::make_unique<Expression>(createExpression(member(_node, "condition")));
	ifStatement.body = createBlock(member(_node, "body"));
	return ifStatement;
}

Case AsmJsonImporter::createCase(Json const& _node)
{
	auto caseStatement = createAsmNode<Case>(_node);
	auto const& value = member(_node, "value");
	if (value.is_string())
		yulAssert(value.get<std::string>() == "default", "Expected default case");
	else
		caseStatement.value = std::make_unique<Literal>(createLiteral(value));
	caseStatement.body = createBlock(member(_node, "body"));
	return caseStatement;
}

Switch AsmJsonImporter::createSwitch(Json const& _node)
{
	auto switchStatement = createAsmNode<Switch>(_node);
	switchStatement.expression = std::make_unique<Expression>(createExpression(member(_node, "expression")));
	for (auto const& var: member(_node, "cases"))
		switchStatement.cases.emplace_back(createCase(var));
	return switchStatement;
}

ForLoop AsmJsonImporter::createForLoop(Json const& _node)
{
	auto forLoop = createAsmNode<ForLoop>(_node);
	forLoop.pre = createBlock(member(_node, "pre"));
	forLoop.condition = std::make_unique<Expression>(createExpression(member(_node, "condition")));
	forLoop.post = createBlock(member(_node, "post"));
	forLoop.body = createBlock(member(_node, "body"));
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
