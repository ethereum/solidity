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


#include <libsolidity/formal/SolverInterface.h>

#include <libsolidity/formal/SSAVariable.h>

#include <libsolidity/ast/ASTVisitor.h>

#include <libsolidity/interface/ReadFile.h>

#include <map>
#include <string>
#include <vector>

namespace dev
{
namespace solidity
{

class VariableUsage;
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
	virtual bool visit(ForStatement const& _node) override;
	virtual void endVisit(VariableDeclarationStatement const& _node) override;
	virtual void endVisit(ExpressionStatement const& _node) override;
	virtual void endVisit(Assignment const& _node) override;
	virtual void endVisit(TupleExpression const& _node) override;
	virtual void endVisit(UnaryOperation const& _node) override;
	virtual void endVisit(BinaryOperation const& _node) override;
	virtual void endVisit(FunctionCall const& _node) override;
	virtual void endVisit(Identifier const& _node) override;
	virtual void endVisit(Literal const& _node) override;

	void arithmeticOperation(BinaryOperation const& _op);
	void compareOperation(BinaryOperation const& _op);
	void booleanOperation(BinaryOperation const& _op);

	/// Division expression in the given type. Requires special treatment because
	/// of rounding for signed division.
	smt::Expression division(smt::Expression _left, smt::Expression _right, IntegerType const& _type);

	void assignment(Declaration const& _variable, Expression const& _value, SourceLocation const& _location);
	void assignment(Declaration const& _variable, smt::Expression const& _value, SourceLocation const& _location);

	/// Maps a variable to an SSA index.
	using VariableSequenceCounters = std::map<Declaration const*, SSAVariable>;

	/// Visits the branch given by the statement, pushes and pops the current path conditions.
	/// @param _condition if present, asserts that this condition is true within the branch.
	/// @returns the variable sequence counter after visiting the branch.
	VariableSequenceCounters visitBranch(Statement const& _statement, smt::Expression const* _condition = nullptr);
	VariableSequenceCounters visitBranch(Statement const& _statement, smt::Expression _condition);

	/// Check that a condition can be satisfied.
	void checkCondition(
		smt::Expression _condition,
		SourceLocation const& _location,
		std::string const& _description,
		std::string const& _additionalValueName = "",
		smt::Expression* _additionalValue = nullptr
	);
	/// Checks that a boolean condition is not constant. Do not warn if the expression
	/// is a literal constant.
	/// @param _description the warning string, $VALUE will be replaced by the constant value.
	void checkBooleanNotConstant(
		Expression const& _condition,
		std::string const& _description
	);
	/// Checks that the value is in the range given by the type.
	void checkUnderOverflow(smt::Expression _value, IntegerType const& _Type, SourceLocation const& _location);


	std::pair<smt::CheckResult, std::vector<std::string>>
	checkSatisfiableAndGenerateModel(std::vector<smt::Expression> const& _expressionsToEvaluate);

	smt::CheckResult checkSatisfiable();

	void initializeLocalVariables(FunctionDefinition const& _function);
	void resetStateVariables();
	void resetVariables(std::vector<Declaration const*> _variables);
	/// Given two different branches and the touched variables,
	/// merge the touched variables into after-branch ite variables
	/// using the branch condition as guard.
	void mergeVariables(std::vector<Declaration const*> const& _variables, smt::Expression const& _condition, VariableSequenceCounters const& _countersEndTrue, VariableSequenceCounters const& _countersEndFalse);
	/// Tries to create an uninitialized variable and returns true on success.
	/// This fails if the type is not supported.
	bool createVariable(VariableDeclaration const& _varDecl);

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

	/// Sets the value of the declaration to zero.
	void setZeroValue(Declaration const& _decl);
	/// Resets the variable to an unknown value (in its range).
	void setUnknownValue(Declaration const& decl);

	/// Returns the expression corresponding to the AST node. Throws if the expression does not exist.
	smt::Expression expr(Expression const& _e);
	/// Creates the expression (value can be arbitrary)
	void createExpr(Expression const& _e);
	/// Creates the expression and sets its value.
	void defineExpr(Expression const& _e, smt::Expression _value);

	/// Adds a new path condition
	void pushPathCondition(smt::Expression const& _e);
	/// Remove the last path condition
	void popPathCondition();
	/// Returns the conjunction of all path conditions or True if empty
	smt::Expression currentPathConditions();
	/// Conjoin the current path conditions with the given parameter and add to the solver
	void addPathConjoinedExpression(smt::Expression const& _e);
	/// Add to the solver: the given expression implied by the current path conditions
	void addPathImpliedExpression(smt::Expression const& _e);

	std::shared_ptr<smt::SolverInterface> m_interface;
	std::shared_ptr<VariableUsage> m_variableUsage;
	bool m_loopExecutionHappened = false;
	std::map<Expression const*, smt::Expression> m_expressions;
	std::map<Declaration const*, SSAVariable> m_variables;
	std::map<Declaration const*, SSAVariable> m_stateVariables;
	std::vector<smt::Expression> m_pathConditions;
	ErrorReporter& m_errorReporter;

	FunctionDefinition const* m_currentFunction = nullptr;
};

}
}
