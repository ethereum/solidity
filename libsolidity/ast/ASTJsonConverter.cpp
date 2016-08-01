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
#include <libsolidity/ast/AST.h>

using namespace std;

namespace dev
{
namespace solidity
{

void ASTJsonConverter::addKeyValue(Json::Value& _obj, string const& _key, string const& _val)
{
	// special handling for booleans
	if (_key == "const" || _key == "public" || _key == "local" ||
		_key == "lvalue" || _key == "local_lvalue" || _key == "prefix")
		_obj[_key] = (_val == "1") ? true : false;
	else
		// else simply add it as a string
		_obj[_key] = _val;
}

void ASTJsonConverter::addJsonNode(
	ASTNode const& _node,
	string const& _nodeName,
	initializer_list<pair<string const, string const>> _list,
	bool _hasChildren = false
)
{
	Json::Value node;

	node["id"] = reinterpret_cast<Json::UInt64>(&_node);
	node["src"] = sourceLocationToString(_node.location());
	node["name"] = _nodeName;
	if (_list.size() != 0)
	{
		Json::Value attrs;
		for (auto& e: _list)
			addKeyValue(attrs, e.first, e.second);
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
	Json::Value children(Json::arrayValue);

	m_astJson["name"] = "root";
	m_astJson["children"] = children;
	m_jsonNodePtrs.push(&m_astJson["children"]);
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

bool ASTJsonConverter::visit(ImportDirective const& _node)
{
	addJsonNode(_node, "Import", { make_pair("file", _node.path())});
	return true;
}

bool ASTJsonConverter::visit(ContractDefinition const& _node)
{
	addJsonNode(_node, "Contract", { make_pair("name", _node.name()) }, true);
	return true;
}

bool ASTJsonConverter::visit(StructDefinition const& _node)
{
	addJsonNode(_node, "Struct", { make_pair("name", _node.name()) }, true);
	return true;
}

bool ASTJsonConverter::visit(ParameterList const& _node)
{
	addJsonNode(_node, "ParameterList", {}, true);
	return true;
}

bool ASTJsonConverter::visit(FunctionDefinition const& _node)
{
	addJsonNode(_node, "Function",
				{ make_pair("name", _node.name()),
					make_pair("public", boost::lexical_cast<std::string>(_node.isPublic())),
					make_pair("const", boost::lexical_cast<std::string>(_node.isDeclaredConst())) },
				true);
	return true;
}

bool ASTJsonConverter::visit(VariableDeclaration const& _node)
{
	addJsonNode(_node, "VariableDeclaration", {
		make_pair("name", _node.name()),
		make_pair("name", _node.name()),
	}, true);
	return true;
}

bool ASTJsonConverter::visit(TypeName const&)
{
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

bool ASTJsonConverter::visit(Mapping const& _node)
{
	addJsonNode(_node, "Mapping", {}, true);
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
	addJsonNode(_node, "VariableDefinition", {}, true);
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
				{ make_pair("prefix", boost::lexical_cast<std::string>(_node.isPrefixOperation())),
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
		make_pair("type_conversion", boost::lexical_cast<std::string>(_node.annotation().isTypeConversion)),
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
	addJsonNode(_node, "MemberAccess",
				{ make_pair("member_name", _node.memberName()),
					make_pair("type", type(_node)) },
				true);
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
	addJsonNode(_node, "ElementaryTypenameExpression",
				{ make_pair("value", _node.typeName().toString()), make_pair("type", type(_node)) });
	return true;
}

bool ASTJsonConverter::visit(Literal const& _node)
{
	char const* tokenString = Token::toString(_node.token());
	addJsonNode(_node, "Literal",
				{ make_pair("string", (tokenString) ? tokenString : "null"),
					make_pair("value", _node.value()),
					make_pair("type", type(_node)) });
	return true;
}

void ASTJsonConverter::endVisit(ImportDirective const&)
{
}

void ASTJsonConverter::endVisit(ContractDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(StructDefinition const&)
{
	goUp();
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

void ASTJsonConverter::endVisit(Mapping const&)
{
}

void ASTJsonConverter::endVisit(InlineAssembly const&)
{
}

void ASTJsonConverter::endVisit(Block const&)
{
	goUp();
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
