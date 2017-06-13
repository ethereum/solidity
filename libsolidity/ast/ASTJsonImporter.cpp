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

namespace
{
template<class T>
dev::solidity::ASTPointer<T> castPointer(dev::solidity::ASTPointer<dev::solidity::ASTNode> _ast)
{
	dev::solidity::ASTPointer<T> ret = dynamic_pointer_cast<T>(_ast);
	astAssert(ret, "");
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
		astAssert(!srcPair.second->isNull(), "");
		astAssert( !member(*srcPair.second, "id").isNull() && member(*srcPair.second,"nodeType") == "SourceUnit", "Nodetype of the top Level must be 'SourceUnit' and have an 'id'.");
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
		BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("'src' field ill-formatted or src-index too high"));
	int start = stoi(pos[0]);
	int end = start + stoi(pos[1]);
	return SourceLocation(
		start,
		end,
		m_sourceLocations[stoi(pos[2])]
	);
}

//helper functions
ASTPointer<ASTString> ASTJsonImporter::memberAsASTString(Json::Value const& _node, string _name)
{
	return make_shared<ASTString>(member(_node, _name).asString());
}

bool ASTJsonImporter::memberAsBool(Json::Value const& _node, string _name)
{
	return member(_node, _name).asBool();
}

Json::Value ASTJsonImporter::member(Json::Value const& _node, string _name)
{
	if (_node.isMember(_name))
		return _node[_name];
	else
		BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("Node '" + _node["nodeType"].asString() + "' (id " + _node["id"].asString() + ") is missing field '" + _name + "'."));
}

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
		BOOST_THROW_EXCEPTION(InvalidAstError() << errinfo_comment("Token string is too long."));
}

ASTPointer<ASTNode> ASTJsonImporter::convertJsonToASTNode(Json::Value const& _json)
{
	astAssert(_json.isMember("nodeType") && _json.isMember("id"), "JSON-Node needs to have 'nodeType' and 'id' fields.");
	string nodeType = _json["nodeType"].asString();
//	cout << nodeType << _json["id"] << std::endl;
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
	ASTPointer<ASTString> const& path = make_shared<ASTString>(member(_node, "file").asString());
	ASTPointer<ASTString> const& unitAlias = make_shared<ASTString>(member(_node, "unitAlias").asString());
	vector<pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> symbolAliases;
	for (auto& tuple: _node["symbolAliases"])
	{
                symbolAliases.push_back( make_pair(
			createIdentifier(tuple["foreign"]),
			tuple["local"].isNull() ? nullptr : make_shared<ASTString>(tuple["local"].asString())
		));
	}
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
	return createASTNode<ContractDefinition>(
		_node,
		name,
		documentation,
		baseContracts,
		subNodes,
		kind
	);
}

ASTPointer<InheritanceSpecifier> ASTJsonImporter::createInheritanceSpecifier(Json::Value const& _node)
{
	ASTPointer<UserDefinedTypeName> baseName = createUserDefinedTypeName(_node["baseName"]);
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg : _node["arguments"])
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	return createASTNode<InheritanceSpecifier>(
		_node,
		baseName,
		arguments
	);

}

ASTPointer<UsingForDirective> ASTJsonImporter::createUsingForDirective(Json::Value const& _node)
{
	ASTPointer<UserDefinedTypeName> libraryName = createUserDefinedTypeName(member(_node, "libraryName"));
	ASTPointer<TypeName> typeName;
	if (!_node["typename"].isNull())
		typeName = castPointer<TypeName>(convertJsonToASTNode(_node["typename"]));
	else
		typeName = nullptr; //equiv. to 'import *'
	return createASTNode<UsingForDirective>(
		_node,
		libraryName,
		typeName
	);

}

ASTPointer<ASTNode> ASTJsonImporter::createStructDefinition(Json::Value const& _node)
{
	ASTPointer<ASTString> const& name = make_shared<ASTString>(_node["name"].asString());
	std::vector<ASTPointer<VariableDeclaration>> members;
	for (auto& member: _node["members"])
		members.push_back(createVariableDeclaration(member));
	return createASTNode<StructDefinition>(
		_node,
		name,
		members
	);

}

ASTPointer<EnumDefinition> ASTJsonImporter::createEnumDefinition(Json::Value const& _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(_node["name"].asString());
	std::vector<ASTPointer<EnumValue>> members;
	for (auto& member: _node["members"])
		members.push_back(createEnumValue(member));
	return createASTNode<EnumDefinition>(
		_node,
		name,
		members
	);

}

ASTPointer<EnumValue> ASTJsonImporter::createEnumValue(Json::Value const& _node)
{
	ASTPointer<ASTString> const& name = make_shared<ASTString>(_node["name"].asString());
	return createASTNode<EnumValue>(
		_node,
		name
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
	Declaration::Visibility vis = visibility(_node);
	bool isConstructor = member(_node, "isConstructor").asBool();
	ASTPointer<ASTString> documentation = make_shared<ASTString>("");
	ASTPointer<ParameterList> parameters = createParameterList(member(_node, "parameters"));
	bool isDeclaredConst = member(_node, "isDeclaredConst").asBool();
	std::vector<ASTPointer<ModifierInvocation>> modifiers;
	for (auto& mod: member(_node, "modifiers"))
		modifiers.push_back(createModifierInvocation(mod));
	ASTPointer<ParameterList> returnParameters = createParameterList(member(_node, "returnParameters"));
	bool isPayable = member(_node, "payable").asBool();
	ASTPointer<Block> body = member(_node, "implemented").asBool() ? createBlock(member(_node, "body")) : nullptr;
	return createASTNode<FunctionDefinition>(
		_node,
		memberAsASTString(_node, "name"),
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

}

ASTPointer<VariableDeclaration> ASTJsonImporter::createVariableDeclaration(Json::Value const& _node)
{
	ASTPointer<TypeName> type = nullOrCast<TypeName>(member(_node, "typeName"));
	ASTPointer<ASTString> name = make_shared<ASTString>(member(_node, "name").asString());
	ASTPointer<Expression> value = nullOrCast<Expression>(member(_node, "value"));
	Declaration::Visibility vis = visibility(_node);
	bool isStateVar = member(_node, "stateVariable").asBool();
	bool isIndexed = _node.isMember("indexed") ? memberAsBool(_node, "indexed") : false;
	bool isConstant = member(_node, "constant").asBool();
	VariableDeclaration::Location referenceLocation = location(_node);
	return createASTNode<VariableDeclaration>(
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

}

ASTPointer<ModifierDefinition> ASTJsonImporter::createModifierDefinition(Json::Value const&  _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(member(_node, "name").asString());
	ASTPointer<ASTString> documentation = make_shared<ASTString>("");
	ASTPointer<ParameterList> parameters = createParameterList(member(_node, "parameters"));
	ASTPointer<Block> body = createBlock(member(_node, "body"));
	return createASTNode<ModifierDefinition>(
		_node,
		name,
		documentation,
		parameters,
		body
	);

}

ASTPointer<ModifierInvocation> ASTJsonImporter::createModifierInvocation(Json::Value const&  _node)
{
	ASTPointer<Identifier> name = createIdentifier(member(_node, "modifierName"));
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	return createASTNode<ModifierInvocation>(
		_node,
		name,
		arguments
	);

}

ASTPointer<EventDefinition> ASTJsonImporter::createEventDefinition(Json::Value const&  _node)
{
	ASTPointer<ASTString> name = make_shared<ASTString>(member(_node, "name").asString());
	ASTPointer<ASTString> documentation = make_shared<ASTString>("");
	ASTPointer<ParameterList> parameters = createParameterList(member(_node, "parameters"));
	bool anonymous = member(_node, "anonymous").asBool();
	return createASTNode<EventDefinition>(
		_node,
		name,
		documentation,
		parameters,
		anonymous
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
	ASTPointer<ParameterList> parameterTypes = createParameterList(member(_node, "parameterTypes"));
	ASTPointer<ParameterList> returnTypes = createParameterList(member(_node, "returnParameterTypes"));
	Declaration::Visibility vis = visibility(_node);
	bool isDeclaredConst = member(_node, "isDeclaredConst").asBool();
	bool isPayable = member(_node, "payable").asBool();
	return createASTNode<FunctionTypeName>(
		_node,
		parameterTypes,
		returnTypes,
		vis,
		isDeclaredConst,
		isPayable
	);

}

ASTPointer<Mapping> ASTJsonImporter::createMapping(Json::Value const&  _node)
{
	ASTPointer<ElementaryTypeName> keyType = createElementaryTypeName(member(_node, "keyType"));
	ASTPointer<TypeName> valueType = castPointer<TypeName>(convertJsonToASTNode(member(_node, "valueType")));
	return createASTNode<Mapping>(
		_node,
		keyType,
		valueType
	);

}

ASTPointer<ArrayTypeName> ASTJsonImporter::createArrayTypeName(Json::Value const&  _node)
{
	ASTPointer<TypeName> baseType = castPointer<TypeName>(convertJsonToASTNode(member(_node, "baseType")));
	ASTPointer<Expression> length = nullOrCast<Expression>(member(_node, "length"));
	return createASTNode<ArrayTypeName>(
		_node,
		baseType,
		length
	);
}

ASTPointer<InlineAssembly> ASTJsonImporter::createInlineAssembly(Json::Value const& _node)
{
	ErrorList tmp_list; //how can this be more elegant?
	ErrorReporter tmp_error(tmp_list);
	assembly::Parser asmParser(tmp_error);
	shared_ptr<Scanner> scanner = make_shared<Scanner>(CharStream(member(_node, "operations").asString()), "");
	std::shared_ptr<assembly::Block> operations = asmParser.parse(scanner);
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	return createASTNode<InlineAssembly>(_node, docString, operations);
}

ASTPointer<Block> ASTJsonImporter::createBlock(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	std::vector<ASTPointer<Statement>> statements;
	for (auto& stat: member(_node, "statements"))
		statements.push_back(castPointer<Statement>(convertJsonToASTNode(stat)));
	return createASTNode<Block>(
		_node,
		docString,
		statements
	);
}

ASTPointer<PlaceholderStatement> ASTJsonImporter::createPlaceholderStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	return createASTNode<PlaceholderStatement>(
		_node,
		docString
	);
}

ASTPointer<IfStatement> ASTJsonImporter::createIfStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(member(_node, "condition")));
	ASTPointer<Statement> trueBody = castPointer<Statement>(convertJsonToASTNode(member(_node, "trueBody")));
	ASTPointer<Statement> falseBody = nullOrCast<Statement>(member(_node, "falseBody"));
	return createASTNode<IfStatement>(
		_node,
		docString,
		condition,
		trueBody,
		falseBody
	);
}

ASTPointer<WhileStatement> ASTJsonImporter::createWhileStatement(Json::Value const&  _node, bool _isDoWhile=false)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(member(_node, "condition")));
	ASTPointer<Statement> body = castPointer<Statement>(convertJsonToASTNode(member(_node, "body")));
	return createASTNode<WhileStatement>(
		_node,
		docString,
		condition,
		body,
		_isDoWhile
	);
}

ASTPointer<ForStatement> ASTJsonImporter::createForStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	ASTPointer<Statement> initExpression = nullOrCast<Statement>(member(_node, "initializationExpression"));
	ASTPointer<Expression> conditionExpression = nullOrCast<Expression>(member(_node, "condition"));
	ASTPointer<ExpressionStatement> loopExpression = nullOrCast<ExpressionStatement>(member(_node, "loopExpression"));
	ASTPointer<Statement> body = castPointer<Statement>(convertJsonToASTNode(member(_node, "body")));
	return createASTNode<ForStatement>(
		_node,
		docString,
		initExpression,
		conditionExpression,
		loopExpression,
		body
	);
}

ASTPointer<Continue> ASTJsonImporter::createContinue(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	return createASTNode<Continue>(_node, docString);
}

ASTPointer<Break> ASTJsonImporter::createBreak(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	return createASTNode<Break>(_node, docString);
}

ASTPointer<Return> ASTJsonImporter::createReturn(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	ASTPointer<Expression> expression = nullOrCast<Expression>(member(_node, "expression"));
	return createASTNode<Return>(
		_node,
		docString,
		expression
	);
}

ASTPointer<Throw> ASTJsonImporter::createThrow(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	return createASTNode<Throw>(_node, docString);

}

ASTPointer<VariableDeclarationStatement> ASTJsonImporter::createVariableDeclarationStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	std::vector<ASTPointer<VariableDeclaration>> variables;
	for (auto& var: member(_node, "declarations"))
		variables.push_back( var.isNull() ? nullptr : createVariableDeclaration(var)); //unnamed components are empty pointers
	ASTPointer<Expression> initialValue = nullOrCast<Expression>(member(_node, "initialValue"));
	return createASTNode<VariableDeclarationStatement>(
		_node,
		docString,
		variables,
		initialValue
	);
}

ASTPointer<ExpressionStatement> ASTJsonImporter::createExpressionStatement(Json::Value const&  _node)
{
	ASTPointer<ASTString> docString = make_shared<ASTString>("");
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(member(_node, "expression")));
	return createASTNode<ExpressionStatement>(
		_node,
		docString,
		expression
	);
}

ASTPointer<Conditional> ASTJsonImporter::createConditional(Json::Value const&  _node)
{
	ASTPointer<Expression> condition = castPointer<Expression>(convertJsonToASTNode(member(_node, "condition")));
	ASTPointer<Expression> trueExpression = castPointer<Expression>(convertJsonToASTNode(member(_node, "trueExpression")));
	ASTPointer<Expression> falseExpression = castPointer<Expression>(convertJsonToASTNode(member(_node, "falseExpression")));
	return createASTNode<Conditional>(
		_node,
		condition,
		trueExpression,
		falseExpression
	);
}

ASTPointer<Assignment> ASTJsonImporter::createAssignment(Json::Value const&  _node)
{
	ASTPointer<Expression> leftHandSide = castPointer<Expression>(convertJsonToASTNode(member(_node, "leftHandSide")));
	Token::Value assignmentOperator = scanSingleToken(member(_node, "operator"));
	ASTPointer<Expression> rightHandSide = castPointer<Expression>(convertJsonToASTNode(member(_node, "rightHandSide")));
	return createASTNode<Assignment>(
		_node,
		leftHandSide,
		assignmentOperator,
		rightHandSide
	);
}

ASTPointer<TupleExpression> ASTJsonImporter::createTupleExpression(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> components;
	for (auto& comp: member(_node, "components"))
		components.push_back(nullOrCast<Expression>(comp));
	bool isArray = member(_node, "isInlineArray").asBool();
	return createASTNode<TupleExpression>(
		_node,
		components,
		isArray
	);
}

ASTPointer<UnaryOperation> ASTJsonImporter::createUnaryOperation(Json::Value const&  _node)
{
	Token::Value operation = scanSingleToken(member(_node, "operator"));
	ASTPointer<Expression> subExpression = castPointer<Expression>(convertJsonToASTNode(member(_node, "subExpression")));
	bool prefix = member(_node, "prefix").asBool();
	return createASTNode<UnaryOperation>(
		_node,
		operation,
		subExpression,
		prefix
	);


}

ASTPointer<BinaryOperation> ASTJsonImporter::createBinaryOperation(Json::Value const&  _node)
{
	ASTPointer<Expression> left = castPointer<Expression>(convertJsonToASTNode(member(_node, "leftExpression")));
	Token::Value operation = scanSingleToken(member(_node, "operator"));
	ASTPointer<Expression> right = castPointer<Expression>(convertJsonToASTNode(member(_node, "rightExpression")));
	return createASTNode<BinaryOperation>(
		_node,
		left,
		operation,
		right
	);
}

ASTPointer<FunctionCall> ASTJsonImporter::createFunctionCall(Json::Value const&  _node)
{
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(member(_node, "expression")));
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(castPointer<Expression>(convertJsonToASTNode(arg)));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: member(_node, "names"))
		names.push_back(make_shared<ASTString>(name.asString()));
	return createASTNode<FunctionCall>(
		_node,
		expression,
		arguments,
		names
	);

}

ASTPointer<NewExpression> ASTJsonImporter::createNewExpression(Json::Value const&  _node)
{
	ASTPointer<TypeName> typeName = castPointer<TypeName>(convertJsonToASTNode(member(_node, "typeName")));
	return createASTNode<NewExpression>(
		_node,
		typeName
	);
}

ASTPointer<MemberAccess> ASTJsonImporter::createMemberAccess(Json::Value const&  _node)
{
	ASTPointer<Expression> expression = castPointer<Expression>(convertJsonToASTNode(member(_node, "expression")));
	ASTPointer<ASTString> memberName = make_shared<ASTString>(member(_node, "memberName").asString());
	return createASTNode<MemberAccess>(
		_node,
		expression,
		memberName
	);
}

ASTPointer<IndexAccess> ASTJsonImporter::createIndexAccess(Json::Value const& _node)
{
	ASTPointer<Expression> base = castPointer<Expression>(convertJsonToASTNode(member(_node, "baseExpression")));
	ASTPointer<Expression> index = nullOrCast<Expression>(member(_node, "indexExpression"));
	return createASTNode<IndexAccess>(
		_node,
		base,
		index
	);
}

ASTPointer<Identifier> ASTJsonImporter::createIdentifier(Json::Value const& _node)
{
	return createASTNode<Identifier>(_node, make_shared<ASTString>(member(_node, "name").asString()));
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
	Token::Value token = literalTokenKind(_node);
	astAssert(!member(_node, "value").isNull() || !member(_node, "hexValue").isNull(), "Literal-value is unset.");
	ASTPointer<ASTString> value = _node["value"].isNull() ?
				make_shared<ASTString>(asString(fromHex(_node["hexValue"].asString()))):
				make_shared<ASTString>(_node["value"].asString());
	Literal::SubDenomination sub = member(_node, "subdenomination").isNull() ? Literal::SubDenomination::None : subdenomination(_node);
	return createASTNode<Literal>(
		_node,
		token,
		value,
		sub
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
	astAssert(!member(_node, "contractKind").isNull(), "Contract-'kind' can not be null.");
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
