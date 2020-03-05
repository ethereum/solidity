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
 * @author julius <djudju@protonmail.com>
 * @date 2019
 * Converts an inlineAssembly AST from JSON format to AsmData

 */

#include <libsolidity/ast/AsmJsonImporter.h>
#include <libsolidity/ast/ASTJsonImporter.h>
#include <libsolidity/ast/Types.h>
#include <libyul/AsmData.h>
#include <libyul/AsmDataForward.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;
using namespace solidity::yul;

namespace solidity::frontend
{

using SourceLocation = langutil::SourceLocation;

SourceLocation const AsmJsonImporter::createSourceLocation(Json::Value const& _node)
{
	astAssert(member(_node, "src").isString(), "'src' must be a string");

	return solidity::langutil::parseSourceLocation(_node["src"].asString(), m_sourceName);
}

template <class T>
T AsmJsonImporter::createAsmNode(Json::Value const& _node)
{
	T r;
	r.location = createSourceLocation(_node);
	astAssert(
		r.location.source && 0 <= r.location.start && r.location.start <= r.location.end,
		"Invalid source location in Asm AST"
	);
	return r;
}

Json::Value AsmJsonImporter::member(Json::Value const& _node, string const& _name)
{
	astAssert(_node.isMember(_name), "Node is missing field '" + _name + "'.");
	return _node[_name];
}

yul::TypedName AsmJsonImporter::createTypedName(Json::Value const& _node)
{
	auto typedName = createAsmNode<yul::TypedName>(_node);
	typedName.type = YulString{member(_node, "type").asString()};
	typedName.name = YulString{member(_node, "name").asString()};
	return typedName;
}

yul::Statement AsmJsonImporter::createStatement(Json::Value const& _node)
{
	Json::Value jsonNodeType = member(_node, "nodeType");
	astAssert(jsonNodeType.isString(), "Expected \"nodeType\" to be of type string!");
	string nodeType = jsonNodeType.asString();

	astAssert(nodeType.substr(0, 3) == "Yul", "Invalid nodeType prefix");
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
		astAssert(false, "Invalid nodeType as statement");
}

yul::Expression AsmJsonImporter::createExpression(Json::Value const& _node)
{
	Json::Value jsonNodeType = member(_node, "nodeType");
	astAssert(jsonNodeType.isString(), "Expected \"nodeType\" to be of type string!");
	string nodeType = jsonNodeType.asString();

	astAssert(nodeType.substr(0, 3) == "Yul", "Invalid nodeType prefix");
	nodeType = nodeType.substr(3);

	if (nodeType == "FunctionCall")
		return createFunctionCall(_node);
	else if (nodeType == "Identifier")
		return createIdentifier(_node);
	else if (nodeType == "Literal")
		return createLiteral(_node);
	else
		astAssert(false, "Invalid nodeType as expression");
}

vector<yul::Expression> AsmJsonImporter::createExpressionVector(Json::Value const& _array)
{
	vector<yul::Expression> ret;
	for (auto& var: _array)
		ret.emplace_back(createExpression(var));
	return ret;
}

vector<yul::Statement> AsmJsonImporter::createStatementVector(Json::Value const& _array)
{
	vector<yul::Statement> ret;
	for (auto& var: _array)
		ret.emplace_back(createStatement(var));
	return ret;
}

yul::Block AsmJsonImporter::createBlock(Json::Value const& _node)
{
	auto block = createAsmNode<yul::Block>(_node);
	block.statements = createStatementVector(_node["statements"]);
	return block;
}

yul::Literal AsmJsonImporter::createLiteral(Json::Value const& _node)
{
	auto lit = createAsmNode<yul::Literal>(_node);
	string kind = member(_node, "kind").asString();

	lit.value = YulString{member(_node, "value").asString()};
	lit.type= YulString{member(_node, "type").asString()};

	if (kind == "number")
	{
		langutil::Scanner scanner{langutil::CharStream(lit.value.str(), "")};
		lit.kind = yul::LiteralKind::Number;
		astAssert(
			scanner.currentToken() == Token::Number,
			"Expected number but got " + langutil::TokenTraits::friendlyName(scanner.currentToken()) + string(" while scanning ") + lit.value.str()
		);
	}
	else if (kind == "bool")
	{
		langutil::Scanner scanner{langutil::CharStream(lit.value.str(), "")};
		lit.kind = yul::LiteralKind::Boolean;
		astAssert(
			scanner.currentToken() == Token::TrueLiteral ||
			scanner.currentToken() == Token::FalseLiteral,
			"Expected true/false literal!"
		);
	}
	else if (kind == "string")
	{
		lit.kind = yul::LiteralKind::String;
		astAssert(
			lit.value.str().size() <= 32,
			"String literal too long (" + to_string(lit.value.str().size()) + " > 32)"
		);
	}
	else
		solAssert(false, "unknown type of literal");

	return lit;
}

yul::Leave AsmJsonImporter::createLeave(Json::Value const& _node)
{
	return createAsmNode<yul::Leave>(_node);
}

yul::Identifier AsmJsonImporter::createIdentifier(Json::Value const& _node)
{
	auto identifier = createAsmNode<yul::Identifier>(_node);
	identifier.name = YulString(member(_node, "name").asString());
	return identifier;
}

yul::Assignment AsmJsonImporter::createAssignment(Json::Value const& _node)
{
	auto assignment = createAsmNode<yul::Assignment>(_node);

	if (_node.isMember("variableNames"))
		for (auto const& var: member(_node, "variableNames"))
			assignment.variableNames.emplace_back(createIdentifier(var));

	assignment.value = make_unique<yul::Expression>(createExpression(member(_node, "value")));
	return assignment;
}

yul::FunctionCall AsmJsonImporter::createFunctionCall(Json::Value const& _node)
{
	auto functionCall = createAsmNode<yul::FunctionCall>(_node);

	for (auto const& var: member(_node, "arguments"))
		functionCall.arguments.emplace_back(createExpression(var));

	functionCall.functionName = createIdentifier(member(_node, "functionName"));

	return functionCall;
}

yul::ExpressionStatement AsmJsonImporter::createExpressionStatement(Json::Value const& _node)
{
	auto statement = createAsmNode<yul::ExpressionStatement>(_node);
	statement.expression = createExpression(member(_node, "expression"));
	return statement;
}

yul::VariableDeclaration AsmJsonImporter::createVariableDeclaration(Json::Value const& _node)
{
	auto varDec = createAsmNode<yul::VariableDeclaration>(_node);
	for (auto const& var: member(_node, "variables"))
		varDec.variables.emplace_back(createTypedName(var));
	varDec.value = make_unique<yul::Expression>(createExpression(member(_node, "value")));
	return varDec;
}

yul::FunctionDefinition AsmJsonImporter::createFunctionDefinition(Json::Value const& _node)
{
	auto funcDef = createAsmNode<yul::FunctionDefinition>(_node);
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

yul::If AsmJsonImporter::createIf(Json::Value const& _node)
{
	auto ifStatement = createAsmNode<yul::If>(_node);
	ifStatement.condition = make_unique<yul::Expression>(createExpression(member(_node, "condition")));
	ifStatement.body = createBlock(member(_node, "body"));
	return ifStatement;
}

yul::Case AsmJsonImporter::createCase(Json::Value const& _node)
{
	auto caseStatement = createAsmNode<yul::Case>(_node);
	auto const& value = member(_node, "value");
	if (value.isString())
		astAssert(value.asString() == "default", "Expected default case");
	else
		caseStatement.value = make_unique<yul::Literal>(createLiteral(value));
	caseStatement.body = createBlock(member(_node, "body"));
	return caseStatement;
}

yul::Switch AsmJsonImporter::createSwitch(Json::Value const& _node)
{
	auto switchStatement = createAsmNode<yul::Switch>(_node);
	switchStatement.expression = make_unique<yul::Expression>(createExpression(member(_node, "expression")));
	for (auto const& var: member(_node, "cases"))
		switchStatement.cases.emplace_back(createCase(var));
	return switchStatement;
}

yul::ForLoop AsmJsonImporter::createForLoop(Json::Value const& _node)
{
	auto forLoop = createAsmNode<yul::ForLoop>(_node);
	forLoop.pre = createBlock(member(_node, "pre"));
	forLoop.condition = make_unique<yul::Expression>(createExpression(member(_node, "condition")));
	forLoop.post = createBlock(member(_node, "post"));
	forLoop.body = createBlock(member(_node, "body"));
	return forLoop;
}

yul::Break AsmJsonImporter::createBreak(Json::Value const& _node)
{
	return createAsmNode<yul::Break>(_node);
}

yul::Continue AsmJsonImporter::createContinue(Json::Value const& _node)
{
	return createAsmNode<yul::Continue>(_node);
}

}
