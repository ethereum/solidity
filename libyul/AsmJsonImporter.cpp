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

#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>

using namespace std;
using namespace solidity::langutil;

namespace solidity::yul
{

using SourceLocation = langutil::SourceLocation;

SourceLocation const AsmJsonImporter::createSourceLocation(Json::Value const& _node)
{
	yulAssert(member(_node, "src").isString(), "'src' must be a string");

	return solidity::langutil::parseSourceLocation(_node["src"].asString(), m_sourceNames);
}

template <class T>
T AsmJsonImporter::createAsmNode(Json::Value const& _node)
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

Json::Value AsmJsonImporter::member(Json::Value const& _node, string const& _name)
{
	if (!_node.isMember(_name))
		return Json::nullValue;
	return _node[_name];
}

TypedName AsmJsonImporter::createTypedName(Json::Value const& _node)
{
	auto typedName = createAsmNode<TypedName>(_node);
	typedName.type = YulString{member(_node, "type").asString()};
	typedName.name = YulString{member(_node, "name").asString()};
	return typedName;
}

Statement AsmJsonImporter::createStatement(Json::Value const& _node)
{
	Json::Value jsonNodeType = member(_node, "nodeType");
	yulAssert(jsonNodeType.isString(), "Expected \"nodeType\" to be of type string!");
	string nodeType = jsonNodeType.asString();

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
}

Expression AsmJsonImporter::createExpression(Json::Value const& _node)
{
	Json::Value jsonNodeType = member(_node, "nodeType");
	yulAssert(jsonNodeType.isString(), "Expected \"nodeType\" to be of type string!");
	string nodeType = jsonNodeType.asString();

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
}

vector<Expression> AsmJsonImporter::createExpressionVector(Json::Value const& _array)
{
	vector<Expression> ret;
	for (auto& var: _array)
		ret.emplace_back(createExpression(var));
	return ret;
}

vector<Statement> AsmJsonImporter::createStatementVector(Json::Value const& _array)
{
	vector<Statement> ret;
	for (auto& var: _array)
		ret.emplace_back(createStatement(var));
	return ret;
}

Block AsmJsonImporter::createBlock(Json::Value const& _node)
{
	auto block = createAsmNode<Block>(_node);
	block.statements = createStatementVector(_node["statements"]);
	return block;
}

Literal AsmJsonImporter::createLiteral(Json::Value const& _node)
{
	auto lit = createAsmNode<Literal>(_node);
	string kind = member(_node, "kind").asString();

	solAssert(member(_node, "hexValue").isString() || member(_node, "value").isString(), "");
	if (_node.isMember("hexValue"))
		lit.value = YulString{util::asString(util::fromHex(member(_node, "hexValue").asString()))};
	else
		lit.value = YulString{member(_node, "value").asString()};

	lit.type= YulString{member(_node, "type").asString()};

	if (kind == "number")
	{
		langutil::CharStream charStream(lit.value.str(), "");
		langutil::Scanner scanner{charStream};
		lit.kind = LiteralKind::Number;
		yulAssert(
			scanner.currentToken() == Token::Number,
			"Expected number but got " + langutil::TokenTraits::friendlyName(scanner.currentToken()) + string(" while scanning ") + lit.value.str()
		);
	}
	else if (kind == "bool")
	{
		langutil::CharStream charStream(lit.value.str(), "");
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
			lit.value.str().size() <= 32,
			"String literal too long (" + to_string(lit.value.str().size()) + " > 32)"
		);
	}
	else
		yulAssert(false, "unknown type of literal");

	return lit;
}

Leave AsmJsonImporter::createLeave(Json::Value const& _node)
{
	return createAsmNode<Leave>(_node);
}

Identifier AsmJsonImporter::createIdentifier(Json::Value const& _node)
{
	auto identifier = createAsmNode<Identifier>(_node);
	identifier.name = YulString(member(_node, "name").asString());
	return identifier;
}

Assignment AsmJsonImporter::createAssignment(Json::Value const& _node)
{
	auto assignment = createAsmNode<Assignment>(_node);

	if (_node.isMember("variableNames"))
		for (auto const& var: member(_node, "variableNames"))
			assignment.variableNames.emplace_back(createIdentifier(var));

	assignment.value = make_unique<Expression>(createExpression(member(_node, "value")));
	return assignment;
}

FunctionCall AsmJsonImporter::createFunctionCall(Json::Value const& _node)
{
	auto functionCall = createAsmNode<FunctionCall>(_node);

	for (auto const& var: member(_node, "arguments"))
		functionCall.arguments.emplace_back(createExpression(var));

	functionCall.functionName = createIdentifier(member(_node, "functionName"));

	return functionCall;
}

ExpressionStatement AsmJsonImporter::createExpressionStatement(Json::Value const& _node)
{
	auto statement = createAsmNode<ExpressionStatement>(_node);
	statement.expression = createExpression(member(_node, "expression"));
	return statement;
}

VariableDeclaration AsmJsonImporter::createVariableDeclaration(Json::Value const& _node)
{
	auto varDec = createAsmNode<VariableDeclaration>(_node);
	for (auto const& var: member(_node, "variables"))
		varDec.variables.emplace_back(createTypedName(var));
	varDec.value = make_unique<Expression>(createExpression(member(_node, "value")));
	return varDec;
}

FunctionDefinition AsmJsonImporter::createFunctionDefinition(Json::Value const& _node)
{
	auto funcDef = createAsmNode<FunctionDefinition>(_node);
	funcDef.name = YulString{member(_node, "name").asString()};

	if (_node.isMember("parameters"))
		for (auto const& var: member(_node, "parameters"))
			funcDef.parameters.emplace_back(createTypedName(var));

	if (_node.isMember("returnVariables"))
		for (auto const& var: member(_node, "returnVariables"))
			funcDef.returnVariables.emplace_back(createTypedName(var));

	funcDef.body = createBlock(member(_node, "body"));
	return funcDef;
}

If AsmJsonImporter::createIf(Json::Value const& _node)
{
	auto ifStatement = createAsmNode<If>(_node);
	ifStatement.condition = make_unique<Expression>(createExpression(member(_node, "condition")));
	ifStatement.body = createBlock(member(_node, "body"));
	return ifStatement;
}

Case AsmJsonImporter::createCase(Json::Value const& _node)
{
	auto caseStatement = createAsmNode<Case>(_node);
	auto const& value = member(_node, "value");
	if (value.isString())
		yulAssert(value.asString() == "default", "Expected default case");
	else
		caseStatement.value = make_unique<Literal>(createLiteral(value));
	caseStatement.body = createBlock(member(_node, "body"));
	return caseStatement;
}

Switch AsmJsonImporter::createSwitch(Json::Value const& _node)
{
	auto switchStatement = createAsmNode<Switch>(_node);
	switchStatement.expression = make_unique<Expression>(createExpression(member(_node, "expression")));
	for (auto const& var: member(_node, "cases"))
		switchStatement.cases.emplace_back(createCase(var));
	return switchStatement;
}

ForLoop AsmJsonImporter::createForLoop(Json::Value const& _node)
{
	auto forLoop = createAsmNode<ForLoop>(_node);
	forLoop.pre = createBlock(member(_node, "pre"));
	forLoop.condition = make_unique<Expression>(createExpression(member(_node, "condition")));
	forLoop.post = createBlock(member(_node, "post"));
	forLoop.body = createBlock(member(_node, "body"));
	return forLoop;
}

Break AsmJsonImporter::createBreak(Json::Value const& _node)
{
	return createAsmNode<Break>(_node);
}

Continue AsmJsonImporter::createContinue(Json::Value const& _node)
{
	return createAsmNode<Continue>(_node);
}

}
