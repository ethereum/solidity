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

using namespace std::string_literals;
using namespace solidity::langutil;
using namespace solidity;

namespace
{

template<typename V, template<typename> typename C>
void addIfSet(std::vector<std::pair<std::string, Json>>& _attributes, std::string const& _name, C<V> const& _value)
{
	if constexpr (std::is_same_v<C<V>, solidity::util::SetOnce<V>>)
	{
		if (!_value.set())
			return;
	}
	else if constexpr (std::is_same_v<C<V>, std::optional<V>>)
	{
		if (!_value.has_value())
			return;
	}

	_attributes.emplace_back(_name, *_value);
}

}

namespace solidity::frontend
{

ASTJsonExporter::ASTJsonExporter(CompilerStack::State _stackState, std::map<std::string, unsigned> _sourceIndices):
	m_stackState(_stackState),
	m_sourceIndices(std::move(_sourceIndices))
{
}


void ASTJsonExporter::setJsonNode(
	ASTNode const& _node,
	std::string const& _nodeName,
	std::initializer_list<std::pair<std::string, Json>>&& _attributes
)
{
	ASTJsonExporter::setJsonNode(
		_node,
		_nodeName,
		std::vector<std::pair<std::string, Json>>(std::move(_attributes))
	);
}

void ASTJsonExporter::setJsonNode(
	ASTNode const& _node,
	std::string const& _nodeType,
	std::vector<std::pair<std::string, Json>>&& _attributes
)
{
	m_currentValue = Json::object();
	m_currentValue["id"] = nodeId(_node);
	m_currentValue["src"] = sourceLocationToString(_node.location());
	if (auto const* documented = dynamic_cast<Documented const*>(&_node))
		if (documented->documentation())
			m_currentValue["documentation"] = *documented->documentation();
	m_currentValue["nodeType"] = _nodeType;
	for (auto& e: _attributes)
		m_currentValue[e.first] = std::move(e.second);
}

std::optional<size_t> ASTJsonExporter::sourceIndexFromLocation(SourceLocation const& _location) const
{
	if (_location.sourceName && m_sourceIndices.count(*_location.sourceName))
		return m_sourceIndices.at(*_location.sourceName);
	else
		return std::nullopt;
}

std::string ASTJsonExporter::sourceLocationToString(SourceLocation const& _location) const
{
	std::optional<size_t> sourceIndexOpt = sourceIndexFromLocation(_location);
	int length = -1;
	if (_location.start >= 0 && _location.end >= 0)
		length = _location.end - _location.start;
	return std::to_string(_location.start) + ":" + std::to_string(length) + ":" + (sourceIndexOpt.has_value() ? std::to_string(sourceIndexOpt.value()) : "-1");
}

Json ASTJsonExporter::sourceLocationsToJson(std::vector<SourceLocation> const& _sourceLocations) const
{
	Json locations = Json::array();

	for (SourceLocation const& location: _sourceLocations)
		locations.emplace_back(sourceLocationToString(location));

	return locations;
}

std::string ASTJsonExporter::namePathToString(std::vector<ASTString> const& _namePath)
{
	return boost::algorithm::join(_namePath, "."s);
}

Json ASTJsonExporter::typePointerToJson(Type const* _tp, bool _withoutDataLocation)
{
	Json typeDescriptions;
	typeDescriptions["typeString"] = _tp ? Json(_tp->toString(_withoutDataLocation)) : Json();
	typeDescriptions["typeIdentifier"] = _tp ? Json(_tp->identifier()) : Json();
	return typeDescriptions;

}
Json ASTJsonExporter::typePointerToJson(std::optional<FuncCallArguments> const& _tps)
{
	if (_tps)
	{
		Json arguments = Json::array();
		for (auto const& tp: _tps->types)
			appendMove(arguments, typePointerToJson(tp));
		return arguments;
	}
	else
		return Json();
}

void ASTJsonExporter::appendExpressionAttributes(
	std::vector<std::pair<std::string, Json>>& _attributes,
	ExpressionAnnotation const& _annotation
)
{
	std::vector<std::pair<std::string, Json>> exprAttributes = {
		std::make_pair("typeDescriptions", typePointerToJson(_annotation.type)),
		std::make_pair("argumentTypes", typePointerToJson(_annotation.arguments))
	};

	addIfSet(exprAttributes, "isLValue", _annotation.isLValue);
	addIfSet(exprAttributes, "isPure", _annotation.isPure);
	addIfSet(exprAttributes, "isConstant", _annotation.isConstant);

	if (m_stackState > CompilerStack::State::ParsedAndImported)
		exprAttributes.emplace_back("lValueRequested", _annotation.willBeWrittenTo);

	_attributes += exprAttributes;
}

Json ASTJsonExporter::inlineAssemblyIdentifierToJson(std::pair<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> _info) const
{
	Json tuple;
	tuple["src"] = sourceLocationToString(nativeLocationOf(*_info.first));
	tuple["declaration"] = idOrNull(_info.second.declaration);
	tuple["isSlot"] = Json(_info.second.suffix == "slot");
	tuple["isOffset"] = Json(_info.second.suffix == "offset");

	if (!_info.second.suffix.empty())
		tuple["suffix"] = Json(_info.second.suffix);

	tuple["valueSize"] = Json(static_cast<Json::number_integer_t>(_info.second.valueSize));

	return tuple;
}

void ASTJsonExporter::print(std::ostream& _stream, ASTNode const& _node, util::JsonFormat const& _format)
{
	_stream << util::jsonPrint(toJson(_node), _format);
}

Json ASTJsonExporter::toJson(ASTNode const& _node)
{
	_node.accept(*this);
	return util::removeNullMembers(std::move(m_currentValue));
}

bool ASTJsonExporter::visit(SourceUnit const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("license", _node.licenseString() ? Json(*_node.licenseString()) : Json()),
		std::make_pair("nodes", toJson(_node.nodes())),
	};

	if (_node.experimentalSolidity())
		attributes.emplace_back("experimentalSolidity", Json(_node.experimentalSolidity()));

	if (_node.annotation().exportedSymbols.set())
	{
		Json exportedSymbols = Json::object();
		for (auto const& sym: *_node.annotation().exportedSymbols)
		{
			exportedSymbols[sym.first] = Json::array();
			for (Declaration const* overload: sym.second)
				exportedSymbols[sym.first].emplace_back(nodeId(*overload));
		}

		attributes.emplace_back("exportedSymbols", exportedSymbols);
	};

	addIfSet(attributes, "absolutePath", _node.annotation().path);

	setJsonNode(_node, "SourceUnit", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(PragmaDirective const& _node)
{
	Json literals = Json::array();
	for (auto const& literal: _node.literals())
		literals.emplace_back(literal);
	setJsonNode(_node, "PragmaDirective", {
		std::make_pair("literals", std::move(literals))
	});
	return false;
}

bool ASTJsonExporter::visit(ImportDirective const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("file", _node.path()),
		std::make_pair("sourceUnit", idOrNull(_node.annotation().sourceUnit)),
		std::make_pair("scope", idOrNull(_node.scope()))
	};

	addIfSet(attributes, "absolutePath", _node.annotation().absolutePath);

	attributes.emplace_back("unitAlias", _node.name());
	attributes.emplace_back("nameLocation", Json(sourceLocationToString(_node.nameLocation())));

	Json symbolAliases = Json::array();
	for (auto const& symbolAlias: _node.symbolAliases())
	{
		Json tuple;
		solAssert(symbolAlias.symbol, "");
		tuple["foreign"] = toJson(*symbolAlias.symbol);
		tuple["local"] =  symbolAlias.alias ? Json(*symbolAlias.alias) : Json();
		tuple["nameLocation"] = sourceLocationToString(_node.nameLocation());
		symbolAliases.emplace_back(tuple);
	}
	attributes.emplace_back("symbolAliases", std::move(symbolAliases));
	setJsonNode(_node, "ImportDirective", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(ContractDefinition const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("contractKind", contractKind(_node.contractKind())),
		std::make_pair("abstract", _node.abstract()),
		std::make_pair("baseContracts", toJson(_node.baseContracts())),
		std::make_pair("contractDependencies", getContainerIds(_node.annotation().contractDependencies | ranges::views::keys)),
		// Do not require call graph because the AST is also created for incorrect sources.
		std::make_pair("usedEvents", getContainerIds(_node.interfaceEvents(false))),
		std::make_pair("usedErrors", getContainerIds(_node.interfaceErrors(false))),
		std::make_pair("nodes", toJson(_node.subNodes())),
		std::make_pair("scope", idOrNull(_node.scope()))
	};
	addIfSet(attributes, "canonicalName", _node.annotation().canonicalName);

	if (_node.annotation().unimplementedDeclarations.has_value())
		attributes.emplace_back("fullyImplemented", _node.annotation().unimplementedDeclarations->empty());
	if (!_node.annotation().linearizedBaseContracts.empty())
		attributes.emplace_back("linearizedBaseContracts", getContainerIds(_node.annotation().linearizedBaseContracts));

	if (!_node.annotation().internalFunctionIDs.empty())
	{
		Json internalFunctionIDs;
		for (auto const& [functionDefinition, internalFunctionID]: _node.annotation().internalFunctionIDs)
			internalFunctionIDs[std::to_string(functionDefinition->id())] = internalFunctionID;
		attributes.emplace_back("internalFunctionIDs", std::move(internalFunctionIDs));
	}

	setJsonNode(_node, "ContractDefinition", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(IdentifierPath const& _node)
{
	Json nameLocations = Json::array();

	for (SourceLocation location: _node.pathLocations())
		nameLocations.emplace_back(sourceLocationToString(location));

	setJsonNode(_node, "IdentifierPath", {
		std::make_pair("name", namePathToString(_node.path())),
		std::make_pair("nameLocations", nameLocations),
		std::make_pair("referencedDeclaration", idOrNull(_node.annotation().referencedDeclaration))
	});
	return false;
}

bool ASTJsonExporter::visit(InheritanceSpecifier const& _node)
{
	setJsonNode(_node, "InheritanceSpecifier", {
		std::make_pair("baseName", toJson(_node.name())),
		std::make_pair("arguments", _node.arguments() ? toJson(*_node.arguments()) : Json())
	});
	return false;
}

bool ASTJsonExporter::visit(UsingForDirective const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("typeName", _node.typeName() ? toJson(*_node.typeName()) : Json())
	};

	if (_node.usesBraces())
	{
		Json functionList = Json::array();
		for (auto&& [function, op]: _node.functionsAndOperators())
		{
			Json functionNode;
			if (!op.has_value())
				functionNode["function"] = toJson(*function);
			else
			{
				functionNode["definition"] = toJson(*function);
				functionNode["operator"] = std::string(TokenTraits::toString(*op));
			}
			functionList.emplace_back(std::move(functionNode));
		}
		attributes.emplace_back("functionList", std::move(functionList));
	}
	else
	{
		auto const& functionAndOperators = _node.functionsAndOperators();
		solAssert(_node.functionsAndOperators().size() == 1);
		solAssert(!functionAndOperators.front().second.has_value());
		attributes.emplace_back("libraryName", toJson(*(functionAndOperators.front().first)));
	}
	attributes.emplace_back("global", _node.global());

	setJsonNode(_node, "UsingForDirective", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(StructDefinition const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		std::make_pair("members", toJson(_node.members())),
		std::make_pair("scope", idOrNull(_node.scope()))
	};

	addIfSet(attributes,"canonicalName", _node.annotation().canonicalName);

	setJsonNode(_node, "StructDefinition", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(EnumDefinition const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("members", toJson(_node.members()))
	};

	addIfSet(attributes,"canonicalName", _node.annotation().canonicalName);

	setJsonNode(_node, "EnumDefinition", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(EnumValue const& _node)
{
	setJsonNode(_node, "EnumValue", {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
	});
	return false;
}

bool ASTJsonExporter::visit(UserDefinedValueTypeDefinition const& _node)
{
	solAssert(_node.underlyingType(), "");
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("underlyingType", toJson(*_node.underlyingType()))
	};
	addIfSet(attributes, "canonicalName", _node.annotation().canonicalName);

	setJsonNode(_node, "UserDefinedValueTypeDefinition", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(ParameterList const& _node)
{
	setJsonNode(_node, "ParameterList", {
		std::make_pair("parameters", toJson(_node.parameters()))
	});
	return false;
}

bool ASTJsonExporter::visit(OverrideSpecifier const& _node)
{
	setJsonNode(_node, "OverrideSpecifier", {
		std::make_pair("overrides", toJson(_node.overrides()))
	});
	return false;
}

bool ASTJsonExporter::visit(FunctionDefinition const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("kind", _node.isFree() ? "freeFunction" : TokenTraits::toString(_node.kind())),
		std::make_pair("stateMutability", stateMutabilityToString(_node.stateMutability())),
		std::make_pair("virtual", _node.markedVirtual()),
		std::make_pair("overrides", _node.overrides() ? toJson(*_node.overrides()) : Json()),
		std::make_pair("parameters", toJson(_node.parameterList())),
		std::make_pair("returnParameters", toJson(*_node.returnParameterList())),
		std::make_pair("modifiers", toJson(_node.modifiers())),
		std::make_pair("body", _node.isImplemented() ? toJson(_node.body()) : Json()),
		std::make_pair("implemented", _node.isImplemented()),
		std::make_pair("scope", idOrNull(_node.scope()))
	};

	std::optional<Visibility> visibility;
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
		attributes.emplace_back(std::make_pair("baseFunctions", getContainerIds(_node.annotation().baseFunctions, true)));

	setJsonNode(_node, "FunctionDefinition", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(VariableDeclaration const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("typeName", toJson(_node.typeName())),
		std::make_pair("constant", _node.isConstant()),
		std::make_pair("mutability", VariableDeclaration::mutabilityToString(_node.mutability())),
		std::make_pair("stateVariable", _node.isStateVariable()),
		std::make_pair("storageLocation", location(_node.referenceLocation())),
		std::make_pair("overrides", _node.overrides() ? toJson(*_node.overrides()) : Json()),
		std::make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		std::make_pair("value", _node.value() ? toJson(*_node.value()) : Json()),
		std::make_pair("scope", idOrNull(_node.scope())),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	};
	if (_node.isStateVariable() && _node.isPublic())
		attributes.emplace_back("functionSelector", _node.externalIdentifierHex());
	if (_node.isStateVariable() && _node.documentation())
		attributes.emplace_back("documentation", toJson(*_node.documentation()));
	if (m_inEvent)
		attributes.emplace_back("indexed", _node.isIndexed());
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(std::make_pair("baseFunctions", getContainerIds(_node.annotation().baseFunctions, true)));
	setJsonNode(_node, "VariableDeclaration", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(ModifierDefinition const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		std::make_pair("parameters", toJson(_node.parameterList())),
		std::make_pair("virtual", _node.markedVirtual()),
		std::make_pair("overrides", _node.overrides() ? toJson(*_node.overrides()) : Json()),
		std::make_pair("body", _node.isImplemented() ? toJson(_node.body()) : Json())
	};
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(std::make_pair("baseModifiers", getContainerIds(_node.annotation().baseFunctions, true)));
	setJsonNode(_node, "ModifierDefinition", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(ModifierInvocation const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes{
		std::make_pair("modifierName", toJson(_node.name())),
		std::make_pair("arguments", _node.arguments() ? toJson(*_node.arguments()) : Json())
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
	std::vector<std::pair<std::string, Json>> _attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("parameters", toJson(_node.parameterList())),
		std::make_pair("anonymous", _node.isAnonymous())
	};
	if (m_stackState >= CompilerStack::State::AnalysisSuccessful)
			_attributes.emplace_back(
				std::make_pair(
					"eventSelector",
					toHex(u256(util::h256::Arith(util::keccak256(_node.functionType(true)->externalSignature()))))
				));

	setJsonNode(_node, "EventDefinition", std::move(_attributes));
	return false;
}

bool ASTJsonExporter::visit(ErrorDefinition const& _node)
{
	std::vector<std::pair<std::string, Json>> _attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toJson(*_node.documentation()) : Json()),
		std::make_pair("parameters", toJson(_node.parameterList()))
	};
	if (m_stackState >= CompilerStack::State::AnalysisSuccessful)
		_attributes.emplace_back(std::make_pair("errorSelector", _node.functionType(true)->externalIdentifierHex()));

	setJsonNode(_node, "ErrorDefinition", std::move(_attributes));
	return false;
}

bool ASTJsonExporter::visit(ElementaryTypeName const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("name", _node.typeName().toString()),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	};

	if (_node.stateMutability())
		attributes.emplace_back(std::make_pair("stateMutability", stateMutabilityToString(*_node.stateMutability())));

	setJsonNode(_node, "ElementaryTypeName", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(UserDefinedTypeName const& _node)
{
	setJsonNode(_node, "UserDefinedTypeName", {
		std::make_pair("pathNode", toJson(_node.pathNode())),
		std::make_pair("referencedDeclaration", idOrNull(_node.pathNode().annotation().referencedDeclaration)),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(FunctionTypeName const& _node)
{
	setJsonNode(_node, "FunctionTypeName", {
		std::make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		std::make_pair("stateMutability", stateMutabilityToString(_node.stateMutability())),
		std::make_pair("parameterTypes", toJson(*_node.parameterTypeList())),
		std::make_pair("returnParameterTypes", toJson(*_node.returnParameterTypeList())),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(Mapping const& _node)
{
	setJsonNode(_node, "Mapping", {
		std::make_pair("keyType", toJson(_node.keyType())),
		std::make_pair("keyName", _node.keyName()),
		std::make_pair("keyNameLocation", sourceLocationToString(_node.keyNameLocation())),
		std::make_pair("valueType", toJson(_node.valueType())),
		std::make_pair("valueName", _node.valueName()),
		std::make_pair("valueNameLocation", sourceLocationToString(_node.valueNameLocation())),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(ArrayTypeName const& _node)
{
	setJsonNode(_node, "ArrayTypeName", {
		std::make_pair("baseType", toJson(_node.baseType())),
		std::make_pair("length", toJsonOrNull(_node.length())),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type, true))
	});
	return false;
}

bool ASTJsonExporter::visit(InlineAssembly const& _node)
{
	std::vector<std::pair<std::string, Json>> externalReferences;

	for (auto const& it: _node.annotation().externalReferences)
		if (it.first)
			externalReferences.emplace_back(std::make_pair(
				it.first->name.str(),
				inlineAssemblyIdentifierToJson(it)
			));

	Json externalReferencesJson = Json::array();

	std::sort(externalReferences.begin(), externalReferences.end());
	for (Json& it: externalReferences | ranges::views::values)
		externalReferencesJson.emplace_back(std::move(it));

	auto const& evmDialect = dynamic_cast<solidity::yul::EVMDialect const&>(_node.dialect());

	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("AST", Json(yul::AsmJsonConverter(evmDialect, sourceIndexFromLocation(_node.location()))(_node.operations().root()))),
		std::make_pair("externalReferences", std::move(externalReferencesJson)),
		std::make_pair("evmVersion", evmDialect.evmVersion().name())
	};

	// TODO: Add test in test/linsolidity/ASTJSON/assembly. This requires adding support for eofVersion in ASTJSONTest
	if (evmDialect.eofVersion())
	{
		solAssert(*evmDialect.eofVersion() > 0);
		attributes.push_back(std::make_pair("eofVersion", *evmDialect.eofVersion()));
	}

	if (_node.flags())
	{
		Json flags = Json::array();
		for (auto const& flag: *_node.flags())
			if (flag)
				flags.emplace_back(*flag);
			else
				flags.emplace_back(Json());
		attributes.emplace_back(std::make_pair("flags", std::move(flags)));
	}
	setJsonNode(_node, "InlineAssembly", std::move(attributes));

	return false;
}

bool ASTJsonExporter::visit(Block const& _node)
{
	setJsonNode(_node, _node.unchecked() ? "UncheckedBlock" : "Block", {
		std::make_pair("statements", toJson(_node.statements()))
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
		std::make_pair("condition", toJson(_node.condition())),
		std::make_pair("trueBody", toJson(_node.trueStatement())),
		std::make_pair("falseBody", toJsonOrNull(_node.falseStatement()))
	});
	return false;
}

bool ASTJsonExporter::visit(TryCatchClause const& _node)
{
	setJsonNode(_node, "TryCatchClause", {
		std::make_pair("errorName", _node.errorName()),
		std::make_pair("parameters", toJsonOrNull(_node.parameters())),
		std::make_pair("block", toJson(_node.block()))
	});
	return false;
}

bool ASTJsonExporter::visit(TryStatement const& _node)
{
	setJsonNode(_node, "TryStatement", {
		std::make_pair("externalCall", toJson(_node.externalCall())),
		std::make_pair("clauses", toJson(_node.clauses()))
	});
	return false;
}

bool ASTJsonExporter::visit(WhileStatement const& _node)
{
	setJsonNode(
		_node,
		_node.isDoWhile() ? "DoWhileStatement" : "WhileStatement",
		{
			std::make_pair("condition", toJson(_node.condition())),
			std::make_pair("body", toJson(_node.body()))
		}
	);
	return false;
}

bool ASTJsonExporter::visit(ForStatement const& _node)
{

	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("initializationExpression", toJsonOrNull(_node.initializationExpression())),
		std::make_pair("condition", toJsonOrNull(_node.condition())),
		std::make_pair("loopExpression", toJsonOrNull(_node.loopExpression())),
		std::make_pair("body", toJson(_node.body()))
	};

	if (_node.annotation().isSimpleCounterLoop.set())
		attributes.emplace_back("isSimpleCounterLoop", *_node.annotation().isSimpleCounterLoop);

	setJsonNode(_node, "ForStatement", std::move(attributes));
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
		std::make_pair("expression", toJsonOrNull(_node.expression())),
		std::make_pair("functionReturnParameters", idOrNull(_node.annotation().functionReturnParameters))
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
		std::make_pair("eventCall", toJson(_node.eventCall()))
	});
	return false;
}

bool ASTJsonExporter::visit(RevertStatement const& _node)
{
	setJsonNode(_node, "RevertStatement", {
		std::make_pair("errorCall", toJson(_node.errorCall()))
	});
	return false;
}

bool ASTJsonExporter::visit(VariableDeclarationStatement const& _node)
{
	Json varDecs = Json::array();
	for (auto const& v: _node.declarations())
		appendMove(varDecs, idOrNull(v.get()));
	setJsonNode(_node, "VariableDeclarationStatement", {
		std::make_pair("assignments", std::move(varDecs)),
		std::make_pair("declarations", toJson(_node.declarations())),
		std::make_pair("initialValue", toJsonOrNull(_node.initialValue()))
	});
	return false;
}

bool ASTJsonExporter::visit(ExpressionStatement const& _node)
{
	setJsonNode(_node, "ExpressionStatement", {
		std::make_pair("expression", toJson(_node.expression()))
	});
	return false;
}

bool ASTJsonExporter::visit(Conditional const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("condition", toJson(_node.condition())),
		std::make_pair("trueExpression", toJson(_node.trueExpression())),
		std::make_pair("falseExpression", toJson(_node.falseExpression()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "Conditional", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(Assignment const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("operator", TokenTraits::toString(_node.assignmentOperator())),
		std::make_pair("leftHandSide", toJson(_node.leftHandSide())),
		std::make_pair("rightHandSide", toJson(_node.rightHandSide()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "Assignment", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(TupleExpression const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("isInlineArray", Json(_node.isInlineArray())),
		std::make_pair("components", toJson(_node.components())),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "TupleExpression", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(UnaryOperation const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("prefix", _node.isPrefixOperation()),
		std::make_pair("operator", TokenTraits::toString(_node.getOperator())),
		std::make_pair("subExpression", toJson(_node.subExpression()))
	};
	// NOTE: This annotation is guaranteed to be set but only if we didn't stop at the parsing stage.
	if (_node.annotation().userDefinedFunction.set() && *_node.annotation().userDefinedFunction != nullptr)
		attributes.emplace_back("function", nodeId(**_node.annotation().userDefinedFunction));
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "UnaryOperation", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(BinaryOperation const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("operator", TokenTraits::toString(_node.getOperator())),
		std::make_pair("leftExpression", toJson(_node.leftExpression())),
		std::make_pair("rightExpression", toJson(_node.rightExpression())),
		std::make_pair("commonType", typePointerToJson(_node.annotation().commonType)),
	};
	// NOTE: This annotation is guaranteed to be set but only if we didn't stop at the parsing stage.
	if (_node.annotation().userDefinedFunction.set() && *_node.annotation().userDefinedFunction != nullptr)
		attributes.emplace_back("function", nodeId(**_node.annotation().userDefinedFunction));
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "BinaryOperation", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(FunctionCall const& _node)
{
	Json names = Json::array();
	for (auto const& name: _node.names())
		names.push_back(Json(*name));
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("expression", toJson(_node.expression())),
		std::make_pair("names", std::move(names)),
		std::make_pair("nameLocations", sourceLocationsToJson(_node.nameLocations())),
		std::make_pair("arguments", toJson(_node.arguments())),
		std::make_pair("tryCall", _node.annotation().tryCall)
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
	Json names = Json::array();
	for (auto const& name: _node.names())
		names.emplace_back(Json(*name));

	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("expression", toJson(_node.expression())),
		std::make_pair("names", std::move(names)),
		std::make_pair("options", toJson(_node.options())),
	};
	appendExpressionAttributes(attributes, _node.annotation());

	setJsonNode(_node, "FunctionCallOptions", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(NewExpression const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("typeName", toJson(_node.typeName()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "NewExpression", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(MemberAccess const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("memberName", _node.memberName()),
		std::make_pair("memberLocation", Json(sourceLocationToString(_node.memberLocation()))),
		std::make_pair("expression", toJson(_node.expression())),
		std::make_pair("referencedDeclaration", idOrNull(_node.annotation().referencedDeclaration)),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "MemberAccess", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(IndexAccess const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("baseExpression", toJson(_node.baseExpression())),
		std::make_pair("indexExpression", toJsonOrNull(_node.indexExpression())),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "IndexAccess", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(IndexRangeAccess const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("baseExpression", toJson(_node.baseExpression())),
		std::make_pair("startExpression", toJsonOrNull(_node.startExpression())),
		std::make_pair("endExpression", toJsonOrNull(_node.endExpression())),
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "IndexRangeAccess", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(Identifier const& _node)
{
	Json overloads = Json::array();
	for (auto const& dec: _node.annotation().overloadedDeclarations)
		overloads.emplace_back(nodeId(*dec));
	setJsonNode(_node, "Identifier", {
		std::make_pair("name", _node.name()),
		std::make_pair("referencedDeclaration", idOrNull(_node.annotation().referencedDeclaration)),
		std::make_pair("overloadedDeclarations", overloads),
		std::make_pair("typeDescriptions", typePointerToJson(_node.annotation().type)),
		std::make_pair("argumentTypes", typePointerToJson(_node.annotation().arguments))
	});
	return false;
}

bool ASTJsonExporter::visit(ElementaryTypeNameExpression const& _node)
{
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("typeName", toJson(_node.type()))
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "ElementaryTypeNameExpression", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(Literal const& _node)
{
	Json value = _node.value();
	if (!util::validateUTF8(_node.value()))
		value = Json();
	Token subdenomination = Token(_node.subDenomination());
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("kind", literalTokenKind(_node.token())),
		std::make_pair("value", value),
		std::make_pair("hexValue", util::toHex(util::asBytes(_node.value()))),
		std::make_pair(
			"subdenomination",
			subdenomination == Token::Illegal ?
			Json() :
			Json(TokenTraits::toString(subdenomination))
		)
	};
	appendExpressionAttributes(attributes, _node.annotation());
	setJsonNode(_node, "Literal", std::move(attributes));
	return false;
}

bool ASTJsonExporter::visit(StructuredDocumentation const& _node)
{
	Json text = *_node.text();
	std::vector<std::pair<std::string, Json>> attributes = {
		std::make_pair("text", text)
	};
	setJsonNode(_node, "StructuredDocumentation", std::move(attributes));
	return false;
}

void ASTJsonExporter::endVisit(EventDefinition const&)
{
	m_inEvent = false;
}

bool ASTJsonExporter::visitNode(ASTNode const& _node)
{
	solAssert(false, _node.experimentalSolidityOnly() ?
		"Attempt to export an AST of experimental solidity." :
		"Attempt to export an AST that contains unexpected nodes."
	);
	return false;
}

std::string ASTJsonExporter::location(VariableDeclaration::Location _location)
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
	case VariableDeclaration::Location::Transient:
		return "transient";
	}
	// To make the compiler happy
	return {};
}

std::string ASTJsonExporter::contractKind(ContractKind _kind)
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

std::string ASTJsonExporter::functionCallKind(FunctionCallKind _kind)
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

std::string ASTJsonExporter::literalTokenKind(Token _token)
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

std::string ASTJsonExporter::type(Expression const& _expression)
{
	return _expression.annotation().type ? _expression.annotation().type->toString() : "Unknown";
}

std::string ASTJsonExporter::type(VariableDeclaration const& _varDecl)
{
	return _varDecl.annotation().type ? _varDecl.annotation().type->toString() : "Unknown";
}

}
