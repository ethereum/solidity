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

ASTPrinter::ASTPrinter(ASTNode const& _ast, string const& _source):
	m_indentation(0), m_source(_source), m_ast(&_ast)
{
}

void ASTPrinter::print(ostream& _stream)
{
	m_ostream = &_stream;
	m_ast->accept(*this);
	m_ostream = nullptr;
}


bool ASTPrinter::visit(ImportDirective const& _node)
{
	writeLine("ImportDirective \"" + _node.getIdentifier() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ContractDefinition const& _node)
{
	writeLine("ContractDefinition \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(InheritanceSpecifier const& _node)
{
	writeLine("InheritanceSpecifier \"" + _node.getName()->getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(StructDefinition const& _node)
{
	writeLine("StructDefinition \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ParameterList const& _node)
{
	writeLine("ParameterList");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(FunctionDefinition const& _node)
{
	writeLine("FunctionDefinition \"" + _node.getName() + "\"" +
			  (_node.isPublic() ? " - public" : "") +
			  (_node.isDeclaredConst() ? " - const" : ""));
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(VariableDeclaration const& _node)
{
	writeLine("VariableDeclaration \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ModifierDefinition const& _node)
{
	writeLine("ModifierDefinition \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ModifierInvocation const& _node)
{
	writeLine("ModifierInvocation \"" + _node.getName()->getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(EventDefinition const& _node)
{
	writeLine("EventDefinition \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(TypeName const& _node)
{
	writeLine("TypeName");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ElementaryTypeName const& _node)
{
	writeLine(string("ElementaryTypeName ") + Token::toString(_node.getTypeName()));
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UserDefinedTypeName const& _node)
{
	writeLine("UserDefinedTypeName \"" + _node.getName() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Mapping const& _node)
{
	writeLine("Mapping");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Statement const& _node)
{
	writeLine("Statement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Block const& _node)
{
	writeLine("Block");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(PlaceholderStatement const& _node)
{
	writeLine("PlaceholderStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(IfStatement const& _node)
{
	writeLine("IfStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(BreakableStatement const& _node)
{
	writeLine("BreakableStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(WhileStatement const& _node)
{
	writeLine("WhileStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ForStatement const& _node)
{
	writeLine("ForStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Continue const& _node)
{
	writeLine("Continue");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Break const& _node)
{
	writeLine("Break");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Return const& _node)
{
	writeLine("Return");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(VariableDefinition const& _node)
{
	writeLine("VariableDefinition");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ExpressionStatement const& _node)
{
	writeLine("ExpressionStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Expression const& _node)
{
	writeLine("Expression");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Assignment const& _node)
{
	writeLine(string("Assignment using operator ") + Token::toString(_node.getAssignmentOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UnaryOperation const& _node)
{
	writeLine(string("UnaryOperation (") + (_node.isPrefixOperation() ? "prefix" : "postfix") +
			  ") " + Token::toString(_node.getOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(BinaryOperation const& _node)
{
	writeLine(string("BinaryOperation using operator ") + Token::toString(_node.getOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(FunctionCall const& _node)
{
	writeLine("FunctionCall");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(NewExpression const& _node)
{
	writeLine("NewExpression");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(MemberAccess const& _node)
{
	writeLine("MemberAccess to member " + _node.getMemberName());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(IndexAccess const& _node)
{
	writeLine("IndexAccess");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(PrimaryExpression const& _node)
{
	writeLine("PrimaryExpression");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Identifier const& _node)
{
	writeLine(string("Identifier ") + _node.getName());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ElementaryTypeNameExpression const& _node)
{
	writeLine(string("ElementaryTypeNameExpression ") + Token::toString(_node.getTypeToken()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Literal const& _node)
{
	char const* tokenString = Token::toString(_node.getToken());
	if (!tokenString)
		tokenString = "[no token]";
	writeLine(string("Literal, token: ") + tokenString + " value: " + _node.getValue());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

void ASTPrinter::endVisit(ImportDirective const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ContractDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(InheritanceSpecifier const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(StructDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ParameterList const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(FunctionDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(VariableDeclaration const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ModifierDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ModifierInvocation const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(EventDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(TypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ElementaryTypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(UserDefinedTypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Mapping const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Statement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Block const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(PlaceholderStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(IfStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(BreakableStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(WhileStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ForStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Continue const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Break const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Return const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(VariableDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ExpressionStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Expression const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Assignment const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(UnaryOperation const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(BinaryOperation const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(FunctionCall const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(NewExpression const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(MemberAccess const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(IndexAccess const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(PrimaryExpression const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Identifier const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ElementaryTypeNameExpression const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Literal const&)
{
	m_indentation--;
}

void ASTPrinter::printSourcePart(ASTNode const& _node)
{
	if (!m_source.empty())
	{
		Location const& location(_node.getLocation());
		*m_ostream << getIndentation() << "   Source: "
				   << escaped(m_source.substr(location.start, location.end - location.start), false) << endl;
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
