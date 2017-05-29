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
 * Converts an AST from json format to an ASTNode
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
	if (!ret)
		solAssert(false, "");
	else
		return ret;
}
}

namespace dev
{
namespace solidity
{

ASTJsonImporter::ASTJsonImporter(map<string, Json::Value const*> _sourceList )
        : m_sourceList(_sourceList)
{
}

map<string, ASTPointer<SourceUnit>> ASTJsonImporter::jsonToSourceUnit()
{
	for (auto const& srcPair: m_sourceList)
	{
//		solAssert(!m_json->isNull(),"");
		solAssert(!srcPair.second->isNull(), "");
		m_currentSource = srcPair.first;
		m_sourceUnits[srcPair.first] =  createSourceUnit(*srcPair.second);
	}
	return m_sourceUnits;
}

SourceLocation const ASTJsonImporter::createSourceLocation(Json::Value const& _node)
{
	solAssert(!_node["src"].isNull(), "JsonValue should not be an ASTNode");
        string srcString = _node["src"].asCString();
        vector<string> pos;
        boost::algorithm::split(pos, srcString, boost::is_any_of(":"));
        int start = stoi(pos[0]);
	int end = start + stoi(pos[1]);
	shared_ptr<string const> source = make_shared<string const>(m_currentSource);
        return SourceLocation(
                start,
                end,
                source
        );
}

//helper functions
template<class T>
ASTPointer<T> ASTJsonImporter::nullOrCast(Json::Value _json)
{
	if(!_json.isNull())
		return castPointer<T>(convertJsonToASTNode(_json));
	else
		return nullptr;
}

Token::Value ASTJsonImporter::scanSingleToken(Json::Value _node)
{
        Token::Value ret;
	Scanner scanner(CharStream(_node.asCString()));
	ret = scanner.currentToken();
	if (scanner.next() == Token::EOS)
		return std::move(ret);
        else
	    BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("Invalid AST entry."));
}

ASTPointer<ASTNode> ASTJsonImporter::convertJsonToASTNode(Json::Value const& _json)
{
	if (_json["nodeType"] == "SourceUnit")
	    return createSourceUnit(_json);
	if (_json["nodeType"].asString() == "PragmaDirective")
	    return createPragmaDirective(_json);
	if (_json["nodeType"].asString() == "ImportDirective")
	    return createImportDirective(_json);
	if (_json["nodeType"].asString() == "ContractDefinition")
	    return createContractDefinition(_json);
	if (_json["nodeType"].asString() == "InheritanceSpecifier")
	    return createInheritanceSpecifier(_json);
	if (_json["nodeType"].asString() == "UsingForDirective")
		return createUsingForDirective(_json);
	if (_json["nodeType"].asString() == "StructDefinition")
		return createStructDefinition(_json);
	if (_json["nodeType"].asString() == "createEnumDefinition")
		return createEnumDefinition(_json);
	if (_json["nodeType"] == "EnumValue")
		return createEnumValue(_json);
	if (_json["nodeType"] == "ParameterList")
		return createParameterList(_json);
	if (_json["nodeType"] == "FunctionDefinition")
		return createFunctionDefinition(_json);
	if (_json["nodeType"] == "VariableDeclaration")
		return createVariableDeclaration(_json);
	if (_json["nodeType"] == "ModifierDefinition")
		return createModifierDefinition(_json);
	if (_json["nodeType"] == "ModifierInvocation")
		return createModifierInvocation(_json);
	if (_json["nodeType"] == "EventDefinition")
		return createEventDefinition(_json);
	if (_json["nodeType"] == "ElementaryTypeName")
		return createElementaryTypeName(_json);
	if (_json["nodeType"] == "UserDefinedTypeName")
		return createUserDefinedTypeName(_json);
	if (_json["nodeType"] == "FunctionTypeName")
		return createFunctionTypeName(_json);
	if (_json["nodeType"] == "Mapping")
		return createMapping(_json);
	if (_json["nodeType"] == "ArrayTypeName")
		return createArrayTypeName(_json);
//	if (_json["nodeType"] == "InlineAssembly")
//		return createInlineAssembly(_json);
	if (_json["nodeType"] == "Block")
		return createBlock(_json);
	if (_json["nodeType"] == "PlaceholderStatement")
		return createPlaceholderStatement(_json);
	if (_json["nodeType"] == "IfStatement")
		return createIfStatement(_json);
	if (_json["nodeType"] == "WhileStatement")
		return createWhileStatement(_json, false);
	if (_json["nodeType"] == "DoWhileStatement")
		return createWhileStatement(_json, true);
	if (_json["nodeType"] == "ForStatement")
		return createForStatement(_json);
	if (_json["nodeType"] == "Continue")
		return createContinue(_json);
	if (_json["nodeType"] == "Break")
		return createBreak(_json);
	if (_json["nodeType"] == "Return")
		return createReturn(_json);
	if (_json["nodeType"] == "Throw")
		return createThrow(_json);
	if (_json["nodeType"] == "VariableDeclarationStatement")
		return createVariableDeclarationStatement(_json);
	if (_json["nodeType"] == "ExpressionStatement")
		return createExpressionStatement(_json);
	if (_json["nodeType"] == "Conditional")
		return createConditional(_json);
	if (_json["nodeType"] == "Assignment")
		return createAssignment(_json);
	if (_json["nodeType"] == "TupleExpression")
		return createTupleExpression(_json);
	if (_json["nodeType"] == "UnaryOperation")
		return createUnaryOperation(_json);
	if (_json["nodeType"] == "BinaryOperation")
		return createBinaryOperation(_json);
	if (_json["nodeType"] == "FunctionCall")
		return createFunctionCall(_json);
	if (_json["nodeType"] == "NewExpression")
		return createNewExpression(_json);
	if (_json["nodeType"] == "MemberAccess")
		return createMemberAccess(_json);
	if (_json["nodeType"] == "IndexAccess")
		return createIndexAccess(_json);
	if (_json["nodeType"] == "Identifier")
		return createIdentifier(createSourceLocation(_json), _json["nodeType"].asCString());
	if (_json["nodeType"] == "ElementaryTypeNameExpression")
		return createElementaryTypeNameExpression(_json);
	if (_json["nodeType"] == "Literal")
		return createLiteral(_json);
	else BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("type of JsonValue is unknown."));
}

ASTPointer<SourceUnit> ASTJsonImporter::createSourceUnit(Json::Value const& _node)
{
	SourceLocation const& location = createSourceLocation(_node);
	vector<ASTPointer<ASTNode>> nodes;
	for (auto& child: _node["nodes"])
		nodes.push_back(convertJsonToASTNode(child));
	ASTPointer<SourceUnit> tmp = make_shared<SourceUnit>(location, nodes);
	tmp->annotation().path = m_currentSource;
	tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<PragmaDirective> ASTJsonImporter::createPragmaDirective(Json::Value const& _node) //2REVIEW
{
	vector<Token::Value> tokens;
	vector<ASTString> literals;
	for (auto const& lit: _node["literals"])
	{
                string l = lit.asCString();
                literals.push_back(l);
                tokens.push_back(scanSingleToken(l));
	}
	SourceLocation const& location = createSourceLocation(_node);
	ASTPointer<PragmaDirective> tmp = make_shared<PragmaDirective>(location, tokens, literals);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ImportDirective> ASTJsonImporter::createImportDirective(Json::Value const& _node){
	//create args for the fields of the node
	SourceLocation const& location = createSourceLocation(_node);
	ASTPointer<ASTString> const& path = make_shared<ASTString>(_node["file"].asCString());
	ASTPointer<ASTString> const& unitAlias = make_shared<ASTString>(_node["unitAlias"].asCString());
	vector<pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> symbolAliases;
	for (auto& tuple: _node["symbolAliases"])
	{
                symbolAliases.push_back( make_pair(
			createIdentifier(location, tuple["foreign"].asCString()), //ATTENTION! is this the correct sourcelocation?? where to get src-location from?
			tuple["local"].isNull() ? make_shared<ASTString>(tuple["local"].asCString()) : nullptr //also check if that's not null?
		));
	}
	//create node
	ASTPointer<ImportDirective> tmp = make_shared<ImportDirective>(location, path, unitAlias, move(symbolAliases));
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ContractDefinition> ASTJsonImporter::createContractDefinition(Json::Value const& _node){
	//create args for the constructor of the node
	SourceLocation const& location = createSourceLocation(_node);
	ASTPointer<ASTString> documentation = _node["documentation"].isNull() ?
				nullptr :
				make_shared<ASTString>(_node["documentation"].asCString()
	);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asCString());
	std::vector<ASTPointer<InheritanceSpecifier>> baseContracts;
	for (auto& base : _node["baseContracts"])
		baseContracts.push_back(createInheritanceSpecifier(base));
	std::vector<ASTPointer<ASTNode>> subNodes;
	for (auto& subnode : _node["nodes"])
		subNodes.push_back(convertJsonToASTNode(subnode));
	ContractDefinition::ContractKind contractKind = getContractKind(_node);
	//create node
	ASTPointer<ContractDefinition> tmp = make_shared<ContractDefinition>(
		location,
		name,
		documentation,
		baseContracts,
		subNodes,
		contractKind
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<InheritanceSpecifier> ASTJsonImporter::createInheritanceSpecifier(Json::Value const& _node)
{
	//create args for the constructor of the node && see how they are filled in parser.cpp
	SourceLocation const& location = createSourceLocation(_node);
	ASTPointer<UserDefinedTypeName> baseName = createUserDefinedTypeName(_node["baseName"]);
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg : _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	//create node
	ASTPointer<InheritanceSpecifier> tmp = make_shared<InheritanceSpecifier>(
		location,
		baseName,
		arguments
	);
	tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<UsingForDirective> ASTJsonImporter::createUsingForDirective(Json::Value const& _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<UserDefinedTypeName> libraryName = createUserDefinedTypeName(_node["libraryName"]);
	ASTPointer<TypeName> typeName;
	if (!_node["typename"].isNull())
		typeName = castPointer<TypeName>(convertJsonToASTNode(_node["typename"]));
	else
		typeName = nullptr;
	ASTPointer<UsingForDirective> tmp = make_shared<UsingForDirective>(
		location,
		libraryName,
		typeName
	);
	//tmp.setID
	return tmp;
}

ASTPointer<ASTNode> ASTJsonImporter::createStructDefinition(Json::Value const& _node)
{
	SourceLocation const& location = createSourceLocation(_node);
	ASTPointer<ASTString> const& name = make_shared<ASTString>(_node["name"].asCString());
	std::vector<ASTPointer<VariableDeclaration>> members;
	for (auto& member: _node["members"])
		members.push_back(createVariableDeclaration(member));
	ASTPointer<StructDefinition> tmp = make_shared<StructDefinition>(
		location,
		name,
		members
	);
	return make_shared<Identifier>(createSourceLocation(_node), make_shared<ASTString>("placeholder"));

}

ASTPointer<EnumDefinition> ASTJsonImporter::createEnumDefinition(Json::Value const& _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asCString());
	std::vector<ASTPointer<EnumValue>> members;
	for (auto& member: _node["members"])
		members.push_back(createEnumValue(member));
	ASTPointer<EnumDefinition> tmp = make_shared<EnumDefinition>(
		location,
		name,
		members
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<EnumValue> ASTJsonImporter::createEnumValue(Json::Value const& _node)
{
	SourceLocation const& location = createSourceLocation(_node);
	ASTPointer<ASTString> const& name = make_shared<ASTString>(_node["name"].asCString());
	ASTPointer<EnumValue> tmp = make_shared<EnumValue>(
		location,
		name
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ParameterList> ASTJsonImporter::createParameterList(Json::Value const&  _node)
{
	SourceLocation const& location = createSourceLocation(_node);
	std::vector<ASTPointer<VariableDeclaration>> parameters;
	for (auto& param: _node["parameters"])
		parameters.push_back(createVariableDeclaration(param));
	ASTPointer<ParameterList> tmp = make_shared<ParameterList>(
		location,
		parameters
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<FunctionDefinition> ASTJsonImporter::createFunctionDefinition(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asCString());
	Declaration::Visibility visibility = getVisibility(_node);
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
	ASTPointer<FunctionDefinition> tmp = make_shared<FunctionDefinition>(
		location,
		name,
		visibility,
		isConstructor,
		documentation,
		parameters,
		isDeclaredConst,
		modifiers,
		returnParameters,
		isPayable,
		body
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<VariableDeclaration> ASTJsonImporter::createVariableDeclaration(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<TypeName> type = nullOrCast<TypeName>(_node["typeName"]);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asCString());
	ASTPointer<Expression> value = nullOrCast<Expression>(_node["value"]);
	Declaration::Visibility visibility = getVisibility(_node);
	bool isStateVar = _node["stateVariable"].asBool();
	bool isIndexed = _node["indexed"].asBool();
	bool isConstant = _node["constant"].asBool();
	VariableDeclaration::Location referenceLocation = getLocation(_node);
	ASTPointer<VariableDeclaration> tmp = make_shared<VariableDeclaration>(
		location,
		type,
		name,
	        value,
		visibility,
		isStateVar,
		isIndexed,
		isConstant,
		referenceLocation
	);
	tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ModifierDefinition> ASTJsonImporter::createModifierDefinition(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asCString());
	ASTPointer<ASTString> documentation = make_shared<ASTString>(""); //postponed
	ASTPointer<ParameterList> parameters = createParameterList(_node["parameters"]);
	ASTPointer<Block> body = createBlock(_node["body"]);
	ASTPointer<ModifierDefinition> tmp = make_shared<ModifierDefinition>(
		location,
		name,
		documentation,
		parameters,
		body
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ModifierInvocation> ASTJsonImporter::createModifierInvocation(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Identifier> name = createIdentifier(location, _node["name"].asCString());
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	ASTPointer<ModifierInvocation> tmp = make_shared<ModifierInvocation>(
		location,
		name,
		arguments
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<EventDefinition> ASTJsonImporter::createEventDefinition(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asCString());
	ASTPointer<ASTString> documentation = make_shared<ASTString>(""); //postponed
	ASTPointer<ParameterList> parameters = createParameterList(_node["parameters"]);
	bool anonymous = _node["anonymous"].asBool();
	ASTPointer<EventDefinition> tmp = make_shared<EventDefinition>(
		location,
		name,
		documentation,
		parameters,
		anonymous
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ElementaryTypeName> ASTJsonImporter::createElementaryTypeName(Json::Value const& _node)
{
	SourceLocation location = createSourceLocation(_node);
	unsigned short firstNum;
	unsigned short secondNum;
	string name = _node["name"].asCString();
	Token::Value token;
	tie(token, firstNum, secondNum) = Token::fromIdentifierOrKeyword(name);
	ElementaryTypeNameToken elem(token, firstNum,  secondNum);
	ASTPointer<ElementaryTypeName> tmp = make_shared<ElementaryTypeName>(
		location,
		elem
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<UserDefinedTypeName> ASTJsonImporter::createUserDefinedTypeName(Json::Value const& _node)
{
	SourceLocation location = createSourceLocation(_node);
	vector<ASTString> namePath;
	vector<string> strs;
	string nameString = _node["name"].asCString();
	boost::algorithm::split(strs, nameString, boost::is_any_of("."));
	for (string s : strs)
		namePath.push_back(ASTString(s));
	ASTPointer<UserDefinedTypeName> tmp = make_shared<UserDefinedTypeName>(
		location,
		namePath
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<FunctionTypeName> ASTJsonImporter::createFunctionTypeName(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ParameterList> parameterTypes = createParameterList(_node["parameterTypes"]);
	ASTPointer<ParameterList> returnTypes = createParameterList(_node["returnParameterTypes"]);
	Declaration::Visibility visibility = getVisibility(_node);
	bool isDeclaredConst = _node["isDeclaredConst"].asBool();
	bool isPayable = _node["payable"].asBool();
	ASTPointer<FunctionTypeName> tmp = make_shared<FunctionTypeName>(
		location,
		parameterTypes,
		returnTypes,
		visibility,
		isDeclaredConst,
		isPayable
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Mapping> ASTJsonImporter::createMapping(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ElementaryTypeName> keyType = createElementaryTypeName(_node["keyType"]);
	ASTPointer<TypeName> valueType = castPointer<TypeName>(convertJsonToASTNode(_node["valueType"]));
	ASTPointer<Mapping> tmp = make_shared<Mapping>(
		location,
		keyType,
		valueType
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ArrayTypeName> ASTJsonImporter::createArrayTypeName(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<TypeName> baseType = castPointer<TypeName>(convertJsonToASTNode(_node));
	ASTPointer<Expression> length = nullOrCast<Expression>(_node["length"]);
	ASTPointer<ArrayTypeName> tmp = make_shared<ArrayTypeName>(
		location,
		baseType,
		length
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

//ASTPointer<InlineAssembly> ASTJsonImporter::createInlineAssembly(Json::Value const&  _node) //TODO LATER
//{
//	return make_shared<Identifier>(createSourceLocation(_node), make_shared<ASTString>("placeholder"));
//}
ASTPointer<Block> ASTJsonImporter::createBlock(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	std::vector<ASTPointer<Statement>> statements;
	for (auto& stat: _node["statements"])
		statements.push_back(castPointer<Statement>(convertJsonToASTNode(stat)));
	ASTPointer<Block> tmp = make_shared<Block>(
		location,
		docString,
		statements
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<PlaceholderStatement> ASTJsonImporter::createPlaceholderStatement(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<PlaceholderStatement> tmp = make_shared<PlaceholderStatement>(
		location,
		docString
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<IfStatement> ASTJsonImporter::createIfStatement(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(_node["condition"]));
	ASTPointer<Statement> trueBody = castPointer<Statement>(convertJsonToASTNode(_node["trueBody"]));
	ASTPointer<Statement> falseBody = nullOrCast<Statement>(_node["falseBody"]);
	ASTPointer<IfStatement> tmp = make_shared<IfStatement>(
		location,
		docString,
		condition,
		trueBody,
		falseBody
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<WhileStatement> ASTJsonImporter::createWhileStatement(Json::Value const&  _node, bool _isDoWhile=false)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(_node["condition"]));
	ASTPointer<Statement> body = castPointer<Statement>(convertJsonToASTNode(_node["Body"]));
	ASTPointer<WhileStatement> tmp = make_shared<WhileStatement>(
		location,
		docString,
		condition,
		body,
		_isDoWhile
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ForStatement> ASTJsonImporter::createForStatement(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Statement> initExpression = nullOrCast<Statement>(_node["initExpression"]);
	ASTPointer<Expression> conditionExpression = nullOrCast<Expression>(_node["condition"]);
	ASTPointer<ExpressionStatement> loopExpression = nullOrCast<ExpressionStatement>(_node["loopExpression"]);
	ASTPointer<Statement> body = castPointer<Statement>(convertJsonToASTNode(_node["body"]));
	ASTPointer<ForStatement> tmp = make_shared<ForStatement>(
		location,
		docString,
		initExpression,
		conditionExpression,
		loopExpression,
		body
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Continue> ASTJsonImporter::createContinue(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Continue> tmp = make_shared<Continue>(location, docString);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Break> ASTJsonImporter::createBreak(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Break> tmp = make_shared<Break>(location, docString);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Return> ASTJsonImporter::createReturn(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	ASTPointer<Return> tmp = make_shared<Return>(
		location,
		docString,
		expression
	);
	//sretID
	return tmp;
}

ASTPointer<Throw> ASTJsonImporter::createThrow(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Throw> tmp = make_shared<Throw>(location, docString);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<VariableDeclarationStatement> ASTJsonImporter::createVariableDeclarationStatement(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	std::vector<ASTPointer<VariableDeclaration>> variables;
	for (auto& var: _node["declarations"])
		variables.push_back(createVariableDeclaration(var));
	ASTPointer<Expression> initialValue = nullOrCast<Expression>(_node["initialValue"]);
	ASTPointer<VariableDeclarationStatement> tmp = make_shared<VariableDeclarationStatement>(
		location,
		docString,
		variables,
		initialValue
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<ExpressionStatement> ASTJsonImporter::createExpressionStatement(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<ASTString> docString = make_shared<ASTString>(""); //postponed
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	ASTPointer<ExpressionStatement> tmp = make_shared<ExpressionStatement>(
		location,
		docString,
		expression
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Conditional> ASTJsonImporter::createConditional(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(_node["condition"]));
	ASTPointer<Expression> trueExpression = castPointer<Expression>(convertJsonToASTNode(_node["trueExpression"]));
	ASTPointer<Expression> falseExpression = castPointer<Expression>(convertJsonToASTNode(_node["falseExpression"]));
	ASTPointer<Conditional> tmp = make_shared<Conditional>(
		location,
		condition,
		trueExpression,
		falseExpression
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Assignment> ASTJsonImporter::createAssignment(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Expression> leftHandSide = castPointer<Expression>(convertJsonToASTNode(_node["leftHandSide"]));
        Token::Value assignmentOperator = scanSingleToken(_node["operator"]);
	ASTPointer<Expression> rightHandSide = castPointer<Expression>(convertJsonToASTNode(_node["rightHandSide"]));
	ASTPointer<Assignment> tmp = make_shared<Assignment>(
		location,
		leftHandSide,
		assignmentOperator,
		rightHandSide
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<TupleExpression> ASTJsonImporter::createTupleExpression(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	std::vector<ASTPointer<Expression>> components;
	for (auto& comp: _node["components"])
		components.push_back(castPointer<Expression>(convertJsonToASTNode(comp)));
	bool isArray = _node["isInlineArray"].asBool();
	ASTPointer<TupleExpression> tmp = make_shared<TupleExpression>(
		location,
		components,
		isArray
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<UnaryOperation> ASTJsonImporter::createUnaryOperation(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
        Token::Value operation = scanSingleToken(_node["operator"]);
	ASTPointer<Expression> subExpression = castPointer<Expression>(convertJsonToASTNode(_node["subExpression"]));
	bool prefix = _node["prefix"].asBool();
	ASTPointer<UnaryOperation> tmp = make_shared<UnaryOperation>(
		location,
		operation,
		subExpression,
		prefix
	);
        tmp->setID(_node["id"].asInt());
	return tmp;

}

ASTPointer<BinaryOperation> ASTJsonImporter::createBinaryOperation(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Expression> left = castPointer<Expression>(convertJsonToASTNode(_node["leftExpression"]));
        Token::Value operation = scanSingleToken(_node["operator"]);
//	Token::Value operation;
//	ASTPointer<Scanner> scanner = make_shared<Scanner>(CharStream(_node["operator"].asCString()), "tmp");
//	operation = scanner->next();
	ASTPointer<Expression> right = castPointer<Expression>(convertJsonToASTNode(_node["rightExpression"]));
	ASTPointer<BinaryOperation> tmp = make_shared<BinaryOperation>(
		location,
		left,
		operation,
		right
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<FunctionCall> ASTJsonImporter::createFunctionCall(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: _node["names"])
		names.push_back(make_shared<ASTString>(name.asCString()));
	ASTPointer<FunctionCall> tmp = make_shared<FunctionCall>(
		location,
		expression,
		arguments,
		names
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<NewExpression> ASTJsonImporter::createNewExpression(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<TypeName> typeName = castPointer<TypeName>(convertJsonToASTNode(_node["typeName"]));
	ASTPointer<NewExpression> tmp = make_shared<NewExpression>(
		location,
		typeName
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<MemberAccess> ASTJsonImporter::createMemberAccess(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(_node["expression"]));
	ASTPointer<ASTString> memberName = make_shared<ASTString>(_node["memberName"].asCString());
	ASTPointer<MemberAccess> tmp = make_shared<MemberAccess>(
		location,
		expression,
		memberName
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<IndexAccess> ASTJsonImporter::createIndexAccess(Json::Value const&  _node)
{
	SourceLocation location = createSourceLocation(_node);
	ASTPointer<Expression> base = castPointer<Expression>(convertJsonToASTNode(_node["baseExpression"]));
	ASTPointer<Expression> index = castPointer<Expression>(convertJsonToASTNode(_node["indexExpression"]));
	ASTPointer<IndexAccess> tmp = make_shared<IndexAccess>(
		location,
		base,
		index
	);
        tmp->setID(_node["id"].asInt());
	return tmp;
}

ASTPointer<Identifier> ASTJsonImporter::createIdentifier(SourceLocation location, string const& name)
{
	return make_shared<Identifier>(location, make_shared<ASTString>(name));
}

ASTPointer<ElementaryTypeNameExpression> ASTJsonImporter::createElementaryTypeNameExpression(Json::Value const&  _node) //needs TEST (elem)
{
	SourceLocation location = createSourceLocation(_node);
	Scanner scanner(CharStream(_node["typeDescriptions"]["typeString"].asCString()), "tmp");
	Token::Value token = scanner.currentToken();
	unsigned firstSize;
	unsigned secondSize;
	tie(firstSize, secondSize) = scanner.currentTokenInfo();
	ElementaryTypeNameToken elem(token, firstSize, secondSize);
	ASTPointer<ElementaryTypeNameExpression> tmp = make_shared<ElementaryTypeNameExpression>(
		location,
		elem
	);
        tmp->setID(_node["id"].asInt());
	return tmp;

}

ASTPointer<ASTNode> ASTJsonImporter::createLiteral(Json::Value const&  _node) //TODO
{
	SourceLocation location = createSourceLocation(_node);
	Token::Value token = getLiteralTokenKind(_node);
	ASTPointer<ASTString> value = make_shared<ASTString>(
				_node["value"].isNull() ?
				"todo" :
				_node["value"].asCString()
	);
	Literal::SubDenomination sub = getSubdenomination(_node);
	ASTPointer<Literal> tmp = make_shared<Literal>(
		location,
		token,
		value,
		sub
	);
	tmp->setID(_node["id"].asInt());
	return tmp;
}

Token::Value ASTJsonImporter::getLiteralTokenKind(Json::Value const& _node)
{
	solAssert(_node.isMember("kind") && !_node["kind"].isNull(), "");
	Token::Value tok;
	if (strcmp(_node["kind"].asCString(), "number") == 0)
		tok = Token::Number;
	else if (strcmp(_node["kind"].asCString(), "string") == 0)
		tok = Token::StringLiteral;
	else if (strcmp(_node["kind"].asCString(), "bool") == 0)
		 tok = (strcmp(_node["value"].asCString(), "true") == 0) ? Token::TrueLiteral : Token::FalseLiteral;
	else
		solAssert(false, "unknown kind of literalString");
	return tok;

}

Declaration::Visibility ASTJsonImporter::getVisibility(Json::Value const& _node)
{
	solAssert(_node.isMember("visibility") && !_node["visibility"].isNull(), "");
	Declaration::Visibility vis;
	if (strcmp(_node["visibility"].asCString(), "default") == 0)
		vis = Declaration::Visibility::Default;
	else if (strcmp(_node["visibility"].asCString(), "private") == 0)
		vis = Declaration::Visibility::Private;
	else if (strcmp( _node["visibility"].asCString(), "internal") == 0)
		vis = Declaration::Visibility::Internal;
	else if (strcmp(_node["visibility"].asCString(), "public") == 0)
		vis = Declaration::Visibility::Public;
	else if (strcmp(_node["visibility"].asCString(), "external") == 0)
		vis = Declaration::Visibility::External;
	else
		solAssert(false, "unknown visibility declaration");
	return vis;
}

VariableDeclaration::Location ASTJsonImporter::getLocation(Json::Value const& _node)
{
	VariableDeclaration::Location loc;
	solAssert(_node.isMember("storageLocation") && !_node["storageLocation"].isNull(), "");
	if (strcmp(_node["storageLocation"].asCString(), "default") == 0)
		loc = VariableDeclaration::Location::Default;
	else if (strcmp(_node["storageLocation"].asCString(), "storage") == 0)
		loc = VariableDeclaration::Location::Storage;
	else if (strcmp(_node["storageLocation"].asCString(), "memory") == 0)
		loc = VariableDeclaration::Location::Memory;
	else
		solAssert(false, "unknown location declaration");
	return loc;
}

ContractDefinition::ContractKind ASTJsonImporter::getContractKind(Json::Value const& _node)
{
	ContractDefinition::ContractKind kind;
	solAssert(_node.isMember("contractKind") && !_node["contractKind"].isNull(), "");
	if (strcmp(_node["contractKind"].asCString(), "interface") == 0)
		kind = ContractDefinition::ContractKind::Interface;
	else if (strcmp(_node["contractKind"].asCString(), "contract") == 0)
		kind = ContractDefinition::ContractKind::Contract;
	else if (strcmp(_node["contractKind"].asCString(), "library") == 0)
		kind = ContractDefinition::ContractKind::Library;
	else
		solAssert(false, "unknown ContractKind ");
	return kind;
}

Literal::SubDenomination ASTJsonImporter::getSubdenomination(Json::Value const& _node)
{
	Literal::SubDenomination kind;
	solAssert(_node.isMember("subdenomination"), "");
	if (_node["subdenomination"].isNull())
		kind = Literal::SubDenomination::None;
	else if (strcmp(_node["subdenomination"].asCString(), "wei") == 0)
		kind = Literal::SubDenomination::Wei;
	else if (strcmp(_node["subdenomination"].asCString(), "szabo") == 0)
		kind = Literal::SubDenomination::Szabo;
	else if (strcmp(_node["subdenomination"].asCString(), "finney") == 0)
		kind = Literal::SubDenomination::Finney;
	else if (strcmp(_node["subdenomination"].asCString(), "ether") == 0)
		kind = Literal::SubDenomination::Ether;
	else if (strcmp(_node["subdenomination"].asCString(), "second") == 0)
		kind = Literal::SubDenomination::Second;
	else if (strcmp(_node["subdenomination"].asCString(), "hour") == 0)
		kind = Literal::SubDenomination::Hour;
	else if (strcmp(_node["subdenomination"].asCString(), "day") == 0)
		kind = Literal::SubDenomination::Day;
	else if (strcmp(_node["subdenomination"].asCString(), "week") == 0)
		kind = Literal::SubDenomination::Week;
	else if (strcmp(_node["subdenomination"].asCString(), "year") == 0)
		kind = Literal::SubDenomination::Year;
	else
		solAssert(false, "unknown subdenomination");
	return kind;
}


}
}
