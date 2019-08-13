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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Pretty-printer for the abstract syntax tree (the "pretty" is arguable), used for debugging.
 */

#include <libsolidity/ast/ASTPrinter.h>

#include <libsolidity/ast/AST.h>
#include <boost/algorithm/string/join.hpp>
#include <json/json.h>

using namespace std;
using namespace langutil;

namespace dev
{
namespace solidity
{

ASTPrinter::ASTPrinter(
	ASTNode const& _ast,
	string const& _source,
	GasEstimator::ASTGasConsumption const& _gasCosts
): m_indentation(0), m_source(_source), m_ast(&_ast), m_gasCosts(_gasCosts)
{
}

void ASTPrinter::print(ostream& _stream)
{
	m_ostream = &_stream;
	m_ast->accept(*this);
	m_ostream = nullptr;
}


bool ASTPrinter::visit(PragmaDirective const& _node)
{
	writeLine("PragmaDirective");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ImportDirective const& _node)
{
	writeLine("ImportDirective \"" + _node.path() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ContractDefinition const& _node)
{
	writeLine("ContractDefinition \"" + _node.name() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(InheritanceSpecifier const& _node)
{
	writeLine("InheritanceSpecifier");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UsingForDirective const& _node)
{
	writeLine("UsingForDirective");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(StructDefinition const& _node)
{
	writeLine("StructDefinition \"" + _node.name() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(EnumDefinition const& _node)
{
	writeLine("EnumDefinition \"" + _node.name() + "\"");
	return goDeeper();
}

bool ASTPrinter::visit(EnumValue const& _node)
{
	writeLine("EnumValue \"" + _node.name() + "\"");
	return goDeeper();
}

bool ASTPrinter::visit(ParameterList const& _node)
{
	writeLine("ParameterList");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(OverrideSpecifier const& _node)
{
	writeLine("OverrideSpecifier");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(FunctionDefinition const& _node)
{
	writeLine(
		"FunctionDefinition \"" +
		_node.name() +
		"\"" +
		(_node.isPublic() ? " - public" : "") +
		(_node.stateMutability() == StateMutability::View ? " - const" : "")
	);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(VariableDeclaration const& _node)
{
	writeLine("VariableDeclaration \"" + _node.name() + "\"");
	*m_ostream << indentation() << (
		_node.annotation().type ?
		string("   Type: ") + _node.annotation().type->toString() :
		string("   Type unknown.")
	) << "\n";
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ModifierDefinition const& _node)
{
	writeLine("ModifierDefinition \"" + _node.name() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ModifierInvocation const& _node)
{
	writeLine("ModifierInvocation \"" + _node.name()->name() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(EventDefinition const& _node)
{
	writeLine("EventDefinition \"" + _node.name() + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ElementaryTypeName const& _node)
{
	writeLine(string("ElementaryTypeName ") + _node.typeName().toString());
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UserDefinedTypeName const& _node)
{
	writeLine("UserDefinedTypeName \"" + boost::algorithm::join(_node.namePath(), ".") + "\"");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(FunctionTypeName const& _node)
{
	writeLine("FunctionTypeName");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Mapping const& _node)
{
	writeLine("Mapping");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ArrayTypeName const& _node)
{
	writeLine("ArrayTypeName");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(InlineAssembly const& _node)
{
	writeLine("InlineAssembly");
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

bool ASTPrinter::visit(WhileStatement const& _node)
{
	writeLine(_node.isDoWhile() ? "DoWhileStatement" : "WhileStatement");
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

bool ASTPrinter::visit(Throw const& _node)
{
	writeLine("Throw");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(EmitStatement const& _node)
{
	writeLine("EmitStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(VariableDeclarationStatement const& _node)
{
	writeLine("VariableDeclarationStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ExpressionStatement const& _node)
{
	writeLine("ExpressionStatement");
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Conditional const& _node)
{
	writeLine("Conditional");
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Assignment const& _node)
{
	writeLine(string("Assignment using operator ") + TokenTraits::toString(_node.assignmentOperator()));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(TupleExpression const& _node)
{
	writeLine(string("TupleExpression"));
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(UnaryOperation const& _node)
{
	writeLine(
		string("UnaryOperation (") +
		(_node.isPrefixOperation() ? "prefix" : "postfix") +
		") " +
		TokenTraits::toString(_node.getOperator())
	);
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(BinaryOperation const& _node)
{
	writeLine(string("BinaryOperation using operator ") + TokenTraits::toString(_node.getOperator()));
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
	writeLine("MemberAccess to member " + _node.memberName());
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

bool ASTPrinter::visit(Identifier const& _node)
{
	writeLine(string("Identifier ") + _node.name());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(ElementaryTypeNameExpression const& _node)
{
	writeLine(string("ElementaryTypeNameExpression ") + _node.typeName().toString());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

bool ASTPrinter::visit(Literal const& _node)
{
	char const* tokenString = TokenTraits::toString(_node.token());
	if (!tokenString)
		tokenString = "[no token]";
	writeLine(string("Literal, token: ") + tokenString + " value: " + _node.value());
	printType(_node);
	printSourcePart(_node);
	return goDeeper();
}

void ASTPrinter::endVisit(PragmaDirective const&)
{
	m_indentation--;
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

void ASTPrinter::endVisit(UsingForDirective const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(StructDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(EnumDefinition const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(EnumValue const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ParameterList const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(OverrideSpecifier const&)
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

void ASTPrinter::endVisit(ElementaryTypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(UserDefinedTypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(FunctionTypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Mapping const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ArrayTypeName const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(InlineAssembly const&)
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

void ASTPrinter::endVisit(Throw const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(EmitStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(VariableDeclarationStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(ExpressionStatement const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Conditional const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(Assignment const&)
{
	m_indentation--;
}

void ASTPrinter::endVisit(TupleExpression const&)
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
	if (m_gasCosts.count(&_node))
		*m_ostream << indentation() << "   Gas costs: " << m_gasCosts.at(&_node) << endl;
	if (!m_source.empty())
	{
		SourceLocation const& location(_node.location());
		*m_ostream <<
			indentation() <<
			"   Source: " <<
			Json::valueToQuotedString(m_source.substr(location.start, location.end - location.start).c_str()) <<
			endl;
	}
}

void ASTPrinter::printType(Expression const& _expression)
{
	if (_expression.annotation().type)
		*m_ostream << indentation() << "   Type: " << _expression.annotation().type->toString() << "\n";
	else
		*m_ostream << indentation() << "   Type unknown.\n";
}

string ASTPrinter::indentation() const
{
	return string(m_indentation * 2, ' ');
}

void ASTPrinter::writeLine(string const& _line)
{
	*m_ostream << indentation() << _line << endl;
}

}
}
