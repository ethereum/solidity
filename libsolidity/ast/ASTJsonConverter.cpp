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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2015
 * Converts the AST into json format
 */

#include <libsolidity/ast/ASTJsonConverter.h>
#include <boost/algorithm/string/join.hpp>
#include <libdevcore/UTF8.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/Exceptions.h>

using namespace std;

namespace dev
{
namespace solidity
{

ASTJsonConverter::ASTJsonConverter(bool _legacy, map<string, unsigned> _sourceIndices):
	m_legacy(_legacy),
	m_sourceIndices(_sourceIndices)
{
}


void ASTJsonConverter::setJsonNode(
	ASTNode const& _node,
	string const& _nodeName,
	initializer_list<pair<string const, Json::Value>>&& _attributes
)
{
	ASTJsonConverter::setJsonNode(
		_node,
		_nodeName,
		std::vector<pair<string const, Json::Value>>(std::move(_attributes))
	);
}
  
void ASTJsonConverter::setJsonNode(
	ASTNode const& _node,
	string const& _nodeType,
	std::vector<pair<string const, Json::Value>>&& _attributes
)
{
	m_currentValue = Json::objectValue;
	m_currentValue["id"] = Json::UInt64(_node.id());
	m_currentValue["src"] = sourceLocationToString(_node.location());
	m_currentValue["nodeType"] = _nodeType;
	for (auto& e: _attributes)
		m_currentValue[e.first] = std::move(e.second);
}

string ASTJsonConverter::sourceLocationToString(SourceLocation const& _location) const
{
	int sourceIndex{-1};
	if (_location.sourceName && m_sourceIndices.count(*_location.sourceName))
		sourceIndex = m_sourceIndices.at(*_location.sourceName);
	int length = -1;
	if (_location.start >= 0 && _location.end >= 0)
		length = _location.end - _location.start;
	return std::to_string(_location.start) + ":" + std::to_string(length) + ":" + std::to_string(sourceIndex);
}

void ASTJsonConverter::print(ostream& _stream, ASTNode const& _node)
{
	_stream << toJson(_node);
}

Json::Value ASTJsonConverter::toJson(ASTNode const& _node)
{
	_node.accept(*this);
	return std::move(m_currentValue);
}

bool ASTJsonConverter::visit(SourceUnit const& _node)
{
	Json::Value exportedSymbols = Json::objectValue;
	for (auto const& sym: _node.annotation().exportedSymbols)
	{
		exportedSymbols[sym.first] = Json::arrayValue;
		for (Declaration const* overload: sym.second)
			exportedSymbols[sym.first].append(overload->id());
	}
	setJsonNode(
		_node,
		"SourceUnit",
		{
			make_pair("absolutePath", _node.annotation().path),
			make_pair("exportedSymbols", move(exportedSymbols)),
			make_pair("nodes", toJson(_node.nodes()))
		}
	);
	return false;
}

bool ASTJsonConverter::visit(PragmaDirective const& _node)
{
	Json::Value literals(Json::arrayValue);
	for (auto const& literal: _node.literals())
		literals.append(literal);
	setJsonNode(
		_node,
		"PragmaDirective",
		{{"literals", literals}}
	);
	return false;
}

bool ASTJsonConverter::visit(ImportDirective const& _node)
{
	std::vector<pair<string const, Json::Value>> attributes = {
		make_pair("file", _node.path()),
		make_pair("absolutePath", _node.annotation().absolutePath),
		make_pair("SourceUnit", _node.annotation().sourceUnit->id())
	};
	attributes.push_back(make_pair("unitAlias", _node.name()));
	Json::Value symbolAliases(Json::arrayValue);
	for (auto const& symbolAlias: _node.symbolAliases())
	{
		Json::Value tuple(Json::objectValue);
		solAssert(symbolAlias.first, "");
		tuple["foreign"] = symbolAlias.first->id();
		tuple["local"] =  symbolAlias.second ? Json::Value(*symbolAlias.second) : Json::nullValue;
		symbolAliases.append(tuple);
	}
	attributes.push_back( make_pair("symbolAliases", symbolAliases));
	setJsonNode(_node, "ImportDirective", std::move(attributes));
	return false;
}

bool ASTJsonConverter::visit(ContractDefinition const& _node)
{
	Json::Value linearizedBaseContracts(Json::arrayValue);
	for (auto const& baseContract: _node.annotation().linearizedBaseContracts)
		linearizedBaseContracts.append(Json::UInt64(baseContract->id()));
	Json::Value contractDependencies(Json::arrayValue);
	for (auto const& dependentContract: _node.annotation().contractDependencies)
		contractDependencies.append(Json::UInt64(dependentContract->id()));
	setJsonNode(_node, "ContractDefinition", {
		make_pair("name", _node.name()),
		make_pair("isLibrary", _node.isLibrary()),
		make_pair("fullyImplemented", _node.annotation().isFullyImplemented),
		make_pair("linearizedBaseContracts", linearizedBaseContracts),
		make_pair("contractDependencies", contractDependencies),
		make_pair("nodes", toJson(_node.subNodes()))
	});
	return false;
}

bool ASTJsonConverter::visit(InheritanceSpecifier const& _node)
{
	//??this node never shows up!
	// assumed usage:
	// import contract.sol <- with bar-contract
	// contract foo is bar {...
	setJsonNode(_node, "InheritanceSpecifier", {
		//TODO:make_pair("baseName", &_node.name().annotation().referencedDeclaration->name()), //or maybe id()?
		//this is only set during 'referenceresolutionstage', nullpointercheck needed?
		// note: i decided not to include userDefinedTypeName.annotation.contractScope
		make_pair("arguments", toJson(_node.arguments()))
		//??this node never shows up! assumed usage contract foo is bar {...}
	});
	return false;
}

bool ASTJsonConverter::visit(UsingForDirective const& _node)
{
	Json::Value libraries(Json::arrayValue);
	for (auto const& lib: _node.libraryName().namePath())
		tmp.append(lib);
	//note: namePath is a vector yet the using for directive only allows for one library
	//design decision: show list, or just the first object
	setJsonNode(_node, "UsingForDirective", {
		make_pair("libraryNames", libraries),
		make_pair("typeName", _node.typeName() ?
			Json::Value(_node.typeName()->annotation().type->toString()) //or maybe use id()?
			: Json::Value("*") )
			    //do we break superlong lines?
	});
	return false;
}

bool ASTJsonConverter::visit(StructDefinition const& _node)
{
	setJsonNode(_node, "StructDefinition", {
		make_pair("name", _node.name()),
		make_pair("canonicalName", _node.annotation().canonicalName),
		make_pair("members", toJson(_node.members()))
	});
	return false;
}

bool ASTJsonConverter::visit(EnumDefinition const& _node)
{
	setJsonNode(_node, "EnumDefinition", {
		make_pair("name", _node.name()),
		make_pair("members", toJson(_node.members()))
	});
	return false;
}

bool ASTJsonConverter::visit(EnumValue const& _node)
{
	setJsonNode(_node, "EnumValue", { make_pair("name", _node.name()) });
	return false;
}

bool ASTJsonConverter::visit(ParameterList const& _node)
{
	setJsonNode(_node, "ParameterList", {
		make_pair("parameters", toJson(_node.parameters()))
			    //todo write check
		    });
	return false;
}

bool ASTJsonConverter::visit(FunctionDefinition const& _node)
{
	std::vector<pair<string const, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("constant", _node.isDeclaredConst()),
		make_pair("payable", _node.isPayable()),
		make_pair("visibility", visibility(_node.visibility()))
		//make_pair("parameters", toJson(_node.parameterList())),
		//make_pair("returnParameters", toJson(_node.returnParameterList())),
		//make_pair("modifiers", toJson(_node.modifiers()))
		//make_pair("body", toJson(_node.body())
	};
	setJsonNode(_node, "FunctionDefinition", std::move(attributes));
	return false;
}

bool ASTJsonConverter::visit(VariableDeclaration const& _node)
{
	std::vector<pair<string const, Json::Value>> attributes = {
		make_pair("name", _node.name()),
		make_pair("type", type(_node)),
		make_pair("constant", _node.isConstant()),
		make_pair("storageLocation", location(_node.referenceLocation())),
		make_pair("visibility", visibility(_node.visibility()))
	};
	if (m_inEvent)
		attributes.push_back(make_pair("indexed", _node.isIndexed()));
	setJsonNode(_node, "VariableDeclaration", std::move(attributes));
	return false;
}

bool ASTJsonConverter::visit(ModifierDefinition const& _node)
{
	setJsonNode(_node, "ModifierDefinition", { make_pair("name", _node.name()) });
	return false;
}

bool ASTJsonConverter::visit(ModifierInvocation const& _node)
{
	setJsonNode(_node, "ModifierInvocation", {});
	return false;
}

bool ASTJsonConverter::visit(TypeName const&)
{
	return false;
}

bool ASTJsonConverter::visit(EventDefinition const& _node)
{
	m_inEvent = true;
	setJsonNode(_node, "EventDefinition", { make_pair("name", _node.name()) });
	return false;
}

bool ASTJsonConverter::visit(ElementaryTypeName const& _node)
{
	setJsonNode(_node, "ElementaryTypeName", { make_pair("name", _node.typeName().toString()) });
	return false;
}

bool ASTJsonConverter::visit(UserDefinedTypeName const& _node)
{
	setJsonNode(_node, "UserDefinedTypeName", {
		make_pair("name", boost::algorithm::join(_node.namePath(), "."))
	});
	return false;
}

bool ASTJsonConverter::visit(FunctionTypeName const& _node)
{
	setJsonNode(_node, "FunctionTypeName", {
		make_pair("payable", _node.isPayable()),
		make_pair("visibility", visibility(_node.visibility())),
		make_pair("constant", _node.isDeclaredConst())
	});
	return false;
}

bool ASTJsonConverter::visit(Mapping const& _node)
{
	setJsonNode(_node, "Mapping", {});
	return false;
}

bool ASTJsonConverter::visit(ArrayTypeName const& _node)
{
	setJsonNode(_node, "ArrayTypeName", {});
	return false;
}

bool ASTJsonConverter::visit(InlineAssembly const& _node)
{
	setJsonNode(_node, "InlineAssembly", {});
	return false;
}

bool ASTJsonConverter::visit(Block const& _node)
{
	setJsonNode(_node, "Block", {});
	return false;
}

bool ASTJsonConverter::visit(PlaceholderStatement const& _node)
{
	setJsonNode(_node, "PlaceholderStatement", {});
	return false;
}

bool ASTJsonConverter::visit(IfStatement const& _node)
{
	setJsonNode(_node, "IfStatement", {});
	return false;
}

bool ASTJsonConverter::visit(WhileStatement const& _node)
{
	setJsonNode(
		_node,
		_node.isDoWhile() ? "DoWhileStatement" : "WhileStatement",
		{}
	);
	return false;
}

bool ASTJsonConverter::visit(ForStatement const& _node)
{
	setJsonNode(_node, "ForStatement", {});
	return false;
}

bool ASTJsonConverter::visit(Continue const& _node)
{
	setJsonNode(_node, "Continue", {});
	return false;
}

bool ASTJsonConverter::visit(Break const& _node)
{
	setJsonNode(_node, "Break", {});
	return false;
}

bool ASTJsonConverter::visit(Return const& _node)
{
	setJsonNode(_node, "Return", {});;
	return false;
}

bool ASTJsonConverter::visit(Throw const& _node)
{
	setJsonNode(_node, "Throw", {});;
	return false;
}

bool ASTJsonConverter::visit(VariableDeclarationStatement const& _node)
{
	setJsonNode(_node, "VariableDeclarationStatement", {});
	return false;
}

bool ASTJsonConverter::visit(ExpressionStatement const& _node)
{
	setJsonNode(_node, "ExpressionStatement", {});
	return false;
}

bool ASTJsonConverter::visit(Conditional const& _node)
{
	setJsonNode(_node, "Conditional", {});
	return false;
}

bool ASTJsonConverter::visit(Assignment const& _node)
{
	setJsonNode(_node, "Assignment", {
		make_pair("operator", Token::toString(_node.assignmentOperator())),
		make_pair("type", type(_node))
	});
	return false;
}

bool ASTJsonConverter::visit(TupleExpression const& _node)
{
	setJsonNode(_node, "TupleExpression",{});
	return false;
}

bool ASTJsonConverter::visit(UnaryOperation const& _node)
{
	setJsonNode(_node, "UnaryOperation", {
		make_pair("prefix", _node.isPrefixOperation()),
		make_pair("operator", Token::toString(_node.getOperator())),
		make_pair("type", type(_node))
	});
	return false;
}

bool ASTJsonConverter::visit(BinaryOperation const& _node)
{
	setJsonNode(_node, "BinaryOperation", {
		make_pair("operator", Token::toString(_node.getOperator())),
		make_pair("type", type(_node))
	});
	return false;
}

bool ASTJsonConverter::visit(FunctionCall const& _node)
{
	setJsonNode(_node, "FunctionCall", {
		make_pair("type_conversion", _node.annotation().isTypeConversion),
		make_pair("type", type(_node))
	});
	return false;
}

bool ASTJsonConverter::visit(NewExpression const& _node)
{
	setJsonNode(_node, "NewExpression", { make_pair("type", type(_node)) });
	return false;
}

bool ASTJsonConverter::visit(MemberAccess const& _node)
{
	setJsonNode(_node, "MemberAccess", {
		make_pair("member_name", _node.memberName()),
		make_pair("type", type(_node))
	});
	return false;
}

bool ASTJsonConverter::visit(IndexAccess const& _node)
{
	setJsonNode(_node, "IndexAccess", { make_pair("type", type(_node)) });
	return false;
}

bool ASTJsonConverter::visit(Identifier const& _node)
{
	setJsonNode(_node, "Identifier",
				{ make_pair("value", _node.name()), make_pair("type", type(_node)) });
	return false;
}

bool ASTJsonConverter::visit(ElementaryTypeNameExpression const& _node)
{
	setJsonNode(_node, "ElementaryTypeNameExpression", {
		make_pair("value", _node.typeName().toString()),
		make_pair("type", type(_node))
	});
	return false;
}

bool ASTJsonConverter::visit(Literal const& _node)
{
	char const* tokenString = Token::toString(_node.token());
	Json::Value value{_node.value()};
	if (!dev::validateUTF8(_node.value()))
		value = Json::nullValue;
	Token::Value subdenomination = Token::Value(_node.subDenomination());
	setJsonNode(_node, "Literal", {
		make_pair("token", tokenString ? tokenString : Json::Value()),
		make_pair("value", value),
		make_pair("hexvalue", toHex(_node.value())),
		make_pair(
			"subdenomination",
			subdenomination == Token::Illegal ?
			Json::nullValue :
			Json::Value{Token::toString(subdenomination)}
		),
		make_pair("type", type(_node))
	});
	return false;
}


void ASTJsonConverter::endVisit(EventDefinition const&)
{
	m_inEvent = false;
}

string ASTJsonConverter::visibility(Declaration::Visibility const& _visibility)
{
	switch (_visibility)
	{
	case Declaration::Visibility::Private:
		return "private";
	case Declaration::Visibility::Internal:
		return "internal";
	case Declaration::Visibility::Public:
		return "public";
	case Declaration::Visibility::External:
		return "external";
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown declaration visibility."));
	}
}

string ASTJsonConverter::location(VariableDeclaration::Location _location)
{
	switch (_location)
	{
	case VariableDeclaration::Location::Default:
		return "default";
	case VariableDeclaration::Location::Storage:
		return "storage";
	case VariableDeclaration::Location::Memory:
		return "memory";
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown declaration location."));
	}
}

string ASTJsonConverter::type(Expression const& _expression)
{
	return _expression.annotation().type ? _expression.annotation().type->toString() : "Unknown";
}

string ASTJsonConverter::type(VariableDeclaration const& _varDecl)
{
	return _varDecl.annotation().type ? _varDecl.annotation().type->toString() : "Unknown";
}

}
}
