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
#include <libsolidity/formal/SolverInterface.h>
#include <libsolidity/interface/ReadFile.h>

#include <map>
#include <string>

namespace dev
{
namespace solidity
{

class ErrorReporter;

class SMTChecker: private ASTConstVisitor
{
public:
	SMTChecker(ErrorReporter& _errorReporter, ReadCallback::Callback const& _readCallback);

	void analyze(SourceUnit const& _sources);

private:
	// TODO: Check that we do not have concurrent reads and writes to a variable,
	// because the order of expression evaluation is undefined
	// TODO: or just force a certain order, but people might have a different idea about that.

	virtual void endVisit(VariableDeclaration const& _node) override;
	virtual bool visit(FunctionDefinition const& _node) override;
	virtual void endVisit(FunctionDefinition const& _node) override;
	virtual bool visit(IfStatement const& _node) override;
	virtual bool visit(WhileStatement const& _node) override;
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
		smt::Expression _condition,
		SourceLocation const& _location,
		std::string const& _description,
		std::string const& _additionalValueName = "",
		smt::Expression* _additionalValue = nullptr
	);

	void createVariable(VariableDeclaration const& _varDecl, bool _setToZero);

	static std::string uniqueSymbol(Declaration const& _decl);
	static std::string uniqueSymbol(Expression const& _expr);

	/// @returns true if _delc is a variable that is known at the current point, i.e.
	/// has a valid sequence number
	bool knownVariable(Declaration const& _decl);
	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the current point.
	smt::Expression currentValue(Declaration const& _decl);
	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the given sequence point. Does not ensure that this sequence point exists.
	smt::Expression valueAtSequence(Declaration const& _decl, int _sequence);
	/// Allocates a new sequence number for the declaration, updates the current
	/// sequence number to this value and returns the expression.
	smt::Expression newValue(Declaration const& _decl);

	/// Sets the value of the declaration either to zero or to its intrinsic range.
	void setValue(Declaration const& _decl, bool _setToZero);

	static smt::Expression minValue(IntegerType const& _t);
	static smt::Expression maxValue(IntegerType const& _t);

	/// Returns the expression corresponding to the AST node. Creates a new expression
	/// if it does not exist yet.
	smt::Expression expr(Expression const& _e);
	/// Returns the function declaration corresponding to the given variable.
	/// The function takes one argument which is the "sequence number".
	smt::Expression var(Declaration const& _decl);

	std::shared_ptr<smt::SolverInterface> m_interface;
	std::map<Declaration const*, int> m_currentSequenceCounter;
	std::map<Declaration const*, int> m_nextFreeSequenceCounter;
	std::map<Expression const*, smt::Expression> m_expressions;
	std::map<Declaration const*, smt::Expression> m_variables;
	ErrorReporter& m_errorReporter;

	FunctionDefinition const* m_currentFunction = nullptr;
};

}
}
