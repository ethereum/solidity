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
								   initializer_list<pair<string const, string const>> _list)
{
	Json::Value node;
	Json::Value attrs;

	node["type"] = _typeName;
	for (auto &e: _list)
		attrs[e.first] = e.second;
	node["attributes"] = attrs;

	m_childrenPtr->append(node);
}

ASTJsonConverter::ASTJsonConverter(ASTNode const& _ast): m_ast(&_ast)
{
	Json::Value attrs;
	Json::Value children;

	m_astJson["type"] = "root";
	attrs["name"] = "nameoffile"; //TODO
	m_astJson["attributes"] = attrs;
	m_astJson["children"] = children;
	m_childrenPtr = &m_astJson["children"];
}

void ASTJsonConverter::print(ostream& _stream)
{
	m_ast->accept(*this);
	_stream << m_astJson;
}


bool ASTJsonConverter::visit(ImportDirective const& _node)
{
	addJsonNode("import", { make_pair("file", _node.getIdentifier())});
	return goDeeper();
}

bool ASTJsonConverter::visit(ContractDefinition const& _node)
{
	// writeLine("ContractDefinition \"" + _node.getName() + "\"");
	// printSourcePart(_node);
	addJsonNode("contract", { make_pair("name", _node.getName())});
	return goDeeper();
}

bool ASTJsonConverter::visit(StructDefinition const& _node)
{
	// writeLine("StructDefinition \"" + _node.getName() + "\"");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(ParameterList const& _node)
{
	// writeLine("ParameterList");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(FunctionDefinition const& _node)
{
	// writeLine("FunctionDefinition \"" + _node.getName() + "\"" +
	// 		  (_node.isPublic() ? " - public" : "") +
	// 		  (_node.isDeclaredConst() ? " - const" : ""));
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(VariableDeclaration const& _node)
{
	// writeLine("VariableDeclaration \"" + _node.getName() + "\"");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(TypeName const& _node)
{
	// writeLine("TypeName");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(ElementaryTypeName const& _node)
{
	// writeLine(string("ElementaryTypeName ") + Token::toString(_node.getTypeName()));
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(UserDefinedTypeName const& _node)
{
	// writeLine("UserDefinedTypeName \"" + _node.getName() + "\"");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Mapping const& _node)
{
	// writeLine("Mapping");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Statement const& _node)
{
	// writeLine("Statement");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Block const& _node)
{
	// writeLine("Block");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(IfStatement const& _node)
{
	// writeLine("IfStatement");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(BreakableStatement const& _node)
{
	// writeLine("BreakableStatement");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(WhileStatement const& _node)
{
	// writeLine("WhileStatement");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(ForStatement const& _node)
{
	// writeLine("ForStatement");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Continue const& _node)
{
	// writeLine("Continue");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Break const& _node)
{
	// writeLine("Break");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Return const& _node)
{
	// writeLine("Return");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(VariableDefinition const& _node)
{
	// writeLine("VariableDefinition");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(ExpressionStatement const& _node)
{
	// writeLine("ExpressionStatement");
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Expression const& _node)
{
	// writeLine("Expression");
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Assignment const& _node)
{
	// writeLine(string("Assignment using operator ") + Token::toString(_node.getAssignmentOperator()));
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(UnaryOperation const& _node)
{
	// writeLine(string("UnaryOperation (") + (_node.isPrefixOperation() ? "prefix" : "postfix") +
	// 		  ") " + Token::toString(_node.getOperator()));
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(BinaryOperation const& _node)
{
	// writeLine(string("BinaryOperation using operator ") + Token::toString(_node.getOperator()));
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(FunctionCall const& _node)
{
	// writeLine("FunctionCall");
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(NewExpression const& _node)
{
	// writeLine("NewExpression");
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(MemberAccess const& _node)
{
	// writeLine("MemberAccess to member " + _node.getMemberName());
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(IndexAccess const& _node)
{
	// writeLine("IndexAccess");
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(PrimaryExpression const& _node)
{
	// writeLine("PrimaryExpression");
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Identifier const& _node)
{
	// writeLine(string("Identifier ") + _node.getName());
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(ElementaryTypeNameExpression const& _node)
{
	// writeLine(string("ElementaryTypeNameExpression ") + Token::toString(_node.getTypeToken()));
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

bool ASTJsonConverter::visit(Literal const& _node)
{
	// char const* tokenString = Token::toString(_node.getToken());
	// if (!tokenString)
	// 	tokenString = "[no token]";
	// writeLine(string("Literal, token: ") + tokenString + " value: " + _node.getValue());
	// printType(_node);
	// printSourcePart(_node);
	return goDeeper();
}

void ASTJsonConverter::endVisit(ImportDirective const&)
{

}

void ASTJsonConverter::endVisit(ContractDefinition const&)
{

}

void ASTJsonConverter::endVisit(StructDefinition const&)
{

}

void ASTJsonConverter::endVisit(ParameterList const&)
{

}

void ASTJsonConverter::endVisit(FunctionDefinition const&)
{

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

}

void ASTJsonConverter::endVisit(Block const&)
{

}

void ASTJsonConverter::endVisit(IfStatement const&)
{

}

void ASTJsonConverter::endVisit(BreakableStatement const&)
{

}

void ASTJsonConverter::endVisit(WhileStatement const&)
{

}

void ASTJsonConverter::endVisit(ForStatement const&)
{

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

}

void ASTJsonConverter::endVisit(ExpressionStatement const&)
{

}

void ASTJsonConverter::endVisit(Expression const&)
{

}

void ASTJsonConverter::endVisit(Assignment const&)
{

}

void ASTJsonConverter::endVisit(UnaryOperation const&)
{

}

void ASTJsonConverter::endVisit(BinaryOperation const&)
{

}

void ASTJsonConverter::endVisit(FunctionCall const&)
{

}

void ASTJsonConverter::endVisit(NewExpression const&)
{

}

void ASTJsonConverter::endVisit(MemberAccess const&)
{

}

void ASTJsonConverter::endVisit(IndexAccess const&)
{

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
