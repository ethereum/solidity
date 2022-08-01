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
 * @date 2017
 * Converts the AST into json format
 */

#include <libsolidity/ast/ASTJsonExporter.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libyul/AsmJsonConverter.h>
#include <libyul/AST.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libsolutil/JSON.h>
#include <libsolutil/UTF8.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string/join.hpp>

#include <utility>
#include <vector>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <range/v3/view/map.hpp>

using namespace std;
using namespace solidity::langutil;

namespace
{

template<typename V, template<typename> typename C>
void addIfSet(std::vector<pair<string, Json::Value>>& _attributes, string const& _name, C<V> const& _value)
{
	if constexpr (std::is_same_v<C<V>, solidity::util::SetOnce<V>>)
	{
		if (!_value.set())
			return;
	}
	else if constexpr (std::is_same_v<C<V>, optional<V>>)
	{
		if (!_value.has_value())
			return;
	}

	_attributes.emplace_back(_name, *_value);
}

}

namespace solidity::frontend
{

ASTJsonExporter::ASTJsonExporter(CompilerStack::State _stackState, map<string, unsigned> _sourceIndices):
	m_stackState(_stackState),
	m_sourceIndices(std::move(_sourceIndices))
{
}


void ASTJsonExporter::setJsonNode(
	ASTNode const& _node,
	string const& _nodeName,
	initializer_list<pair<string, Json::Value>>&& _attributes
)
{
	ASTJsonExporter::setJsonNode(
		_node,
		_nodeName,
		std::vector<pair<string, Json::Value>>(std::move(_attributes))
	);
}

void ASTJsonExporter::setJsonNode(
	ASTNode const& _node,
	string const& _nodeType,
	std::vector<pair<string, Json::Value>>&& _attributes
)
{
	m_currentValue = Json::objectValue;
	m_currentValue["id"] = nodeId(_node);
	m_currentValue["src"] = sourceLocationToString(_node.location());
	if (auto const* documented = dynamic_cast<Documented const*>(&_node))
		if (documented->documentation())
			m_currentValue["documentation"] = *documented->documentation();
	m_currentValue["nodeType"] = _nodeType;
	for (auto& e: _attributes)
		m_currentValue[e.first] = std::move(e.second);
}

optional<size_t> ASTJsonExporter::sourceIndexFromLocation(SourceLocation const& _location) const
{
	if (_location.sourceName && m_sourceIndices.count(*_location.sourceName))
		return m_sourceIndices.at(*_location.sourceName);
	else
		return nullopt;
}

string ASTJsonExporter::sourceLocationToString(SourceLocation const& _location) const
{
	optional<size_t> sourceIndexOpt = sourceIndexFromLocation(_location);
	int length = -1;
	if (_location.start >= 0 && _location.end >= 0)
		length = _location.end - _location.start;
	return to_string(_location.start) + ":" + to_string(length) + ":" + (sourceIndexOpt.has_value() ? to_string(sourceIndexOpt.value()) : "-1");
}

Json::Value ASTJsonExporter::sourceLocationsToJson(vector<SourceLocation> const& _sourceLocations) const
{
	Json::Value locations = Json::arrayValue;

	for (SourceLocation const& location: _sourceLocations)
		locations.append(sourceLocationToString(location));

	return locations;
}

string ASTJsonExporter::namePathToString(std::vector<ASTString> const& _namePath)
{
	return boost::algorithm::join(_namePath, ".");
}

Json::Value ASTJsonExporter::typePointerToJson(Type const* _tp, bool _withoutDataLocation)
{
	Json::Value typeDescriptions(Json::objectValue);
	typeDescriptions["typeString"] = _tp ? Json::Value(_tp->toString(_withoutDataLocation)) : Json::nullValue;
	typeDescriptions["typeIdentifier"] = _tp ? Json::Value(_tp->identifier()) : Json::nullValue;
	return typeDescriptions;

}
Json::Value ASTJsonExporter::typePointerToJson(std::optional<FuncCallArguments> const& _tps)
{
	if (_tps)
	{
		Json::Value arguments(Json::arrayValue);
		for (auto const& tp: _tps->types)
			appendMove(arguments, typePointerToJson(tp));
		return arguments;
	}
	else
		return Json::nullValue;
}

void ASTJsonExporter::appendExpressionAttributes(
	std::vector<pair<string, Json::Value>>& _attributes,
	ExpressionAnnotation const& _annotation
)
{
	std::vector<pair<string, Json::Value>> exprAttributes = {
		make_pair("typeDescriptions", typePointerToJson(_annotation.type)),
		make_pair("argumentTypes", typePointerToJson(_annotation.arguments))
	};

	addIfSet(exprAttributes, "isLValue", _annotation.isLValue);
	addIfSet(exprAttributes, "isPure", _annotation.isPure);
	addIfSet(exprAttributes, "isConstant", _annotation.isConstant);

	if (m_stackState > CompilerStack::State::ParsedAndImported)
		exprAttributes.emplace_back("lValueRequested", _annotation.willBeWrittenTo);

	_attributes += exprAttributes;
}

Json::Value ASTJsonExporter::inlineAssemblyIdentifierToJson(pair<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> _info) const
{
	Json::Value tuple(Json::objectValue);
	tuple["src"] = sourceLocationToString(nativeLocationOf(*_info.first));
	tuple["declaration"] = idOrNull(_info.second.declaration);
	tuple["isSlot"] = Json::Value(_info.second.suffix == "slot");
	tuple["isOffset"] = Json::Value(_info.second.suffix == "offset");

	if (!_info.second.suffix.empty())
		tuple["suffix"] = Json::Value(_info.second.suffix);

	tuple["valueSize"] = Json::Value(Json::LargestUInt(_info.second.valueSize));

	return tuple;
}

void ASTJsonExporter::print(ostream& _stream, ASTNode const& _node, util::JsonFormat const& _format)
{
	_stream << util::jsonPrint(toJson(_node), _format);
}

Json::Value ASTJsonExporter::toJson(ASTNode const& _node)
{
	_node.accept(*this);
	return util::removeNullMembers(std::move(m_currentValue));
}

bool ASTJsonExporter::visit(SourceUnit const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("license", _node.licenseString() ? Json::Value(*_node.licenseString()) : Json::nullValue),
		make_pair("nodes", toJson(_node.nodes()))
	};

	if (_node.annotation().exportedSymbols.set())
	{
		Json::Value exportedSymbols = Json::objectValue;
		for (auto const& sym: *_node.annotation().exportedSymbols)
		{
			exportedSymbols[sym.first] = Json::arrayValue;
			for (Declaration const* overload: sym.second)
				exportedSymbols[sym.first].append(nodeId(*overload));
		}

		attributes.emplace_back("exportedSymbols", exportedSymbols);
	};

	addIfSet(attributes, "absolutePath", _node.annotation().path);

	setJsonNode(_node, "SourceUnit", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(PragmaDirective const& _node)
{
	Json::Value literals(Json::arrayValue);
	for (auto const& literal: _node.literals())
		literals.append(literal);
	setJsonNode(_node, "PragmaDirective", {
		make_pair("literals", std::move(literals))
	});
	return false;
}

bool ASTJsonExporter::visit(ImportDirective const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("file", _node.path()),
		make_pair("sourceUnit", idOrNull(_node.annotation().sourceUnit)),
		make_pair("scope", idOrNull(_node.scope()))
	};

	addIfSet(attributes, "absolutePath", _node.annotation().absolutePath);

	attributes.emplace_back("unitAlias", _node.name());
	attributes.emplace_back("nameLocation", Json::Value(sourceLocationToString(_node.nameLocation())));

	Json::Value symbolAliases(Json::arrayValue);
	for (auto const& symbolAlias: _node.symbolAliases())
	{
		Json::Value tuple(Json::objectValue);
		solAssert(symbolAlias.symbol, "");
		tuple["foreign"] = toJson(*symbolAlias.symbol);
		tuple["local"] =  symbolAlias.alias ? Json::Value(*symbolAlias.alias) : Json::nullValue;
		tuple["nameLocation"] = sourceLocationToString(_node.nameLocation());
		symbolAliases.append(tuple);
	}
	attributes.emplace_back("symbolAliases", std::move(symbolAliases));
	setJsonNode(_node, "ImportDirective", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(ContractDefinition const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json::nullValue),
		make_pair("contractKind", contractKind(_node.contractKind())),
		make_pair("abstract", _node.abstract()),
		make_pair("baseContracts", toJson(_node.baseContracts())),
		make_pair("contractDependencies", getContainerIds(_node.annotation().contractDependencies | ranges::views::keys)),
		make_pair("usedErrors", getContainerIds(_node.interfaceErrors(false))),
		make_pair("nodes", toJson(_node.subNodes())),
		make_pair("scope", idOrNull(_node.scope()))
	};
	addIfSet(attributes, "canonicalName", _node.annotation().canonicalName);

	if (_node.annotation().unimplementedDeclarations.has_value())
		attributes.emplace_back("fullyImplemented", _node.annotation().unimplementedDeclarations->empty());
	if (!_node.annotation().linearizedBaseContracts.empty())
		attributes.emplace_back("linearizedBaseContracts", getContainerIds(_node.annotation().linearizedBaseContracts));

	setJsonNode(_node, "ContractDefinition", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(IdentifierPath const& _node)
{
	Json::Value nameLocations = Json::arrayValue;

	for (SourceLocation location: _node.pathLocations())
		nameLocations.append(sourceLocationToString(location));

	setJsonNode(_node, "IdentifierPath", {
		make_pair("name", namePathToString(_node.path())),
		make_pair("nameLocations", nameLocations),
		make_pair("referencedDeclaration", idOrNull(_node.annotation().referencedDeclaration))
	});
	return false;
}

bool ASTJsonExporter::visit(InheritanceSpecifier const& _node)
{
	setJsonNode(_node, "InheritanceSpecifier", {
		make_pair("baseName", toJson(_node.name())),
		make_pair("arguments", _node.arguments() ? toJson(*_node.arguments()) : Json::nullValue)
	});
	return false;
}

bool ASTJsonExporter::visit(UsingForDirective const& _node)
{
	vector<pair<string, Json::Value>> attributes = {
		make_pair("typeName", _node.typeName() ? toJson(*_node.typeName()) : Json::nullValue)
	};

	if (_node.usesBraces())
	{
		Json::Value functionList;
		for (auto&& [function, op]: _node.functionsAndOperators())
		{
			Json::Value functionNode;
			functionNode["function"] = toJson(*function);
			if (op)
				functionNode["operator"] = string(TokenTraits::toString(*op));
			functionList.append(move(functionNode));
		}
		attributes.emplace_back("functionList", std::move(functionList));
	}
	else
		attributes.emplace_back("libraryName", toJson(*_node.functionsOrLibrary().front()));
	attributes.emplace_back("global", _node.global());

	setJsonNode(_node, "UsingForDirective", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(StructDefinition const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		make_pair("members", toJson(_node.members())),
		make_pair("scope", idOrNull(_node.scope()))
	};

	addIfSet(attributes,"canonicalName", _node.annotation().canonicalName);

	setJsonNode(_node, "StructDefinition", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(EnumDefinition const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("members", toJson(_node.members()))
	};

	addIfSet(attributes,"canonicalName", _node.annotation().canonicalName);

	setJsonNode(_node, "EnumDefinition", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(EnumValue const& _node)
{
	setJsonNode(_node, "EnumValue", {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
	});
	return false;
}

bool ASTJsonExporter::visit(UserDefinedValueTypeDefinition const& _node)
{
	solAssert(_node.underlyingType(), "");
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("underlyingType", toJson(*_node.underlyingType()))
	};
	addIfSet(attributes, "canonicalName", _node.annotation().canonicalName);

	setJsonNode(_node, "UserDefinedValueTypeDefinition", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(ParameterList const& _node)
{
	setJsonNode(_node, "ParameterList", {
		make_pair("parameters", toJson(_node.parameters()))
	});
	return false;
}

bool ASTJsonExporter::visit(OverrideSpecifier const& _node)
{
	setJsonNode(_node, "OverrideSpecifier", {
		make_pair("overrides", toJson(_node.overrides()))
	});
	return false;
}

bool ASTJsonExporter::visit(FunctionDefinition const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json::nullValue),
		make_pair("kind", _node.isFree() ? "freeFunction" : TokenTraits::toString(_node.kind())),
		make_pair("stateMutability", stateMutabilityToString(_node.stateMutability())),
		make_pair("virtual", _node.markedVirtual()),
		make_pair("overrides", _node.overrides() ? toJson(*_node.overrides()) : Json::nullValue),
		make_pair("parameters", toJson(_node.parameterList())),
		make_pair("returnParameters", toJson(*_node.returnParameterList())),
		make_pair("modifiers", toJson(_node.modifiers())),
		make_pair("body", _node.isImplemented() ? toJson(_node.body()) : Json::nullValue),
		make_pair("implemented", _node.isImplemented()),
		make_pair("scope", idOrNull(_node.scope()))
	};

	optional<Visibility> visibility;
	if (_node.isConstructor())
	{
		if (_node.annotation().contract)
			visibility = _node.annotation().contract->abstract() ? Visibility::Internal : Visibility::Public;
	}
	else
		visibility = _node.visibility();

	if (visibility)
		attributes.emplace_back("visibility", Declaration::visibilityToString(*visibility));

	if (_node.isPartOfExternalInterface() && m_stackState > CompilerStack::State::ParsedAndImported)
		attributes.emplace_back("functionSelector", _node.externalIdentifierHex());
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(make_pair("baseFunctions", getContainerIds(_node.annotation().baseFunctions, true)));
	setJsonNode(_node, "FunctionDefinition", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(VariableDeclaration const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("typeName", toJson(_node.typeName())),
		make_pair("constant", _node.isConstant()),
		make_pair("mutability", VariableDeclaration::mutabilityToString(_node.mutability())),
		make_pair("stateVariable", _node.isStateVariable()),
		make_pair("storageLocation", location(_node.referenceLocation())),
		make_pair("overrides", _node.overrides() ? toJson(*_node.overrides()) : Json::nullValue),
		make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		make_pair("value", _node.value() ? toJson(*_node.value()) : Json::nullValue),
		make_pair("scope", idOrNull(_node.scope())),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	};
	if (_node.isStateVariable() && _node.isPublic())
		attributes.emplace_back("functionSelector", _node.externalIdentifierHex());
	if (_node.isStateVariable() && _node.documentation())
		attributes.emplace_back("documentation", toJson(*_node.documentation()));
	if (m_inEvent)
		attributes.emplace_back("indexed", _node.isIndexed());
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(make_pair("baseFunctions", getContainerIds(_node.annotation().baseFunctions, true)));
	setJsonNode(_node, "VariableDeclaration", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(ModifierDefinition const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json::nullValue),
		make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		make_pair("parameters", toJson(_node.parameterList())),
		make_pair("virtual", _node.markedVirtual()),
		make_pair("overrides", _node.overrides() ? toJson(*_node.overrides()) : Json::nullValue),
		make_pair("body", _node.isImplemented() ? toJson(_node.body()) : Json::nullValue)
	};
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(make_pair("baseModifiers", getContainerIds(_node.annotation().baseFunctions, true)));
	setJsonNode(_node, "ModifierDefinition", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(ModifierInvocation const& _node)
{
	std::vector<pair<string, Json::Value>> attributes{
		make_pair("modifierName", toJson(_node.name())),
		make_pair("arguments", _node.arguments() ? toJson(*_node.arguments()) : Json::nullValue)
	};
	if (Declaration const* declaration = _node.name().annotation().referencedDeclaration)
	{
		if (dynamic_cast<ModifierDefinition const*>(declaration))
			attributes.emplace_back("kind", "modifierInvocation");
		else if (dynamic_cast<ContractDefinition const*>(declaration))
			attributes.emplace_back("kind", "baseConstructorSpecifier");
	}
	setJsonNode(_node, "ModifierInvocation", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(EventDefinition const& _node)
{
	m_inEvent = true;
	std::vector<pair<string, Json::Value>> _attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json::nullValue),
		make_pair("parameters", toJson(_node.parameterList())),
		make_pair("anonymous", _node.isAnonymous())
	};
	if (m_stackState >= CompilerStack::State::AnalysisPerformed)
			_attributes.emplace_back(
				make_pair(
					"eventSelector",
					toHex(u256(util::h256::Arith(util::keccak256(_node.functionType(true)->externalSignature()))))
				));

	setJsonNode(_node, "EventDefinition", std::move(_attributes));
	return false;
}

bool ASTJsonExporter::visit(ErrorDefinition const& _node)
{
	std::vector<pair<string, Json::Value>> _attributes = {
		make_pair("name", _node.name()),
		make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json::nullValue),
		make_pair("parameters", toJson(_node.parameterList()))
	};
	if (m_stackState >= CompilerStack::State::AnalysisPerformed)
		_attributes.emplace_back(make_pair("errorSelector", _node.functionType(true)->externalIdentifierHex()));

	setJsonNode(_node, "ErrorDefinition", std::move(_attributes));
	return false;
}

bool ASTJsonExporter::visit(ElementaryTypeName const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("name", _node.typeName().toString()),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	};

	if (_node.stateMutability())
		attributes.emplace_back(make_pair("stateMutability", stateMutabilityToString(*_node.stateMutability())));

	setJsonNode(_node, "ElementaryTypeName", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(UserDefinedTypeName const& _node)
{
	setJsonNode(_node, "UserDefinedTypeName", {
		make_pair("pathNode", toJson(_node.pathNode())),
		make_pair("referencedDeclaration", idOrNull(_node.pathNode().annotation().referencedDeclaration)),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(FunctionTypeName const& _node)
{
	setJsonNode(_node, "FunctionTypeName", {
		make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		make_pair("stateMutability", stateMutabilityToString(_node.stateMutability())),
		make_pair("parameterTypes", toJson(*_node.parameterTypeList())),
		make_pair("returnParameterTypes", toJson(*_node.returnParameterTypeList())),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(Mapping const& _node)
{
	setJsonNode(_node, "Mapping", {
		make_pair("keyType", toJson(_node.keyType())),
		make_pair("valueType", toJson(_node.valueType())),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(ArrayTypeName const& _node)
{
	setJsonNode(_node, "ArrayTypeName", {
		make_pair("baseType", toJson(_node.baseType())),
		make_pair("length", toJsonOrNull(_node.length())),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(InlineAssembly const& _node)
{
	vector<pair<string, Json::Value>> externalReferences;

	for (auto const& it: _node.annotation().externalReferences)
		if (it.first)
			externalReferences.emplace_back(make_pair(
				it.first->name.str(),
				inlineAssemblyIdentifierToJson(it)
			));

	Json::Value externalReferencesJson = Json::arrayValue;

	std::sort(externalReferences.begin(), externalReferences.end());
	for (Json::Value& it: externalReferences | ranges::views::values)
		externalReferencesJson.append(std::move(it));

	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("AST", Json::Value(yul::AsmJsonConverter(sourceIndexFromLocation(_node.location()))(_node.operations()))),
		make_pair("externalReferences", std::move(externalReferencesJson)),
		make_pair("evmVersion", dynamic_cast<solidity::yul::EVMDialect const&>(_node.dialect()).evmVersion().name())
	};

	if (_node.flags())
	{
		Json::Value flags(Json::arrayValue);
		for (auto const& flag: *_node.flags())
			if (flag)
				flags.append(*flag);
			else
				flags.append(Json::nullValue);
		attributes.emplace_back(make_pair("flags", std::move(flags)));
	}
	setJsonNode(_node, "InlineAssembly", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(Block const& _node)
{
	setJsonNode(_node, _node.unchecked() ? "UncheckedBlock" : "Block", {
		make_pair("statements", toJson(_node.statements()))
	});
	return false;
}

bool ASTJsonExporter::visit(PlaceholderStatement const& _node)
{
	setJsonNode(_node, "PlaceholderStatement", {});
	return false;
}

bool ASTJsonExporter::visit(IfStatement const& _node)
{
	setJsonNode(_node, "IfStatement", {
		make_pair("condition", toJson(_node.condition())),
		make_pair("trueBody", toJson(_node.trueStatement())),
		make_pair("falseBody", toJsonOrNull(_node.falseStatement()))
	});
	return false;
}

bool ASTJsonExporter::visit(TryCatchClause const& _node)
{
	setJsonNode(_node, "TryCatchClause", {
		make_pair("errorName", _node.errorName()),
		make_pair("parameters", toJsonOrNull(_node.parameters())),
		make_pair("block", toJson(_node.block()))
	});
	return false;
}

bool ASTJsonExporter::visit(TryStatement const& _node)
{
	setJsonNode(_node, "TryStatement", {
		make_pair("externalCall", toJson(_node.externalCall())),
		make_pair("clauses", toJson(_node.clauses()))
	});
	return false;
}

bool ASTJsonExporter::visit(WhileStatement const& _node)
{
	setJsonNode(
		_node,
		_node.isDoWhile() ? "DoWhileStatement" : "WhileStatement",
		{
			make_pair("condition", toJson(_node.condition())),
			make_pair("body", toJson(_node.body()))
		}
	);
	return false;
}

bool ASTJsonExporter::visit(ForStatement const& _node)
{
	setJsonNode(_node, "ForStatement", {
		make_pair("initializationExpression", toJsonOrNull(_node.initializationExpression())),
		make_pair("condition", toJsonOrNull(_node.condition())),
		make_pair("loopExpression", toJsonOrNull(_node.loopExpression())),
		make_pair("body", toJson(_node.body()))
	});
	return false;
}

bool ASTJsonExporter::visit(Continue const& _node)
{
	setJsonNode(_node, "Continue", {});
	return false;
}

bool ASTJsonExporter::visit(Break const& _node)
{
	setJsonNode(_node, "Break", {});
	return false;
}

bool ASTJsonExporter::visit(Return const& _node)
{
	setJsonNode(_node, "Return", {
		make_pair("expression", toJsonOrNull(_node.expression())),
		make_pair("functionReturnParameters", idOrNull(_node.annotation().functionReturnParameters))
	});
	return false;
}

bool ASTJsonExporter::visit(Throw const& _node)
{
	setJsonNode(_node, "Throw", {});
	return false;
}

bool ASTJsonExporter::visit(EmitStatement const& _node)
{
	setJsonNode(_node, "EmitStatement", {
		make_pair("eventCall", toJson(_node.eventCall()))
	});
	return false;
}

bool ASTJsonExporter::visit(RevertStatement const& _node)
{
	setJsonNode(_node, "RevertStatement", {
		make_pair("errorCall", toJson(_node.errorCall()))
	});
	return false;
}

bool ASTJsonExporter::visit(VariableDeclarationStatement const& _node)
{
	Json::Value varDecs(Json::arrayValue);
	for (auto const& v: _node.declarations())
		appendMove(varDecs, idOrNull(v.get()));
	setJsonNode(_node, "VariableDeclarationStatement", {
		make_pair("assignments", std::move(varDecs)),
		make_pair("declarations", toJson(_node.declarations())),
		make_pair("initialValue", toJsonOrNull(_node.initialValue()))
	});
	return false;
}

bool ASTJsonExporter::visit(ExpressionStatement const& _node)
{
	setJsonNode(_node, "ExpressionStatement", {
		make_pair("expression", toJson(_node.expression()))
	});
	return false;
}

bool ASTJsonExporter::visit(Conditional const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("condition", toJson(_node.condition())),
		make_pair("trueExpression", toJson(_node.trueExpression())),
		make_pair("falseExpression", toJson(_node.falseExpression()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "Conditional", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(Assignment const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("operator", TokenTraits::toString(_node.assignmentOperator())),
		make_pair("leftHandSide", toJson(_node.leftHandSide())),
		make_pair("rightHandSide", toJson(_node.rightHandSide()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "Assignment", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(TupleExpression const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("isInlineArray", Json::Value(_node.isInlineArray())),
		make_pair("components", toJson(_node.components())),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "TupleExpression", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(UnaryOperation const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("prefix", _node.isPrefixOperation()),
		make_pair("operator", TokenTraits::toString(_node.getOperator())),
		make_pair("subExpression", toJson(_node.subExpression()))
	};
	if (FunctionDefinition const* function = *_node.annotation().userDefinedFunction)
		attributes.emplace_back("function", nodeId(*function));
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "UnaryOperation", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(BinaryOperation const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("operator", TokenTraits::toString(_node.getOperator())),
		make_pair("leftExpression", toJson(_node.leftExpression())),
		make_pair("rightExpression", toJson(_node.rightExpression())),
		make_pair("commonType", typePointerToJson(_node.annotation().commonType)),
	};
	if (FunctionDefinition const* function = *_node.annotation().userDefinedFunction)
		attributes.emplace_back("function", nodeId(*function));
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "BinaryOperation", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(FunctionCall const& _node)
{
	Json::Value names(Json::arrayValue);
	for (auto const& name: _node.names())
		names.append(Json::Value(*name));
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("expression", toJson(_node.expression())),
		make_pair("names", std::move(names)),
		make_pair("nameLocations", sourceLocationsToJson(_node.nameLocations())),
		make_pair("arguments", toJson(_node.arguments())),
		make_pair("tryCall", _node.annotation().tryCall)
	};

	if (_node.annotation().kind.set())
	{
		FunctionCallKind nodeKind = *_node.annotation().kind;
		attributes.emplace_back("kind", functionCallKind(nodeKind));
	}

	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "FunctionCall", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(FunctionCallOptions const& _node)
{
	Json::Value names(Json::arrayValue);
	for (auto const& name: _node.names())
		names.append(Json::Value(*name));

	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("expression", toJson(_node.expression())),
		make_pair("names", std::move(names)),
		make_pair("options", toJson(_node.options())),
	};
	appendExpressionAttributes(attributes, _node.annotation());

	setJsonNode(_node, "FunctionCallOptions", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(NewExpression const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("typeName", toJson(_node.typeName()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "NewExpression", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(MemberAccess const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("memberName", _node.memberName()),
		make_pair("memberLocation", Json::Value(sourceLocationToString(_node.memberLocation()))),
		make_pair("expression", toJson(_node.expression())),
		make_pair("referencedDeclaration", idOrNull(_node.annotation().referencedDeclaration)),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "MemberAccess", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(IndexAccess const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("baseExpression", toJson(_node.baseExpression())),
		make_pair("indexExpression", toJsonOrNull(_node.indexExpression())),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "IndexAccess", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(IndexRangeAccess const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("baseExpression", toJson(_node.baseExpression())),
		make_pair("startExpression", toJsonOrNull(_node.startExpression())),
		make_pair("endExpression", toJsonOrNull(_node.endExpression())),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "IndexRangeAccess", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(Identifier const& _node)
{
	Json::Value overloads(Json::arrayValue);
	for (auto const& dec: _node.annotation().overloadedDeclarations)
		overloads.append(nodeId(*dec));
	setJsonNode(_node, "Identifier", {
		make_pair("name", _node.name()),
		make_pair("referencedDeclaration", idOrNull(_node.annotation().referencedDeclaration)),
		make_pair("overloadedDeclarations", overloads),
		make_pair("typeDescriptions", typePointerToJson(_node.annotation().type)),
		make_pair("argumentTypes", typePointerToJson(_node.annotation().arguments))
	});
	return false;
}

bool ASTJsonExporter::visit(ElementaryTypeNameExpression const& _node)
{
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("typeName", toJson(_node.type()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "ElementaryTypeNameExpression", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(Literal const& _node)
{
	Json::Value value{_node.value()};
	if (!util::validateUTF8(_node.value()))
		value = Json::nullValue;
	Token subdenomination = Token(_node.subDenomination());
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("kind", literalTokenKind(_node.token())),
		make_pair("value", value),
		make_pair("hexValue", util::toHex(util::asBytes(_node.value()))),
		make_pair(
			"subdenomination",
			subdenomination == Token::Illegal ?
			Json::nullValue :
			Json::Value{TokenTraits::toString(subdenomination)}
		)
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "Literal", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(StructuredDocumentation const& _node)
{
	Json::Value text{*_node.text()};
	std::vector<pair<string, Json::Value>> attributes = {
		make_pair("text", text)
	};
	setJsonNode(_node, "StructuredDocumentation", std::move(attributes));
	return false;
}



void ASTJsonExporter::endVisit(EventDefinition const&)
{
	m_inEvent = false;
}

string ASTJsonExporter::location(VariableDeclaration::Location _location)
{
	switch (_location)
	{
	case VariableDeclaration::Location::Unspecified:
		return "default";
	case VariableDeclaration::Location::Storage:
		return "storage";
	case VariableDeclaration::Location::Memory:
		return "memory";
	case VariableDeclaration::Location::CallData:
		return "calldata";
	}
	// To make the compiler happy
	return {};
}

string ASTJsonExporter::contractKind(ContractKind _kind)
{
	switch (_kind)
	{
	case ContractKind::Interface:
		return "interface";
	case ContractKind::Contract:
		return "contract";
	case ContractKind::Library:
		return "library";
	}

	// To make the compiler happy
	return {};
}

string ASTJsonExporter::functionCallKind(FunctionCallKind _kind)
{
	switch (_kind)
	{
	case FunctionCallKind::FunctionCall:
		return "functionCall";
	case FunctionCallKind::TypeConversion:
		return "typeConversion";
	case FunctionCallKind::StructConstructorCall:
		return "structConstructorCall";
	default:
		solAssert(false, "Unknown kind of function call.");
	}
}

string ASTJsonExporter::literalTokenKind(Token _token)
{
	switch (_token)
	{
	case Token::Number:
		return "number";
	case Token::StringLiteral:
		return "string";
	case Token::UnicodeStringLiteral:
		return "unicodeString";
	case Token::HexStringLiteral:
		return "hexString";
	case Token::TrueLiteral:
	case Token::FalseLiteral:
		return "bool";
	default:
		solAssert(false, "Unknown kind of literal token.");
	}
}

string ASTJsonExporter::type(Expression const& _expression)
{
	return _expression.annotation().type ? _expression.annotation().type->toString() : "Unknown";
}

string ASTJsonExporter::type(VariableDeclaration const& _varDecl)
{
	return _varDecl.annotation().type ? _varDecl.annotation().type->toString() : "Unknown";
}

}
