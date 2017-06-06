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
 * @date 2017
 * Converts an AST from json format to an ASTNode.
 */

#include <libsolidity/ast/ASTJsonImporter.h>
#include <libsolidity/parsing/Scanner.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <libsolidity/parsing/Token.h>


using namespace std;

namespace
{
template<class T>
dev::solidity::ASTPointer<T> castPointer(dev::solidity::ASTPointer<dev::solidity::ASTNode> _ast)
{
	dev::solidity::ASTPointer<T> ret = dynamic_pointer_cast<T>(_ast);
	solAssert(ret, "");
	return ret;
}
}

using namespace dev;
using namespace solidity;

template <typename T, typename... Args>
ASTPointer<T> ASTJsonImporter::createASTNode(Json::Value const& _node, Args&&... _args)
{
    auto n = make_shared<T>(createSourceLocation(_node), forward<Args>(_args)...);
    n->setID(_node["id"].asInt());
    return n;
}

ASTJsonImporter::ASTJsonImporter(map<string, Json::Value const*> _sourceList )
        : m_sourceList(_sourceList)
{
	for (auto const& src: _sourceList)
		m_sourceLocations.emplace_back(make_shared<string const>(src.first));
}

map<string, ASTPointer<SourceUnit>> ASTJsonImporter::jsonToSourceUnit()
{
	for (auto const& srcPair: m_sourceList)
	{
		solAssert(!srcPair.second->isNull(), "");
		try
		{
			m_sourceUnits[srcPair.first] =  createSourceUnit(*srcPair.second, srcPair.first);
		}
		//not sure whether this is the right place to catch this (thrown by boost:split in createSourceLocation())
		catch (boost::exception const& _e)
		{
			solAssert(false, "JSON-Node has invalid src-string" + boost::diagnostic_information(_e));
		}
	}
	return m_sourceUnits;
}

SourceLocation const ASTJsonImporter::createSourceLocation(Json::Value const& _node)
{
	solAssert(!_node["src"].isNull(), "JsonValue should not be an ASTNode");
	string srcString = _node["src"].asString();
	vector<string> pos;
	boost::algorithm::split(pos, srcString, boost::is_any_of(":"));
	solAssert(pos.size() == 3, "invalid source location string");
	int start = stoi(pos[0]);
	int end = start + stoi(pos[1]);
	solAssert(int(m_sourceLocations.size()) >= stoi(pos[2]), "higher src-index than sourceNames");
	return SourceLocation(
		start,
		end,
		m_sourceLocations[stoi(pos[2])]
	);
}

//helper functions
template<class T>
ASTPointer<T> ASTJsonImporter::nullOrCast(Json::Value _json)
{
	if (_json.isNull())
		return nullptr;
	else
		return castPointer<T>(convertJsonToASTNode(_json));
}

Token::Value ASTJsonImporter::scanSingleToken(Json::Value _node)
{
	Scanner scanner(CharStream(_node.asString()));
	if (scanner.peekNextToken() == Token::EOS)
		return scanner.currentToken();
	else
		BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("Invalid AST entry."));
}

ASTPointer<ASTNode> ASTJsonImporter::convertJsonToASTNode(Json::Value const& _json)
{
	string nodeType = _json["nodeType"].asString();
	if (nodeType == "PragmaDirective")
	    return createPragmaDirective(_json);
	if (nodeType == "ImportDirective")
	    return createImportDirective(_json);
	if (nodeType == "ContractDefinition")
	    return createContractDefinition(_json);
	if (nodeType == "InheritanceSpecifier")
	    return createInheritanceSpecifier(_json);
	if (nodeType == "UsingForDirective")
		return createUsingForDirective(_json);
	if (nodeType == "StructDefinition")
		return createStructDefinition(_json);
	if (nodeType == "createEnumDefinition")
		return createEnumDefinition(_json);
	if (nodeType == "EnumValue")
		return createEnumValue(_json);
	if (nodeType == "ParameterList")
		return createParameterList(_json);
	if (nodeType == "FunctionDefinition")
		return createFunctionDefinition(_json);
	if (nodeType == "VariableDeclaration")
		return createVariableDeclaration(_json);
	if (nodeType == "ModifierDefinition")
		return createModifierDefinition(_json);
	if (nodeType == "ModifierInvocation")
		return createModifierInvocation(_json);
	if (nodeType == "EventDefinition")
		return createEventDefinition(_json);
	if (nodeType == "ElementaryTypeName")
		return createElementaryTypeName(_json);
	if (nodeType == "UserDefinedTypeName")
		return createUserDefinedTypeName(_json);
	if (nodeType == "FunctionTypeName")
		return createFunctionTypeName(_json);
	if (nodeType == "Mapping")
		return createMapping(_json);
	if (nodeType == "ArrayTypeName")
		return createArrayTypeName(_json);
//	if (_json["nodeType"] == "InlineAssembly")
//		return createInlineAssembly(_json);
	if (nodeType == "Block")
		return createBlock(_json);
	if (nodeType == "PlaceholderStatement")
		return createPlaceholderStatement(_json);
	if (nodeType == "IfStatement")
		return createIfStatement(_json);
	if (nodeType == "WhileStatement")
		return createWhileStatement(_json, false);
	if (nodeType == "DoWhileStatement")
		return createWhileStatement(_json, true);
	if (nodeType == "ForStatement")
		return createForStatement(_json);
	if (nodeType == "Continue")
		return createContinue(_json);
	if (nodeType == "Break")
		return createBreak(_json);
	if (nodeType == "Return")
		return createReturn(_json);
	if (nodeType == "Throw")
		return createThrow(_json);
	if (nodeType == "VariableDeclarationStatement")
		return createVariableDeclarationStatement(_json);
	if (nodeType == "ExpressionStatement")
		return createExpressionStatement(_json);
	if (nodeType == "Conditional")
		return createConditional(_json);
	if (nodeType == "Assignment")
		return createAssignment(_json);
	if (nodeType == "TupleExpression")
		return createTupleExpression(_json);
	if (nodeType == "UnaryOperation")
		return createUnaryOperation(_json);
	if (nodeType == "BinaryOperation")
		return createBinaryOperation(_json);
	if (nodeType == "FunctionCall")
		return createFunctionCall(_json);
	if (nodeType == "NewExpression")
		return createNewExpression(_json);
	if (nodeType == "MemberAccess")
		return createMemberAccess(_json);
	if (nodeType == "IndexAccess")
		return createIndexAccess(_json);
	if (nodeType == "Identifier")
		return createIdentifier(_json, _json["nodeType"].asString());
	if (nodeType == "ElementaryTypeNameExpression")
		return createElementaryTypeNameExpression(_json);
	if (nodeType == "Literal")
		return createLiteral(_json);
	else BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("type of JsonValue is unknown."));
}

ASTPointer<SourceUnit> ASTJsonImporter::createSourceUnit(Json::Value const& _node, string const& _srcName)
{
	vector<ASTPointer<ASTNode>> nodes;
	for (auto& child: _node["nodes"])
		nodes.emplace_back(convertJsonToASTNode(child));
	ASTPointer<SourceUnit> tmp = createASTNode<SourceUnit>(_node, nodes);
	tmp->annotation().path = _srcName;
	return tmp;
}

ASTPointer<PragmaDirective> ASTJsonImporter::createPragmaDirective(Json::Value const& _node)
{
	vector<Token::Value> tokens;
	vector<ASTString> literals;
	for (auto const& lit: _node["literals"])
	{
		string l = lit.asString();
                literals.push_back(l);
                tokens.push_back(scanSingleToken(l));
	}
	ASTPointer<PragmaDirective> tmp = createASTNode<PragmaDirective>(_node, tokens, literals);
	return tmp;
}

ASTPointer<ImportDirective> ASTJsonImporter::createImportDirective(Json::Value const& _node){
	ASTPointer<ASTString> const& path = make_shared<ASTString>(_node["file"].asString());
	ASTPointer<ASTString> const& unitAlias = make_shared<ASTString>(_node["unitAlias"].asString());
	vector<pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> symbolAliases;
	for (auto& tuple: _node["symbolAliases"])
	{
                symbolAliases.push_back( make_pair(
			createIdentifier(_node, tuple["foreign"].asString()),
			tuple["local"].isNull() ? make_shared<ASTString>(tuple["local"].asString()) : nullptr //also check if that's not null?
		));
	}
	//create node
	ASTPointer<ImportDirective> tmp = createASTNode<ImportDirective>(_node, path, unitAlias, move(symbolAliases));
	tmp->annotation().absolutePath = _node["absolutePath"].asString();
	return tmp;
}

ASTPointer<ContractDefinition> ASTJsonImporter::createContractDefinition(Json::Value const& _node){
	ASTPointer<ASTString> documentation = _node["documentation"].isNull() ?
				nullptr :
				make_shared<ASTString>(_node["documentation"].asString()
	);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	std::vector<ASTPointer<InheritanceSpecifier>> baseContracts;
	for (auto& base : _node["baseContracts"])
		baseContracts.push_back(createInheritanceSpecifier(base));
	std::vector<ASTPointer<ASTNode>> subNodes;
	for (auto& subnode : _node["nodes"])
		subNodes.push_back(convertJsonToASTNode(subnode));
	ContractDefinition::ContractKind kind = contractKind(_node);
	//create node
	ASTPointer<ContractDefinition> tmp = createASTNode<ContractDefinition>(
		_node,
		name,
		documentation,
		baseContracts,
		subNodes,
		kind
	);
	return tmp;
}

ASTPointer<InheritanceSpecifier> ASTJsonImporter::createInheritanceSpecifier(Json::Value const& _node)
{
	ASTPointer<UserDefinedTypeName> baseName = createUserDefinedTypeName(_node["baseName"]);
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg : _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	//create node
	ASTPointer<InheritanceSpecifier> tmp = createASTNode<InheritanceSpecifier>(
		_node,
		baseName,
		arguments
	);
	return tmp;
}

ASTPointer<UsingForDirective> ASTJsonImporter::createUsingForDirective(Json::Value const& _node)
{
	ASTPointer<UserDefinedTypeName> libraryName = createUserDefinedTypeName(_node["libraryName"]);
	ASTPointer<TypeName> typeName;
	if (!_node["typename"].isNull())
		typeName = castPointer<TypeName>(convertJsonToASTNode(_node["typename"]));
	else
		typeName = nullptr;
	ASTPointer<UsingForDirective> tmp = createASTNode<UsingForDirective>(
		_node,
		libraryName,
		typeName
	);
	return tmp;
}

ASTPointer<ASTNode> ASTJsonImporter::createStructDefinition(Json::Value const& _node)
{
	ASTPointer<ASTString> const& name = make_shared<ASTString>(_node["name"].asString());
	std::vector<ASTPointer<VariableDeclaration>> members;
	for (auto& member: _node["members"])
		members.push_back(createVariableDeclaration(member));
	ASTPointer<StructDefinition> tmp = createASTNode<StructDefinition>(
		_node,
		name,
		members
	);
	return tmp;
}

ASTPointer<EnumDefinition> ASTJsonImporter::createEnumDefinition(Json::Value const& _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	std::vector<ASTPointer<EnumValue>> members;
	for (auto& member: _node["members"])
		members.push_back(createEnumValue(member));
	ASTPointer<EnumDefinition> tmp = createASTNode<EnumDefinition>(
		_node,
		name,
		members
	);
	return tmp;
}

ASTPointer<EnumValue> ASTJsonImporter::createEnumValue(Json::Value const& _node)
{
	ASTPointer<ASTString> const& name = make_shared<ASTString>(_node["name"].asString());
	ASTPointer<EnumValue> tmp = createASTNode<EnumValue>(
		_node,
		name
	);
	return tmp;
}

ASTPointer<ParameterList> ASTJsonImporter::createParameterList(Json::Value const&  _node)
{
	std::vector<ASTPointer<VariableDeclaration>> parameters;
	for (auto& param: _node["parameters"])
		parameters.push_back(createVariableDeclaration(param));
	ASTPointer<ParameterList> tmp = createASTNode<ParameterList>(
		_node,
		parameters
	);
	return tmp;
}

ASTPointer<FunctionDefinition> ASTJsonImporter::createFunctionDefinition(Json::Value const&  _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	Declaration::Visibility vis = visibility(_node);
	bool isConstructor = _node["isConstructor"].asBool();
	ASTPointer<ASTString> documentation = make_shared<ASTString>(""); //postponed
	ASTPointer<ParameterList> parameters = createParameterList(_node["parameters"]);
	bool isDeclaredConst = _node["isDeclaredConst"].asBool();
	std::vector<ASTPointer<ModifierInvocation>> modifiers;
	for (auto& mod: _node["modifiers"])
		modifiers.push_back(createModifierInvocation(mod));
	ASTPointer<ParameterList> returnParameters = createParameterList(_node["returnParameters"]);
	bool isPayable = _node["payable"].asBool();
	ASTPointer<Block> body = createBlock(_node["body"]);
	ASTPointer<FunctionDefinition> tmp = createASTNode<FunctionDefinition>(
		_node,
		name,
		vis,
		isConstructor,
		documentation,
		parameters,
		isDeclaredConst,
		modifiers,
		returnParameters,
		isPayable,
		body
	);
	return tmp;
}

ASTPointer<VariableDeclaration> ASTJsonImporter::createVariableDeclaration(Json::Value const&  _node)
{
	ASTPointer<TypeName> type = nullOrCast<TypeName>(_node["typeName"]);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	ASTPointer<Expression> value = nullOrCast<Expression>(_node["value"]);
	Declaration::Visibility vis = visibility(_node);
	bool isStateVar = _node["stateVariable"].asBool();
	bool isIndexed = _node["indexed"].asBool();
	bool isConstant = _node["constant"].asBool();
	VariableDeclaration::Location referenceLocation = location(_node);
	ASTPointer<VariableDeclaration> tmp = createASTNode<VariableDeclaration>(
		_node,
		type,
		name,
	        value,
		vis,
		isStateVar,
		isIndexed,
		isConstant,
		referenceLocation
	);
	return tmp;
}

ASTPointer<ModifierDefinition> ASTJsonImporter::createModifierDefinition(Json::Value const&  _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	ASTPointer<ASTString> documentation = make_shared<ASTString>(""); //postponed
	ASTPointer<ParameterList> parameters = createParameterList(_node["parameters"]);
	ASTPointer<Block> body = createBlock(_node["body"]);
	ASTPointer<ModifierDefinition> tmp = createASTNode<ModifierDefinition>(
		_node,
		name,
		documentation,
		parameters,
		body
	);
	return tmp;
}

ASTPointer<ModifierInvocation> ASTJsonImporter::createModifierInvocation(Json::Value const&  _node)
{
	ASTPointer<Identifier> name = createIdentifier(_node, _node["name"].asString());
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	ASTPointer<ModifierInvocation> tmp = createASTNode<ModifierInvocation>(
		_node,
		name,
		arguments
	);
	return tmp;
}

ASTPointer<EventDefinition> ASTJsonImporter::createEventDefinition(Json::Value const&  _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	ASTPointer<ASTString> documentation = make_shared<ASTString>(""); //postponed
	ASTPointer<ParameterList> parameters = createParameterList(_node["parameters"]);
	bool anonymous = _node["anonymous"].asBool();
	ASTPointer<EventDefinition> tmp = createASTNode<EventDefinition>(
		_node,
		name,
		documentation,
		parameters,
		anonymous
	);
	return tmp;
}

ASTPointer<ElementaryTypeName> ASTJsonImporter::createElementaryTypeName(Json::Value const& _node)
{
	unsigned short firstNum;
	unsigned short secondNum;
	string name = _node["name"].asString();
	Token::Value token;
	tie(token, firstNum, secondNum) = Token::fromIdentifierOrKeyword(name);
	ElementaryTypeNameToken elem(token, firstNum,  secondNum);
	ASTPointer<ElementaryTypeName> tmp = createASTNode<ElementaryTypeName>(
		_node,
		elem
	);
	return tmp;
}

ASTPointer<UserDefinedTypeName> ASTJsonImporter::createUserDefinedTypeName(Json::Value const& _node)
{
	vector<ASTString> namePath;
	vector<string> strs;
	string nameString = _node["name"].asString();
	boost::algorithm::split(strs, nameString, boost::is_any_of("."));
	for (string s : strs)
		namePath.push_back(ASTString(s));
	ASTPointer<UserDefinedTypeName> tmp = createASTNode<UserDefinedTypeName>(
		_node,
		namePath
	);
	return tmp;
}

ASTPointer<FunctionTypeName> ASTJsonImporter::createFunctionTypeName(Json::Value const&  _node)
{
	ASTPointer<ParameterList> parameterTypes = createParameterList(_node["parameterTypes"]);
	ASTPointer<ParameterList> returnTypes = createParameterList(_node["returnParameterTypes"]);
	Declaration::Visibility vis = visibility(_node);
	bool isDeclaredConst = _node["isDeclaredConst"].asBool();
	bool isPayable = _node["payable"].asBool();
	ASTPointer<FunctionTypeName> tmp = createASTNode<FunctionTypeName>(
		_node,
		parameterTypes,
		returnTypes,
		vis,
		isDeclaredConst,
		isPayable
	);
	return tmp;
}

ASTPointer<Mapping> ASTJsonImporter::createMapping(Json::Value const&  _node)
{
	ASTPointer<ElementaryTypeName> keyType = createElementaryTypeName(_node["keyType"]);
	ASTPointer<TypeName> valueType = castPointer<TypeName>(convertJsonToASTNode(_node["valueType"]));
	ASTPointer<Mapping> tmp = createASTNode<Mapping>(
		_node,
		keyType,
		valueType
	);
	return tmp;
}

ASTPointer<ArrayTypeName> ASTJsonImporter::createArrayTypeName(Json::Value const&  _node)
{
	ASTPointer<TypeName> baseType = castPointer<TypeName>(convertJsonToASTNode(_node));
	ASTPointer<Expression> length = nullOrCast<Expression>(_node["length"]);
	ASTPointer<ArrayTypeName> tmp = createASTNode<ArrayTypeName>(
		_node,
		baseType,
		length
	);
	return tmp;
}

//ASTPointer<InlineAssembly> ASTJsonImporter::createInlineAssembly(Json::Value const&  _node) //TODO LATER
//{
//	return make_shared<Identifier>(createSourceLocation(_node), make_shared<ASTString>("placeholder"));
//}
ASTPointer<Block> ASTJsonImporter::createBlock(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	std::vector<ASTPointer<Statement>> statements;
	for (auto& stat: _node["statements"])
		statements.push_back(castPointer<Statement>(convertJsonToASTNode(stat)));
	ASTPointer<Block> tmp = createASTNode<Block>(
		_node,
		docString,
		statements
	);
	return tmp;
}

ASTPointer<PlaceholderStatement> ASTJsonImporter::createPlaceholderStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<PlaceholderStatement> tmp = createASTNode<PlaceholderStatement>(
		_node,
		docString
	);
	return tmp;
}

ASTPointer<IfStatement> ASTJsonImporter::createIfStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(_node["condition"]));
	ASTPointer<Statement> trueBody = castPointer<Statement>(convertJsonToASTNode(_node["trueBody"]));
	ASTPointer<Statement> falseBody = nullOrCast<Statement>(_node["falseBody"]);
	ASTPointer<IfStatement> tmp = createASTNode<IfStatement>(
		_node,
		docString,
		condition,
		trueBody,
		falseBody
	);
	return tmp;
}

ASTPointer<WhileStatement> ASTJsonImporter::createWhileStatement(Json::Value const&  _node, bool _isDoWhile=false)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(_node["condition"]));
	ASTPointer<Statement> body = castPointer<Statement>(convertJsonToASTNode(_node["Body"]));
	ASTPointer<WhileStatement> tmp = createASTNode<WhileStatement>(
		_node,
		docString,
		condition,
		body,
		_isDoWhile
	);
	return tmp;
}

ASTPointer<ForStatement> ASTJsonImporter::createForStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Statement> initExpression = nullOrCast<Statement>(_node["initExpression"]);
	ASTPointer<Expression> conditionExpression = nullOrCast<Expression>(_node["condition"]);
	ASTPointer<ExpressionStatement> loopExpression = nullOrCast<ExpressionStatement>(_node["loopExpression"]);
	ASTPointer<Statement> body = castPointer<Statement>(convertJsonToASTNode(_node["body"]));
	ASTPointer<ForStatement> tmp = createASTNode<ForStatement>(
		_node,
		docString,
		initExpression,
		conditionExpression,
		loopExpression,
		body
	);
	return tmp;
}

ASTPointer<Continue> ASTJsonImporter::createContinue(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Continue> tmp = createASTNode<Continue>(_node, docString);
	return tmp;
}

ASTPointer<Break> ASTJsonImporter::createBreak(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Break> tmp = createASTNode<Break>(_node, docString);
	return tmp;
}

ASTPointer<Return> ASTJsonImporter::createReturn(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	ASTPointer<Return> tmp = createASTNode<Return>(
		_node,
		docString,
		expression
	);
	return tmp;
}

ASTPointer<Throw> ASTJsonImporter::createThrow(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Throw> tmp = createASTNode<Throw>(_node, docString);
	return tmp;
}

ASTPointer<VariableDeclarationStatement> ASTJsonImporter::createVariableDeclarationStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	std::vector<ASTPointer<VariableDeclaration>> variables;
	for (auto& var: _node["declarations"])
		variables.push_back(createVariableDeclaration(var));
	ASTPointer<Expression> initialValue = nullOrCast<Expression>(_node["initialValue"]);
	ASTPointer<VariableDeclarationStatement> tmp = createASTNode<VariableDeclarationStatement>(
		_node,
		docString,
		variables,
		initialValue
	);
	return tmp;
}

ASTPointer<ExpressionStatement> ASTJsonImporter::createExpressionStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	ASTPointer<ExpressionStatement> tmp = createASTNode<ExpressionStatement>(
		_node,
		docString,
		expression
	);
	return tmp;
}

ASTPointer<Conditional> ASTJsonImporter::createConditional(Json::Value const&  _node)
{
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(_node["condition"]));
	ASTPointer<Expression> trueExpression = castPointer<Expression>(convertJsonToASTNode(_node["trueExpression"]));
	ASTPointer<Expression> falseExpression = castPointer<Expression>(convertJsonToASTNode(_node["falseExpression"]));
	ASTPointer<Conditional> tmp = createASTNode<Conditional>(
		_node,
		condition,
		trueExpression,
		falseExpression
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Assignment> ASTJsonImporter::createAssignment(Json::Value const&  _node)
{
	ASTPointer<Expression> leftHandSide = castPointer<Expression>(convertJsonToASTNode(_node["leftHandSide"]));
        Token::Value assignmentOperator = scanSingleToken(_node["operator"]);
	ASTPointer<Expression> rightHandSide = castPointer<Expression>(convertJsonToASTNode(_node["rightHandSide"]));
	ASTPointer<Assignment> tmp = createASTNode<Assignment>(
		_node,
		leftHandSide,
		assignmentOperator,
		rightHandSide
	);
	return tmp;
}

ASTPointer<TupleExpression> ASTJsonImporter::createTupleExpression(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> components;
	for (auto& comp: _node["components"])
		components.push_back(castPointer<Expression>(convertJsonToASTNode(comp)));
	bool isArray = _node["isInlineArray"].asBool();
	ASTPointer<TupleExpression> tmp = createASTNode<TupleExpression>(
		_node,
		components,
		isArray
	);
	return tmp;
}

ASTPointer<UnaryOperation> ASTJsonImporter::createUnaryOperation(Json::Value const&  _node)
{
        Token::Value operation = scanSingleToken(_node["operator"]);
	ASTPointer<Expression> subExpression = castPointer<Expression>(convertJsonToASTNode(_node["subExpression"]));
	bool prefix = _node["prefix"].asBool();
	ASTPointer<UnaryOperation> tmp = createASTNode<UnaryOperation>(
		_node,
		operation,
		subExpression,
		prefix
	);
	return tmp;

}

ASTPointer<BinaryOperation> ASTJsonImporter::createBinaryOperation(Json::Value const&  _node)
{
	ASTPointer<Expression> left = castPointer<Expression>(convertJsonToASTNode(_node["leftExpression"]));
        Token::Value operation = scanSingleToken(_node["operator"]);
//	Token::Value operation;
//	ASTPointer<Scanner> scanner = make_shared<Scanner>(CharStream(_node["operator"].asString()), "tmp");
//	operation = scanner->next();
	ASTPointer<Expression> right = castPointer<Expression>(convertJsonToASTNode(_node["rightExpression"]));
	ASTPointer<BinaryOperation> tmp = createASTNode<BinaryOperation>(
		_node,
		left,
		operation,
		right
	);
	return tmp;
}

ASTPointer<FunctionCall> ASTJsonImporter::createFunctionCall(Json::Value const&  _node)
{
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: _node["names"])
		names.push_back(make_shared<ASTString>(name.asString()));
	ASTPointer<FunctionCall> tmp = createASTNode<FunctionCall>(
		_node,
		expression,
		arguments,
		names
	);
	return tmp;
}

ASTPointer<NewExpression> ASTJsonImporter::createNewExpression(Json::Value const&  _node)
{
	ASTPointer<TypeName> typeName = castPointer<TypeName>(convertJsonToASTNode(_node["typeName"]));
	ASTPointer<NewExpression> tmp = createASTNode<NewExpression>(
		_node,
		typeName
	);
	return tmp;
}

ASTPointer<MemberAccess> ASTJsonImporter::createMemberAccess(Json::Value const&  _node)
{
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	ASTPointer<ASTString> memberName = make_shared<ASTString>(_node["memberName"].asString());
	ASTPointer<MemberAccess> tmp = createASTNode<MemberAccess>(
		_node,
		expression,
		memberName
	);
	return tmp;
}

ASTPointer<IndexAccess> ASTJsonImporter::createIndexAccess(Json::Value const& _node)
{
	ASTPointer<Expression> base = castPointer<Expression>(convertJsonToASTNode(_node["baseExpression"]));
	ASTPointer<Expression> index = castPointer<Expression>(convertJsonToASTNode(_node["indexExpression"]));
	ASTPointer<IndexAccess> tmp = createASTNode<IndexAccess>(
		_node,
		base,
		index
	);
	return tmp;
}

ASTPointer<Identifier> ASTJsonImporter::createIdentifier(Json::Value const& _node, string const& name)
{
	return createASTNode<Identifier>(_node, make_shared<ASTString>(name));
}

ASTPointer<ElementaryTypeNameExpression> ASTJsonImporter::createElementaryTypeNameExpression(Json::Value const&  _node) //needs TEST (elem)
{
	Scanner scanner(CharStream(_node["typeDescriptions"]["typeString"].asString()), "tmp");
	Token::Value token = scanner.currentToken();
	unsigned firstSize;
	unsigned secondSize;
	tie(firstSize, secondSize) = scanner.currentTokenInfo();
	ElementaryTypeNameToken elem(token, firstSize, secondSize);
	ASTPointer<ElementaryTypeNameExpression> tmp = createASTNode<ElementaryTypeNameExpression>(
		_node,
		elem
	);
	return tmp;

}

ASTPointer<ASTNode> ASTJsonImporter::createLiteral(Json::Value const&  _node) //TODO
{
	Token::Value token = literalTokenKind(_node);
	ASTPointer<ASTString> value = make_shared<ASTString>(
				_node["value"].isNull() ?
				"todo" :
				_node["value"].asString()
	);
	Literal::SubDenomination sub = subdenomination(_node);
	ASTPointer<Literal> tmp = createASTNode<Literal>(
		_node,
		token,
		value,
		sub
	);
	return tmp;
}

Token::Value ASTJsonImporter::literalTokenKind(Json::Value const& _node)
{
	solAssert(_node.isMember("kind") && !_node["kind"].isNull(), "");
	Token::Value tok;
	if (_node["kind"].asString(), "number")
		tok = Token::Number;
	else if (_node["kind"].asString() == "string")
		tok = Token::StringLiteral;
	else if (_node["kind"].asString() == "bool")
		 tok = (_node["value"].asString() == "true") ? Token::TrueLiteral : Token::FalseLiteral;
	else
		solAssert(false, "unknown kind of literalString");
	return tok;

}

Declaration::Visibility ASTJsonImporter::visibility(Json::Value const& _node)
{
	solAssert(_node.isMember("visibility") && !_node["visibility"].isNull(), "");
	Declaration::Visibility vis;
	if (_node["visibility"].asString() == "default")
		vis = Declaration::Visibility::Default;
	else if (_node["visibility"].asString() == "private")
		vis = Declaration::Visibility::Private;
	else if ( _node["visibility"].asString() == "internal")
		vis = Declaration::Visibility::Internal;
	else if (_node["visibility"].asString() == "public")
		vis = Declaration::Visibility::Public;
	else if (_node["visibility"].asString() == "external")
		vis = Declaration::Visibility::External;
	else
		solAssert(false, "unknown visibility declaration");
	return vis;
}

VariableDeclaration::Location ASTJsonImporter::location(Json::Value const& _node)
{
	VariableDeclaration::Location loc;
	solAssert(_node.isMember("storageLocation") && !_node["storageLocation"].isNull(), "");
	if (_node["storageLocation"].asString() == "default")
		loc = VariableDeclaration::Location::Default;
	else if (_node["storageLocation"].asString() == "storage")
		loc = VariableDeclaration::Location::Storage;
	else if (_node["storageLocation"].asString() == "memory")
		loc = VariableDeclaration::Location::Memory;
	else
		solAssert(false, "unknown location declaration");
	return loc;
}

ContractDefinition::ContractKind ASTJsonImporter::contractKind(Json::Value const& _node)
{
	ContractDefinition::ContractKind kind;
	solAssert(_node.isMember("contractKind") && !_node["contractKind"].isNull(), "");
	if (_node["contractKind"].asString() == "interface")
		kind = ContractDefinition::ContractKind::Interface;
	else if (_node["contractKind"].asString() == "contract")
		kind = ContractDefinition::ContractKind::Contract;
	else if (_node["contractKind"].asString() == "library")
		kind = ContractDefinition::ContractKind::Library;
	else
		solAssert(false, "unknown ContractKind ");
	return kind;
}

Literal::SubDenomination ASTJsonImporter::subdenomination(Json::Value const& _node)
{
	Literal::SubDenomination kind;
	solAssert(_node.isMember("subdenomination"), "");
	if (_node["subdenomination"].isNull())
		kind = Literal::SubDenomination::None;
	else if (_node["subdenomination"].asString() == "wei")
		kind = Literal::SubDenomination::Wei;
	else if (_node["subdenomination"].asString() == "szabo")
		kind = Literal::SubDenomination::Szabo;
	else if (_node["subdenomination"].asString() == "finney")
		kind = Literal::SubDenomination::Finney;
	else if (_node["subdenomination"].asString() == "ether")
		kind = Literal::SubDenomination::Ether;
	else if (_node["subdenomination"].asString() == "second")
		kind = Literal::SubDenomination::Second;
	else if (_node["subdenomination"].asString() == "hour")
		kind = Literal::SubDenomination::Hour;
	else if (_node["subdenomination"].asString() == "day")
		kind = Literal::SubDenomination::Day;
	else if (_node["subdenomination"].asString() == "week")
		kind = Literal::SubDenomination::Week;
	else if (_node["subdenomination"].asString() == "year")
		kind = Literal::SubDenomination::Year;
	else
		solAssert(false, "unknown subdenomination");
	return kind;
}
