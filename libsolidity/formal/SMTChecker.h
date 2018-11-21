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
#include <libsolidity/formal/SymbolicVariables.h>

#include <libsolidity/ast/ASTVisitor.h>

#include <libsolidity/interface/ReadFile.h>

#include <liblangutil/Scanner.h>

#include <unordered_map>
#include <string>
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

class VariableUsage;

class SMTChecker: private ASTConstVisitor
{
public:
	SMTChecker(langutil::ErrorReporter& _errorReporter, ReadCallback::Callback const& _readCallback);

	void analyze(SourceUnit const& _sources, std::shared_ptr<langutil::Scanner> const& _scanner);

private:
	// TODO: Check that we do not have concurrent reads and writes to a variable,
	// because the order of expression evaluation is undefined
	// TODO: or just force a certain order, but people might have a different idea about that.

	bool visit(ContractDefinition const& _node) override;
	void endVisit(ContractDefinition const& _node) override;
	void endVisit(VariableDeclaration const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
	void endVisit(VariableDeclarationStatement const& _node) override;
	void endVisit(Assignment const& _node) override;
	void endVisit(TupleExpression const& _node) override;
	void endVisit(UnaryOperation const& _node) override;
	void endVisit(BinaryOperation const& _node) override;
	void endVisit(FunctionCall const& _node) override;
	void endVisit(Identifier const& _node) override;
	void endVisit(Literal const& _node) override;
	void endVisit(Return const& _node) override;
	bool visit(MemberAccess const& _node) override;

	void arithmeticOperation(BinaryOperation const& _op);
	void compareOperation(BinaryOperation const& _op);
	void booleanOperation(BinaryOperation const& _op);

	void visitAssert(FunctionCall const&);
	void visitRequire(FunctionCall const&);
	void visitGasLeft(FunctionCall const&);
	void visitBlockHash(FunctionCall const&);
	/// Visits the FunctionDefinition of the called function
	/// if available and inlines the return value.
	void inlineFunctionCall(FunctionCall const&);

	void defineSpecialVariable(std::string const& _name, Expression const& _expr, bool _increaseIndex = false);
	void defineUninterpretedFunction(std::string const& _name, smt::SortPointer _sort);

	/// Division expression in the given type. Requires special treatment because
	/// of rounding for signed division.
	smt::Expression division(smt::Expression _left, smt::Expression _right, IntegerType const& _type);

	void assignment(VariableDeclaration const& _variable, Expression const& _value, langutil::SourceLocation const& _location);
	void assignment(VariableDeclaration const& _variable, smt::Expression const& _value, langutil::SourceLocation const& _location);

	/// Maps a variable to an SSA index.
	using VariableIndices = std::unordered_map<VariableDeclaration const*, int>;

	/// Visits the branch given by the statement, pushes and pops the current path conditions.
	/// @param _condition if present, asserts that this condition is true within the branch.
	/// @returns the variable indices after visiting the branch.
	VariableIndices visitBranch(Statement const& _statement, smt::Expression const* _condition = nullptr);
	VariableIndices visitBranch(Statement const& _statement, smt::Expression _condition);

	/// Check that a condition can be satisfied.
	void checkCondition(
		smt::Expression _condition,
		langutil::SourceLocation const& _location,
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
	void checkUnderOverflow(smt::Expression _value, IntegerType const& _Type, langutil::SourceLocation const& _location);


	std::pair<smt::CheckResult, std::vector<std::string>>
	checkSatisfiableAndGenerateModel(std::vector<smt::Expression> const& _expressionsToEvaluate);

	smt::CheckResult checkSatisfiable();

	void initializeLocalVariables(FunctionDefinition const& _function);
	void initializeFunctionCallParameters(FunctionDefinition const& _function, std::vector<smt::Expression> const& _callArgs);
	void resetStateVariables();
	void resetVariables(std::vector<VariableDeclaration const*> _variables);
	/// Given two different branches and the touched variables,
	/// merge the touched variables into after-branch ite variables
	/// using the branch condition as guard.
	void mergeVariables(std::vector<VariableDeclaration const*> const& _variables, smt::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse);
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
	/// Resets the variable to an unknown value (in its range).
	void setUnknownValue(VariableDeclaration const& decl);

	/// Returns the expression corresponding to the AST node. Throws if the expression does not exist.
	smt::Expression expr(Expression const& _e);
	/// Creates the expression (value can be arbitrary)
	void createExpr(Expression const& _e);
	/// Checks if expression was created
	bool knownExpr(Expression const& _e) const;
	/// Creates the expression and sets its value.
	void defineExpr(Expression const& _e, smt::Expression _value);

	/// Checks if special variable was seen.
	bool knownSpecialVariable(std::string const& _var) const;

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

	/// Removes local variables from the context.
	void removeLocalVariables();

	/// Copy the SSA indices of m_variables.
	VariableIndices copyVariableIndices();
	/// Resets the variable indices.
	void resetVariableIndices(VariableIndices const& _indices);

	std::shared_ptr<smt::SolverInterface> m_interface;
	std::shared_ptr<VariableUsage> m_variableUsage;
	bool m_loopExecutionHappened = false;
	/// An Expression may have multiple smt::Expression due to
	/// repeated calls to the same function.
	std::unordered_map<Expression const*, std::shared_ptr<SymbolicVariable>> m_expressions;
	std::unordered_map<VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> m_variables;
	std::unordered_map<std::string, std::shared_ptr<SymbolicVariable>> m_specialVariables;
	/// Stores the declaration of an Uninterpreted Function.
	std::unordered_map<std::string, smt::Expression> m_uninterpretedFunctions;
	/// Stores the instances of an Uninterpreted Function applied to arguments.
	/// Used to retrieve models.
	std::vector<Expression const*> m_uninterpretedTerms;
	std::vector<smt::Expression> m_pathConditions;
	langutil::ErrorReporter& m_errorReporter;
	std::shared_ptr<langutil::Scanner> m_scanner;

	/// Stores the current path of function calls.
	std::vector<FunctionDefinition const*> m_functionPath;
	/// Returns true if the current function was not visited by
	/// a function call.
	bool isRootFunction();
	/// Returns true if _funDef was already visited.
	bool visitedFunction(FunctionDefinition const* _funDef);
};

}
}
