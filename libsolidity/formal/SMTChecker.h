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


#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SolverInterface.h>
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

class SMTChecker: private ASTConstVisitor
{
public:
	SMTChecker(langutil::ErrorReporter& _errorReporter, std::map<h256, std::string> const& _smtlib2Responses);

	void analyze(SourceUnit const& _sources, std::shared_ptr<langutil::Scanner> const& _scanner);

	/// This is used if the SMT solver is not directly linked into this binary.
	/// @returns a list of inputs to the SMT solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries() { return m_interface->unhandledQueries(); }

	/// @return the FunctionDefinition of a called function if possible and should inline,
	/// otherwise nullptr.
	static FunctionDefinition const* inlinedFunctionCallToDefinition(FunctionCall const& _funCall);

private:
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
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
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
	/// @returns _op(_left, _right).
	/// Used by the function above, compound assignments and
	/// unary increment/decrement.
	smt::Expression arithmeticOperation(
		Token _op,
		smt::Expression const& _left,
		smt::Expression const& _right,
		TypePointer const& _commonType,
		langutil::SourceLocation const& _location
	);
	void compareOperation(BinaryOperation const& _op);
	void booleanOperation(BinaryOperation const& _op);

	void visitAssert(FunctionCall const& _funCall);
	void visitRequire(FunctionCall const& _funCall);
	void visitGasLeft(FunctionCall const& _funCall);
	void visitTypeConversion(FunctionCall const& _funCall);
	/// Visits the FunctionDefinition of the called function
	/// if available and inlines the return value.
	void inlineFunctionCall(FunctionCall const& _funCall);
	/// Creates an uninterpreted function call.
	void abstractFunctionCall(FunctionCall const& _funCall);
	void visitFunctionIdentifier(Identifier const& _identifier);

	/// Encodes a modifier or function body according to the modifier
	/// visit depth.
	void visitFunctionOrModifier();

	void defineGlobalVariable(std::string const& _name, Expression const& _expr, bool _increaseIndex = false);
	void defineGlobalFunction(std::string const& _name, Expression const& _expr);
	/// Handles the side effects of assignment
	/// to variable of some SMT array type
	/// while aliasing is not supported.
	void arrayAssignment();
	/// Handles assignment to SMT array index.
	void arrayIndexAssignment(Expression const& _expr, smt::Expression const& _rightHandSide);

	/// Division expression in the given type. Requires special treatment because
	/// of rounding for signed division.
	smt::Expression division(smt::Expression _left, smt::Expression _right, IntegerType const& _type);

	void assignment(VariableDeclaration const& _variable, Expression const& _value, langutil::SourceLocation const& _location);
	/// Handles assignments to variables of different types.
	void assignment(VariableDeclaration const& _variable, smt::Expression const& _value, langutil::SourceLocation const& _location);
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

	/// Check that a condition can be satisfied.
	void checkCondition(
		smt::Expression _condition,
		langutil::SourceLocation const& _location,
		std::string const& _description,
		std::string const& _additionalValueName = "",
		smt::Expression const* _additionalValue = nullptr
	);
	/// Checks that a boolean condition is not constant. Do not warn if the expression
	/// is a literal constant.
	/// @param _description the warning string, $VALUE will be replaced by the constant value.
	void checkBooleanNotConstant(
		Expression const& _condition,
		std::string const& _description
	);

	using CallStackEntry = std::pair<CallableDeclaration const*, ASTNode const*>;

	struct OverflowTarget
	{
		enum class Type { Underflow, Overflow, All } type;
		TypePointer intType;
		smt::Expression value;
		smt::Expression path;
		langutil::SourceLocation const& location;
		std::vector<CallStackEntry> callStack;

		OverflowTarget(Type _type, TypePointer _intType, smt::Expression _value, smt::Expression _path, langutil::SourceLocation const& _location, std::vector<CallStackEntry> _callStack):
			type(_type),
			intType(_intType),
			value(_value),
			path(_path),
			location(_location),
			callStack(move(_callStack))
		{
			solAssert(dynamic_cast<IntegerType const*>(intType), "");
		}
	};

	/// Checks that the value is in the range given by the type.
	void checkUnderflow(OverflowTarget& _target);
	void checkOverflow(OverflowTarget& _target);
	/// Calls the functions above for all elements in m_overflowTargets accordingly.
	void checkUnderOverflow();
	/// Adds an overflow target for lazy check at the end of the function.
	void addOverflowTarget(OverflowTarget::Type _type, TypePointer _intType, smt::Expression _value, langutil::SourceLocation const& _location);

	std::pair<smt::CheckResult, std::vector<std::string>>
	checkSatisfiableAndGenerateModel(std::vector<smt::Expression> const& _expressionsToEvaluate);

	smt::CheckResult checkSatisfiable();

	void initializeLocalVariables(FunctionDefinition const& _function);
	void initializeFunctionCallParameters(CallableDeclaration const& _function, std::vector<smt::Expression> const& _callArgs);
	void resetVariable(VariableDeclaration const& _variable);
	void resetStateVariables();
	void resetStorageReferences();
	void resetVariables(std::set<VariableDeclaration const*> const& _variables);
	void resetVariables(std::function<bool(VariableDeclaration const&)> const& _filter);
	/// @returns the type without storage pointer information if it has it.
	TypePointer typeWithoutPointer(TypePointer const& _type);

	/// Given two different branches and the touched variables,
	/// merge the touched variables into after-branch ite variables
	/// using the branch condition as guard.
	void mergeVariables(std::set<VariableDeclaration const*> const& _variables, smt::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse);
	/// Tries to create an uninitialized variable and returns true on success.
	/// This fails if the type is not supported.
	bool createVariable(VariableDeclaration const& _varDecl);

	/// @returns true if _delc is a variable that is known at the current point, i.e.
	/// has a valid index
	bool knownVariable(VariableDeclaration const& _decl);
	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the current point.
	smt::Expression currentValue(VariableDeclaration const& _decl);
	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the given index. Does not ensure that this index exists.
	smt::Expression valueAtIndex(VariableDeclaration const& _decl, int _index);
	/// Allocates a new index for the declaration, updates the current
	/// index to this value and returns the expression.
	smt::Expression newValue(VariableDeclaration const& _decl);

	/// Sets the value of the declaration to zero.
	void setZeroValue(VariableDeclaration const& _decl);
	void setZeroValue(SymbolicVariable& _variable);
	/// Resets the variable to an unknown value (in its range).
	void setUnknownValue(VariableDeclaration const& decl);
	void setUnknownValue(SymbolicVariable& _variable);

	/// Returns the expression corresponding to the AST node. Throws if the expression does not exist.
	smt::Expression expr(Expression const& _e);
	/// Creates the expression (value can be arbitrary)
	void createExpr(Expression const& _e);
	/// Checks if expression was created
	bool knownExpr(Expression const& _e) const;
	/// Creates the expression and sets its value.
	void defineExpr(Expression const& _e, smt::Expression _value);

	/// Checks if special variable or function was seen.
	bool knownGlobalSymbol(std::string const& _var) const;

	/// Adds a new path condition
	void pushPathCondition(smt::Expression const& _e);
	/// Remove the last path condition
	void popPathCondition();
	/// Returns the conjunction of all path conditions or True if empty
	smt::Expression currentPathConditions();
	/// Returns the current callstack. Used for models.
	langutil::SecondarySourceLocation currentCallStack();
	/// Copies and pops the last called node.
	CallStackEntry popCallStack();
	/// Adds (_definition, _node) to the callstack.
	void pushCallStack(CallStackEntry _entry);
	/// Conjoin the current path conditions with the given parameter and add to the solver
	void addPathConjoinedExpression(smt::Expression const& _e);
	/// Add to the solver: the given expression implied by the current path conditions
	void addPathImpliedExpression(smt::Expression const& _e);

	/// Removes local variables from the context.
	void removeLocalVariables();

	/// Copy the SSA indices of m_variables.
	VariableIndices copyVariableIndices();
	/// Resets the variable indices.
	void resetVariableIndices(VariableIndices const& _indices);

	/// @returns variables that are touched in _node's subtree.
	std::set<VariableDeclaration const*> touchedVariables(ASTNode const& _node);

	/// @returns the VariableDeclaration referenced by an Identifier or nullptr.
	VariableDeclaration const* identifierToVariable(Expression const& _expr);

	std::shared_ptr<smt::SolverInterface> m_interface;
	VariableUsage m_variableUsage;
	bool m_loopExecutionHappened = false;
	bool m_arrayAssignmentHappened = false;
	bool m_externalFunctionCallHappened = false;
	// True if the "No SMT solver available" warning was already created.
	bool m_noSolverWarning = false;
	/// An Expression may have multiple smt::Expression due to
	/// repeated calls to the same function.
	std::unordered_map<Expression const*, std::shared_ptr<SymbolicVariable>> m_expressions;
	std::unordered_map<VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> m_variables;
	std::unordered_map<std::string, std::shared_ptr<SymbolicVariable>> m_globalContext;

	/// Stores the instances of an Uninterpreted Function applied to arguments.
	/// These may be direct application of UFs or Array index access.
	/// Used to retrieve models.
	std::set<Expression const*> m_uninterpretedTerms;
	std::vector<smt::Expression> m_pathConditions;
	/// ErrorReporter that comes from CompilerStack.
	langutil::ErrorReporter& m_errorReporterReference;
	/// Local SMTChecker ErrorReporter.
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

	std::vector<OverflowTarget> m_overflowTargets;

	/// Depth of visit to modifiers.
	/// When m_modifierDepth == #modifiers the function can be visited
	/// when placeholder is visited.
	/// Needs to be a stack because of function calls.
	std::vector<int> m_modifierDepthStack;

	/// Stores the context of the encoding.
	smt::EncodingContext m_context;
};

}
}
