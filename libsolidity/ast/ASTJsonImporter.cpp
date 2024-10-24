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
#include <libsolidity/ast/UserDefinableOperators.h>

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

namespace solidity::frontend
{

using SourceLocation = langutil::SourceLocation;

template<class T>
ASTPointer<T> ASTJsonImporter::nullOrCast(Json const& _json)
{
	if (_json.is_null())
		return nullptr;
	else
		return std::dynamic_pointer_cast<T>(convertJsonToASTNode(_json));
}


// ============ public ===========================

std::map<std::string, ASTPointer<SourceUnit>> ASTJsonImporter::jsonToSourceUnit(std::map<std::string, Json> const& _sourceList)
{
	for (auto const& src: _sourceList)
		m_sourceNames.emplace_back(std::make_shared<std::string const>(src.first));
	for (auto const& srcPair: _sourceList)
	{
		astAssert(!srcPair.second.is_null());
		astAssert(member(srcPair.second,"nodeType") == "SourceUnit", "The 'nodeType' of the highest node must be 'SourceUnit'.");
		m_sourceUnits[srcPair.first] = createSourceUnit(srcPair.second, srcPair.first);
	}
	return m_sourceUnits;
}

// ============ private ===========================

// =========== general creation functions ==============
template <typename T, typename... Args>
ASTPointer<T> ASTJsonImporter::createASTNode(Json const& _node, Args&&... _args)
{
	static_assert(std::is_same_v<Json::number_unsigned_t, uint64_t>);
	astAssert(member(_node, "id").is_number_integer(), "'id'-field must be 64bit integer.");

	int64_t id = static_cast<Json::number_integer_t>(_node["id"]);

	astAssert(m_usedIDs.insert(id).second, "Found duplicate node ID!");

	auto n = std::make_shared<T>(
		id,
		createSourceLocation(_node),
		std::forward<Args>(_args)...
	);
	return n;
}

SourceLocation const ASTJsonImporter::createSourceLocation(Json const& _node)
{
	astAssert(member(_node, "src").is_string(), "'src' must be a string");

	return solidity::langutil::parseSourceLocation(_node["src"].get<std::string>(), m_sourceNames);
}

std::optional<std::vector<SourceLocation>> ASTJsonImporter::createSourceLocations(Json const& _node) const
{
	std::vector<SourceLocation> locations;

	if (_node.contains("nameLocations") && _node["nameLocations"].is_array())
	{
		for (auto const& val: _node["nameLocations"])
			locations.emplace_back(langutil::parseSourceLocation(val.get<std::string>(), m_sourceNames));
		return locations;
	}

	return std::nullopt;
}

SourceLocation ASTJsonImporter::createNameSourceLocation(Json const& _node)
{
	astAssert(member(_node, "nameLocation").is_string(), "'nameLocation' must be a string");

	return solidity::langutil::parseSourceLocation(_node["nameLocation"].get<std::string>(), m_sourceNames);
}

SourceLocation ASTJsonImporter::createKeyNameSourceLocation(Json const& _node)
{
	astAssert(member(_node, "keyNameLocation").is_string(), "'keyNameLocation' must be a string");

	return solidity::langutil::parseSourceLocation(_node["keyNameLocation"].get<std::string>(), m_sourceNames);
}

SourceLocation ASTJsonImporter::createValueNameSourceLocation(Json const& _node)
{
	astAssert(member(_node, "valueNameLocation").is_string(), "'valueNameLocation' must be a string");

	return solidity::langutil::parseSourceLocation(_node["valueNameLocation"].get<std::string>(), m_sourceNames);
}

template<class T>
ASTPointer<T> ASTJsonImporter::convertJsonToASTNode(Json const& _node)
{
	ASTPointer<T> ret = std::dynamic_pointer_cast<T>(convertJsonToASTNode(_node));
	astAssert(ret, "cast of converted json-node must not be nullptr");
	return ret;
}


ASTPointer<ASTNode> ASTJsonImporter::convertJsonToASTNode(Json const& _json)
{
	astAssert(_json["nodeType"].is_string() && _json.contains("id"), "JSON-Node needs to have 'nodeType' and 'id' fields.");
	std::string nodeType = _json["nodeType"].get<std::string>();
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

ASTPointer<SourceUnit> ASTJsonImporter::createSourceUnit(Json const& _node, std::string const& _srcName)
{
	std::optional<std::string> license;
	if (_node.contains("license") && !_node["license"].is_null())
		license = _node["license"].get<std::string>();

	bool experimentalSolidity = false;
	if (_node.contains("experimentalSolidity") && !_node["experimentalSolidity"].is_null())
		experimentalSolidity = _node["experimentalSolidity"].get<bool>();

	std::vector<ASTPointer<ASTNode>> nodes;
	for (auto& child: member(_node, "nodes"))
		nodes.emplace_back(convertJsonToASTNode(child));

	ASTPointer<SourceUnit> tmp = createASTNode<SourceUnit>(_node, license, nodes, experimentalSolidity);
	tmp->annotation().path = _srcName;
	return tmp;
}

ASTPointer<PragmaDirective> ASTJsonImporter::createPragmaDirective(Json const& _node)
{
	std::vector<Token> tokens;
	std::vector<ASTString> literals;
	for (auto const& lit: member(_node, "literals"))
	{
		std::string l = lit.get<std::string>();
		literals.push_back(l);
		tokens.push_back(scanSingleToken(l));
	}
	return createASTNode<PragmaDirective>(_node, tokens, literals);
}

ASTPointer<ImportDirective> ASTJsonImporter::createImportDirective(Json const& _node)
{
	ASTPointer<ASTString> unitAlias = memberAsASTString(_node, "unitAlias");
	ASTPointer<ASTString> path = memberAsASTString(_node, "file");
	ImportDirective::SymbolAliasList symbolAliases;

	for (auto& tuple: member(_node, "symbolAliases"))
	{
		astAssert(tuple["local"].is_null() || tuple["local"].is_string(), "expected 'local' to be a string or null!");

		symbolAliases.push_back({
			createIdentifier(tuple["foreign"]),
			tuple["local"].is_null() ? nullptr : std::make_shared<ASTString>(tuple["local"].get<std::string>()),
			createSourceLocation(tuple["foreign"])}
		);
	}
	ASTPointer<ImportDirective> tmp = createASTNode<ImportDirective>(
		_node,
		path,
		unitAlias,
		createNameSourceLocation(_node),
		std::move(symbolAliases)
	);

	astAssert(_node["absolutePath"].is_string(), "Expected 'absolutePath' to be a string!");

	tmp->annotation().absolutePath = _node["absolutePath"].get<std::string>();
	return tmp;
}

ASTPointer<ContractDefinition> ASTJsonImporter::createContractDefinition(Json const& _node)
{
	astAssert(_node["name"].is_string(), "Expected 'name' to be a string!");

	std::vector<ASTPointer<InheritanceSpecifier>> baseContracts;

	for (auto& base: _node["baseContracts"])
		baseContracts.push_back(createInheritanceSpecifier(base));

	std::vector<ASTPointer<ASTNode>> subNodes;

	for (auto& subnode: _node["nodes"])
		subNodes.push_back(convertJsonToASTNode(subnode));

	return createASTNode<ContractDefinition>(
		_node,
		std::make_shared<ASTString>(_node["name"].get<std::string>()),
		createNameSourceLocation(_node),
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr,
		baseContracts,
		subNodes,
		contractKind(_node),
		memberAsBool(_node, "abstract")
	);
}

ASTPointer<IdentifierPath> ASTJsonImporter::createIdentifierPath(Json const& _node)
{
	astAssert(_node["name"].is_string(), "Expected 'name' to be a string!");

	std::vector<ASTString> namePath;
	std::vector<SourceLocation> namePathLocations;
	std::vector<std::string> strs;
	std::string nameString = member(_node, "name").get<std::string>();
	boost::algorithm::split(strs, nameString, boost::is_any_of("."));
	astAssert(!strs.empty(), "Expected at least one element in IdentifierPath.");
	for (std::string s: strs)
	{
		astAssert(!s.empty(), "Expected non-empty string for IdentifierPath element.");
		namePath.emplace_back(s);
	}

	if (_node.contains("nameLocations") && _node["nameLocations"].is_array())
		for (auto const& val: _node["nameLocations"])
			namePathLocations.emplace_back(langutil::parseSourceLocation(val.get<std::string>(), m_sourceNames));
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

ASTPointer<InheritanceSpecifier> ASTJsonImporter::createInheritanceSpecifier(Json const& _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(convertJsonToASTNode<Expression>(arg));
	return createASTNode<InheritanceSpecifier>(
		_node,
		createIdentifierPath(member(_node, "baseName")),
		member(_node, "arguments").is_null() ? nullptr : std::make_unique<std::vector<ASTPointer<Expression>>>(arguments)
	);
}

ASTPointer<UsingForDirective> ASTJsonImporter::createUsingForDirective(Json const& _node)
{
	std::vector<ASTPointer<IdentifierPath>> functions;
	std::vector<std::optional<Token>> operators;
	if (_node.contains("libraryName"))
	{
		astAssert(!_node["libraryName"].is_array());
		astAssert(!_node["libraryName"].contains("operator"));
		functions.emplace_back(createIdentifierPath(_node["libraryName"]));
		operators.emplace_back(std::nullopt);
	}
	else if (_node.contains("functionList"))
		for (Json const& function: _node["functionList"])
		{
			if (function.contains("function"))
			{
				astAssert(!function.contains("operator"));
				astAssert(!function.contains("definition"));

				functions.emplace_back(createIdentifierPath(function["function"]));
				operators.emplace_back(std::nullopt);
			}
			else
			{
				astAssert(function.contains("operator"));
				astAssert(function.contains("definition"));

				Token const operatorName = scanSingleToken(function["operator"]);
				astAssert(util::contains(frontend::userDefinableOperators, operatorName));

				functions.emplace_back(createIdentifierPath(function["definition"]));
				operators.emplace_back(operatorName);
			}
		}

	return createASTNode<UsingForDirective>(
		_node,
		std::move(functions),
		std::move(operators),
		!_node.contains("libraryName"),
		(_node.contains("typeName") && !_node["typeName"].is_null()) ? convertJsonToASTNode<TypeName>(_node["typeName"]) : nullptr,
		memberAsBool(_node, "global")
	);
}

ASTPointer<ASTNode> ASTJsonImporter::createStructDefinition(Json const& _node)
{
	std::vector<ASTPointer<VariableDeclaration>> members;
	for (auto& member: _node["members"])
		members.push_back(createVariableDeclaration(member));
	return createASTNode<StructDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		members,
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr
	);
}

ASTPointer<EnumDefinition> ASTJsonImporter::createEnumDefinition(Json const& _node)
{
	std::vector<ASTPointer<EnumValue>> members;
	for (auto& member: _node["members"])
		members.push_back(createEnumValue(member));
	return createASTNode<EnumDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		members,
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr
	);
}

ASTPointer<EnumValue> ASTJsonImporter::createEnumValue(Json const& _node)
{
	return createASTNode<EnumValue>(
		_node,
		memberAsASTString(_node, "name")
	);
}

ASTPointer<UserDefinedValueTypeDefinition> ASTJsonImporter::createUserDefinedValueTypeDefinition(Json const& _node)
{
	return createASTNode<UserDefinedValueTypeDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		convertJsonToASTNode<TypeName>(member(_node, "underlyingType"))
	);
}

ASTPointer<ParameterList> ASTJsonImporter::createParameterList(Json const&  _node)
{
	std::vector<ASTPointer<VariableDeclaration>> parameters;
	for (auto& param: _node["parameters"])
		parameters.push_back(createVariableDeclaration(param));
	return createASTNode<ParameterList>(
		_node,
		parameters
	);
}

ASTPointer<OverrideSpecifier> ASTJsonImporter::createOverrideSpecifier(Json const&  _node)
{
	std::vector<ASTPointer<IdentifierPath>> overrides;

	if ( _node.contains("overrides"))
		for (auto& param: _node["overrides"])
			overrides.push_back(createIdentifierPath(param));

	return createASTNode<OverrideSpecifier>(
		_node,
		overrides
	);
}

ASTPointer<FunctionDefinition> ASTJsonImporter::createFunctionDefinition(Json const&  _node)
{
	astAssert(_node["kind"].is_string(), "Expected 'kind' to be a string!");

	Token kind;
	bool freeFunction = false;
	std::string kindStr = member(_node, "kind").get<std::string>();

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
		(_node.contains("overrides") && !_node["overrides"].is_null()) ? createOverrideSpecifier(member(_node, "overrides")) : nullptr,
		(_node.contains("documentation") && !_node["documentation"].is_null()) ?  createDocumentation(member(_node, "documentation")) : nullptr,
		createParameterList(member(_node, "parameters")),
		modifiers,
		createParameterList(member(_node, "returnParameters")),
		memberAsBool(_node, "implemented") ? createBlock(member(_node, "body"), false) : nullptr
	);
}

ASTPointer<VariableDeclaration> ASTJsonImporter::createVariableDeclaration(Json const& _node)
{
	astAssert(_node["name"].is_string(), "Expected 'name' to be a string!");

	VariableDeclaration::Mutability mutability{};
	astAssert(member(_node, "mutability").is_string(), "'mutability' expected to be string.");
	std::string const mutabilityStr = member(_node, "mutability").get<std::string>();
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
		std::make_shared<ASTString>(member(_node, "name").get<std::string>()),
		createNameSourceLocation(_node),
		nullOrCast<Expression>(member(_node, "value")),
		visibility(_node),
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr,
		(_node.contains("indexed") && !_node["indexed"].is_null()) ? memberAsBool(_node, "indexed") : false,
		mutability,
		(_node.contains("overrides") && !_node["overrides"].is_null()) ? createOverrideSpecifier(member(_node, "overrides")) : nullptr,
		location(_node)
	);
}

ASTPointer<ModifierDefinition> ASTJsonImporter::createModifierDefinition(Json const&  _node)
{
	return createASTNode<ModifierDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr,
		createParameterList(member(_node, "parameters")),
		memberAsBool(_node, "virtual"),
		(_node.contains("overrides") && !_node["overrides"].is_null()) ? createOverrideSpecifier(member(_node, "overrides")) : nullptr,
		(_node.contains("body") && !_node["body"].is_null()) ? createBlock(member(_node, "body"), false) : nullptr
	);
}

ASTPointer<ModifierInvocation> ASTJsonImporter::createModifierInvocation(Json const&  _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(convertJsonToASTNode<Expression>(arg));
	return createASTNode<ModifierInvocation>(
		_node,
		createIdentifierPath(member(_node, "modifierName")),
		member(_node, "arguments").is_null() ? nullptr : std::make_unique<std::vector<ASTPointer<Expression>>>(arguments)
	);
}

ASTPointer<EventDefinition> ASTJsonImporter::createEventDefinition(Json const&  _node)
{
	return createASTNode<EventDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr,
		createParameterList(member(_node, "parameters")),
		memberAsBool(_node, "anonymous")
	);
}

ASTPointer<ErrorDefinition> ASTJsonImporter::createErrorDefinition(Json const&  _node)
{
	return createASTNode<ErrorDefinition>(
		_node,
		memberAsASTString(_node, "name"),
		createNameSourceLocation(_node),
		(_node.contains("documentation") && !_node["documentation"].is_null()) ? createDocumentation(member(_node, "documentation")) : nullptr,
		createParameterList(member(_node, "parameters"))
	);
}

ASTPointer<ElementaryTypeName> ASTJsonImporter::createElementaryTypeName(Json const& _node)
{
	unsigned short firstNum;
	unsigned short secondNum;

	astAssert(_node["name"].is_string(), "Expected 'name' to be a string!");

	std::string name = member(_node, "name").get<std::string>();
	Token token;
	std::tie(token, firstNum, secondNum) = TokenTraits::fromIdentifierOrKeyword(name);
	ElementaryTypeNameToken elem(token, firstNum,  secondNum);

	std::optional<StateMutability> mutability = {};
	if (_node.contains("stateMutability"))
		mutability = stateMutability(_node);

	return createASTNode<ElementaryTypeName>(_node, elem, mutability);
}

ASTPointer<UserDefinedTypeName> ASTJsonImporter::createUserDefinedTypeName(Json const& _node)
{
	return createASTNode<UserDefinedTypeName>(
		_node,
		createIdentifierPath(member(_node, "pathNode"))
	);
}

ASTPointer<FunctionTypeName> ASTJsonImporter::createFunctionTypeName(Json const&  _node)
{
	return createASTNode<FunctionTypeName>(
		_node,
		createParameterList(member(_node, "parameterTypes")),
		createParameterList(member(_node, "returnParameterTypes")),
		visibility(_node),
		stateMutability(_node)
	);
}

ASTPointer<Mapping> ASTJsonImporter::createMapping(Json const&  _node)
{
	return createASTNode<Mapping>(
		_node,
		convertJsonToASTNode<TypeName>(member(_node, "keyType")),
		memberAsASTString(_node, "keyName"),
		createKeyNameSourceLocation(_node),
		convertJsonToASTNode<TypeName>(member(_node, "valueType")),
		memberAsASTString(_node, "valueName"),
		createValueNameSourceLocation(_node)
	);
}

ASTPointer<ArrayTypeName> ASTJsonImporter::createArrayTypeName(Json const&  _node)
{
	return createASTNode<ArrayTypeName>(
		_node,
		convertJsonToASTNode<TypeName>(member(_node, "baseType")),
		nullOrCast<Expression>(member(_node, "length"))
	);
}

ASTPointer<InlineAssembly> ASTJsonImporter::createInlineAssembly(Json const& _node)
{
	astAssert(_node["evmVersion"].is_string(), "Expected evmVersion to be a string!");
	auto evmVersion = langutil::EVMVersion::fromString(_node["evmVersion"].get<std::string>());
	astAssert(evmVersion.has_value(), "Invalid EVM version!");
	astAssert(m_evmVersion == evmVersion, "Imported tree evm version differs from configured evm version!");

	// TODO: Add test in test/linsolidity/ASTJSON/assembly. This requires adding support for eofVersion in ASTJSONTest
	std::optional<uint8_t> eofVersion;
	if (auto const it = _node.find("eofVersion"); it != _node.end())
	{
		eofVersion = it->get<uint8_t>();
		astAssert(eofVersion > 0);
	}
	astAssert(m_eofVersion == eofVersion, "Imported tree EOF version differs from configured EOF version!");

	yul::Dialect const& dialect = yul::EVMDialect::strictAssemblyForEVM(evmVersion.value(), eofVersion);
	ASTPointer<std::vector<ASTPointer<ASTString>>> flags;
	if (_node.contains("flags"))
	{
		flags = std::make_shared<std::vector<ASTPointer<ASTString>>>();
		Json const& flagsNode = _node["flags"];
		astAssert(flagsNode.is_array(), "Assembly flags must be an array.");
		for (auto const& flag: flagsNode)
		{
			astAssert(flag.is_string(), "Assembly flag must be a string.");
			flags->emplace_back(std::make_shared<ASTString>(flag.get<std::string>()));
		}
	}
	std::shared_ptr<yul::AST> operations = std::make_shared<yul::AST>(
		yul::AsmJsonImporter(dialect, m_sourceNames).createAST(member(_node, "AST"))
	);
	return createASTNode<InlineAssembly>(
		_node,
		nullOrASTString(_node, "documentation"),
		dialect,
		std::move(flags),
		operations
	);
}

ASTPointer<Block> ASTJsonImporter::createBlock(Json const& _node, bool _unchecked)
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

ASTPointer<PlaceholderStatement> ASTJsonImporter::createPlaceholderStatement(Json const&  _node)
{
	return createASTNode<PlaceholderStatement>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<IfStatement> ASTJsonImporter::createIfStatement(Json const&  _node)
{
	return createASTNode<IfStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		convertJsonToASTNode<Expression>(member(_node, "condition")),
		convertJsonToASTNode<Statement>(member(_node, "trueBody")),
		nullOrCast<Statement>(member(_node, "falseBody"))
	);
}

ASTPointer<TryCatchClause> ASTJsonImporter::createTryCatchClause(Json const&  _node)
{
	return createASTNode<TryCatchClause>(
		_node,
		memberAsASTString(_node, "errorName"),
		nullOrCast<ParameterList>(member(_node, "parameters")),
		convertJsonToASTNode<Block>(member(_node, "block"))
	);
}

ASTPointer<TryStatement> ASTJsonImporter::createTryStatement(Json const&  _node)
{
	std::vector<ASTPointer<TryCatchClause>> clauses;

	for (auto& param: _node["clauses"])
		clauses.emplace_back(createTryCatchClause(param));

	return createASTNode<TryStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		convertJsonToASTNode<Expression>(member(_node, "externalCall")),
		clauses
	);
}

ASTPointer<WhileStatement> ASTJsonImporter::createWhileStatement(Json const&  _node, bool _isDoWhile=false)
{
	return createASTNode<WhileStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		convertJsonToASTNode<Expression>(member(_node, "condition")),
		convertJsonToASTNode<Statement>(member(_node, "body")),
		_isDoWhile
	);
}

ASTPointer<ForStatement> ASTJsonImporter::createForStatement(Json const&  _node)
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

ASTPointer<Continue> ASTJsonImporter::createContinue(Json const&  _node)
{
	return createASTNode<Continue>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<Break> ASTJsonImporter::createBreak(Json const&  _node)
{
	return createASTNode<Break>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<Return> ASTJsonImporter::createReturn(Json const&  _node)
{
	return createASTNode<Return>(
		_node,
		nullOrASTString(_node, "documentation"),
		nullOrCast<Expression>(member(_node, "expression"))
	);
}

ASTPointer<Throw> ASTJsonImporter::createThrow(Json const&  _node)
{
	return createASTNode<Throw>(
		_node,
		nullOrASTString(_node, "documentation")
	);
}

ASTPointer<EmitStatement> ASTJsonImporter::createEmitStatement(Json const&  _node)
{
	return createASTNode<EmitStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		createFunctionCall(member(_node, "eventCall"))
	);
}

ASTPointer<RevertStatement> ASTJsonImporter::createRevertStatement(Json const&  _node)
{
	return createASTNode<RevertStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		createFunctionCall(member(_node, "errorCall"))
	);
}

ASTPointer<VariableDeclarationStatement> ASTJsonImporter::createVariableDeclarationStatement(Json const& _node)
{
	std::vector<ASTPointer<VariableDeclaration>> variables;
	for (auto& var: member(_node, "declarations"))
		variables.push_back(var.is_null() ? nullptr : createVariableDeclaration(var)); //unnamed components are empty pointers
	return createASTNode<VariableDeclarationStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		variables,
		nullOrCast<Expression>(member(_node, "initialValue"))
	);
}

ASTPointer<ExpressionStatement> ASTJsonImporter::createExpressionStatement(Json const&  _node)
{
	return createASTNode<ExpressionStatement>(
		_node,
		nullOrASTString(_node, "documentation"),
		convertJsonToASTNode<Expression>(member(_node, "expression"))
	);
}

ASTPointer<Conditional> ASTJsonImporter::createConditional(Json const&  _node)
{
	return createASTNode<Conditional>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "condition")),
		convertJsonToASTNode<Expression>(member(_node, "trueExpression")),
		convertJsonToASTNode<Expression>(member(_node, "falseExpression"))
	);
}

ASTPointer<Assignment> ASTJsonImporter::createAssignment(Json const&  _node)
{
	return createASTNode<Assignment>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "leftHandSide")),
		scanSingleToken(member(_node, "operator")),
		convertJsonToASTNode<Expression>(member(_node, "rightHandSide"))
	);
}

ASTPointer<TupleExpression> ASTJsonImporter::createTupleExpression(Json const&  _node)
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

ASTPointer<UnaryOperation> ASTJsonImporter::createUnaryOperation(Json const&  _node)
{
	return createASTNode<UnaryOperation>(
		_node,
		scanSingleToken(member(_node, "operator")),
		convertJsonToASTNode<Expression>(member(_node, "subExpression")),
		memberAsBool(_node, "prefix")
	);
}

ASTPointer<BinaryOperation> ASTJsonImporter::createBinaryOperation(Json const&  _node)
{
	return createASTNode<BinaryOperation>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "leftExpression")),
		scanSingleToken(member(_node, "operator")),
		convertJsonToASTNode<Expression>(member(_node, "rightExpression"))
	);
}

ASTPointer<FunctionCall> ASTJsonImporter::createFunctionCall(Json const&  _node)
{
	std::vector<ASTPointer<Expression>> arguments;
	for (auto& arg: member(_node, "arguments"))
		arguments.push_back(convertJsonToASTNode<Expression>(arg));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: member(_node, "names"))
	{
		astAssert(name.is_string(), "Expected 'names' members to be strings!");
		names.push_back(std::make_shared<ASTString>(name.get<std::string>()));
	}

	std::optional<std::vector<SourceLocation>> sourceLocations = createSourceLocations(_node);

	return createASTNode<FunctionCall>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "expression")),
		arguments,
		names,
		sourceLocations ?
			*sourceLocations :
			std::vector<SourceLocation>(names.size())
	);
}

ASTPointer<FunctionCallOptions> ASTJsonImporter::createFunctionCallOptions(Json const&  _node)
{
	std::vector<ASTPointer<Expression>> options;
	for (auto& option: member(_node, "options"))
		options.push_back(convertJsonToASTNode<Expression>(option));
	std::vector<ASTPointer<ASTString>> names;
	for (auto& name: member(_node, "names"))
	{
		astAssert(name.is_string(), "Expected 'names' members to be strings!");
		names.push_back(std::make_shared<ASTString>(name.get<std::string>()));
	}

	return createASTNode<FunctionCallOptions>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "expression")),
		options,
		names
	);
}

ASTPointer<NewExpression> ASTJsonImporter::createNewExpression(Json const&  _node)
{
	return createASTNode<NewExpression>(
		_node,
		convertJsonToASTNode<TypeName>(member(_node, "typeName"))
	);
}

ASTPointer<MemberAccess> ASTJsonImporter::createMemberAccess(Json const&  _node)
{
	SourceLocation memberLocation;
	if (member(_node, "memberLocation").is_string())
		memberLocation = solidity::langutil::parseSourceLocation(_node["memberLocation"].get<std::string>(), m_sourceNames);

	return createASTNode<MemberAccess>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "expression")),
		memberAsASTString(_node, "memberName"),
		std::move(memberLocation)
	);
}

ASTPointer<IndexAccess> ASTJsonImporter::createIndexAccess(Json const& _node)
{
	return createASTNode<IndexAccess>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "baseExpression")),
		nullOrCast<Expression>(member(_node, "indexExpression"))
	);
}

ASTPointer<IndexRangeAccess> ASTJsonImporter::createIndexRangeAccess(Json const& _node)
{
	return createASTNode<IndexRangeAccess>(
		_node,
		convertJsonToASTNode<Expression>(member(_node, "baseExpression")),
		nullOrCast<Expression>(member(_node, "startExpression")),
		nullOrCast<Expression>(member(_node, "endExpression"))
	);
}

ASTPointer<Identifier> ASTJsonImporter::createIdentifier(Json const& _node)
{
	return createASTNode<Identifier>(_node, memberAsASTString(_node, "name"));
}

ASTPointer<ElementaryTypeNameExpression> ASTJsonImporter::createElementaryTypeNameExpression(Json const&  _node)
{
	return createASTNode<ElementaryTypeNameExpression>(
		_node,
		createElementaryTypeName(member(_node, "typeName"))
	);
}

ASTPointer<ASTNode> ASTJsonImporter::createLiteral(Json const&  _node)
{
	static std::string const valStr = "value";
	static std::string const hexValStr = "hexValue";

	astAssert(member(_node, valStr).is_string() || member(_node, hexValStr).is_string(), "Literal-value is unset.");

	ASTPointer<ASTString> value = _node.contains(hexValStr) ?
		std::make_shared<ASTString>(util::asString(util::fromHex(_node[hexValStr].get<std::string>()))) :
		std::make_shared<ASTString>(_node[valStr].get<std::string>());

	return createASTNode<Literal>(
		_node,
		literalTokenKind(_node),
		value,
		member(_node, "subdenomination").is_null() ? Literal::SubDenomination::None : subdenomination(_node)
	);
}

ASTPointer<StructuredDocumentation> ASTJsonImporter::createDocumentation(Json const&  _node)
{
	static std::string const textString = "text";

	astAssert(member(_node, textString).is_string(), "'text' must be a string");

	return createASTNode<StructuredDocumentation>(
		_node,
		std::make_shared<ASTString>(_node[textString].get<std::string>())
	);
}

// ===== helper functions ==========

Json ASTJsonImporter::member(Json const& _node, std::string const& _name)
{
	if (!_node.contains(_name))
		return Json();
	return _node[_name];
}

Token ASTJsonImporter::scanSingleToken(Json const& _node)
{
	langutil::CharStream charStream(_node.get<std::string>(), "");
	langutil::Scanner scanner{charStream};
	astAssert(scanner.peekNextToken() == Token::EOS, "Token string is too long.");
	return scanner.currentToken();
}

ASTPointer<ASTString> ASTJsonImporter::nullOrASTString(Json const& _json, std::string const& _name)
{
	return (_json.contains(_name) && (_json[_name].is_string())) ? memberAsASTString(_json, _name) : nullptr;
}

ASTPointer<ASTString> ASTJsonImporter::memberAsASTString(Json const& _node, std::string const& _name)
{
	Json value = member(_node, _name);
	astAssert(value.is_string(), "field " + _name + " must be of type string.");
	return std::make_shared<ASTString>(_node[_name].get<std::string>());
}

bool ASTJsonImporter::memberAsBool(Json const& _node, std::string const& _name)
{
	Json value = member(_node, _name);
	astAssert(value.is_boolean(), "field " + _name + " must be of type boolean.");
	return _node[_name].get<bool>();
}


// =========== JSON to definition helpers =======================

ContractKind ASTJsonImporter::contractKind(Json const& _node)
{
	ContractKind kind;
	astAssert(!member(_node, "contractKind").is_null(), "'Contract-kind' can not be null.");
	if (_node["contractKind"].get<std::string>() == "interface")
		kind = ContractKind::Interface;
	else if (_node["contractKind"].get<std::string>() == "contract")
		kind = ContractKind::Contract;
	else if (_node["contractKind"].get<std::string>() == "library")
		kind = ContractKind::Library;
	else
		astAssert(false, "Unknown ContractKind");
	return kind;
}

Token ASTJsonImporter::literalTokenKind(Json const& _node)
{
	astAssert(member(_node, "kind").is_string(), "Token-'kind' expected to be a string.");
	Token tok;
	if (_node["kind"].get<std::string>() == "number")
		tok = Token::Number;
	else if (_node["kind"].get<std::string>() == "string")
		tok = Token::StringLiteral;
	else if (_node["kind"].get<std::string>() == "unicodeString")
		tok = Token::UnicodeStringLiteral;
	else if (_node["kind"].get<std::string>() == "hexString")
		tok = Token::HexStringLiteral;
	else if (_node["kind"].get<std::string>() == "bool")
		tok = (member(_node, "value").get<std::string>() == "true") ? Token::TrueLiteral : Token::FalseLiteral;
	else
		astAssert(false, "Unknown kind of literalString");
	return tok;
}

Visibility ASTJsonImporter::visibility(Json const& _node)
{
	Json visibility = member(_node, "visibility");
	astAssert(visibility.is_string(), "'visibility' expected to be a string.");

	std::string const visibilityStr = visibility.get<std::string>();

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

VariableDeclaration::Location ASTJsonImporter::location(Json const& _node)
{
	Json storageLoc = member(_node, "storageLocation");
	astAssert(storageLoc.is_string(), "'storageLocation' expected to be a string.");

	std::string const storageLocStr = storageLoc.get<std::string>();

	if (storageLocStr == "default")
		return VariableDeclaration::Location::Unspecified;
	else if (storageLocStr == "storage")
		return VariableDeclaration::Location::Storage;
	else if (storageLocStr == "memory")
		return VariableDeclaration::Location::Memory;
	else if (storageLocStr == "calldata")
		return VariableDeclaration::Location::CallData;
	else if (storageLocStr == "transient")
		return VariableDeclaration::Location::Transient;
	else
		astAssert(false, "Unknown location declaration");

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

Literal::SubDenomination ASTJsonImporter::subdenomination(Json const& _node)
{
	Json subDen = member(_node, "subdenomination");

	if (subDen.is_null())
		return Literal::SubDenomination::None;

	astAssert(subDen.is_string(), "'subDenomination' expected to be string.");

	std::string const subDenStr = subDen.get<std::string>();

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

StateMutability ASTJsonImporter::stateMutability(Json const& _node)
{
	astAssert(member(_node, "stateMutability").is_string(), "StateMutability' expected to be string.");
	std::string const mutabilityStr = member(_node, "stateMutability").get<std::string>();

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
