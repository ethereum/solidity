#pragma once

#include <ostream>
#include <libsolidity/ASTVisitor.h>

namespace dev {
namespace solidity {

class ASTPrinter : public ASTVisitor
{
public:
	/// Create a printer for the given abstract syntax tree. If the source is specified,
	/// the corresponding parts of the source are printed with each node.
	ASTPrinter(ptr<ASTNode> _ast, const std::string& _source = std::string());
	/// Output the string representation of the AST to _stream.
	void print(std::ostream& _stream);

	bool visit(ContractDefinition& _node);
	bool visit(StructDefinition& _node);
	bool visit(ParameterList& _node);
	bool visit(FunctionDefinition& _node);
	bool visit(VariableDeclaration& _node);
	bool visit(TypeName& _node);
	bool visit(ElementaryTypeName& _node);
	bool visit(UserDefinedTypeName& _node);
	bool visit(Mapping& _node);
	bool visit(Statement& _node);
	bool visit(Block& _node);
	bool visit(IfStatement& _node);
	bool visit(BreakableStatement& _node);
	bool visit(WhileStatement& _node);
	bool visit(Continue& _node);
	bool visit(Break& _node);
	bool visit(Return& _node);
	bool visit(VariableDefinition& _node);
	bool visit(Expression& _node);
	bool visit(Assignment& _node);
	bool visit(UnaryOperation& _node);
	bool visit(BinaryOperation& _node);
	bool visit(FunctionCall& _node);
	bool visit(MemberAccess& _node);
	bool visit(IndexAccess& _node);
	bool visit(PrimaryExpression& _node);
	bool visit(ElementaryTypeNameExpression& _node);
	bool visit(Literal& _node);

	void endVisit(ASTNode & _node);
	void endVisit(ContractDefinition&);
	void endVisit(StructDefinition&);
	void endVisit(ParameterList&);
	void endVisit(FunctionDefinition&);
	void endVisit(VariableDeclaration&);
	void endVisit(TypeName&);
	void endVisit(ElementaryTypeName&);
	void endVisit(UserDefinedTypeName&);
	void endVisit(Mapping&);
	void endVisit(Statement&);
	void endVisit(Block&);
	void endVisit(IfStatement&);
	void endVisit(BreakableStatement&);
	void endVisit(WhileStatement&);
	void endVisit(Continue&);
	void endVisit(Break&);
	void endVisit(Return&);
	void endVisit(VariableDefinition&);
	void endVisit(Expression&);
	void endVisit(Assignment&);
	void endVisit(UnaryOperation&);
	void endVisit(BinaryOperation&);
	void endVisit(FunctionCall&);
	void endVisit(MemberAccess&);
	void endVisit(IndexAccess&);
	void endVisit(PrimaryExpression&);
	void endVisit(ElementaryTypeNameExpression&);
	void endVisit(Literal&);

private:
	void printSourcePart(ASTNode const& _node);
	std::string getIndentation() const;
	void writeLine(std::string const& _line);
	bool goDeeper() { m_indentation++; return true; }
	int m_indentation;
	std::string m_source;
	ptr<ASTNode> m_ast;
	std::ostream* m_ostream;
};

} }
