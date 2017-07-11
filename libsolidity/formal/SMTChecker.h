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
#include <libsolidity/formal/SMTLib2Interface.h>
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
	SMTChecker(ErrorReporter& _errorReporter, ReadFile::Callback const& _readCallback);

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
		smt::Expression _condition,
		SourceLocation const& _location,
		std::string const& _description,
		std::string const& _additionalValueName = "",
		smt::Expression* _additionalValue = nullptr
	);

	void createVariable(VariableDeclaration const& _varDecl, bool _setToZero);

	std::string uniqueSymbol(Declaration const& _decl);
	std::string uniqueSymbol(Expression const& _expr);
	bool knownVariable(Declaration const& _decl);
	smt::Expression currentValue(Declaration const& _decl);
	smt::Expression newValue(Declaration const& _decl);

	smt::Expression minValue(IntegerType const& _t);
	smt::Expression maxValue(IntegerType const& _t);

	/// Returns the expression corresponding to the AST node. Creates a new expression
	/// if it does not exist yet.
	smt::Expression expr(Expression const& _e);
	/// Returns the function declaration corresponding to the given variable.
	/// The function takes one argument which is the "sequence number".
	smt::Expression var(Declaration const& _decl);

	smt::SMTLib2Interface m_interface;
	std::map<Declaration const*, int> m_currentSequenceCounter;
	std::map<Expression const*, smt::Expression> m_z3Expressions;
	std::map<Declaration const*, smt::Expression> m_z3Variables;
	ErrorReporter& m_errorReporter;

	FunctionDefinition const* m_currentFunction = nullptr;
};

}
}
