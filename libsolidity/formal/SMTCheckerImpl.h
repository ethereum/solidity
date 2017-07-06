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

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

#include <z3++.h>

#include <map>
#include <string>

namespace dev
{
namespace solidity
{

class ErrorReporter;

class SMTCheckerImpl: private ASTConstVisitor
{
public:
	SMTCheckerImpl(ErrorReporter& _errorReporter);

	void analyze(SourceUnit const& _sources);

private:
	// TODO: Check that we do not have concurrent reads and writes to a variable,
	// because the order of expression evaluation is undefined
	// TODO: or just force a certain order, but people might have a different idea about that.

	virtual void endVisit(VariableDeclaration const& _node) override;
	virtual bool visit(FunctionDefinition const& _node) override;
	virtual void endVisit(FunctionDefinition const& _node) override;
	virtual void endVisit(VariableDeclarationStatement const& _node) override;
	virtual void endVisit(ExpressionStatement const& _node) override;
	virtual void endVisit(Assignment const& _node) override;
	virtual void endVisit(TupleExpression const& _node) override;
	virtual void endVisit(BinaryOperation const& _node) override;
	virtual void endVisit(FunctionCall const& _node) override;
	virtual void endVisit(Identifier const& _node) override;
	virtual void endVisit(Literal const& _node) override;

	void arithmeticOperation(BinaryOperation const& _op);
	void compareOperation(BinaryOperation const& _op);
	void booleanOperation(BinaryOperation const& _op);

	void checkCondition(
		z3::expr _condition,
		SourceLocation const& _location,
		std::string const& _description,
		std::string const& _additionalValueName = "",
		z3::expr* _additionalValue = nullptr
	);

	void createVariable(VariableDeclaration const& _varDecl, bool _setToZero);

	std::string uniqueSymbol(Declaration const& _decl);
	std::string uniqueSymbol(Expression const& _expr);
	bool knownVariable(Declaration const& _decl);
	z3::expr currentValue(Declaration const& _decl);
	z3::expr newValue(Declaration const& _decl);

	z3::expr minValue(IntegerType const& _t);
	z3::expr maxValue(IntegerType const& _t);

	/// Returns the z3 expression corresponding to the AST node. Creates a new expression
	/// if it does not exist yet.
	z3::expr expr(Expression const& _e);
	/// Returns the z3 function declaration corresponding to the given variable.
	/// The function takes one argument which is the "sequence number".
	z3::func_decl var(Declaration const& _decl);

	z3::context m_context;
	z3::solver m_solver;
	std::map<Declaration const*, int> m_currentSequenceCounter;
	std::map<Expression const*, z3::expr> m_z3Expressions;
	std::map<Declaration const*, z3::func_decl> m_z3Variables;
	ErrorReporter& m_errorReporter;

	FunctionDefinition const* m_currentFunction = nullptr;
};

}
}
