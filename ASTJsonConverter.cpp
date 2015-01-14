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

void ASTJsonConverter::addJsonNode(string const& _typeName,
								   initializer_list<pair<string const, string const>> _list,
								   bool _hasChildren = false)
{
	Json::Value node;
	Json::Value attrs;

	node["type"] = _typeName;
	for (auto &e: _list)
		attrs[e.first] = e.second;
	node["attributes"] = attrs;

	m_jsonNodePtrs.top().append(node);

	if (_hasChildren) {
		Json::Value children(Json::arrayValue);
		node["children"] = children;
		m_jsonNodePtrs.push(node["children"]);
		m_depth ++;
		cout << "goDown" << endl;
	}
}

ASTJsonConverter::ASTJsonConverter(ASTNode const& _ast): m_ast(&_ast), m_depth(0)
{
	Json::Value attrs;
	Json::Value children(Json::arrayValue);

	m_astJson["type"] = "root";
	m_astJson["attributes"] = attrs;
	attrs["name"] = "nameoffile"; //TODO
	m_astJson["children"] = children;
	m_jsonNodePtrs.push(m_astJson["children"]);
}

void ASTJsonConverter::print(ostream& _stream)
{
	m_ast->accept(*this);
	_stream << m_astJson;
}


bool ASTJsonConverter::visit(ImportDirective const& _node)
{
	addJsonNode("import", { make_pair("file", _node.getIdentifier())});
	return true;
}

bool ASTJsonConverter::visit(ContractDefinition const& _node)
{
	addJsonNode("contract", { make_pair("name", _node.getName())}, true);
	return true;
}

bool ASTJsonConverter::visit(StructDefinition const& _node)
{
	addJsonNode("struct", { make_pair("name", _node.getName())}, true);
	return true;
}

bool ASTJsonConverter::visit(ParameterList const& _node)
{
	addJsonNode("parameter_list", {}, true);
	return true;
}

bool ASTJsonConverter::visit(FunctionDefinition const& _node)
{
	addJsonNode("function",
				{ make_pair("name", _node.getName()),
					make_pair("public", boost::lexical_cast<std::string>(_node.isPublic())),
					make_pair("const", boost::lexical_cast<std::string>(_node.isDeclaredConst()))},
				true);
	return true;
}

bool ASTJsonConverter::visit(VariableDeclaration const& _node)
{
	addJsonNode("variable_declaration",
				{ //make_pair("type", _node.getTypeName()->getName()),
					make_pair("name", _node.getName()),
					make_pair("local", boost::lexical_cast<std::string>(_node.isLocalVariable()))});
	return true;
}

bool ASTJsonConverter::visit(TypeName const& _node)
{
	// writeLine("TypeName");
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(ElementaryTypeName const& _node)
{
	// writeLine(string("ElementaryTypeName ") + Token::toString(_node.getTypeName()));
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(UserDefinedTypeName const& _node)
{
	// writeLine("UserDefinedTypeName \"" + _node.getName() + "\"");
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(Mapping const& _node)
{
	// writeLine("Mapping");
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(Statement const& _node)
{
	addJsonNode("statement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Block const& _node)
{
	addJsonNode("block", {}, true);
	return true;
}

bool ASTJsonConverter::visit(IfStatement const& _node)
{
	addJsonNode("if_statement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(BreakableStatement const& _node)
{
	// writeLine("BreakableStatement");
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(WhileStatement const& _node)
{
	addJsonNode("while_statement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ForStatement const& _node)
{
	addJsonNode("for_statement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Continue const& _node)
{
	addJsonNode("continue", {});
	return true;
}

bool ASTJsonConverter::visit(Break const& _node)
{
	addJsonNode("break", {});
	return true;
}

bool ASTJsonConverter::visit(Return const& _node)
{
	addJsonNode("return", {});;
	return true;
}

bool ASTJsonConverter::visit(VariableDefinition const& _node)
{
	addJsonNode("variable_definition", {}, true);
	return true;
}

bool ASTJsonConverter::visit(ExpressionStatement const& _node)
{
	addJsonNode("expression_statement", {}, true);
	return true;
}

bool ASTJsonConverter::visit(Expression const& _node)
{
	addJsonNode("expression",
				{
					make_pair("type", _node.getType()->toString()),
					make_pair("lvalue", boost::lexical_cast<std::string>(_node.isLValue())),
					make_pair("local_lvalue", boost::lexical_cast<std::string>(_node.isLocalLValue())),
				},
				true);
	// writeLine("Expression");
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(Assignment const& _node)
{
	addJsonNode("assignment", {make_pair("operator", Token::toString(_node.getAssignmentOperator()))}, true);
	// writeLine(string("Assignment using operator ") + Token::toString(_node.getAssignmentOperator()));
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(UnaryOperation const& _node)
{
	addJsonNode("unary_op",
				{make_pair("prefix", boost::lexical_cast<std::string>(_node.isPrefixOperation())),
					make_pair("operator", Token::toString(_node.getOperator()))},
				true);

	// writeLine(string("UnaryOperation (") + (_node.isPrefixOperation() ? "prefix" : "postfix") +
	// 		  ") " + Token::toString(_node.getOperator()));
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(BinaryOperation const& _node)
{
	addJsonNode("binary_op",
				{make_pair("operator", Token::toString(_node.getOperator()))},
				true);
	// writeLine(string("BinaryOperation using operator ") + Token::toString(_node.getOperator()));
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(FunctionCall const& _node)
{
	addJsonNode("function_call",
				{make_pair("type_conversion", boost::lexical_cast<std::string>(_node.isTypeConversion()))},
				true);
	// writeLine("FunctionCall");
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(NewExpression const& _node)
{
	addJsonNode("new_expression", {}, true);
	// writeLine("NewExpression");
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(MemberAccess const& _node)
{
	addJsonNode("member_access", {make_pair("member_name", _node.getMemberName())}, true);
	// writeLine("MemberAccess to member " + _node.getMemberName());
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(IndexAccess const& _node)
{
	addJsonNode("index_access", {}, true);
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(PrimaryExpression const& _node)
{
	// writeLine("PrimaryExpression");
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(Identifier const& _node)
{
	addJsonNode("identifier", {make_pair("value", _node.getName())});
	// writeLine(string("Identifier ") + _node.getName());
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(ElementaryTypeNameExpression const& _node)
{
	addJsonNode("elementary_typename_expression",
				{make_pair("value", Token::toString(_node.getTypeToken()))});
	// writeLine(string("ElementaryTypeNameExpression ") + Token::toString(_node.getTypeToken()));
	// printType(_node);
	// printSourcePart(_node);
	return true;
}

bool ASTJsonConverter::visit(Literal const& _node)
{
	char const* tokenString = Token::toString(_node.getToken());
	addJsonNode("literal",
				{
					make_pair("string", (tokenString) ? tokenString : "null"),
					make_pair("value", _node.getValue())});

	// char const* tokenString = Token::toString(_node.getToken());
	// if (!tokenString)
	// 	tokenString = "[no token]";
	// writeLine(string("Literal, token: ") + tokenString + " value: " + _node.getValue());
	// printType(_node);
	// printSourcePart(_node);
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

void ASTJsonConverter::printType(Expression const& _expression)
{
	// if (_expression.getType())
	// 	*m_ostream << getIndentation() << "   Type: " << _expression.getType()->toString() << "\n";
	// else
	// 	*m_ostream << getIndentation() << "   Type unknown.\n";
}


}
}
