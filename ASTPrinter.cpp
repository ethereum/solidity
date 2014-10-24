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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Pretty-printer for the abstract syntax tree (the "pretty" is arguable), used for debugging.
 */

#include <libsolidity/ASTPrinter.h>
#include <libsolidity/AST.h>

using namespace std;

namespace dev
{
namespace solidity
{

ASTPrinter::ASTPrinter(ASTPointer<ASTNode> const& _ast, string const& _source):
	m_indentation(0), m_source(_source), m_ast(_ast)
{
}

void ASTPrinter::print(ostream& _stream)
{
	m_ostream = &_stream;
	m_ast->accept(*this);
	m_ostream = nullptr;
}


bool ASTPrinter::visit(ContractDefinition& _node)
{
	writeLine("ContractDefinition \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(StructDefinition& _node)
{
	writeLine("StructDefinition \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ParameterList& _node)
{
	writeLine("ParameterList");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(FunctionDefinition& _node)
{
	writeLine("FunctionDefinition \"" + _node.getName() + "\"" +
			  (_node.isPublic() ? " - public" : "") +
			  (_node.isDeclaredConst() ? " - const" : ""));
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(VariableDeclaration& _node)
{
	writeLine("VariableDeclaration \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(TypeName& _node)
{
	writeLine("TypeName");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ElementaryTypeName& _node)
{
	writeLine(string("ElementaryTypeName ") + Token::toString(_node.getTypeName()));
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UserDefinedTypeName& _node)
{
	writeLine("UserDefinedTypeName \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Mapping& _node)
{
	writeLine("Mapping");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Statement& _node)
{
	writeLine("Statement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Block& _node)
{
	writeLine("Block");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(IfStatement& _node)
{
	writeLine("IfStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(BreakableStatement& _node)
{
	writeLine("BreakableStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(WhileStatement& _node)
{
	writeLine("WhileStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Continue& _node)
{
	writeLine("Continue");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Break& _node)
{
	writeLine("Break");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Return& _node)
{
	writeLine("Return");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(VariableDefinition& _node)
{
	writeLine("VariableDefinition");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Expression& _node)
{
	writeLine("Expression");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Assignment& _node)
{
	writeLine(string("Assignment using operator ") + Token::toString(_node.getAssignmentOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UnaryOperation& _node)
{
	writeLine(string("UnaryOperation (") + (_node.isPrefixOperation() ? "prefix" : "postfix") +
			  ") " + Token::toString(_node.getOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(BinaryOperation& _node)
{
	writeLine(string("BinaryOperation using operator ") + Token::toString(_node.getOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(FunctionCall& _node)
{
	writeLine("FunctionCall");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(MemberAccess& _node)
{
	writeLine("MemberAccess to member " + _node.getMemberName());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(IndexAccess& _node)
{
	writeLine("IndexAccess");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(PrimaryExpression& _node)
{
	writeLine("PrimaryExpression");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Identifier& _node)
{
	writeLine(string("Identifier ") + _node.getName());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ElementaryTypeNameExpression& _node)
{
	writeLine(string("ElementaryTypeNameExpression ") + Token::toString(_node.getTypeToken()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Literal& _node)
{
	char const* tokenString = Token::toString(_node.getToken());
	if (!tokenString)
		tokenString = "[no token]";
	writeLine(string("Literal, token: ") + tokenString + " value: " + _node.getValue());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

void ASTPrinter::endVisit(ASTNode&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ContractDefinition&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(StructDefinition&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ParameterList&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(FunctionDefinition&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(VariableDeclaration&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(TypeName&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ElementaryTypeName&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(UserDefinedTypeName&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Mapping&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Statement&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Block&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(IfStatement&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(BreakableStatement&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(WhileStatement&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Continue&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Break&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Return&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(VariableDefinition&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Expression&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Assignment&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(UnaryOperation&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(BinaryOperation&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(FunctionCall&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(MemberAccess&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(IndexAccess&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(PrimaryExpression&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Identifier&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ElementaryTypeNameExpression&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Literal&)
{
	m_indentation--;
}

void ASTPrinter::printSourcePart(ASTNode const& _node)
{
	if (!m_source.empty())
	{
		Location const& location(_node.getLocation());
		*m_ostream << getIndentation() << "   Source: |"
				   << m_source.substr(location.start, location.end - location.start) << "|" << endl;
	}
}

void ASTPrinter::printType(Expression const& _expression)
{
	if (_expression.getType())
		*m_ostream << getIndentation() << "   Type: " << _expression.getType()->toString() << "\n";
	else
		*m_ostream << getIndentation() << "   Type unknown.\n";
}

string ASTPrinter::getIndentation() const
{
	return string(m_indentation * 2, ' ');
}

void ASTPrinter::writeLine(string const& _line)
{
	*m_ostream << getIndentation() << _line << endl;
}

}
}
