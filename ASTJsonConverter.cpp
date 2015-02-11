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

#include <libsolidity/ASTJsonConverter.h>
#include <libsolidity/AST.h>

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

void ASTJsonConverter::addJsonNode(string const& _nodeName,
								   initializer_list<pair<string const, string const>> _list,
								   bool _hasChildren = false)
{
	Json::Value node;

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

ASTJsonConverter::ASTJsonConverter(ASTNode const& _ast): m_ast(&_ast)
{
	Json::Value children(Json::arrayValue);

	m_astJson["name"] = "root";
	m_astJson["children"] = children;
	m_jsonNodePtrs.push(&m_astJson["children"]);
}

void ASTJsonConverter::print(ostream& _stream)
{
	m_ast->accept(*this);
	_stream << m_astJson;
}

bool ASTJsonConverter::visit(ImportDirective const& _node)
{
	addJsonNode("Import", { make_pair("file", _node.getIdentifier())});
	return true;
}

bool ASTJsonConverter::visit(ContractDefinition const& _node)
{
	addJsonNode("Contract", { make_pair("name", _node.getName()) }, true);
	return true;
}

bool ASTJsonConverter::visit(StructDefinition const& _node)
{
	addJsonNode("Struct", { make_pair("name", _node.getName()) }, true);
	return true;
}

bool ASTJsonConverter::visit(ParameterList const&)
{
	addJsonNode("ParameterList", {}, true);
	return true;
}

bool ASTJsonConverter::visit(FunctionDefinition const& _node)
{
	addJsonNode("Function",
				{ make_pair("name", _node.getName()),
					make_pair("public", boost::lexical_cast<std::string>(_node.isPublic())),
					make_pair("const", boost::lexical_cast<std::string>(_node.isDeclaredConst())) },
				true);
	return true;
}

bool ASTJsonConverter::visit(VariableDeclaration const& _node)
{
	bool isLocalVariable = (_node.getLValueType() == VariableDeclaration::LValueType::Local);
	addJsonNode("VariableDeclaration",
				{   make_pair("name", _node.getName()),
					make_pair("local", boost::lexical_cast<std::string>(isLocalVariable))},
				true);
	return true;
}

bool ASTJsonConverter::visit(TypeName const&)
{
	return true;
}

bool ASTJsonConverter::visit(ElementaryTypeName const& _node)
{
	addJsonNode("ElementaryTypeName", { make_pair("name", Token::toString(_node.getTypeName())) });
	return true;
}

bool ASTJsonConverter::visit(UserDefinedTypeName const& _node)
{
	addJsonNode("UserDefinedTypeName", { make_pair("name", _node.getName()) });
	return true;
}

bool ASTJsonConverter::visit(Mapping const&)
{
	addJsonNode("Mapping", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Statement const&)
{
	addJsonNode("Statement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Block const&)
{
	addJsonNode("Block", {}, true);
	return true;
}

bool ASTJsonConverter::visit(IfStatement const&)
{
	addJsonNode("IfStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(BreakableStatement const&)
{
	return true;
}

bool ASTJsonConverter::visit(WhileStatement const&)
{
	addJsonNode("WhileStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ForStatement const&)
{
	addJsonNode("ForStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Continue const&)
{
	addJsonNode("Continue", {});
	return true;
}

bool ASTJsonConverter::visit(Break const&)
{
	addJsonNode("Break", {});
	return true;
}

bool ASTJsonConverter::visit(Return const&)
{
	addJsonNode("Return", {}, true);;
	return true;
}

bool ASTJsonConverter::visit(VariableDefinition const&)
{
	addJsonNode("VariableDefinition", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ExpressionStatement const&)
{
	addJsonNode("ExpressionStatement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Expression const& _node)
{
	addJsonNode("Expression",
				{ make_pair("type", getType(_node)),
					make_pair("lvalue", boost::lexical_cast<std::string>(_node.isLValue())),
					make_pair("local_lvalue", boost::lexical_cast<std::string>(_node.isLocalLValue())) },
				true);
	return true;
}

bool ASTJsonConverter::visit(Assignment const& _node)
{
	addJsonNode("Assignment",
				{ make_pair("operator", Token::toString(_node.getAssignmentOperator())),
					make_pair("type", getType(_node)) },
				true);
	return true;
}

bool ASTJsonConverter::visit(UnaryOperation const& _node)
{
	addJsonNode("UnaryOperation",
				{ make_pair("prefix", boost::lexical_cast<std::string>(_node.isPrefixOperation())),
					make_pair("operator", Token::toString(_node.getOperator())),
					make_pair("type", getType(_node)) },
				true);
	return true;
}

bool ASTJsonConverter::visit(BinaryOperation const& _node)
{
	addJsonNode("BinaryOperation",
				{ make_pair("operator", Token::toString(_node.getOperator())),
					make_pair("type", getType(_node))},
				true);
	return true;
}

bool ASTJsonConverter::visit(FunctionCall const& _node)
{
	addJsonNode("FunctionCall",
				{ make_pair("type_conversion", boost::lexical_cast<std::string>(_node.isTypeConversion())),
					make_pair("type", getType(_node)) },
				true);
	return true;
}

bool ASTJsonConverter::visit(NewExpression const& _node)
{
	addJsonNode("NewExpression", { make_pair("type", getType(_node)) }, true);
	return true;
}

bool ASTJsonConverter::visit(MemberAccess const& _node)
{
	addJsonNode("MemberAccess",
				{ make_pair("member_name", _node.getMemberName()),
					make_pair("type", getType(_node)) },
				true);
	return true;
}

bool ASTJsonConverter::visit(IndexAccess const& _node)
{
	addJsonNode("IndexAccess", { make_pair("type", getType(_node)) }, true);
	return true;
}

bool ASTJsonConverter::visit(PrimaryExpression const&)
{
	return true;
}

bool ASTJsonConverter::visit(Identifier const& _node)
{
	addJsonNode("Identifier",
				{ make_pair("value", _node.getName()), make_pair("type", getType(_node)) });
	return true;
}

bool ASTJsonConverter::visit(ElementaryTypeNameExpression const& _node)
{
	addJsonNode("ElementaryTypenameExpression",
				{ make_pair("value", Token::toString(_node.getTypeToken())), make_pair("type", getType(_node)) });
	return true;
}

bool ASTJsonConverter::visit(Literal const& _node)
{
	char const* tokenString = Token::toString(_node.getToken());
	addJsonNode("Literal",
				{ make_pair("string", (tokenString) ? tokenString : "null"),
					make_pair("value", _node.getValue()),
					make_pair("type", getType(_node)) });
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

void ASTJsonConverter::endVisit(Statement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Block const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(IfStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(BreakableStatement const&)
{
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

void ASTJsonConverter::endVisit(VariableDefinition const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(ExpressionStatement const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Expression const&)
{
	goUp();
}

void ASTJsonConverter::endVisit(Assignment const&)
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

void ASTJsonConverter::endVisit(PrimaryExpression const&)
{
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

string ASTJsonConverter::getType(Expression const& _expression)
{
	return  (_expression.getType()) ? _expression.getType()->toString() : "Unknown";
}

}
}
