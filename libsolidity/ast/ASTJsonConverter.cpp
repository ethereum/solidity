/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
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

using namespace std;

namespace dev
{
namespace solidity
{

void ASTJsonConverter::addJsonNode(
	ASTNode const& _node,
	string const& _nodeName,
	initializer_list<pair<string const, Json::Value const>> _attributes,
	bool _hasChildren = false
)
{
	Json::Value node;

	node["id"] = reinterpret_cast<Json::UInt64>(&_node);
	node["src"] = sourceLocationToString(_node.location());
	node["name"] = _nodeName;
	if (_attributes.size() != 0)
	{
		Json::Value attrs;
		for (auto& e: _attributes)
			attrs[e.first] = e.second;
		node["attributes"] = attrs;
	}

	m_jsonNodePtrs.top()->append(node);

	if (_hasChildren)
	{
		Json::Value& addedNode = (*m_jsonNodePtrs.top())[m_jsonNodePtrs.top()->size() - 1];
		Json::Value children(Json::arrayValue);
		addedNode["children"] = children;
		m_jsonNodePtrs.push(&addedNode["children"]);
	}
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

ASTJsonConverter::ASTJsonConverter(
	ASTNode const& _ast,
	map<string, unsigned> _sourceIndices
): m_ast(&_ast), m_sourceIndices(_sourceIndices)
{
}

void ASTJsonConverter::print(ostream& _stream)
{
	process();
	_stream << m_astJson;
}

Json::Value const& ASTJsonConverter::json()
{
	process();
	return m_astJson;
}

bool ASTJsonConverter::visit(SourceUnit const&)
{
	Json::Value children(Json::arrayValue);

	m_astJson["name"] = "SourceUnit";
	m_astJson["children"] = children;
	m_jsonNodePtrs.push(&m_astJson["children"]);

	return true;
}

bool ASTJsonConverter::visit(PragmaDirective const& _node)
{
	Json::Value literals(Json::arrayValue);
	for (auto const& literal: _node.literals())
		literals.append(literal);
	addJsonNode(_node, "PragmaDirective", { make_pair("literals", literals) });
	return true;
}

bool ASTJsonConverter::visit(ImportDirective const& _node)
{
	addJsonNode(_node, "ImportDirective", { make_pair("file", _node.path())});
	return true;
}

bool ASTJsonConverter::visit(ContractDefinition const& _node)
{
	Json::Value linearizedBaseContracts(Json::arrayValue);
	for (auto const& baseContract: _node.annotation().linearizedBaseContracts)
		linearizedBaseContracts.append(reinterpret_cast<Json::UInt64>(baseContract));
	addJsonNode(_node, "ContractDefinition", {
		make_pair("name", _node.name()),
		make_pair("isLibrary", _node.isLibrary()),
		make_pair("fullyImplemented", _node.annotation().isFullyImplemented),
		make_pair("linearizedBaseContracts", linearizedBaseContracts),
	}, true);
	return true;
}

bool ASTJsonConverter::visit(InheritanceSpecifier const& _node)
{
	addJsonNode(_node, "InheritanceSpecifier", {}, true);
	return true;
}

bool ASTJsonConverter::visit(UsingForDirective const& _node)
{
	addJsonNode(_node, "UsingForDirective", {}, true);
	return true;
}

bool ASTJsonConverter::visit(StructDefinition const& _node)
{
	addJsonNode(_node, "StructDefinition", { make_pair("name", _node.name()) }, true);
	return true;
}

bool ASTJsonConverter::visit(EnumDefinition const& _node)
{
	addJsonNode(_node, "EnumDefinition", { make_pair("name", _node.name()) }, true);
	return true;
}

bool ASTJsonConverter::visit(EnumValue const& _node)
{
	addJsonNode(_node, "EnumValue", { make_pair("name", _node.name()) });
	return true;
}

bool ASTJsonConverter::visit(ParameterList const& _node)
{
	addJsonNode(_node, "ParameterList", {}, true);
	return true;
}

bool ASTJsonConverter::visit(FunctionDefinition const& _node)
{
	addJsonNode(_node, "FunctionDefinition", {
		make_pair("name", _node.name()),
		make_pair("public", _node.isPublic()),
		make_pair("constant", _node.isDeclaredConst())
	}, true);
	return true;
}

bool ASTJsonConverter::visit(VariableDeclaration const& _node)
{
	addJsonNode(_node, "VariableDeclaration", {
		make_pair("name", _node.name()),
		make_pair("type", type(_node))
	}, true);
	return true;
}

bool ASTJsonConverter::visit(ModifierDefinition const& _node)
{
	addJsonNode(_node, "ModifierDefinition", { make_pair("name", _node.name()) }, true);
	return true;
}

bool ASTJsonConverter::visit(ModifierInvocation const& _node)
{
	addJsonNode(_node, "ModifierInvocation", {}, true);
	return true;
}

bool ASTJsonConverter::visit(TypeName const&)
{
	return true;
}

bool ASTJsonConverter::visit(EventDefinition const& _node)
{
	addJsonNode(_node, "EventDefinition", { make_pair("name", _node.name()) }, true);
	return true;
}

bool ASTJsonConverter::visit(ElementaryTypeName const& _node)
{
	addJsonNode(_node, "ElementaryTypeName", { make_pair("name", _node.typeName().toString()) });
	return true;
}

bool ASTJsonConverter::visit(UserDefinedTypeName const& _node)
{
	addJsonNode(_node, "UserDefinedTypeName", {
		make_pair("name", boost::algorithm::join(_node.namePath(), "."))
	});
	return true;
}

bool ASTJsonConverter::visit(FunctionTypeName const& _node)
{
	addJsonNode(_node, "FunctionTypeName", {
		make_pair("payable", _node.isPayable()),
		make_pair("constant", _node.isDeclaredConst())
	});
	return true;
}

bool ASTJsonConverter::visit(Mapping const& _node)
{
	addJsonNode(_node, "Mapping", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ArrayTypeName const& _node)
{
	addJsonNode(_node, "ArrayTypeName", {}, true);
	return true;
}

bool ASTJsonConverter::visit(InlineAssembly const& _node)
{
	addJsonNode(_node, "InlineAssembly", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Block const& _node)
{
	addJsonNode(_node, "Block", {}, true);
	return true;
}

bool ASTJsonConverter::visit(PlaceholderStatement const& _node)
{
	addJsonNode(_node, "PlaceholderStatement", {});
	return true;
}

bool ASTJsonConverter::visit(IfStatement const& _node)
{
	addJsonNode(_node, "IfStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(WhileStatement const& _node)
{
	addJsonNode(_node, "WhileStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ForStatement const& _node)
{
	addJsonNode(_node, "ForStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Continue const& _node)
{
	addJsonNode(_node, "Continue", {});
	return true;
}

bool ASTJsonConverter::visit(Break const& _node)
{
	addJsonNode(_node, "Break", {});
	return true;
}

bool ASTJsonConverter::visit(Return const& _node)
{
	addJsonNode(_node, "Return", {}, true);;
	return true;
}

bool ASTJsonConverter::visit(Throw const& _node)
{
	addJsonNode(_node, "Throw", {}, true);;
	return true;
}

bool ASTJsonConverter::visit(VariableDeclarationStatement const& _node)
{
	addJsonNode(_node, "VariableDefinitionStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ExpressionStatement const& _node)
{
	addJsonNode(_node, "ExpressionStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Conditional const& _node)
{
	addJsonNode(_node, "Conditional", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Assignment const& _node)
{
	addJsonNode(_node, "Assignment",
				{ make_pair("operator", Token::toString(_node.assignmentOperator())),
					make_pair("type", type(_node)) },
				true);
	return true;
}

bool ASTJsonConverter::visit(TupleExpression const& _node)
{
	addJsonNode(_node, "TupleExpression",{}, true);
	return true;
}

bool ASTJsonConverter::visit(UnaryOperation const& _node)
{
	addJsonNode(_node, "UnaryOperation",
				{ make_pair("prefix", _node.isPrefixOperation()),
					make_pair("operator", Token::toString(_node.getOperator())),
					make_pair("type", type(_node)) },
				true);
	return true;
}

bool ASTJsonConverter::visit(BinaryOperation const& _node)
{
	addJsonNode(_node, "BinaryOperation", {
		make_pair("operator", Token::toString(_node.getOperator())),
		make_pair("type", type(_node))
	}, true);
	return true;
}

bool ASTJsonConverter::visit(FunctionCall const& _node)
{
	addJsonNode(_node, "FunctionCall", {
		make_pair("type_conversion", _node.annotation().isTypeConversion),
		make_pair("type", type(_node))
	}, true);
	return true;
}

bool ASTJsonConverter::visit(NewExpression const& _node)
{
	addJsonNode(_node, "NewExpression", { make_pair("type", type(_node)) }, true);
	return true;
}

bool ASTJsonConverter::visit(MemberAccess const& _node)
{
	addJsonNode(_node, "MemberAccess", {
		make_pair("member_name", _node.memberName()),
		make_pair("type", type(_node))
	}, true);
	return true;
}

bool ASTJsonConverter::visit(IndexAccess const& _node)
{
	addJsonNode(_node, "IndexAccess", { make_pair("type", type(_node)) }, true);
	return true;
}

bool ASTJsonConverter::visit(Identifier const& _node)
{
	addJsonNode(_node, "Identifier",
				{ make_pair("value", _node.name()), make_pair("type", type(_node)) });
	return true;
}

bool ASTJsonConverter::visit(ElementaryTypeNameExpression const& _node)
{
	addJsonNode(_node, "ElementaryTypenameExpression", {
		make_pair("value", _node.typeName().toString()),
		make_pair("type", type(_node))
	});
	return true;
}

bool ASTJsonConverter::visit(Literal const& _node)
{
	char const* tokenString = Token::toString(_node.token());
	size_t invalidPos = 0;
	Json::Value value{_node.value()};
	if (!dev::validate(_node.value(), invalidPos))
		value = Json::nullValue;
	Token::Value subdenomination = Token::Value(_node.subDenomination());
	addJsonNode(_node, "Literal", {
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
	return true;
}

void ASTJsonConverter::endVisit(SourceUnit const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(PragmaDirective const&)
{
}

void ASTJsonConverter::endVisit(ImportDirective const&)
{
}

void ASTJsonConverter::endVisit(ContractDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(InheritanceSpecifier const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(UsingForDirective const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(StructDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(EnumDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(EnumValue const&)
{
}

void ASTJsonConverter::endVisit(ParameterList const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(FunctionDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(VariableDeclaration const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(ModifierDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(ModifierInvocation const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(EventDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(TypeName const&)
{
}

void ASTJsonConverter::endVisit(ElementaryTypeName const&)
{
}

void ASTJsonConverter::endVisit(UserDefinedTypeName const&)
{
}

void ASTJsonConverter::endVisit(FunctionTypeName const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Mapping const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(ArrayTypeName const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(InlineAssembly const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Block const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(PlaceholderStatement const&)
{
}

void ASTJsonConverter::endVisit(IfStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(WhileStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(ForStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Continue const&)
{
}

void ASTJsonConverter::endVisit(Break const&)
{
}

void ASTJsonConverter::endVisit(Return const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Throw const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(VariableDeclarationStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(ExpressionStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Conditional const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Assignment const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(TupleExpression const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(UnaryOperation const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(BinaryOperation const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(FunctionCall const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(NewExpression const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(MemberAccess const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(IndexAccess const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Identifier const&)
{
}

void ASTJsonConverter::endVisit(ElementaryTypeNameExpression const&)
{
}

void ASTJsonConverter::endVisit(Literal const&)
{
}

void ASTJsonConverter::process()
{
	if (!processed)
		m_ast->accept(*this);
	processed = true;
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
