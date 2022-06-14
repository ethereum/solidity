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
 *Component that imports an AST from json format to the internal format
 */

#include <libsolidity/ast/ASTJsonImporter.h>

#include <libyul/AsmJsonImporter.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceLocation.h>
#include <liblangutil/Token.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace solidity::frontend
{

using SourceLocation = langutil::SourceLocation;

template<class T>
ASTPointer<T> ASTJsonImporter::nullOrCast(Json::Value const& _json)
{
	if (_json.isNull())
		return nullptr;
	else
		return dynamic_pointer_cast<T>(convertJsonToASTNode(_json));
}


// ============ public ===========================

map<string, ASTPointer<SourceUnit>> ASTJsonImporter::jsonToSourceUnit(map<string, Json::Value> const& _sourceList)
{
	for (auto const& src: _sourceList)
		m_sourceNames.emplace_back(make_shared<string const>(src.first));
	for (auto const& srcPair: _sourceList)
	{
		astAssert(!srcPair.second.isNull());
		astAssert(member(srcPair.second,"nodeType") == "SourceUnit", "The 'nodeType' of the highest node must be 'SourceUnit'.");
		m_sourceUnits[srcPair.first] = createSourceUnit(srcPair.second, srcPair.first);
	}
	return m_sourceUnits;
}

// ============ private ===========================

// =========== general creation functions ==============
template <typename T, typename... Args>
ASTPointer<T> ASTJsonImporter::createASTNode(Json::Value const& _node, Args&&... _args)
{
	astAssert(member(_node, "id").isInt64(), "'id'-field must be 64bit integer.");

	int64_t id = _node["id"].asInt64();

	astAssert(m_usedIDs.insert(id).second, "Found duplicate node ID!");

	auto n = make_shared<T>(
		id,
		createSourceLocation(_node),
		forward<Args>(_args)...
	);
	return n;
}

SourceLocation const ASTJsonImporter::createSourceLocation(Json::Value const& _node)
{
	astAssert(member(_node, "src").isString(), "'src' must be a string");

	return solidity::langutil::parseSourceLocation(_node["src"].asString(), m_sourceNames);
}

SourceLocation ASTJsonImporter::createNameSourceLocation(Json::Value const& _node)
{
	astAssert(member(_node, "nameLocation").isString(), "'nameLocation' must be a string");

	return solidity::langutil::parseSourceLocation(_node["nameLocation"].asString(), m_sourceNames);
}

template<class T>
ASTPointer<T> ASTJsonImporter::convertJsonToASTNode(Json::Value const& _node)
{
	ASTPointer<T> ret = dynamic_pointer_cast<T>(convertJsonToASTNode(_node));
	astAssert(ret, "cast of converted json-node must not be nullptr");
	return ret;
}


ASTPointer<ASTNode> ASTJsonImporter::convertJsonToASTNode(Json::Value const& _json)
{
	astAssert(_json["nodeType"].isString() && _json.isMember("id"), "JSON-Node needs to have 'nodeType' and 'id' fields.");
	string nodeType = _json["nodeType"].asString();
	if (nodeType == "PragmaDirective")
		return createPragmaDirective(_json);
	if (nodeType == "ImportDirective")
		return createImportDirective(_json);
	if (nodeType == "ContractDefinition")
		return createContractDefinition(_json);
	if (nodeType == "IdentifierPath")
		return createIdentifierPath(_json);
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
	if (nodeType == "UserDefinedValueTypeDefinition")
		return createUserDefinedValueTypeDefinition(_json);
	if (nodeType == "ParameterList")
		return createParameterList(_json);
	if (nodeType == "OverrideSpecifier")
		return createOverrideSpecifier(_json);
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
	if (nodeType == "ErrorDefinition")
		return createErrorDefinition(_json);
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
	if (nodeType == "InlineAssembly")
		return createInlineAssembly(_json);
	if (nodeType == "Block")
		return createBlock(_json, false);
	if (nodeType == "UncheckedBlock")
		return createBlock(_json, true);
	if (nodeType == "PlaceholderStatement")
		return createPlaceholderStatement(_json);
	if (nodeType == "IfStatement")
		return createIfStatement(_json);
	if (nodeType == "TryCatchClause")
		return createTryCatchClause(_json);
	if (nodeType == "TryStatement")
		return createTryStatement(_json);
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
	if (nodeType == "EmitStatement")
		return createEmitStatement(_json);
	if (nodeType == "RevertStatement")
		return createRevertStatement(_json);
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
	if (nodeType == "FunctionCallOptions")
		return createFunctionCallOptions(_json);
	if (nodeType == "NewExpression")
		return createNewExpression(_json);
	if (nodeType == "MemberAccess")
		return createMemberAccess(_json);
	if (nodeType == "IndexAccess")
		return createIndexAccess(_json);
	if (nodeType == "IndexRangeAccess")
		return createIndexRangeAccess(_json);
	if (nodeType == "Identifier")
		return createIdentifier(_json);
	if (nodeType == "ElementaryTypeNameExpression")
		return createElementaryTypeNameExpression(_json);
	if (nodeType == "Literal")
		return createLiteral(_json);
	if (nodeType == "StructuredDocumentation")
		return createDocumentation(_json);
	else
		astAssert(false, "Unknown type of ASTNode: " + nodeType);

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

// ============ functions to instantiate the AST-Nodes from Json-Nodes ==============

ASTPointer<SourceUnit> ASTJsonImporter::createSourceUnit(Json::Value const& _node, string const& _srcName)
{
	optional<string> license;
	if (_node.isMember("license") && !_node["license"].isNull())
		license = _node["license"].asString();

	vector<ASTPointer<ASTNode>> nodes;
	for (auto& child: member(_node, "nodes"))
		nodes.emplace_back(convertJsonToASTNode(child));

	ASTPointer<SourceUnit> tmp = createASTNode<SourceUnit>(_node, license, nodes);
	tmp->annotation().path = _srcName;
	return tmp;
}

ASTPointer<PragmaDirective> ASTJsonImporter::createPragmaDirective(Json::Value const& _node)
{
	vector<Token> tokens;
	vector<ASTString> literals;
	for (auto const& lit: member(_node, "literals"))
	{
		string l = lit.asString();
		literals.push_back(l);
		tokens.push_back(scanSingleToken(l));
	}
	return createASTNode<PragmaDirective>(_node, tokens, literals);
}

ASTPointer<ImportDirective> ASTJsonImporter::createImportDirective(Json::Value const& _node)
{
	ASTPointer<ASTString> unitAlias = memberAsASTString(_node, "unitAlias");
	ASTPointer<ASTString> path = memberAsASTString(_node, "file");
	ImportDirective::SymbolAliasList symbolAliases;

	for (auto& tuple: member(_node, "symbolAliases"))
	{
		astAssert(tuple["local"].isNull() || tuple["local"].isString(), "expected 'local' to be a string or null!");

		symbolAliases.push_back({
			createIdentifier(tuple["foreign"]),
			tuple["local"].isNull() ? nullptr : make_shared<ASTString>(tuple["local"].asString()),
			createSourceLocation(tuple["foreign"])}
		);
	}
	ASTPointer<ImportDirective> tmp = createASTNode<ImportDirective>(
		_node,
		path,
		unitAlias,
		createNameSourceLocation(_node),
		move(symbolAliases)
	);

	astAssert(_node["absolutePath"].isString(), "Expected 'absolutePath' to be a string!");

	tmp->annotation().absolutePath = _node["absolutePath"].asString();
	return tmp;
}

ASTPointer<ContractDefinition> ASTJsonImporter::createContractDefinition(Json::Value const& _node)
{
	astAssert(_node["name"].isString(), "Expected 'name' to be a string!");

	std::vector<ASTPointer<InheritanceSpecifier>> baseContracts;

	for (auto& base: _node["baseContracts"])
		baseContracts.push_back(createInheritanceSpecifier(base));

	std::vector<ASTPointer<ASTNode>> subNodes;

	for (auto& subnode: _node["nodes"])
		subNodes.push_back(convertJsonToASTNode(subnode));

	return createASTNode<ContractDefinition>(
		_node,
		make_shared<ASTString>(_node["name"].asString()),
		createNameSourceLocation(_node),
		_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
		baseContracts,
		subNodes,
		contractKind(_node),
		memberAsBool(_node, "abstract")
	);
}

ASTPointer<IdentifierPath> ASTJsonImporter::createIdentifierPath(Json::Value const& _node)
{
	astAssert(_node["name"].isString(), "Expected 'name' to be a string!");

	vector<ASTString> namePath;
	vector<SourceLocation> namePathLocations;
	vector<string> strs;
	string nameString = member(_node, "name").asString();
	boost::algorithm::split(strs, nameString, boost::is_any_of("."));
	astAssert(!strs.empty(), "Expected at least one element in IdentifierPath.");
	for (string s: strs)
	{
		astAssert(!s.empty(), "Expected non-empty string for IdentifierPath element.");
		namePath.emplace_back(s);
	}

	if (_node.isMember("nameLocations") && _node["nameLocations"].isArray())
		for (auto const& val: _node["nameLocations"])
			namePathLocations.emplace_back(langutil::parseSourceLocation(val.asString(), m_sourceNames));
	else
		namePathLocations.resize(namePath.size());

	astAssert(
		namePath.size() == namePathLocations.size(),
		"SourceLocations don't match name paths."
	);

	return createASTNode<IdentifierPath>(
		_node,
		namePath,
		namePathLocations
	);
}

ASTPointer<InheritanceSpecifier> ASTJsonImporter::createInheritanceSpecifier(Json::Value const& _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(convertJsonToASTNode<Expression>(arg));
	return createASTNode<InheritanceSpecifier>(
		_node,
		createIdentifierPath(member(_node, "baseName")),
		member(_node, "arguments").isNull() ? nullptr : make_unique<std::vector<ASTPointer<Expression>>>(arguments)
	);
}

ASTPointer<UsingForDirective> ASTJsonImporter::createUsingForDirective(Json::Value const& _node)
{
	vector<ASTPointer<IdentifierPath>> functions;
	if (_node.isMember("libraryName"))
		functions.emplace_back(createIdentifierPath(_node["libraryName"]));
	else if (_node.isMember("functionList"))
		for (Json::Value const& function: _node["functionList"])
			functions.emplace_back(createIdentifierPath(function["function"]));

	return createASTNode<UsingForDirective>(
		_node,
		move(functions),
		!_node.isMember("libraryName"),
		_node["typeName"].isNull() ? nullptr  : convertJsonToASTNode<TypeName>(_node["typeName"]),
		memberAsBool(_node, "global")
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
		createNameSourceLocation(_node),
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
		createNameSourceLocation(_node),
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

ASTPointer<UserDefinedValueTypeDefinition> ASTJsonImporter::createUserDefinedValueTypeDefinition(Json::Value const& _node)
{
	return createASTNode<UserDefinedValueTypeDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		convertJsonToASTNode<TypeName>(member(_node, "underlyingType"))
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

ASTPointer<OverrideSpecifier> ASTJsonImporter::createOverrideSpecifier(Json::Value const&  _node)
{
	std::vector<ASTPointer<IdentifierPath>> overrides;

	for (auto& param: _node["overrides"])
		overrides.push_back(createIdentifierPath(param));

	return createASTNode<OverrideSpecifier>(
		_node,
		overrides
	);
}

ASTPointer<FunctionDefinition> ASTJsonImporter::createFunctionDefinition(Json::Value const&  _node)
{
	astAssert(_node["kind"].isString(), "Expected 'kind' to be a string!");

	Token kind;
	bool freeFunction = false;
	string kindStr = member(_node, "kind").asString();

	if (kindStr == "constructor")
		kind = Token::Constructor;
	else if (kindStr == "function")
		kind = Token::Function;
	else if (kindStr == "fallback")
		kind = Token::Fallback;
	else if (kindStr == "receive")
		kind = Token::Receive;
	else if (kindStr == "freeFunction")
	{
		kind = Token::Function;
		freeFunction = true;
	}
	else
		astAssert(false, "Expected 'kind' to be one of [constructor, function, fallback, receive]");

	std::vector<ASTPointer<ModifierInvocation>> modifiers;
	for (auto& mod: member(_node, "modifiers"))
		modifiers.push_back(createModifierInvocation(mod));

	Visibility vis = Visibility::Default;
	// Ignore public visibility for constructors
	if (kind == Token::Constructor)
		vis = (visibility(_node) == Visibility::Public) ? Visibility::Default : visibility(_node);
	else if (!freeFunction)
		vis = visibility(_node);
	return createASTNode<FunctionDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		vis,
		stateMutability(_node),
		freeFunction,
		kind,
		memberAsBool(_node, "virtual"),
		_node["overrides"].isNull() ? nullptr : createOverrideSpecifier(member(_node, "overrides")),
		_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
		createParameterList(member(_node, "parameters")),
		modifiers,
		createParameterList(member(_node, "returnParameters")),
		memberAsBool(_node, "implemented") ? createBlock(member(_node, "body"), false) : nullptr
	);
}

ASTPointer<VariableDeclaration> ASTJsonImporter::createVariableDeclaration(Json::Value const& _node)
{
	astAssert(_node["name"].isString(), "Expected 'name' to be a string!");

	VariableDeclaration::Mutability mutability{};
	astAssert(member(_node, "mutability").isString(), "'mutability' expected to be string.");
	string const mutabilityStr = member(_node, "mutability").asString();
	if (mutabilityStr == "constant")
	{
		mutability = VariableDeclaration::Mutability::Constant;
		astAssert(memberAsBool(_node, "constant"));
	}
	else
	{
		astAssert(!memberAsBool(_node, "constant"));
		if (mutabilityStr == "mutable")
			mutability = VariableDeclaration::Mutability::Mutable;
		else if (mutabilityStr == "immutable")
			mutability = VariableDeclaration::Mutability::Immutable;
		else
			astAssert(false);
	}

	return createASTNode<VariableDeclaration>(
		_node,
		nullOrCast<TypeName>(member(_node, "typeName")),
		make_shared<ASTString>(member(_node, "name").asString()),
		createNameSourceLocation(_node),
		nullOrCast<Expression>(member(_node, "value")),
		visibility(_node),
		_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
		_node.isMember("indexed") ? memberAsBool(_node, "indexed") : false,
		mutability,
		_node["overrides"].isNull() ? nullptr : createOverrideSpecifier(member(_node, "overrides")),
		location(_node)
	);
}

ASTPointer<ModifierDefinition> ASTJsonImporter::createModifierDefinition(Json::Value const&  _node)
{
	return createASTNode<ModifierDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
		createParameterList(member(_node, "parameters")),
		memberAsBool(_node, "virtual"),
		_node["overrides"].isNull() ? nullptr : createOverrideSpecifier(member(_node, "overrides")),
		_node["body"].isNull() ? nullptr: createBlock(member(_node, "body"), false)
	);
}

ASTPointer<ModifierInvocation> ASTJsonImporter::createModifierInvocation(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(convertJsonToASTNode<Expression>(arg));
	return createASTNode<ModifierInvocation>(
		_node,
		createIdentifierPath(member(_node, "modifierName")),
		member(_node, "arguments").isNull() ? nullptr : make_unique<std::vector<ASTPointer<Expression>>>(arguments)
	);
}

ASTPointer<EventDefinition> ASTJsonImporter::createEventDefinition(Json::Value const&  _node)
{
	return createASTNode<EventDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
		createParameterList(member(_node, "parameters")),
		memberAsBool(_node, "anonymous")
	);
}

ASTPointer<ErrorDefinition> ASTJsonImporter::createErrorDefinition(Json::Value const&  _node)
{
	return createASTNode<ErrorDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
		createParameterList(member(_node, "parameters"))
	);
}

ASTPointer<ElementaryTypeName> ASTJsonImporter::createElementaryTypeName(Json::Value const& _node)
{
	unsigned short firstNum;
	unsigned short secondNum;

	astAssert(_node["name"].isString(), "Expected 'name' to be a string!");

	string name = member(_node, "name").asString();
	Token token;
	tie(token, firstNum, secondNum) = TokenTraits::fromIdentifierOrKeyword(name);
	ElementaryTypeNameToken elem(token, firstNum,  secondNum);

	std::optional<StateMutability> mutability = {};
	if (_node.isMember("stateMutability"))
		mutability = stateMutability(_node);

	return createASTNode<ElementaryTypeName>(_node, elem, mutability);
}

ASTPointer<UserDefinedTypeName> ASTJsonImporter::createUserDefinedTypeName(Json::Value const& _node)
{
	return createASTNode<UserDefinedTypeName>(
		_node,
		createIdentifierPath(member(_node, "pathNode"))
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
		convertJsonToASTNode<TypeName>(member(_node, "keyType")),
		convertJsonToASTNode<TypeName>(member(_node, "valueType"))
	);
}

ASTPointer<ArrayTypeName> ASTJsonImporter::createArrayTypeName(Json::Value const&  _node)
{
	return createASTNode<ArrayTypeName>(
		_node,
		convertJsonToASTNode<TypeName>(member(_node, "baseType")),
		nullOrCast<Expression>(member(_node, "length"))
	);
}

ASTPointer<InlineAssembly> ASTJsonImporter::createInlineAssembly(Json::Value const& _node)
{
	astAssert(_node["evmVersion"].isString(), "Expected evmVersion to be a string!");
	auto evmVersion = langutil::EVMVersion::fromString(_node["evmVersion"].asString());
	astAssert(evmVersion.has_value(), "Invalid EVM version!");
	astAssert(m_evmVersion == evmVersion, "Imported tree evm version differs from configured evm version!");

	yul::Dialect const& dialect = yul::EVMDialect::strictAssemblyForEVM(evmVersion.value());
	ASTPointer<vector<ASTPointer<ASTString>>> flags;
	if (_node.isMember("flags"))
	{
		flags = make_shared<vector<ASTPointer<ASTString>>>();
		Json::Value const& flagsNode = _node["flags"];
		astAssert(flagsNode.isArray(), "Assembly flags must be an array.");
		for (Json::ArrayIndex i = 0; i < flagsNode.size(); ++i)
		{
			astAssert(flagsNode[i].isString(), "Assembly flag must be a string.");
			flags->emplace_back(make_shared<ASTString>(flagsNode[i].asString()));
		}
	}
	shared_ptr<yul::Block> operations = make_shared<yul::Block>(yul::AsmJsonImporter(m_sourceNames).createBlock(member(_node, "AST")));
	return createASTNode<InlineAssembly>(
		_node,
		nullOrASTString(_node, "documentation"),
		dialect,
		move(flags),
		operations
	);
}

ASTPointer<Block> ASTJsonImporter::createBlock(Json::Value const& _node, bool _unchecked)
{
	std::vector<ASTPointer<Statement>> statements;
	for (auto& stat: member(_node, "statements"))
		statements.push_back(convertJsonToASTNode<Statement>(stat));
	return createASTNode<Block>(
		_node,
		nullOrASTString(_node, "documentation"),
		_unchecked,
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
		convertJsonToASTNode<Expression>(member(_node, "condition")),
		convertJsonToASTNode<Statement>(member(_node, "trueBody")),
		nullOrCast<Statement>(member(_node, "falseBody"))
	);
}

ASTPointer<TryCatchClause> ASTJsonImporter::createTryCatchClause(Json::Value const&  _node)
{
	return createASTNode<TryCatchClause>(
		_node,
		memberAsASTString(_node, "errorName"),
		nullOrCast<ParameterList>(member(_node, "parameters")),
		convertJsonToASTNode<Block>(member(_node, "block"))
	);
}

ASTPointer<TryStatement> ASTJsonImporter::createTryStatement(Json::Value const&  _node)
{
	vector<ASTPointer<TryCatchClause>> clauses;

	for (auto& param: _node["clauses"])
		clauses.emplace_back(createTryCatchClause(param));

	return createASTNode<TryStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		convertJsonToASTNode<Expression>(member(_node, "externalCall")),
		clauses
	);
}

ASTPointer<WhileStatement> ASTJsonImporter::createWhileStatement(Json::Value const&  _node, bool _isDoWhile=false)
{
	return createASTNode<WhileStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		convertJsonToASTNode<Expression>(member(_node, "condition")),
		convertJsonToASTNode<Statement>(member(_node, "body")),
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
		convertJsonToASTNode<Statement>(member(_node, "body"))
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

ASTPointer<Return> ASTJsonImporter::createReturn(Json::Value const&  _node)
{
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

ASTPointer<EmitStatement> ASTJsonImporter::createEmitStatement(Json::Value const&  _node)
{
	return createASTNode<EmitStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		createFunctionCall(member(_node, "eventCall"))
	);
}

ASTPointer<RevertStatement> ASTJsonImporter::createRevertStatement(Json::Value const&  _node)
{
	return createASTNode<RevertStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		createFunctionCall(member(_node, "errorCall"))
	);
}

ASTPointer<VariableDeclarationStatement> ASTJsonImporter::createVariableDeclarationStatement(Json::Value const& _node)
{
	std::vector<ASTPointer<VariableDeclaration>> variables;
	for (auto& var: member(_node, "declarations"))
		variables.push_back(var.isNull() ? nullptr : createVariableDeclaration(var)); //unnamed components are empty pointers
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
		convertJsonToASTNode<Expression>(member(_node, "expression"))
	);
}

ASTPointer<Conditional> ASTJsonImporter::createConditional(Json::Value const&  _node)
{
	return createASTNode<Conditional>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "condition")),
		convertJsonToASTNode<Expression>(member(_node, "trueExpression")),
		convertJsonToASTNode<Expression>(member(_node, "falseExpression"))
	);
}

ASTPointer<Assignment> ASTJsonImporter::createAssignment(Json::Value const&  _node)
{
	return createASTNode<Assignment>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "leftHandSide")),
		scanSingleToken(member(_node, "operator")),
		convertJsonToASTNode<Expression>(member(_node, "rightHandSide"))
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
		convertJsonToASTNode<Expression>(member(_node, "subExpression")),
		memberAsBool(_node, "prefix")
	);
}

ASTPointer<BinaryOperation> ASTJsonImporter::createBinaryOperation(Json::Value const&  _node)
{
	return createASTNode<BinaryOperation>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "leftExpression")),
		scanSingleToken(member(_node, "operator")),
		convertJsonToASTNode<Expression>(member(_node, "rightExpression"))
	);
}

ASTPointer<FunctionCall> ASTJsonImporter::createFunctionCall(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(convertJsonToASTNode<Expression>(arg));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: member(_node, "names"))
	{
		astAssert(name.isString(), "Expected 'names' members to be strings!");
		names.push_back(make_shared<ASTString>(name.asString()));
	}
	return createASTNode<FunctionCall>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "expression")),
		arguments,
		names
	);
}

ASTPointer<FunctionCallOptions> ASTJsonImporter::createFunctionCallOptions(Json::Value const&  _node)
{
	std::vector<ASTPointer<Expression>> options;
	for (auto& option: member(_node, "options"))
		options.push_back(convertJsonToASTNode<Expression>(option));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: member(_node, "names"))
	{
		astAssert(name.isString(), "Expected 'names' members to be strings!");
		names.push_back(make_shared<ASTString>(name.asString()));
	}

	return createASTNode<FunctionCallOptions>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "expression")),
		options,
		names
	);
}

ASTPointer<NewExpression> ASTJsonImporter::createNewExpression(Json::Value const&  _node)
{
	return createASTNode<NewExpression>(
		_node,
		convertJsonToASTNode<TypeName>(member(_node, "typeName"))
	);
}

ASTPointer<MemberAccess> ASTJsonImporter::createMemberAccess(Json::Value const&  _node)
{
	SourceLocation memberLocation;
	if (member(_node, "memberLocation").isString())
		memberLocation = solidity::langutil::parseSourceLocation(_node["memberLocation"].asString(), m_sourceNames);

	return createASTNode<MemberAccess>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "expression")),
		memberAsASTString(_node, "memberName"),
		memberLocation
	);
}

ASTPointer<IndexAccess> ASTJsonImporter::createIndexAccess(Json::Value const& _node)
{
	return createASTNode<IndexAccess>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "baseExpression")),
		nullOrCast<Expression>(member(_node, "indexExpression"))
	);
}

ASTPointer<IndexRangeAccess> ASTJsonImporter::createIndexRangeAccess(Json::Value const& _node)
{
	return createASTNode<IndexRangeAccess>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "baseExpression")),
		nullOrCast<Expression>(member(_node, "startExpression")),
		nullOrCast<Expression>(member(_node, "endExpression"))
	);
}

ASTPointer<Identifier> ASTJsonImporter::createIdentifier(Json::Value const& _node)
{
	return createASTNode<Identifier>(_node, memberAsASTString(_node, "name"));
}

ASTPointer<ElementaryTypeNameExpression> ASTJsonImporter::createElementaryTypeNameExpression(Json::Value const&  _node)
{
	return createASTNode<ElementaryTypeNameExpression>(
		_node,
		createElementaryTypeName(member(_node, "typeName"))
	);
}

ASTPointer<ASTNode> ASTJsonImporter::createLiteral(Json::Value const&  _node)
{
	static string const valStr = "value";
	static string const hexValStr = "hexValue";

	astAssert(member(_node, valStr).isString() || member(_node, hexValStr).isString(), "Literal-value is unset.");

	ASTPointer<ASTString> value = _node.isMember(hexValStr) ?
		make_shared<ASTString>(util::asString(util::fromHex(_node[hexValStr].asString()))) :
		make_shared<ASTString>(_node[valStr].asString());

	return createASTNode<Literal>(
		_node,
		literalTokenKind(_node),
		value,
		member(_node, "subdenomination").isNull() ? Literal::SubDenomination::None : subdenomination(_node)
	);
}

ASTPointer<StructuredDocumentation> ASTJsonImporter::createDocumentation(Json::Value const&  _node)
{
	static string const textString = "text";

	astAssert(member(_node, textString).isString(), "'text' must be a string");

	return createASTNode<StructuredDocumentation>(
		_node,
		make_shared<ASTString>(_node[textString].asString())
	);
}

// ===== helper functions ==========

Json::Value ASTJsonImporter::member(Json::Value const& _node, string const& _name)
{
	if (!_node.isMember(_name))
		return Json::nullValue;
	return _node[_name];
}

Token ASTJsonImporter::scanSingleToken(Json::Value const& _node)
{
	langutil::CharStream charStream(_node.asString(), "");
	langutil::Scanner scanner{charStream};
	astAssert(scanner.peekNextToken() == Token::EOS, "Token string is too long.");
	return scanner.currentToken();
}

ASTPointer<ASTString> ASTJsonImporter::nullOrASTString(Json::Value const& _json, string const& _name)
{
	return _json[_name].isString() ? memberAsASTString(_json, _name) : nullptr;
}

ASTPointer<ASTString> ASTJsonImporter::memberAsASTString(Json::Value const& _node, string const& _name)
{
	Json::Value value = member(_node, _name);
	astAssert(value.isString(), "field " + _name + " must be of type string.");
	return make_shared<ASTString>(_node[_name].asString());
}

bool ASTJsonImporter::memberAsBool(Json::Value const& _node, string const& _name)
{
	Json::Value value = member(_node, _name);
	astAssert(value.isBool(), "field " + _name + " must be of type boolean.");
	return _node[_name].asBool();
}


// =========== JSON to definition helpers =======================

ContractKind ASTJsonImporter::contractKind(Json::Value const& _node)
{
	ContractKind kind;
	astAssert(!member(_node, "contractKind").isNull(), "'Contract-kind' can not be null.");
	if (_node["contractKind"].asString() == "interface")
		kind = ContractKind::Interface;
	else if (_node["contractKind"].asString() == "contract")
		kind = ContractKind::Contract;
	else if (_node["contractKind"].asString() == "library")
		kind = ContractKind::Library;
	else
		astAssert(false, "Unknown ContractKind");
	return kind;
}

Token ASTJsonImporter::literalTokenKind(Json::Value const& _node)
{
	astAssert(member(_node, "kind").isString(), "Token-'kind' expected to be a string.");
	Token tok;
	if (_node["kind"].asString() == "number")
		tok = Token::Number;
	else if (_node["kind"].asString() == "string")
		tok = Token::StringLiteral;
	else if (_node["kind"].asString() == "unicodeString")
		tok = Token::UnicodeStringLiteral;
	else if (_node["kind"].asString() == "hexString")
		tok = Token::HexStringLiteral;
	else if (_node["kind"].asString() == "bool")
		tok = (member(_node, "value").asString() == "true") ? Token::TrueLiteral : Token::FalseLiteral;
	else
		astAssert(false, "Unknown kind of literalString");
	return tok;
}

Visibility ASTJsonImporter::visibility(Json::Value const& _node)
{
	Json::Value visibility = member(_node, "visibility");
	astAssert(visibility.isString(), "'visibility' expected to be a string.");

	string const visibilityStr = visibility.asString();

	if (visibilityStr == "default")
		return Visibility::Default;
	else if (visibilityStr == "private")
		return Visibility::Private;
	else if ( visibilityStr == "internal")
		return Visibility::Internal;
	else if (visibilityStr == "public")
		return Visibility::Public;
	else if (visibilityStr == "external")
		return Visibility::External;
	else
		astAssert(false, "Unknown visibility declaration");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

VariableDeclaration::Location ASTJsonImporter::location(Json::Value const& _node)
{
	Json::Value storageLoc = member(_node, "storageLocation");
	astAssert(storageLoc.isString(), "'storageLocation' expected to be a string.");

	string const storageLocStr = storageLoc.asString();

	if (storageLocStr == "default")
		return VariableDeclaration::Location::Unspecified;
	else if (storageLocStr == "storage")
		return VariableDeclaration::Location::Storage;
	else if (storageLocStr == "memory")
		return VariableDeclaration::Location::Memory;
	else if (storageLocStr == "calldata")
		return VariableDeclaration::Location::CallData;
	else
		astAssert(false, "Unknown location declaration");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

Literal::SubDenomination ASTJsonImporter::subdenomination(Json::Value const& _node)
{
	Json::Value subDen = member(_node, "subdenomination");

	if (subDen.isNull())
		return Literal::SubDenomination::None;

	astAssert(subDen.isString(), "'subDenomination' expected to be string.");

	string const subDenStr = subDen.asString();

	if (subDenStr == "wei")
		return Literal::SubDenomination::Wei;
	else if (subDenStr == "gwei")
		return Literal::SubDenomination::Gwei;
	else if (subDenStr == "ether")
		return Literal::SubDenomination::Ether;
	else if (subDenStr == "seconds")
		return Literal::SubDenomination::Second;
	else if (subDenStr == "minutes")
		return Literal::SubDenomination::Minute;
	else if (subDenStr == "hours")
		return Literal::SubDenomination::Hour;
	else if (subDenStr == "days")
		return Literal::SubDenomination::Day;
	else if (subDenStr == "weeks")
		return Literal::SubDenomination::Week;
	else if (subDenStr == "years")
		return Literal::SubDenomination::Year;
	else
		astAssert(false, "Unknown subdenomination");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

StateMutability ASTJsonImporter::stateMutability(Json::Value const& _node)
{
	astAssert(member(_node, "stateMutability").isString(), "StateMutability' expected to be string.");
	string const mutabilityStr = member(_node, "stateMutability").asString();

	if (mutabilityStr == "pure")
		return StateMutability::Pure;
	else if (mutabilityStr == "view")
		return StateMutability::View;
	else if (mutabilityStr == "nonpayable")
		return StateMutability::NonPayable;
	else if (mutabilityStr == "payable")
		return StateMutability::Payable;
	else
		astAssert(false, "Unknown stateMutability");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

}
