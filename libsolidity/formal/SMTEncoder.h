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
// SPDX-License-Identifier: GPL-3.0
/**
 * Encodes Solidity into SMT expressions without creating
 * any verification targets.
 * Also implements the SSA scheme for branches.
 */

#pragma once


#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/ModelCheckerSettings.h>
#include <libsolidity/formal/SymbolicVariables.h>
#include <libsolidity/formal/VariableUsage.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/ReadFile.h>
#include <liblangutil/UniqueErrorReporter.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
class CharStreamProvider;
}

namespace solidity::frontend
{

class SMTEncoder: public ASTConstVisitor
{
public:
	SMTEncoder(
		smt::EncodingContext& _context,
		ModelCheckerSettings _settings,
		langutil::UniqueErrorReporter& _errorReporter,
		langutil::UniqueErrorReporter& _unsupportedErrorReporter,
		langutil::CharStreamProvider const& _charStreamProvider
	);

	/// @returns the leftmost identifier in a multi-d IndexAccess.
	static Expression const* leftmostBase(IndexAccess const& _indexAccess);

	/// @returns the key type in _type.
	/// _type must allow IndexAccess, that is,
	/// it must be either ArrayType or MappingType
	static Type const* keyType(Type const* _type);

	/// @returns the innermost element in a chain of 1-tuples if applicable,
	/// otherwise _expr.
	static Expression const* innermostTuple(Expression const& _expr);

	/// @returns the underlying type if _type is UserDefinedValueType,
	/// and _type otherwise.
	static Type const* underlyingType(Type const* _type);

	static TypePointers replaceUserTypes(TypePointers const& _types);

	/// @returns {_funCall.expression(), nullptr} if function call option values are not given, and
	/// {_funCall.expression().expression(), _funCall.expression()} if they are.
	static std::pair<Expression const*, FunctionCallOptions const*> functionCallExpression(FunctionCall const& _funCall);

	/// @returns the expression after stripping redundant syntactic sugar.
	/// Currently supports stripping:
	/// 1. 1-tuple; i.e. ((x)) -> x
	/// 2. Explicit cast from string to bytes; i.e. bytes(s) -> s; for s of type string
	static Expression const* cleanExpression(Expression const& _expr);

	/// @returns the FunctionDefinition of a FunctionCall
	/// if possible or nullptr.
	/// @param _scopeContract is the contract that contains the function currently being
	///        analyzed, if applicable.
	/// @param _contextContract is the most derived contract currently being analyzed.
	/// The difference between the two parameters appears in the case of inheritance.
	/// Let A and B be two contracts so that B derives from A, and A defines a function `f`
	/// that `B` does not override. Function `f` is visited twice:
	/// - Once when A is the most derived contract, where both _scopeContract and _contextContract are A.
	/// - Once when B is the most derived contract, where _scopeContract is A and _contextContract is B.
	static FunctionDefinition const* functionCallToDefinition(
		FunctionCall const& _funCall,
		ContractDefinition const* _scopeContract,
		ContractDefinition const* _contextContract
	);

	static std::vector<VariableDeclaration const*> stateVariablesIncludingInheritedAndPrivate(ContractDefinition const& _contract);
	static std::vector<VariableDeclaration const*> stateVariablesIncludingInheritedAndPrivate(FunctionDefinition const& _function);

	static std::vector<VariableDeclaration const*> localVariablesIncludingModifiers(FunctionDefinition const& _function, ContractDefinition const* _contract);
	static std::vector<VariableDeclaration const*> modifiersVariables(FunctionDefinition const& _function, ContractDefinition const* _contract);
	static std::vector<VariableDeclaration const*> tryCatchVariables(FunctionDefinition const& _function);

	/// @returns the ModifierDefinition of a ModifierInvocation if possible, or nullptr.
	static ModifierDefinition const* resolveModifierInvocation(ModifierInvocation const& _invocation, ContractDefinition const* _contract);

	/// @returns the arguments for each base constructor call in the hierarchy of @a _contract.
	std::map<ContractDefinition const*, std::vector<ASTPointer<frontend::Expression>>> baseArguments(ContractDefinition const& _contract);

	/// @returns a valid RationalNumberType pointer if _expr has type
	/// RationalNumberType or can be const evaluated, and nullptr otherwise.
	static RationalNumberType const* isConstant(Expression const& _expr);

	static std::set<FunctionCall const*, ASTCompareByID<FunctionCall>> collectABICalls(ASTNode const* _node);

	/// @returns all the sources that @param _source depends on,
	/// including itself.
	static std::set<SourceUnit const*, ASTNode::CompareByID> sourceDependencies(SourceUnit const& _source);

protected:
	void resetSourceAnalysis();

	// TODO: Check that we do not have concurrent reads and writes to a variable,
	// because the order of expression evaluation is undefined
	// TODO: or just force a certain order, but people might have a different idea about that.

	bool visit(ImportDirective const& _node) override;
	bool visit(ContractDefinition const& _node) override;
	void endVisit(ContractDefinition const& _node) override;
	void endVisit(VariableDeclaration const& _node) override;
	bool visit(ModifierDefinition const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visit(Block const& _node) override;
	void endVisit(Block const& _node) override;
	bool visit(PlaceholderStatement const& _node) override;
	bool visit(IfStatement const&) override { return false; }
	bool visit(WhileStatement const&) override { return false; }
	bool visit(ForStatement const&) override { return false; }
	void endVisit(ForStatement const&) override {}
	void endVisit(VariableDeclarationStatement const& _node) override;
	bool visit(Assignment const& _node) override;
	void endVisit(Assignment const& _node) override;
	void endVisit(TupleExpression const& _node) override;
	bool visit(UnaryOperation const& _node) override;
	void endVisit(UnaryOperation const& _node) override;
	bool visit(BinaryOperation const& _node) override;
	void endVisit(BinaryOperation const& _node) override;
	bool visit(Conditional const& _node) override;
	bool visit(FunctionCall const& _node) override;
	void endVisit(FunctionCall const& _node) override;
	bool visit(ModifierInvocation const& _node) override;
	void endVisit(Identifier const& _node) override;
	void endVisit(ElementaryTypeNameExpression const& _node) override;
	void endVisit(Literal const& _node) override;
	void endVisit(Return const& _node) override;
	bool visit(MemberAccess const& _node) override;
	void endVisit(IndexAccess const& _node) override;
	void endVisit(IndexRangeAccess const& _node) override;
	bool visit(InlineAssembly const& _node) override;
	bool visit(Break const&) override { return false; }
	void endVisit(Break const&) override {}
	bool visit(Continue const&) override { return false; }
	void endVisit(Continue const&) override {}
	bool visit(TryCatchClause const&) override { return true; }
	void endVisit(TryCatchClause const&) override {}
	bool visit(TryStatement const&) override { return false; }

	virtual void pushInlineFrame(CallableDeclaration const&);
	virtual void popInlineFrame(CallableDeclaration const&);

	/// Do not visit subtree if node is a RationalNumber.
	/// Symbolic _expr is the rational literal.
	bool shortcutRationalNumber(Expression const& _expr);
	void arithmeticOperation(BinaryOperation const& _op);
	/// @returns _op(_left, _right) with and without modular arithmetic.
	/// Used by the function above, compound assignments and
	/// unary increment/decrement.
	virtual std::pair<smtutil::Expression, smtutil::Expression> arithmeticOperation(
		Token _op,
		smtutil::Expression const& _left,
		smtutil::Expression const& _right,
		Type const* _commonType,
		Expression const& _expression
	);

	smtutil::Expression bitwiseOperation(
		Token _op,
		smtutil::Expression const& _left,
		smtutil::Expression const& _right,
		Type const* _commonType
	);

	void compareOperation(BinaryOperation const& _op);
	void booleanOperation(BinaryOperation const& _op);
	void bitwiseOperation(BinaryOperation const& _op);
	void bitwiseNotOperation(UnaryOperation const& _op);

	void initContract(ContractDefinition const& _contract);
	void initFunction(FunctionDefinition const& _function);
	void visitAssert(FunctionCall const& _funCall);
	void visitRequire(FunctionCall const& _funCall);
	void visitABIFunction(FunctionCall const& _funCall);
	void visitCryptoFunction(FunctionCall const& _funCall);
	void visitGasLeft(FunctionCall const& _funCall);
	virtual void visitAddMulMod(FunctionCall const& _funCall);
	void visitWrapUnwrap(FunctionCall const& _funCall);
	void visitObjectCreation(FunctionCall const& _funCall);
	void visitTypeConversion(FunctionCall const& _funCall);
	void visitStructConstructorCall(FunctionCall const& _funCall);
	void visitFunctionIdentifier(Identifier const& _identifier);
	virtual void visitPublicGetter(FunctionCall const& _funCall);

	/// @returns true if @param _contract is set for analysis in the settings
	/// and it is not abstract.
	bool shouldAnalyze(ContractDefinition const& _contract) const;
	/// @returns true if @param _source is set for analysis in the settings.
	bool shouldAnalyze(SourceUnit const& _source) const;

	/// @returns the state variable returned by a public getter if
	/// @a _expr is a call to a public getter,
	/// otherwise nullptr.
	VariableDeclaration const* publicGetter(Expression const& _expr) const;

	smtutil::Expression contractAddressValue(FunctionCall const& _f);

	/// Encodes a modifier or function body according to the modifier
	/// visit depth.
	void visitFunctionOrModifier();

	/// Inlines a modifier or base constructor call.
	void inlineModifierInvocation(ModifierInvocation const* _invocation, CallableDeclaration const* _definition);

	/// Inlines the constructor hierarchy into a single constructor.
	void inlineConstructorHierarchy(ContractDefinition const& _contract);

	/// Defines a new global variable or function.
	void defineGlobalVariable(std::string const& _name, Expression const& _expr, bool _increaseIndex = false);

	/// Handles the side effects of assignment
	/// to variable of some SMT array type
	/// while aliasing is not supported.
	void arrayAssignment();
	/// Handles assignments to index or member access.
	void indexOrMemberAssignment(Expression const& _expr, smtutil::Expression const& _rightHandSide);

	void arrayPush(FunctionCall const& _funCall);
	void arrayPop(FunctionCall const& _funCall);
	/// Allows BMC and CHC to create verification targets for popping
	/// an empty array.
	virtual void makeArrayPopVerificationTarget(FunctionCall const&) {}
	/// Allows BMC and CHC to create verification targets for out of bounds access.
	virtual void makeOutOfBoundsVerificationTarget(IndexAccess const&) {}

	void addArrayLiteralAssertions(
		smt::SymbolicArrayVariable& _symArray,
		std::vector<smtutil::Expression> const& _elementValues
	);

	void bytesToFixedBytesAssertions(
		smt::SymbolicArrayVariable& _symArray,
		Expression const& _fixedBytes
	);

	/// @returns a pair of expressions representing _left / _right and _left mod _right, respectively.
	/// Uses slack variables and additional constraints to express the results using only operations
	/// more friendly to the SMT solver (multiplication, addition, subtraction and comparison).
	std::pair<smtutil::Expression, smtutil::Expression>	divModWithSlacks(
		smtutil::Expression _left,
		smtutil::Expression _right,
		IntegerType const& _type
	);

	/// Handles the actual assertion of the new value to the encoding context.
	/// Other assignment methods should use this one in the end.
	virtual void assignment(smt::SymbolicVariable& _symVar, smtutil::Expression const& _value);

	void assignment(VariableDeclaration const& _variable, Expression const& _value);
	/// Handles assignments to variables of different types.
	void assignment(VariableDeclaration const& _variable, smtutil::Expression const& _value);
	/// Handles assignments between generic expressions.
	/// Will also be used for assignments of tuple components.
	void assignment(Expression const& _left, smtutil::Expression const& _right);
	void assignment(
		Expression const& _left,
		smtutil::Expression const& _right,
		Type const* _type
	);
	/// Handle assignments between tuples.
	void tupleAssignment(Expression const& _left, Expression const& _right);
	/// Computes the right hand side of a compound assignment.
	smtutil::Expression compoundAssignment(Assignment const& _assignment);
	/// Handles assignment of an expression to a tuple of variables.
	void expressionToTupleAssignment(std::vector<std::shared_ptr<VariableDeclaration>> const& _variables, Expression const& _rhs);

	/// Maps a variable to an SSA index.
	using VariableIndices = std::unordered_map<VariableDeclaration const*, unsigned>;

	/// Visits the branch given by the statement, pushes and pops the current path conditions.
	/// @param _condition if present, asserts that this condition is true within the branch.
	/// @returns the variable indices after visiting the branch and the expression representing
	/// the path condition at the end of the branch.
	std::pair<VariableIndices, smtutil::Expression> visitBranch(ASTNode const* _statement, smtutil::Expression const* _condition = nullptr);
	std::pair<VariableIndices, smtutil::Expression> visitBranch(ASTNode const* _statement, smtutil::Expression _condition);

	using CallStackEntry = std::pair<CallableDeclaration const*, ASTNode const*>;

	void createStateVariables(ContractDefinition const& _contract);
	void initializeStateVariables(ContractDefinition const& _contract);
	void createLocalVariables(FunctionDefinition const& _function);
	void initializeLocalVariables(FunctionDefinition const& _function);
	void initializeFunctionCallParameters(CallableDeclaration const& _function, std::vector<smtutil::Expression> const& _callArgs);
	void resetStateVariables();
	void resetStorageVariables();
	void resetMemoryVariables();
	void resetBalances();
	/// Resets all references/pointers that have the same type or have
	/// a subexpression of the same type as _varDecl.
	void resetReferences(VariableDeclaration const& _varDecl);
	/// Resets all references/pointers that have type _type.
	void resetReferences(Type const* _type);
	/// @returns the type without storage pointer information if it has it.
	Type const* typeWithoutPointer(Type const* _type);
	/// @returns whether _a or a subtype of _a is the same as _b.
	bool sameTypeOrSubtype(Type const* _a, Type const* _b);

	bool isSupportedType(Type const& _type) const;

	/// Given the state of the symbolic variables at the end of two different branches,
	/// create a merged state using the given branch condition.
	void mergeVariables(smtutil::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse);
	/// Tries to create an uninitialized variable and returns true on success.
	bool createVariable(VariableDeclaration const& _varDecl);

	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the current point.
	smtutil::Expression currentValue(VariableDeclaration const& _decl) const;
	/// @returns an expression denoting the value of the variable declared in @a _decl
	/// at the given index. Does not ensure that this index exists.
	smtutil::Expression valueAtIndex(VariableDeclaration const& _decl, unsigned _index) const;
	/// Returns the expression corresponding to the AST node.
	/// If _targetType is not null apply conversion.
	/// Throws if the expression does not exist.
	smtutil::Expression expr(Expression const& _e, Type const* _targetType = nullptr);
	/// Creates the expression (value can be arbitrary)
	void createExpr(Expression const& _e);
	/// Creates the expression and sets its value.
	void defineExpr(Expression const& _e, smtutil::Expression _value);
	/// Creates the tuple expression and sets its value.
	void defineExpr(Expression const& _e, std::vector<std::optional<smtutil::Expression>> const& _values);
	/// Overwrites the current path condition
	void setPathCondition(smtutil::Expression const& _e);
	/// Adds a new path condition
	void pushPathCondition(smtutil::Expression const& _e);
	/// Remove the last path condition
	void popPathCondition();
	/// Returns the conjunction of all path conditions or True if empty
	smtutil::Expression currentPathConditions();
	/// @returns a human-readable call stack. Used for models.
	langutil::SecondarySourceLocation callStackMessage(std::vector<CallStackEntry> const& _callStack);
	/// Copies and pops the last called node.
	CallStackEntry popCallStack();
	/// Adds (_definition, _node) to the callstack.
	void pushCallStack(CallStackEntry _entry);
	/// Add to the solver: the given expression implied by the current path conditions
	void addPathImpliedExpression(smtutil::Expression const& _e);

	/// Copy the SSA indices of m_variables.
	VariableIndices copyVariableIndices();
	/// Resets the variable indices.
	void resetVariableIndices(VariableIndices const& _indices);
	/// Used when starting a new block.
	virtual void clearIndices(ContractDefinition const* _contract, FunctionDefinition const* _function = nullptr);


	/// @returns variables that are touched in _node's subtree.
	std::set<VariableDeclaration const*> touchedVariables(ASTNode const& _node);

	/// @returns the declaration referenced by _expr, if any,
	/// and nullptr otherwise.
	Declaration const* expressionToDeclaration(Expression const& _expr) const;

	/// @returns the VariableDeclaration referenced by an Expression or nullptr.
	VariableDeclaration const* identifierToVariable(Expression const& _expr) const;

	/// @returns the MemberAccess <expression>.push if _expr is an empty array push call,
	/// otherwise nullptr.
	MemberAccess const* isEmptyPush(Expression const& _expr) const;

	/// @returns true if the given expression is `this`.
	/// This means we don't have to abstract away effects of external function calls to this contract.
	static bool isExternalCallToThis(Expression const* _expr);

	/// Creates symbolic expressions for the returned values
	/// and set them as the components of the symbolic tuple.
	void createReturnedExpressions(FunctionDefinition const* _funDef, Expression const& _calledExpr);

	/// @returns the symbolic arguments for a function call,
	/// taking into account attached functions and
	/// type conversion.
	std::vector<smtutil::Expression> symbolicArguments(
		std::vector<ASTPointer<VariableDeclaration>> const& _funParameters,
		std::vector<Expression const*> const& _arguments,
		std::optional<Expression const*> _calledExpr
	);

	smtutil::Expression constantExpr(Expression const& _expr, VariableDeclaration const& _var);

	/// Traverses all source units available collecting free functions
	/// and internal library functions in m_freeFunctions.
	void collectFreeFunctions(std::set<SourceUnit const*, ASTNode::CompareByID> const& _sources);
	std::set<FunctionDefinition const*, ASTNode::CompareByID> const& allFreeFunctions() const { return m_freeFunctions; }
	/// Create symbolic variables for the free constants in all @param _sources.
	void createFreeConstants(std::set<SourceUnit const*, ASTNode::CompareByID> const& _sources);

	/// @returns a note to be added to warnings.
	std::string extraComment();

	struct VerificationTarget
	{
		VerificationTargetType type;
		smtutil::Expression value;
		smtutil::Expression constraints;
	};

	smt::VariableUsage m_variableUsage;
	bool m_arrayAssignmentHappened = false;

	/// Stores the instances of an Uninterpreted Function applied to arguments.
	/// These may be direct application of UFs or Array index access.
	/// Used to retrieve models.
	std::set<Expression const*> m_uninterpretedTerms;
	std::vector<smtutil::Expression> m_pathConditions;

	/// Whether the currently visited block uses checked
	/// or unchecked arithmetic.
	bool m_checked = true;

	langutil::UniqueErrorReporter& m_errorReporter;
	langutil::UniqueErrorReporter& m_unsupportedErrors;

	/// Stores the current function/modifier call/invocation path.
	std::vector<CallStackEntry> m_callStack;

	/// Stack of scopes.
	std::vector<ScopeOpener const*> m_scopes;

	/// Returns true if the current function was not visited by
	/// a function call.
	bool isRootFunction();
	/// Returns true if _funDef was already visited.
	bool visitedFunction(FunctionDefinition const* _funDef);
	/// @returns the contract that contains the current FunctionDefinition that is being visited,
	/// or nullptr if the analysis is not inside a FunctionDefinition.
	ContractDefinition const* currentScopeContract();

	/// @returns FunctionDefinitions of the given contract (including its constructor and inherited methods),
	/// taking into account overriding of the virtual functions.
	std::set<FunctionDefinition const*, ASTNode::CompareByID> const& contractFunctions(ContractDefinition const& _contract);
	/// Cache for the method contractFunctions.
	std::map<ContractDefinition const*, std::set<FunctionDefinition const*, ASTNode::CompareByID>> m_contractFunctions;

	/// @returns FunctionDefinitions of the given contract (including its constructor and methods of bases),
	/// without taking into account overriding of the virtual functions.
	std::set<FunctionDefinition const*, ASTNode::CompareByID> const& contractFunctionsWithoutVirtual(ContractDefinition const& _contract);
	/// Cache for the method contractFunctionsWithoutVirtual.
	std::map<ContractDefinition const*, std::set<FunctionDefinition const*, ASTNode::CompareByID>> m_contractFunctionsWithoutVirtual;

	/// Depth of visit to modifiers.
	/// When m_modifierDepth == #modifiers the function can be visited
	/// when placeholder is visited.
	/// Needs to be a stack because of function calls.
	std::vector<int> m_modifierDepthStack;

	std::map<ContractDefinition const*, ModifierInvocation const*> m_baseConstructorCalls;

	ContractDefinition const* m_currentContract = nullptr;

	/// Stores the free functions and internal library functions.
	/// Those need to be encoded repeatedely for every analyzed contract.
	std::set<FunctionDefinition const*, ASTNode::CompareByID> m_freeFunctions;

	/// Stores the context of the encoding.
	smt::EncodingContext& m_context;

	ModelCheckerSettings m_settings;

	/// Character stream for each source,
	/// used for retrieving source text of expressions for e.g. counter-examples.
	langutil::CharStreamProvider const& m_charStreamProvider;

	smt::SymbolicState& state();
};

}
