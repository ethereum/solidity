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

#include <libsolidity/ast/ASTCoqExporter.h>

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
void addIfSet(std::vector<std::pair<std::string, std::string>>& _attributes, std::string const& _name, C<V> const& _value)
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

ASTCoqExporter::ASTCoqExporter(CompilerStack::State _stackState, std::map<std::string, unsigned> _sourceIndices):
	m_stackState(_stackState),
	m_sourceIndices(std::move(_sourceIndices))
{
}


void ASTCoqExporter::setCoqNode(
	ASTNode const& _node,
	std::string const& _nodeName,
	std::initializer_list<std::pair<std::string, std::string>>&& _attributes
)
{
	ASTCoqExporter::setCoqNode(
		_node,
		_nodeName,
		std::vector<std::pair<std::string, std::string>>(std::move(_attributes))
	);
}

void ASTCoqExporter::setCoqNode(
	ASTNode const& _node,
	std::string const& _nodeType,
	std::vector<std::pair<std::string, std::string>>&& _attributes
)
{
	m_currentValue = "(* Beginning of a node! *)\n";
	m_currentValue += "id: "s + std::to_string(nodeId(_node)) + "\n";
	m_currentValue += "src: " + sourceLocationToString(_node.location()) + "\n";
	if (auto const* documented = dynamic_cast<Documented const*>(&_node))
		if (documented->documentation())
			m_currentValue += "documentation: " + *documented->documentation() + "\n";
	m_currentValue += "nodeType: " + _nodeType + "\n";
	for (auto& e: _attributes)
		m_currentValue += e.first + ": " + std::move(e.second) + "\n";
}

std::optional<size_t> ASTCoqExporter::sourceIndexFromLocation(SourceLocation const& _location) const
{
	if (_location.sourceName && m_sourceIndices.count(*_location.sourceName))
		return m_sourceIndices.at(*_location.sourceName);
	else
		return std::nullopt;
}

std::string ASTCoqExporter::sourceLocationToString(SourceLocation const& _location) const
{
	std::optional<size_t> sourceIndexOpt = sourceIndexFromLocation(_location);
	int length = -1;
	if (_location.start >= 0 && _location.end >= 0)
		length = _location.end - _location.start;
	return std::to_string(_location.start) + ":" +
		std::to_string(length) + ":" +
		(sourceIndexOpt.has_value() ? std::to_string(sourceIndexOpt.value()) : "-1");
}

std::string ASTCoqExporter::sourceLocationsToCoq(
	std::vector<SourceLocation> const& _sourceLocations
) const
{
	std::string locations = "[";

	for (SourceLocation const& location: _sourceLocations)
		locations += sourceLocationToString(location) + ", ";

	return locations + "]";
}

std::string ASTCoqExporter::namePathToString(std::vector<ASTString> const& _namePath)
{
	return boost::algorithm::join(_namePath, "."s);
}

std::string ASTCoqExporter::typePointerToCoq(Type const* _tp, bool _withoutDataLocation)
{
	std::string typeDescriptions;
	typeDescriptions += "typeString "s + (_tp ? _tp->toString(_withoutDataLocation) : "");
	typeDescriptions += "typeIdentifier s" + (_tp ? _tp->identifier() : "");

	return typeDescriptions;
}

std::string ASTCoqExporter::typePointerToCoq(std::optional<FuncCallArguments> const& _tps)
{
	if (_tps)
	{
		std::string arguments = "[";
		for (auto const& tp: _tps->types)
			arguments += typePointerToCoq(tp) + ", ";
		arguments += "]";
		return arguments;
	}
	else
		return "";
}

void ASTCoqExporter::appendExpressionAttributes(
	std::vector<std::pair<std::string, std::string>>& _attributes,
	ExpressionAnnotation const& _annotation
)
{
	std::vector<std::pair<std::string, std::string>> exprAttributes = {
		std::make_pair("typeDescriptions", typePointerToCoq(_annotation.type)),
		std::make_pair("argumentTypes", typePointerToCoq(_annotation.arguments))
	};

	if (_annotation.isLValue.set())
		exprAttributes.emplace_back("isLValue", std::to_string(*_annotation.isLValue));
	if (_annotation.isPure.set())
		exprAttributes.emplace_back("isPure", std::to_string(*_annotation.isPure));
	if (_annotation.isConstant.set())
		exprAttributes.emplace_back("isConstant", std::to_string(*_annotation.isConstant));

	if (m_stackState > CompilerStack::State::ParsedAndImported)
		exprAttributes.emplace_back("lValueRequested", std::to_string(_annotation.willBeWrittenTo));

	_attributes += exprAttributes;
}

std::string ASTCoqExporter::inlineAssemblyIdentifierToCoq(std::pair<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> _info) const
{
	std::string tuple;
	tuple += "src: " + sourceLocationToString(nativeLocationOf(*_info.first));
	tuple += "declaration: " + idOrNull(_info.second.declaration);
	tuple += "isSlot: " + (_info.second.suffix == "slot");
	tuple += "isOffset: " + (_info.second.suffix == "offset");

	if (!_info.second.suffix.empty())
		tuple += "suffix: " + _info.second.suffix;

	tuple += "valueSize: " + static_cast<Json::number_integer_t>(_info.second.valueSize);

	return tuple;
}

void ASTCoqExporter::print(std::ostream& _stream, ASTNode const& _node)
{
	_stream << toCoq(_node);
}

std::string ASTCoqExporter::toCoq(ASTNode const& _node)
{
	_node.accept(*this);
	return m_currentValue;
}

std::string ASTCoqExporter::indent()
{
	return std::string(2 * m_indent, ' ');
}

std::string ASTCoqExporter::paren(std::string const& content)
{
	if (m_withParens.top())
		return "("s + content + ")"s;
	else
		return content;
}

bool ASTCoqExporter::visit(SourceUnit const& _node)
{
	std::vector<std::pair<std::string, std::string>> attributes;

	if (_node.experimentalSolidity())
		attributes.emplace_back("experimentalSolidity", std::to_string(_node.experimentalSolidity()));

	if (_node.annotation().exportedSymbols.set())
	{
		std::string exportedSymbols = "{";
		for (auto const& sym: *_node.annotation().exportedSymbols)
		{
			exportedSymbols += sym.first + ": [";
			for (Declaration const* overload: sym.second)
				exportedSymbols += std::to_string(nodeId(*overload)) + ", ";
			exportedSymbols += "], ";
		}
		exportedSymbols += "}";

		attributes.emplace_back("exportedSymbols", exportedSymbols);
	};

	addIfSet(attributes, "absolutePath", _node.annotation().path);

	// setCoqNode(_node, "SourceUnit", std::move(attributes));

	std::string body = toCoqs("\n\n", _node.nodes());
	m_currentValue = "(* Generated by coq-of-solidity *)\n"s;
	m_currentValue += "Require Import CoqOfSolidity.CoqOfSolidity.\n\n"s;

	if (_node.licenseString()) {
		m_currentValue += "(* License: "s + *_node.licenseString() + " *)\n\n"s;
	}

	m_currentValue += body;

	return false;
}

bool ASTCoqExporter::visit(PragmaDirective const& _node)
{
	std::string literals = ""s;
	bool isFirst = true;

	for (auto const& literal: _node.literals()) {
		if (!isFirst) {
			literals += ", "s;
		}
		literals += literal;
	}

	m_currentValue = "(* Pragma "s + literals + " *)"s;

	return false;
}

bool ASTCoqExporter::visit(ImportDirective const& _node)
{
	// std::vector<std::pair<std::string, std::string>> attributes = {
	// 	std::make_pair("file", _node.path()),
	// 	std::make_pair("sourceUnit", idOrNull(_node.annotation().sourceUnit)),
	// 	std::make_pair("scope", idOrNull(_node.scope()))
	// };

	// addIfSet(attributes, "absolutePath", _node.annotation().absolutePath);

	// attributes.emplace_back("unitAlias", _node.name());
	// attributes.emplace_back("nameLocation", sourceLocationToString(_node.nameLocation()));

	// std::string symbolAliases = "[";
	// for (auto const& symbolAlias: _node.symbolAliases())
	// {
	// 	std::string tuple = "(";
	// 	solAssert(symbolAlias.symbol, "");
	// 	tuple += "foreign: " + toCoq(*symbolAlias.symbol) + ", ";
	// 	tuple += "local: " + (symbolAlias.alias ? *symbolAlias.alias : "")  + ", ";
	// 	tuple += "nameLocation: " + sourceLocationToString(_node.nameLocation())  + ", ";
	// 	tuple += ")";
	// 	symbolAliases += tuple + ", ";
	// }
	// symbolAliases += "]";
	// attributes.emplace_back("symbolAliases", symbolAliases);

	if (_node.annotation().absolutePath.set()) {
		std::string absolutePath = *_node.annotation().absolutePath;
		std::filesystem::path currentPath = std::filesystem::current_path();
		std::filesystem::path testPath = currentPath / "..";
		std::filesystem::path relativePath = std::filesystem::relative(absolutePath, testPath);
		std::string relativePathStr = relativePath.string();
		// Replace '/' by '.'
		std::replace(relativePathStr.begin(), relativePathStr.end(), '/', '.');
		// Replace '-' by '_'
		std::replace(relativePathStr.begin(), relativePathStr.end(), '-', '_');
		// Remove '.sol' extension
		relativePathStr.erase(relativePathStr.end() - 4, relativePathStr.end());
		m_currentValue = "Require "s + relativePathStr + ".\n"s;
	} else {
		m_currentValue = "Missing import absolute path.\n"s;
	}

	return false;
}

bool ASTCoqExporter::visit(ContractDefinition const& _node)
{
	m_indent++;
	std::string body = toCoqs("\n\n"s + indent(), _node.subNodes());
	m_indent--;

	m_currentValue = "(* "s + contractKind(_node.contractKind()) + " *)\n"s;
	m_currentValue += "Module "s + _node.name() + ".\n"s;
	m_indent++;
	m_currentValue += indent() + body + "\n"s;
	m_indent--;
	m_currentValue += "End "s + _node.name() + ".\n"s;

	return false;
}

bool ASTCoqExporter::visit(IdentifierPath const& _node)
{
	m_currentValue = "\""s + namePathToString(_node.path()) + "\""s;

	return false;
}

bool ASTCoqExporter::visit(InheritanceSpecifier const& _node)
{
	setCoqNode(_node, "InheritanceSpecifier", {
		std::make_pair("baseName", toCoq(_node.name())),
		std::make_pair("arguments", _node.arguments() ? toCoqs(", ", *_node.arguments()) : "")
	});
	return false;
}

bool ASTCoqExporter::visit(UsingForDirective const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string typeName = toCoqOption(_node.typeName());
	std::string global = _node.global() ? "UsingKind.Global" : "UsingKind.Local";

	if (_node.usesBraces())
	{
		std::string functionList = "[\n"s;
		m_withParens.push(true);
		m_indent++;
		for (auto&& [fun, op]: _node.functionsAndOperators())
		{
			std::string function;
			if (!op.has_value())
				function = indent() + "UsingFunction.Function "s + toCoq(*fun);
			else
			{
				function = indent() + "UsingFunction.Operator \""s + TokenTraits::toString(*op) + "\" "s;
				function += toCoq(*fun);
			}
			functionList += function + ",\n"s;
		}
		m_indent--;
		m_withParens.pop();
		functionList += indent() + "]";
		m_indent--;
		m_withParens.pop();

		m_currentValue = "Axiom using : M.usingFunctions (|\n"s;
		m_indent++;
		m_indent++;
		m_currentValue += indent() + typeName + ",\n"s;
		m_currentValue += indent() + global + ",\n"s;
		m_currentValue += indent() + functionList + "\n"s;
		m_indent--;
		m_currentValue += indent() + "|).";
		m_indent--;
	}
	else
	{
		auto const& functionAndOperators = _node.functionsAndOperators();
		solAssert(functionAndOperators.size() == 1);
		solAssert(!functionAndOperators.front().second.has_value());

		std::string libraryName = toCoq(*(functionAndOperators.front().first));
		m_indent--;
		m_withParens.pop();

		m_currentValue = "Axiom using : M.usingLibrary (|\n"s;
		m_indent++;
		m_indent++;
		m_currentValue += indent() + typeName + ",\n"s;
		m_currentValue += indent() + global + ",\n"s;
		m_currentValue += indent() + libraryName + "\n"s;
		m_indent--;
		m_currentValue += indent() + "|).";
		m_indent--;
	}

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(StructDefinition const& _node)
{
	m_currentValue = "(* Struct "s + _node.name() + " *)"s;

	return false;
}

bool ASTCoqExporter::visit(EnumDefinition const& _node)
{
	m_currentValue = "(* Enum "s + _node.name() + " *)"s;

	return false;
}

bool ASTCoqExporter::visit(EnumValue const& _node)
{
	m_currentValue = paren("Value.Enum \""s + _node.name() + "\""s);

	return false;
}

bool ASTCoqExporter::visit(UserDefinedValueTypeDefinition const& _node)
{
	solAssert(_node.underlyingType(), "expected an underlying type");

	m_withParens.push(false);
	std::string underlyingType = toCoq(*_node.underlyingType());
	m_withParens.pop();

	m_currentValue =
		"Axiom user_type_"s + _node.name() + " : Ty.path \""s + _node.name() + "\" = "s +
		underlyingType + "."s;

	return false;
}

bool ASTCoqExporter::visit(ParameterList const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string parameters = toCoqs(";\n"s, _node.parameters());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "ParameterList.make ["s;
	if (!_node.parameters().empty())
	{
		m_currentValue += "\n"s;
		m_indent++;
		m_currentValue += indent() + parameters + "\n"s;
		m_indent--;
		m_currentValue += indent() + "]";
	}
	else
		m_currentValue += "]";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(OverrideSpecifier const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string overrides = toCoqs(";\n"s, _node.overrides());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "OverrideSpecifier.make ["s;
	if (!_node.overrides().empty())
	{
		m_currentValue += "\n"s;
		m_indent++;
		m_currentValue += indent() + overrides + "\n"s;
		m_indent--;
		m_currentValue += indent() + "]";
	}
	else
		m_currentValue += "]";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(FunctionDefinition const& _node)
{
	std::vector<std::pair<std::string, std::string>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toCoq(*_node.documentation()) : ""),
		std::make_pair("kind", _node.isFree() ? "freeFunction" : TokenTraits::toString(_node.kind())),
		std::make_pair("stateMutability", stateMutabilityToString(_node.stateMutability())),
		std::make_pair("virtual", std::to_string(_node.markedVirtual())),
		std::make_pair("overrides", _node.overrides() ? toCoq(*_node.overrides()) : ""),
		std::make_pair("parameters", toCoq(_node.parameterList())),
		std::make_pair("returnParameters", toCoq(*_node.returnParameterList())),
		std::make_pair("modifiers", toCoqs(", ", _node.modifiers())),
		std::make_pair("body", _node.isImplemented() ? toCoq(_node.body()) : ""),
		std::make_pair("implemented", std::to_string(_node.isImplemented())),
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

	m_indent += 2;
	std::string body = _node.isImplemented() ? toCoq(_node.body()) : "not implemented";
	m_indent -= 2;

	m_currentValue = "Definition "s + _node.name() + " (α : list Value.t) : M :=\n"s;
	m_indent++;
	m_currentValue += indent() + "match α with\n"s;
	m_currentValue += indent() + "| ["s;
	bool isFirst = true;
	for (auto const& param: _node.parameterList().parameters()) {
		if (!isFirst)
			m_currentValue += ", ";
		else
			isFirst = false;
		std::string name = param->name();
		m_currentValue += name == "" ? "_" : name;
	}
	m_currentValue += "] =>\n"s;
	m_indent++;
	m_currentValue += indent() + body + "\n"s;
	m_indent--;
	m_currentValue += indent() + "| _ => M.impossible \"invalid number of parameters\"\n"s;
	m_currentValue += indent() + "end."s;
	m_indent--;

	return false;
}

bool ASTCoqExporter::visit(VariableDeclaration const& _node)
{
	std::vector<std::pair<std::string, std::string>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("typeName", toCoq(_node.typeName())),
		std::make_pair("constant", std::to_string(_node.isConstant())),
		std::make_pair("mutability", VariableDeclaration::mutabilityToString(_node.mutability())),
		std::make_pair("stateVariable", std::to_string(_node.isStateVariable())),
		std::make_pair("storageLocation", location(_node.referenceLocation())),
		std::make_pair("overrides", _node.overrides() ? toCoq(*_node.overrides()) : ""),
		std::make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		std::make_pair("value", _node.value() ? toCoq(*_node.value()) : ""),
		std::make_pair("scope", idOrNull(_node.scope())),
		std::make_pair("typeDescriptions", typePointerToCoq(_node.annotation().type, true))
	};
	if (_node.isStateVariable() && _node.isPublic())
		attributes.emplace_back("functionSelector", _node.externalIdentifierHex());
	if (_node.isStateVariable() && _node.documentation())
		attributes.emplace_back("documentation", toCoq(*_node.documentation()));
	if (m_inEvent)
		attributes.emplace_back("indexed", std::to_string(_node.isIndexed()));
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(std::make_pair("baseFunctions", getContainerIds(_node.annotation().baseFunctions, true)));

	m_indent++;
	std::string value = _node.value() ? toCoq(*_node.value()) : "undefined";
	m_indent--;

	m_currentValue = "Definition "s + _node.name() + " : Value.t :=\n"s;
	m_indent++;
	m_currentValue += indent() + value + "."s;
	m_indent--;

	return false;
}

bool ASTCoqExporter::visit(ModifierDefinition const& _node)
{
	std::vector<std::pair<std::string, std::string>> attributes = {
		std::make_pair("name", _node.name()),
		std::make_pair("nameLocation", sourceLocationToString(_node.nameLocation())),
		std::make_pair("documentation", _node.documentation() ? toCoq(*_node.documentation()) : ""),
		std::make_pair("visibility", Declaration::visibilityToString(_node.visibility())),
		std::make_pair("parameters", toCoq(_node.parameterList())),
		std::make_pair("virtual", std::to_string(_node.markedVirtual())),
		std::make_pair("overrides", _node.overrides() ? toCoq(*_node.overrides()) : ""),
		std::make_pair("body", _node.isImplemented() ? toCoq(_node.body()) : "")
	};
	if (!_node.annotation().baseFunctions.empty())
		attributes.emplace_back(std::make_pair("baseModifiers", getContainerIds(_node.annotation().baseFunctions, true)));
	setCoqNode(_node, "ModifierDefinition", std::move(attributes));
	return false;
}

bool ASTCoqExporter::visit(ModifierInvocation const& _node)
{
	std::vector<std::pair<std::string, std::string>> attributes{
		std::make_pair("modifierName", toCoq(_node.name())),
		std::make_pair("arguments", _node.arguments() ? toCoqs(", ", *_node.arguments()) : "")
	};
	if (Declaration const* declaration = _node.name().annotation().referencedDeclaration)
	{
		if (dynamic_cast<ModifierDefinition const*>(declaration))
			attributes.emplace_back("kind", "modifierInvocation");
		else if (dynamic_cast<ContractDefinition const*>(declaration))
			attributes.emplace_back("kind", "baseConstructorSpecifier");
	}
	setCoqNode(_node, "ModifierInvocation", std::move(attributes));
	return false;
}

bool ASTCoqExporter::visit(EventDefinition const& _node)
{
	m_currentValue = "(* Event "s + _node.name() + " *)"s;

	return false;
}

bool ASTCoqExporter::visit(ErrorDefinition const& _node)
{
	m_currentValue = "(* Error "s + _node.name() + " *)"s;

	return false;
}

bool ASTCoqExporter::visit(ElementaryTypeName const& _node)
{
	m_currentValue = paren("Ty.path \""s + _node.typeName().toString() + "\""s);

	return false;
}

bool ASTCoqExporter::visit(UserDefinedTypeName const& _node)
{
	m_withParens.push(true);
	std::string path = toCoq(_node.pathNode());
	m_withParens.pop();

	m_currentValue = paren("Ty.path "s + path);

	return false;
}

bool ASTCoqExporter::visit(FunctionTypeName const& _node)
{
	m_withParens.push(true);
	m_indent++;
	std::string parameterTypes = toCoq(*_node.parameterTypeList());
	std::string returnParameterTypes = toCoq(*_node.returnParameterTypeList());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "Ty.function\n"s;
	m_indent++;
	m_currentValue += indent() + parameterTypes + "\n"s;
	m_currentValue += indent() + returnParameterTypes;
	m_indent--;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(Mapping const& _node)
{
	setCoqNode(_node, "Mapping", {
		std::make_pair("keyType", toCoq(_node.keyType())),
		std::make_pair("keyName", _node.keyName()),
		std::make_pair("keyNameLocation", sourceLocationToString(_node.keyNameLocation())),
		std::make_pair("valueType", toCoq(_node.valueType())),
		std::make_pair("valueName", _node.valueName()),
		std::make_pair("valueNameLocation", sourceLocationToString(_node.valueNameLocation())),
		std::make_pair("typeDescriptions", typePointerToCoq(_node.annotation().type, true))
	});
	return false;
}

bool ASTCoqExporter::visit(ArrayTypeName const& _node)
{
	m_withParens.push(true);
	std::string baseType = toCoq(_node.baseType());
	std::string length = toCoqOption(_node.length());
	m_withParens.pop();

	m_currentValue = paren("Ty.array "s + baseType + " "s + length);

	return false;
}

bool ASTCoqExporter::visit(InlineAssembly const& _node)
{
	std::vector<std::pair<std::string, std::string>> externalReferences;

	for (auto const& it: _node.annotation().externalReferences)
		if (it.first)
			externalReferences.emplace_back(std::make_pair(
				it.first->name.str(),
				inlineAssemblyIdentifierToCoq(it)
			));

	std::string externalReferencesString = "";

	std::sort(externalReferences.begin(), externalReferences.end());
	for (std::string& it: externalReferences | ranges::views::values)
		externalReferencesString += std::move(it);

	std::vector<std::pair<std::string, std::string>> attributes = {
		// std::make_pair("AST", Json(yul::AsmJsonConverter(sourceIndexFromLocation(_node.location()))(_node.operations()))),
		std::make_pair("AST", "TODO YulJson"),
		std::make_pair("externalReferences", std::move(externalReferencesString)),
		std::make_pair("evmVersion", dynamic_cast<solidity::yul::EVMDialect const&>(_node.dialect()).evmVersion().name())
	};

	if (_node.flags())
	{
		std::string flags = "[";
		for (auto const& flag: *_node.flags())
			if (flag)
				flags += *flag + ", ";
		flags += "]";
		attributes.emplace_back(std::make_pair("flags", flags));
	}
	setCoqNode(_node, "InlineAssembly", std::move(attributes));

	return false;
}

bool ASTCoqExporter::visit(Block const& _node)
{
	std::string currentIndent = indent();
	m_indent++;
	std::string statements = toCoqs(" in\n"s + currentIndent + "let _ :=\n"s + indent(), _node.statements());
	m_indent--;

	m_currentValue = "let _ :=\n"s;
	m_indent++;
	m_currentValue += indent() + statements + " in\n"s;
	m_indent--;
	m_currentValue += indent() + "Value.Tuple []"s;

	return false;
}

bool ASTCoqExporter::visit(PlaceholderStatement const& _node)
{
	setCoqNode(_node, "PlaceholderStatement", {});
	return false;
}

bool ASTCoqExporter::visit(IfStatement const& _node)
{
	m_withParens.push(false);
	std::string condition = toCoq(_node.condition());
	m_indent++;
	std::string trueBody = toCoq(_node.trueStatement());
	std::string falseBody =
		_node.falseStatement() ?
		toCoq(*_node.falseStatement()) :
		"Value.Tuple []"s;
	m_indent--;
	m_withParens.pop();

	m_currentValue = "if "s + condition + " then\n"s;
	m_indent++;
	m_currentValue += indent() + trueBody + "\n"s;
	m_indent--;
	m_currentValue += indent() + "else\n"s;
	m_indent++;
	m_currentValue += indent() + falseBody;
	m_indent--;

	return false;
}

bool ASTCoqExporter::visit(TryCatchClause const& _node)
{
	std::string errorName = _node.errorName();
	std::string parameters = toCoqOrNull(_node.parameters());
	std::string block = toCoq(_node.block());

	m_currentValue = "TryCatchClause "s + errorName + " "s + parameters + " "s + block;

	return false;
}

bool ASTCoqExporter::visit(TryStatement const& _node)
{
	std::string externalCall = toCoq(_node.externalCall());
	std::string clauses = toCoqs(", ", _node.clauses());

	m_currentValue = "TryStatement "s + externalCall + " "s + clauses;

	return false;
}

bool ASTCoqExporter::visit(WhileStatement const& _node)
{
	m_indent++;
	m_withParens.push(false);
	std::string condition = toCoq(_node.condition());
	std::string body = toCoq(_node.body());
	m_withParens.pop();
	m_indent--;
	
	std::string whileKind = _node.isDoWhile() ? "DoWhile" : "While";

	m_currentValue = "M.while (|\n"s;
	m_indent++;
	m_currentValue += indent() + "WhileKind."s + whileKind + ",\n"s;
	m_currentValue += indent() + condition + ",\n"s;
	m_currentValue += indent() + body + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(ForStatement const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string initializationExpression = toCoqOrUnit(_node.initializationExpression());
	std::string condition = toCoqOrUnit(_node.condition());
	std::string loopExpression = toCoqOrUnit(_node.loopExpression());
	std::string body = toCoq(_node.body());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.for_ (|\n"s;
	m_indent++;
	m_currentValue += indent() + initializationExpression + ",\n"s;
	m_currentValue += indent() + condition + ",\n"s;
	m_currentValue += indent() + loopExpression + ",\n"s;
	m_currentValue += indent() + body + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(Continue const& _node __attribute__((unused)))
{
	m_currentValue = "M.continue (||)"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(Break const& _node __attribute__((unused)))
{
	m_currentValue = "M.break (||)"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(Return const& _node)
{
	m_indent++;
	m_withParens.push(false);
	std::string expression = _node.expression() ? toCoq(*_node.expression()) : "Value.Tuple []"s;
	m_withParens.pop();
	m_indent--;

	m_currentValue = "M.return_ (|\n"s;
	m_indent++;
	m_currentValue += indent() + expression + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(Throw const& _node __attribute__((unused)))
{
	m_currentValue = "M.throw (||)"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(EmitStatement const& _node)
{
	std::string eventCall = toCoq(_node.eventCall());

	m_currentValue = "EmitStatement "s + eventCall + "\n"s;

	return false;
}

bool ASTCoqExporter::visit(RevertStatement const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string errorCall = toCoq(_node.errorCall());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.revert (|\n"s;
	m_indent++;
	m_currentValue += indent() + errorCall + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(VariableDeclarationStatement const& _node)
{
	m_indent++;
	std::string initialValue = _node.initialValue() ? toCoq(*_node.initialValue()) : "Value.Default"s;
	m_indent--;

	m_currentValue = "M.define (|\n"s;
	m_indent++;
	m_currentValue += indent() + "["s;

	bool isFirst = true;
	for (auto const& v: _node.declarations()) {
		if (!isFirst)
			m_currentValue += ";"s;
		else
			isFirst = false;
		m_currentValue += " \""s + v->name() + "\""s;
	}

	m_currentValue += " ],\n"s;
	m_currentValue += indent() + initialValue + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(ExpressionStatement const& _node)
{
	m_currentValue = toCoq(_node.expression());

	return false;
}

bool ASTCoqExporter::visit(Conditional const& _node)
{
	std::string condition = toCoq(_node.condition());
	std::string trueExpression = toCoq(_node.trueExpression());
	std::string falseExpression = toCoq(_node.falseExpression());

	m_currentValue = "Conditional "s + condition + " "s + trueExpression + " "s + falseExpression + "\n"s;

	return false;
}

bool ASTCoqExporter::visit(Assignment const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string leftHandSide = toCoq(_node.leftHandSide());
	std::string rightHandSide = toCoq(_node.rightHandSide());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.assign (|\n"s;
	m_indent++;
	m_currentValue += indent() + "\""s + TokenTraits::toString(_node.assignmentOperator()) + "\",\n"s;
	m_currentValue += indent() + leftHandSide + ",\n"s;
	m_currentValue += indent() + rightHandSide + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(TupleExpression const& _node)
{
	bool isArray = _node.isInlineArray();

	m_withParens.push(false);
	m_indent++;
	std::string components = toCoqs(",\n"s + indent(), _node.components());
	m_indent--;
	m_withParens.pop();

	m_currentValue = (isArray ? "Value.Array"s : "Value.Tuple"s) + " [\n"s;
	m_indent++;
	m_currentValue += indent() + components + "\n"s;
	m_indent--;
	m_currentValue += indent() + "]"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(UnaryOperation const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string subExpression = toCoq(_node.subExpression());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.un_op (|\n"s;
	m_indent++;
	m_currentValue += indent() + (_node.isPrefixOperation() ? "true"s : "false"s) + ",\n"s;
	m_currentValue += indent() + "\""s + TokenTraits::toString(_node.getOperator()) + "\",\n"s;
	m_currentValue += indent() + subExpression + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(BinaryOperation const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string leftExpression = toCoq(_node.leftExpression());
	std::string rightExpression = toCoq(_node.rightExpression());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.bin_op (|\n"s;
	m_indent++;
	m_currentValue += indent() + "\""s + TokenTraits::toString(_node.getOperator()) + "\",\n"s;
	m_currentValue += indent() + leftExpression + ",\n"s;
	m_currentValue += indent() + rightExpression + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(FunctionCall const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string expression = toCoq(_node.expression());
	m_indent++;
	std::string arguments = toCoqs(",\n"s + indent(), _node.arguments());
	m_indent--;
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.call (|\n"s;
	m_indent++;
	m_currentValue += indent() + expression + ",\n"s;
	if (_node.arguments().empty()) {
		m_currentValue += indent() + "[]\n"s;
	} else {
		m_currentValue += indent() + "[\n"s;
		m_indent++;
		m_currentValue += indent() + arguments + "\n"s;
		m_indent--;
		m_currentValue += indent() + "]\n"s;
	}
	m_indent--;
	m_currentValue += indent() + "|)"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(FunctionCallOptions const& _node)
{
	std::string names = "[";
	for (auto const& name: _node.names())
		names += *name + ", ";
	names += "]";

	std::string expression = toCoq(_node.expression());
	std::string options = toCoqs(", ", _node.options());

	// appendExpressionAttributes(attributes, _node.annotation());

	m_currentValue = "FunctionCallOptions "s + expression + " "s + names + " "s + options + "\n"s;

	return false;
}

bool ASTCoqExporter::visit(NewExpression const& _node)
{
	std::string typeName = toCoq(_node.typeName());
	// appendExpressionAttributes(attributes, _node.annotation());

	m_currentValue = "NewExpression "s + typeName + "\n"s;

	return false;
}

bool ASTCoqExporter::visit(MemberAccess const& _node)
{
	std::string memberName = _node.memberName();

	m_withParens.push(false);
	m_indent++;
	std::string expression = toCoq(_node.expression());
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.member_access (|\n"s;
	m_indent++;
	m_currentValue += indent() + expression + ",\n"s;
	m_currentValue += indent() + "\""s + memberName + "\"\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(IndexAccess const& _node)
{
	m_withParens.push(false);
	m_indent++;
	std::string baseExpression = toCoq(_node.baseExpression());
	m_withParens.push(true);
	std::string indexExpression =
		_node.indexExpression() ?
		"Some "s + toCoq(*_node.indexExpression()) :
		"None"s;
	m_withParens.pop();
	m_indent--;
	m_withParens.pop();

	m_currentValue = "M.index_access (|\n"s;
	m_indent++;
	m_currentValue += indent() + baseExpression + ",\n"s;
	m_currentValue += indent() + indexExpression + "\n"s;
	m_indent--;
	m_currentValue += indent() + "|)";

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(IndexRangeAccess const& _node)
{
	std::string baseExpression = toCoq(_node.baseExpression());
	std::string startExpression = toCoqOrNull(_node.startExpression());
	std::string endExpression = toCoqOrNull(_node.endExpression());

	// appendExpressionAttributes(attributes, _node.annotation());

	m_currentValue =
		"IndexRangeAccess "s + baseExpression + " "s + startExpression + " "s + endExpression + "\n"s;

	return false;
}

bool ASTCoqExporter::visit(Identifier const& _node)
{
	m_currentValue = "M.get_name (| \""s + _node.name() + "\" |)"s;

	m_currentValue = paren(m_currentValue);

	return false;
}

bool ASTCoqExporter::visit(ElementaryTypeNameExpression const& _node)
{
	m_currentValue = toCoq(_node.type());

	return false;
}

bool ASTCoqExporter::visit(Literal const& _node)
{
	std::string value = _node.value();
	if (!util::validateUTF8(_node.value()))
		value = "";
	Token subdenomination = Token(_node.subDenomination());

	std::string kind = literalTokenKind(_node.token());
	std::string hexValue = util::toHex(util::asBytes(_node.value()));
	std::string subdenominationString = subdenomination == Token::Illegal ? "" : TokenTraits::toString(subdenomination);

	// appendExpressionAttributes(attributes, _node.annotation());

	switch (_node.token())
	{
	case Token::Number:
		m_currentValue = paren("Value.Integer "s + _node.value());
		break;
	case Token::StringLiteral:
		m_currentValue = paren("Value.String \""s + _node.value() + "\""s);
		break;
	case Token::UnicodeStringLiteral:
		m_currentValue = "unicodeString";
		break;
	case Token::HexStringLiteral:
		m_currentValue = "hexString";
		break;
	case Token::TrueLiteral:
		m_currentValue = paren("Value.Bool true"s);
		break;
	case Token::FalseLiteral:
		m_currentValue = paren("Value.Bool false"s);
		break;
	default:
		solAssert(false, "Unknown kind of literal token.");
	}

	return false;
}

bool ASTCoqExporter::visit(StructuredDocumentation const& _node)
{
	std::string text = *_node.text();

	m_currentValue = "StructuredDocumentation "s + text + "\n"s;

	return false;
}

void ASTCoqExporter::endVisit(EventDefinition const&)
{
	m_inEvent = false;
}

bool ASTCoqExporter::visitNode(ASTNode const& _node)
{
	solAssert(false, _node.experimentalSolidityOnly() ?
		"Attempt to export an AST of experimental solidity." :
		"Attempt to export an AST that contains unexpected nodes."
	);
	return false;
}

std::string ASTCoqExporter::location(VariableDeclaration::Location _location)
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

std::string ASTCoqExporter::contractKind(ContractKind _kind)
{
	switch (_kind)
	{
	case ContractKind::Interface:
		return "Interface";
	case ContractKind::Contract:
		return "Contract";
	case ContractKind::Library:
		return "Library";
	}

	// To make the compiler happy
	return {};
}

std::string ASTCoqExporter::functionCallKind(FunctionCallKind _kind)
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

std::string ASTCoqExporter::literalTokenKind(Token _token)
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

std::string ASTCoqExporter::type(Expression const& _expression)
{
	return _expression.annotation().type ? _expression.annotation().type->toString() : "Unknown";
}

std::string ASTCoqExporter::type(VariableDeclaration const& _varDecl)
{
	return _varDecl.annotation().type ? _varDecl.annotation().type->toString() : "Unknown";
}

}
