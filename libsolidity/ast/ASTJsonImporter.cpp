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
#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/interface/ErrorReporter.h>

using namespace std;

using namespace dev;
using namespace solidity;

template<class T>
ASTPointer<T> castPointer(ASTPointer<ASTNode> _ast)
{
	ASTPointer<T> ret = dynamic_pointer_cast<T>(_ast);
	astAssert(ret, "");
	return ret;
}


template <typename T, typename... Args>
ASTPointer<T> ASTJsonImporter::createASTNode(Json::Value const& _node, Args&&... _args)
{
	astAssert(!member(_node, "id").isNull(), "'id'-field must not be null.");
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
		astAssert(!srcPair.second->isNull(), "");
		astAssert(member(*srcPair.second,"nodeType") == "SourceUnit", "The 'nodeType' of the highest node must be 'SourceUnit'.");
		m_currentSourceName = srcPair.first;
		m_sourceUnits[srcPair.first] =  createSourceUnit(*srcPair.second, srcPair.first);
	}
	return m_sourceUnits;
}

SourceLocation const ASTJsonImporter::createSourceLocation(Json::Value const& _node)
{
	astAssert(!member(_node, "src").isNull(), "'src' can not be null");
	string srcString = _node["src"].asString();
	vector<string> pos;
	boost::algorithm::split(pos, srcString, boost::is_any_of(":"));
	if (pos.size() != 3 || int(m_sourceLocations.size()) < stoi(pos[2]))
		BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("'src'-field ill-formatted or src-index too high"));
	int start = stoi(pos[0]);
	int end = start + stoi(pos[1]);
	return SourceLocation(
		start,
		end,
		m_sourceLocations[stoi(pos[2])]
	);
}

//helper functions
ASTPointer<ASTString> ASTJsonImporter::memberAsASTString(Json::Value const& _node, string const& _name)
{
	Json::Value value = member(_node, _name);
	astAssert(value.isString(), "field " + _name + " must be of type string.");
	return make_shared<ASTString>(_node[_name].asString()); }

bool ASTJsonImporter::memberAsBool(Json::Value const& _node, string const& _name)
{

	Json::Value value = member(_node, _name);
	astAssert(value.isBool(), "field " + _name + " must be of type boolean.");
	return _node[_name].asBool();
}

Json::Value ASTJsonImporter::member(Json::Value const& _node, string const& _name)
{
	astAssert(_node.isMember(_name), "Node '" + _node["nodeType"].asString() + "' (id " + _node["id"].asString() + ") is missing field '" + _name + "'.");
	return _node[_name];
}

template<class T>
ASTPointer<T> ASTJsonImporter::nullOrCast(Json::Value _json)
{
	if (_json.isNull())
		return nullptr;
	else
		return castPointer<T>(convertJsonToASTNode(_json));
}

ASTPointer<ASTString> ASTJsonImporter::nullOrASTString(Json::Value _json, string const& _name){
	return _json[_name].isString() ? memberAsASTString(_json, _name) : nullptr;
}

Token::Value ASTJsonImporter::scanSingleToken(Json::Value _node)
{
	Scanner scanner(CharStream(_node.asString()));
	astAssert(scanner.peekNextToken() == Token::EOS, "Token string is too long.");
	return scanner.currentToken();
}

ASTPointer<ASTNode> ASTJsonImporter::convertJsonToASTNode(Json::Value const& _json)
{
	astAssert(_json.isMember("nodeType") && _json.isMember("id"), "JSON-Node needs to have 'nodeType' and 'id' fields.");
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
	if (nodeType == "EnumDefinition")
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
	if (_json["nodeType"] == "InlineAssembly")
		return createInlineAssembly(_json);
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
		return createIdentifier(_json);
	if (nodeType == "ElementaryTypeNameExpression")
		return createElementaryTypeNameExpression(_json);
	if (nodeType == "Literal")
		return createLiteral(_json);
	else
		BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("Unknown type of ASTNode."));
}

ASTPointer<SourceUnit> ASTJsonImporter::createSourceUnit(Json::Value const& _node, string const& _srcName)
{
	vector<ASTPointer<ASTNode>> nodes;
	for (auto& child: member(_node, "nodes"))
		nodes.emplace_back(convertJsonToASTNode(child));
	ASTPointer<SourceUnit> tmp = createASTNode<SourceUnit>(_node, nodes);
	tmp->annotation().path = _srcName;
	return tmp;
}

ASTPointer<PragmaDirective> ASTJsonImporter::createPragmaDirective(Json::Value const& _node)
{
	vector<Token::Value> tokens;
	vector<ASTString> literals;
	for (auto const& lit: member(_node, "literals"))
	{
		string l = lit.asString();
		literals.push_back(l);
		tokens.push_back(scanSingleToken(l));
	}
	return createASTNode<PragmaDirective>(_node, tokens, literals);
}

ASTPointer<ImportDirective> ASTJsonImporter::createImportDirective(Json::Value const& _node){
	ASTPointer<ASTString> unitAlias = memberAsASTString(_node, "unitAlias");
	ASTPointer<ASTString> path = memberAsASTString(_node, "file");
	vector<pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> symbolAliases;
	for (auto& tuple: _node["symbolAliases"])
	{
                symbolAliases.push_back( make_pair(
			createIdentifier(tuple["foreign"]),
			tuple["local"].isNull() ? nullptr : make_shared<ASTString>(tuple["local"].asString())
		));
	}
	ASTPointer<ImportDirective> tmp = createASTNode<ImportDirective>(
		_node,
		path,
		unitAlias,
		move(symbolAliases)
	);
	tmp->annotation().absolutePath = _node["absolutePath"].asString();
	return tmp;
}

ASTPointer<ContractDefinition> ASTJsonImporter::createContractDefinition(Json::Value const& _node){
	std::vector<ASTPointer<InheritanceSpecifier>> baseContracts;
	for (auto& base : _node["baseContracts"])
		baseContracts.push_back(createInheritanceSpecifier(base));
	std::vector<ASTPointer<ASTNode>> subNodes;
	for (auto& subnode : _node["nodes"])
		subNodes.push_back(convertJsonToASTNode(subnode));
	return createASTNode<ContractDefinition>(
		_node,
		make_shared<ASTString>(_node["name"].asString()),
		nullOrASTString(_node, "documentation"),
		baseContracts,
		subNodes,
		contractKind(_node)
	);
}

ASTPointer<InheritanceSpecifier> ASTJsonImporter::createInheritanceSpecifier(Json::Value const& _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg : _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	return createASTNode<InheritanceSpecifier>(
		_node,
		createUserDefinedTypeName(_node["baseName"]),
		arguments
	);

}

ASTPointer<UsingForDirective> ASTJsonImporter::createUsingForDirective(Json::Value const& _node)
{
	return createASTNode<UsingForDirective>(
		_node,
		createUserDefinedTypeName(member(_node, "libraryName")),
		!_node["typename"].isNull() ?  castPointer<TypeName>(convertJsonToASTNode(_node["typename"])) : nullptr
	);

}

ASTPointer<ASTNode> ASTJsonImporter::createStructDefinition(Json::Value const& _node)
{
	std::vector<ASTPointer<VariableDeclaration>> members;
	for (auto& member: _node["members"])
		members.push_back(createVariableDeclaration(member));
	return createASTNode<StructDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		members
	);

}

ASTPointer<EnumDefinition> ASTJsonImporter::createEnumDefinition(Json::Value const& _node)
{
	std::vector<ASTPointer<EnumValue>> members;
	for (auto& member: _node["members"])
		members.push_back(createEnumValue(member));
	return createASTNode<EnumDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		members
	);

}

ASTPointer<EnumValue> ASTJsonImporter::createEnumValue(Json::Value const& _node)
{
	return createASTNode<EnumValue>(
		_node,
		memberAsASTString(_node, "name")
	);

}

ASTPointer<ParameterList> ASTJsonImporter::createParameterList(Json::Value const&  _node)
{
	std::vector<ASTPointer<VariableDeclaration>> parameters;
	for (auto& param: _node["parameters"])
		parameters.push_back(createVariableDeclaration(param));
	return createASTNode<ParameterList>(
		_node,
		parameters
	);

}

ASTPointer<FunctionDefinition> ASTJsonImporter::createFunctionDefinition(Json::Value const&  _node)
{
	std::vector<ASTPointer<ModifierInvocation>> modifiers;
	for (auto& mod: member(_node, "modifiers"))
		modifiers.push_back(createModifierInvocation(mod));
	return createASTNode<FunctionDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		visibility(_node),
		stateMutability(_node),
		memberAsBool(_node, "isConstructor"),
		nullOrASTString(_node, "documentation"),
		createParameterList(member(_node, "parameters")),
		modifiers,
		createParameterList(member(_node, "returnParameters")),
		memberAsBool(_node, "implemented") ? createBlock(member(_node, "body")) : nullptr
	);

}

ASTPointer<VariableDeclaration> ASTJsonImporter::createVariableDeclaration(Json::Value const& _node)
{
	return createASTNode<VariableDeclaration>(
		_node,
		nullOrCast<TypeName>(member(_node, "typeName")),
		make_shared<ASTString>(member(_node, "name").asString()),
		nullOrCast<Expression>(member(_node, "value")),
		visibility(_node),
		memberAsBool(_node, "stateVariable"),
		_node.isMember("indexed") ? memberAsBool(_node, "indexed") : false,
		memberAsBool(_node, "constant"),
		location(_node)
	);

}

ASTPointer<ModifierDefinition> ASTJsonImporter::createModifierDefinition(Json::Value const&  _node)
{
	return createASTNode<ModifierDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		nullOrASTString(_node,"documentation"),
		createParameterList(member(_node, "parameters")),
		createBlock(member(_node, "body"))
	);

}

ASTPointer<ModifierInvocation> ASTJsonImporter::createModifierInvocation(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	return createASTNode<ModifierInvocation>(
		_node,
		createIdentifier(member(_node, "modifierName")),
		arguments
	);

}

ASTPointer<EventDefinition> ASTJsonImporter::createEventDefinition(Json::Value const&  _node)
{
	return createASTNode<EventDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		nullOrASTString(_node, "documentation"),
		createParameterList(member(_node, "parameters")),
		memberAsBool(_node, "anonymous")
	);

}

ASTPointer<ElementaryTypeName> ASTJsonImporter::createElementaryTypeName(Json::Value const& _node)
{
	unsigned short firstNum;
	unsigned short secondNum;
	string name = member(_node, "name").asString();
	Token::Value token;
	tie(token, firstNum, secondNum) = Token::fromIdentifierOrKeyword(name);
	ElementaryTypeNameToken elem(token, firstNum,  secondNum);
	return createASTNode<ElementaryTypeName>(
		_node,
		elem
	);

}

ASTPointer<UserDefinedTypeName> ASTJsonImporter::createUserDefinedTypeName(Json::Value const& _node)
{
	vector<ASTString> namePath;
	vector<string> strs;
	string nameString = member(_node, "name").asString();
	boost::algorithm::split(strs, nameString, boost::is_any_of("."));
	for (string s : strs)
		namePath.push_back(ASTString(s));
	return createASTNode<UserDefinedTypeName>(
		_node,
		namePath
	);

}

ASTPointer<FunctionTypeName> ASTJsonImporter::createFunctionTypeName(Json::Value const&  _node)
{
	return createASTNode<FunctionTypeName>(
		_node,
		createParameterList(member(_node, "parameterTypes")),
		createParameterList(member(_node, "returnParameterTypes")),
		visibility(_node),
		stateMutability(_node)
	);

}

ASTPointer<Mapping> ASTJsonImporter::createMapping(Json::Value const&  _node)
{
	return createASTNode<Mapping>(
		_node,
		createElementaryTypeName(member(_node, "keyType")),
		castPointer<TypeName>(convertJsonToASTNode(member(_node, "valueType")))
	);

}

ASTPointer<ArrayTypeName> ASTJsonImporter::createArrayTypeName(Json::Value const&  _node)
{
	return createASTNode<ArrayTypeName>(
		_node,
		castPointer<TypeName>(convertJsonToASTNode(member(_node, "baseType"))),
		nullOrCast<Expression>(member(_node, "length"))
	);
}

ASTPointer<InlineAssembly> ASTJsonImporter::createInlineAssembly(Json::Value const& _node)
{
	ErrorList tmp_list;
	ErrorReporter tmp_error(tmp_list);
        assembly::Parser asmParser(tmp_error);
	shared_ptr<Scanner> scanner = make_shared<Scanner>(
		CharStream( member(_node, "operations").asString()),
		m_currentSourceName
	);
        std::shared_ptr<assembly::Block> operations = asmParser.parse(scanner, false);
	return createASTNode<InlineAssembly>(
		_node,
		nullOrASTString(_node, "documentation"),
		operations
	);
}

ASTPointer<Block> ASTJsonImporter::createBlock(Json::Value const&  _node)
{
	std::vector<ASTPointer<Statement>> statements;
	for (auto& stat: member(_node, "statements"))
		statements.push_back(castPointer<Statement>(convertJsonToASTNode(stat)));
	return createASTNode<Block>(
		_node,
		nullOrASTString(_node, "documentation"),
		statements
	);
}

ASTPointer<PlaceholderStatement> ASTJsonImporter::createPlaceholderStatement(Json::Value const&  _node)
{
	return createASTNode<PlaceholderStatement>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<IfStatement> ASTJsonImporter::createIfStatement(Json::Value const&  _node)
{
	return createASTNode<IfStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "condition"))),
		castPointer<Statement>(convertJsonToASTNode(member(_node, "trueBody"))),
		nullOrCast<Statement>(member(_node, "falseBody"))
	);
}

ASTPointer<WhileStatement> ASTJsonImporter::createWhileStatement(Json::Value const&  _node, bool _isDoWhile=false)
{
	return createASTNode<WhileStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "condition"))),
		castPointer<Statement>(convertJsonToASTNode(member(_node, "body"))),
		_isDoWhile
	);
}

ASTPointer<ForStatement> ASTJsonImporter::createForStatement(Json::Value const&  _node)
{
	return createASTNode<ForStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		nullOrCast<Statement>(member(_node, "initializationExpression")),
		nullOrCast<Expression>(member(_node, "condition")),
		nullOrCast<ExpressionStatement>(member(_node, "loopExpression")),
		castPointer<Statement>(convertJsonToASTNode(member(_node, "body")))
	);
}

ASTPointer<Continue> ASTJsonImporter::createContinue(Json::Value const&  _node)
{
	return createASTNode<Continue>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<Break> ASTJsonImporter::createBreak(Json::Value const&  _node)
{
	return createASTNode<Break>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<Return> ASTJsonImporter::createReturn(Json::Value const&  _node) {
	return createASTNode<Return>(
		_node,
		nullOrASTString(_node, "documentation"),
		nullOrCast<Expression>(member(_node, "expression"))
	);
}

ASTPointer<Throw> ASTJsonImporter::createThrow(Json::Value const&  _node)
{
	return createASTNode<Throw>(
		_node,
		nullOrASTString(_node, "documentation")
	);

}

ASTPointer<VariableDeclarationStatement> ASTJsonImporter::createVariableDeclarationStatement(Json::Value const&  _node)
{
	std::vector<ASTPointer<VariableDeclaration>> variables;
	for (auto& var: member(_node, "declarations"))
		variables.push_back( var.isNull() ? nullptr : createVariableDeclaration(var)); //unnamed components are empty pointers
	return createASTNode<VariableDeclarationStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		variables,
		nullOrCast<Expression>(member(_node, "initialValue"))
	);
}

ASTPointer<ExpressionStatement> ASTJsonImporter::createExpressionStatement(Json::Value const&  _node)
{
	return createASTNode<ExpressionStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "expression")))
	);
}

ASTPointer<Conditional> ASTJsonImporter::createConditional(Json::Value const&  _node)
{
	return createASTNode<Conditional>(
		_node,
		castPointer<Expression>(convertJsonToASTNode(member(_node, "condition"))),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "trueExpression"))),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "falseExpression")))
	);
}

ASTPointer<Assignment> ASTJsonImporter::createAssignment(Json::Value const&  _node)
{
	return createASTNode<Assignment>(
		_node,
		castPointer<Expression>(convertJsonToASTNode(member(_node, "leftHandSide"))),
		scanSingleToken(member(_node, "operator")),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "rightHandSide")))
	);
}

ASTPointer<TupleExpression> ASTJsonImporter::createTupleExpression(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> components;
	for (auto& comp: member(_node, "components"))
		components.push_back(nullOrCast<Expression>(comp));
	return createASTNode<TupleExpression>(
		_node,
		components,
		memberAsBool(_node, "isInlineArray")
	);
}

ASTPointer<UnaryOperation> ASTJsonImporter::createUnaryOperation(Json::Value const&  _node)
{
	return createASTNode<UnaryOperation>(
		_node,
		scanSingleToken(member(_node, "operator")),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "subExpression"))),
		memberAsBool(_node, "prefix")
	);


}

ASTPointer<BinaryOperation> ASTJsonImporter::createBinaryOperation(Json::Value const&  _node)
{
	return createASTNode<BinaryOperation>(
		_node,
		castPointer<Expression>(convertJsonToASTNode(member(_node, "leftExpression"))),
		scanSingleToken(member(_node, "operator")),
		castPointer<Expression>(convertJsonToASTNode(member(_node, "rightExpression")))
	);
}

ASTPointer<FunctionCall> ASTJsonImporter::createFunctionCall(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: member(_node, "names"))
		names.push_back(make_shared<ASTString>(name.asString()));
	return createASTNode<FunctionCall>(
		_node,
		castPointer<Expression>(convertJsonToASTNode(member(_node, "expression"))),
		arguments,
		names
	);

}

ASTPointer<NewExpression> ASTJsonImporter::createNewExpression(Json::Value const&  _node)
{
	return createASTNode<NewExpression>(
		_node,
		castPointer<TypeName>(convertJsonToASTNode(member(_node, "typeName")))
	);
}

ASTPointer<MemberAccess> ASTJsonImporter::createMemberAccess(Json::Value const&  _node)
{
	return createASTNode<MemberAccess>(
		_node,
		castPointer<Expression>(convertJsonToASTNode(member(_node, "expression"))),
		memberAsASTString(_node, "memberName")
	);
}

ASTPointer<IndexAccess> ASTJsonImporter::createIndexAccess(Json::Value const& _node)
{
	return createASTNode<IndexAccess>(
		_node,
		castPointer<Expression>(convertJsonToASTNode(member(_node, "baseExpression"))),
		nullOrCast<Expression>(member(_node, "indexExpression"))
	);
}

ASTPointer<Identifier> ASTJsonImporter::createIdentifier(Json::Value const& _node)
{
	return createASTNode<Identifier>(_node, memberAsASTString(_node, "name"));
}

ASTPointer<ElementaryTypeNameExpression> ASTJsonImporter::createElementaryTypeNameExpression(Json::Value const&  _node)
{
	Scanner scanner(CharStream(member(_node, "typeName").asString()), "");
	Token::Value token = scanner.currentToken();
	unsigned firstSize;
	unsigned secondSize;
	tie(firstSize, secondSize) = scanner.currentTokenInfo();
	ElementaryTypeNameToken elem(token, firstSize, secondSize);
	return createASTNode<ElementaryTypeNameExpression>(
		_node,
		elem
	);
}

ASTPointer<ASTNode> ASTJsonImporter::createLiteral(Json::Value const&  _node)
{
	astAssert(!member(_node, "value").isNull() || !member(_node, "hexValue").isNull(), "Literal-value is unset.");
	ASTPointer<ASTString> value = _node["value"].isNull() ?
				make_shared<ASTString>(asString(fromHex(_node["hexValue"].asString()))):
				make_shared<ASTString>(_node["value"].asString());
	return createASTNode<Literal>(
		_node,
		literalTokenKind(_node),
		value,
		member(_node, "subdenomination").isNull() ? Literal::SubDenomination::None : subdenomination(_node)
	);
}

Token::Value ASTJsonImporter::literalTokenKind(Json::Value const& _node)
{
	astAssert(!member(_node, "kind").isNull(), "Token-'kind' can not be null.");
	Token::Value tok;
	if (_node["kind"].asString() == "number")
		tok = Token::Number;
	else if (_node["kind"].asString() == "string")
		tok = Token::StringLiteral;
	else if (_node["kind"].asString() == "bool")
		 tok = (member(_node, "value").asString() == "true") ? Token::TrueLiteral : Token::FalseLiteral;
	else
		astAssert(false, "Unknown kind of literalString");
	return tok;
}

Declaration::Visibility ASTJsonImporter::visibility(Json::Value const& _node)
{
	astAssert(!member(_node, "visibility").isNull(), "'visibility' can not be null.");
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
		astAssert(false, "Unknown visibility declaration");
	return vis;
}

VariableDeclaration::Location ASTJsonImporter::location(Json::Value const& _node)
{
	VariableDeclaration::Location loc;
	astAssert(!member(_node, "storageLocation").isNull(), "'storageLocation' can not be null.");
	if (_node["storageLocation"].asString() == "default")
		loc = VariableDeclaration::Location::Default;
	else if (_node["storageLocation"].asString() == "storage")
		loc = VariableDeclaration::Location::Storage;
	else if (_node["storageLocation"].asString() == "memory")
		loc = VariableDeclaration::Location::Memory;
	else
		astAssert(false, "Unknown location declaration");
	return loc;
}

ContractDefinition::ContractKind ASTJsonImporter::contractKind(Json::Value const& _node)
{
	ContractDefinition::ContractKind kind;
	astAssert(!member(_node, "contractKind").isNull(), "'Contract-kind' can not be null.");
	if (_node["contractKind"].asString() == "interface")
		kind = ContractDefinition::ContractKind::Interface;
	else if (_node["contractKind"].asString() == "contract")
		kind = ContractDefinition::ContractKind::Contract;
	else if (_node["contractKind"].asString() == "library")
		kind = ContractDefinition::ContractKind::Library;
	else
		astAssert(false, "Unknown ContractKind");
	return kind;
}

Literal::SubDenomination ASTJsonImporter::subdenomination(Json::Value const& _node)
{
	Literal::SubDenomination kind;
	if (member(_node, "subdenomination").isNull())
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
		astAssert(false, "Unknown subdenomination");
	return kind;
}

StateMutability ASTJsonImporter::stateMutability(Json::Value const& _node)
{
	StateMutability mutability;
	astAssert(!member(_node, "stateMutability").isNull(), "StateMutability' can not be null.");
	if (_node["stateMutability"].asString() == "pure")
		mutability = StateMutability::Pure;
	else if (_node["stateMutability"].asString() == "view")
		mutability = StateMutability::View;
	else if (_node["stateMutability"].asString() == "nonpayable")
		mutability = StateMutability::NonPayable;
	else if (_node["stateMutability"].asString() == "payable")
		mutability = StateMutability::Payable;
	else
		astAssert(false, "Unknown stateMutability");
	return mutability;
}
