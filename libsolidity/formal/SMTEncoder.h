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
 * Encodes Solidity into SMT expressions without creating
 * any verification targets.
 * Also implements the SSA scheme for branches.
 */

#pragma once


#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SymbolicVariables.h>
#include <libsolidity/formal/VariableUsage.h>

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/ReadFile.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace dev
{
namespace solidity
{

class SMTEncoder: public ASTConstVisitor
{
public:
	SMTEncoder(smt::EncodingContext& _context);

	/// @returns the leftmost identifier in a multi-d IndexAccess.
	static Expression const* leftmostBase(IndexAccess const& _indexAccess);

	/// @returns the FunctionDefinition of a FunctionCall
	/// if possible or nullptr.
	static FunctionDefinition const* functionCallToDefinition(FunctionCall const& _funCall);

protected:
	// TODO: Check that we do not have concurrent reads and writes to a variable,
	// because the order of expression evaluation is undefined
	// TODO: or just force a certain order, but people might have a different idea about that.

	bool visit(ContractDefinition const& _node) override;
	void endVisit(ContractDefinition const& _node) override;
	void endVisit(VariableDeclaration const& _node) override;
	bool visit(ModifierDefinition const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visit(PlaceholderStatement const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(WhileStatement const&) override { return false; }
	bool visit(ForStatement const&) override { return false; }
	void endVisit(VariableDeclarationStatement const& _node) override;
	void endVisit(Assignment const& _node) override;
	void endVisit(TupleExpression const& _node) override;
	bool visit(UnaryOperation const& _node) override;
	void endVisit(UnaryOperation const& _node) override;
	bool visit(BinaryOperation const& _node) override;
	void endVisit(BinaryOperation const& _node) override;
	void endVisit(FunctionCall const& _node) override;
	void endVisit(Identifier const& _node) override;
	void endVisit(Literal const& _node) override;
	void endVisit(Return const& _node) override;
	bool visit(MemberAccess const& _node) override;
	void endVisit(IndexAccess const& _node) override;
	bool visit(InlineAssembly const& _node) override;

	/// Do not visit subtree if node is a RationalNumber.
	/// Symbolic _expr is the rational literal.
	bool shortcutRationalNumber(Expression const& _expr);
	void arithmeticOperation(BinaryOperation const& _op);
	/// @returns _op(_left, _right) with and without modular arithmetic.
	/// Used by the function above, compound assignments and
	/// unary increment/decrement.
	virtual std::pair<smt::Expression, smt::Expression> arithmeticOperation(
		Token _op,
		smt::Expression const& _left,
		smt::Expression const& _right,
		TypePointer const& _commonType,
		Expression const& _expression
	);
	void compareOperation(BinaryOperation const& _op);
	void booleanOperation(BinaryOperation const& _op);

	void initFunction(FunctionDefinition const& _function);
	void visitAssert(FunctionCall const& _funCall);
	void visitRequire(FunctionCall const& _funCall);
	void visitGasLeft(FunctionCall const& _funCall);
	void visitTypeConversion(FunctionCall const& _funCall);
	void visitFunctionIdentifier(Identifier const& _identifier);

	/// Encodes a modifier or function body according to the modifier
	/// visit depth.
	void visitFunctionOrModifier();

	/// Defines a new global variable or function.
	void defineGlobalVariable(std::string const& _name, Expression const& _expr, bool _increaseIndex = false);

	/// Handles the side effects of assignment
	/// to variable of some SMT array type
	/// while aliasing is not supported.
	void arrayAssignment();
	/// Handles assignment to SMT array index.
	void arrayIndexAssignment(Expression const& _expr, smt::Expression const& _rightHandSide);

	/// Division expression in the given type. Requires special treatment because
	/// of rounding for signed division.
	smt::Expression division(smt::Expression _left, smt::Expression _right, IntegerType const& _type);

	void assignment(VariableDeclaration const& _variable, Expression const& _value);
	/// Handles assignments to variables of different types.
	void assignment(VariableDeclaration const& _variable, smt::Expression const& _value);
	/// Handles assignments between generic expressions.
	/// Will also be used for assignments of tuple components.
	void assignment(
		Expression const& _left,
		std::vector<smt::Expression> const& _right,
		TypePointer const& _type,
		langutil::SourceLocation const& _location
	);
	/// Computes the right hand side of a compound assignment.
	smt::Expression compoundAssignment(Assignment const& _assignment);

	/// Maps a variable to an SSA index.
	using VariableIndices = std::unordered_map<VariableDeclaration const*, int>;

	/// Visits the branch given by the statement, pushes and pops the current path conditions.
	/// @param _condition if present, asserts that this condition is true within the branch.
	/// @returns the variable indices after visiting the branch.
	VariableIndices visitBranch(ASTNode const* _statement, smt::Expression const* _condition = nullptr);
	VariableIndices visitBranch(ASTNode const* _statement, smt::Expression _condition);

	using CallStackEntry = std::pair<CallableDeclaration const*, ASTNode const*>;

	void initializeStateVariables(ContractDefinition const& _contract);
	void initializeLocalVariables(FunctionDefinition const& _function);
	void initializeFunctionCallParameters(CallableDeclaration const& _function, std::vector<smt::Expression> const& _callArgs);
	void resetStateVariables();
	/// @returns the type without storage pointer information if it has it.
	TypePointer typeWithoutPointer(TypePointer const& _type);

	/// Given two different branches and the touched variables,
	/// merge the touched variables into after-branch ite variables
	/// using the branch condition as guard.
	void mergeVariables(std::set<VariableDeclaration const*> const& _variables, smt::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse);
	/// Tries to create an uninitialized variable and returns true on success.
	bool createVariable(VariableDeclaration const& _varDecl);

	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the current point.
	smt::Expression currentValue(VariableDeclaration const& _decl);
	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the given index. Does not ensure that this index exists.
	smt::Expression valueAtIndex(VariableDeclaration const& _decl, int _index);
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
	/// @returns a human-readable call stack. Used for models.
	langutil::SecondarySourceLocation callStackMessage(std::vector<CallStackEntry> const& _callStack);
	/// Copies and pops the last called node.
	CallStackEntry popCallStack();
	/// Adds (_definition, _node) to the callstack.
	void pushCallStack(CallStackEntry _entry);
	/// Add to the solver: the given expression implied by the current path conditions
	void addPathImpliedExpression(smt::Expression const& _e);

	/// Copy the SSA indices of m_variables.
	VariableIndices copyVariableIndices();
	/// Resets the variable indices.
	void resetVariableIndices(VariableIndices const& _indices);

	/// @returns variables that are touched in _node's subtree.
	std::set<VariableDeclaration const*> touchedVariables(ASTNode const& _node);

	/// @returns the VariableDeclaration referenced by an Identifier or nullptr.
	VariableDeclaration const* identifierToVariable(Expression const& _expr);

	/// Creates symbolic expressions for the returned values
	/// and set them as the components of the symbolic tuple.
	void createReturnedExpressions(FunctionCall const& _funCall);

	/// @returns a note to be added to warnings.
	std::string extraComment();

	smt::VariableUsage m_variableUsage;
	bool m_arrayAssignmentHappened = false;
	// True if the "No SMT solver available" warning was already created.
	bool m_noSolverWarning = false;

	/// Stores the instances of an Uninterpreted Function applied to arguments.
	/// These may be direct application of UFs or Array index access.
	/// Used to retrieve models.
	std::set<Expression const*> m_uninterpretedTerms;
	std::vector<smt::Expression> m_pathConditions;
	/// Local SMTEncoder ErrorReporter.
	/// This is necessary to show the "No SMT solver available"
	/// warning before the others in case it's needed.
	langutil::ErrorReporter m_errorReporter;
	langutil::ErrorList m_smtErrors;
	std::shared_ptr<langutil::Scanner> m_scanner;

	/// Stores the current function/modifier call/invocation path.
	std::vector<CallStackEntry> m_callStack;
	/// Returns true if the current function was not visited by
	/// a function call.
	bool isRootFunction();
	/// Returns true if _funDef was already visited.
	bool visitedFunction(FunctionDefinition const* _funDef);

	/// Depth of visit to modifiers.
	/// When m_modifierDepth == #modifiers the function can be visited
	/// when placeholder is visited.
	/// Needs to be a stack because of function calls.
	std::vector<int> m_modifierDepthStack;

	ContractDefinition const* m_currentContract = nullptr;

	/// Stores the context of the encoding.
	smt::EncodingContext& m_context;
};

}
}
