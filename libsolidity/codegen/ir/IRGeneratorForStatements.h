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
 * Component that translates Solidity code into Yul at statement level and below.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/ir/IRLValue.h>
#include <libsolidity/codegen/ir/IRVariable.h>

#include <functional>

namespace solidity::frontend
{

class IRGenerationContext;
class YulUtilFunctions;

/**
 * Component that translates Solidity's AST into Yul at statement level and below.
 * It is an AST visitor that appends to an internal string buffer.
 */
class IRGeneratorForStatements: public ASTConstVisitor
{
public:
	IRGeneratorForStatements(IRGenerationContext& _context, YulUtilFunctions& _utils):
		m_context(_context),
		m_utils(_utils)
	{}

	std::string code() const;

	/// Generate the code for the statements in the block;
	void generate(Block const& _block);

	/// Generates code to initialize the given state variable.
	void initializeStateVar(VariableDeclaration const& _varDecl);
	/// Generates code to initialize the given local variable.
	void initializeLocalVar(VariableDeclaration const& _varDecl);

	/// Calculates expression's value and returns variable where it was stored
	IRVariable evaluateExpression(Expression const& _expression, Type const& _to);

	/// @returns the name of a function that computes the value of the given constant
	/// and also generates the function.
	std::string constantValueFunction(VariableDeclaration const& _constant);

	void endVisit(VariableDeclarationStatement const& _variableDeclaration) override;
	bool visit(Conditional const& _conditional) override;
	bool visit(Assignment const& _assignment) override;
	bool visit(TupleExpression const& _tuple) override;
	bool visit(Block const& _block) override;
	void endVisit(Block const& _block) override;
	bool visit(IfStatement const& _ifStatement) override;
	bool visit(ForStatement const& _forStatement) override;
	bool visit(WhileStatement const& _whileStatement) override;
	bool visit(Continue const& _continueStatement) override;
	bool visit(Break const& _breakStatement) override;
	void endVisit(Return const& _return) override;
	void endVisit(UnaryOperation const& _unaryOperation) override;
	bool visit(BinaryOperation const& _binOp) override;
	void endVisit(FunctionCall const& _funCall) override;
	void endVisit(FunctionCallOptions const& _funCallOptions) override;
	void endVisit(MemberAccess const& _memberAccess) override;
	bool visit(InlineAssembly const& _inlineAsm) override;
	void endVisit(IndexAccess const& _indexAccess) override;
	void endVisit(IndexRangeAccess const& _indexRangeAccess) override;
	void endVisit(Identifier const& _identifier) override;
	bool visit(Literal const& _literal) override;

	bool visit(TryStatement const& _tryStatement) override;
	bool visit(TryCatchClause const& _tryCatchClause) override;

private:
	/// Handles all catch cases of a try statement, except the success-case.
	void handleCatch(TryStatement const& _tryStatement);
	void handleCatchStructuredAndFallback(
		TryCatchClause const& _structured,
		TryCatchClause const* _fallback
	);
	void handleCatchFallback(TryCatchClause const& _fallback);

	/// Generates code to rethrow an exception.
	void rethrow();

	void handleVariableReference(
		VariableDeclaration const& _variable,
		Expression const& _referencingExpression
	);

	/// Appends code to call an external function with the given arguments.
	/// All involved expressions have already been visited.
	void appendExternalFunctionCall(
		FunctionCall const& _functionCall,
		std::vector<ASTPointer<Expression const>> const& _arguments
	);

	/// Appends code for .call / .delegatecall / .staticcall.
	/// All involved expressions have already been visited.
	void appendBareCall(
		FunctionCall const& _functionCall,
		std::vector<ASTPointer<Expression const>> const& _arguments
	);

	/// @returns code that evaluates to the first unused memory slot (which does not have to
	/// be empty).
	static std::string freeMemory();

	/// Generates the required conversion code and @returns an IRVariable referring to the value of @a _variable
	/// converted to type @a _to.
	IRVariable convert(IRVariable const& _variable, Type const& _to);

	/// @returns a Yul expression representing the current value of @a _expression,
	/// converted to type @a _to if it does not yet have that type.
	/// If @a _forceCleanup is set to true, it also cleans the value, in case it already has type @a _to.
	std::string expressionAsType(Expression const& _expression, Type const& _to, bool _forceCleanup = false);

	/// @returns an output stream that can be used to define @a _var using a function call or
	/// single stack slot expression.
	std::ostream& define(IRVariable const& _var);
	/// Defines @a _var using the value of @a _value while performing type conversions, if required.
	void define(IRVariable const& _var, IRVariable const& _value) { declareAssign(_var, _value, true); }
	/// Assigns @a _var to the value of @a _value while performing type conversions, if required.
	void assign(IRVariable const& _var, IRVariable const& _value) { declareAssign(_var, _value, false); }
	/// Declares variable @a _var.
	void declare(IRVariable const& _var);

	void declareAssign(IRVariable const& _var, IRVariable const& _value, bool _define);

	/// @returns an IRVariable with the zero
	/// value of @a _type.
	/// @param _splitFunctionTypes if false, returns two zeroes
	IRVariable zeroValue(Type const& _type, bool _splitFunctionTypes = true);

	void appendAndOrOperatorCode(BinaryOperation const& _binOp);
	void appendSimpleUnaryOperation(UnaryOperation const& _operation, Expression const& _expr);

	/// @returns code to perform the given binary operation in the given type on the two values.
	std::string binaryOperation(
		langutil::Token _op,
		Type const& _type,
		std::string const& _left,
		std::string const& _right
	);

	/// @returns code to perform the given shift operation.
	/// The operation itself will be performed in the type of the value,
	/// while the amount to shift can have its own type.
	std::string shiftOperation(langutil::Token _op, IRVariable const& _value, IRVariable const& _shiftAmount);

	/// Assigns the value of @a _value to the lvalue @a _lvalue.
	void writeToLValue(IRLValue const& _lvalue, IRVariable const& _value);
	/// @returns a fresh IR variable containing the value of the lvalue @a _lvalue.
	IRVariable readFromLValue(IRLValue const& _lvalue);

	/// Stores the given @a _lvalue in m_currentLValue, if it will be written to (willBeWrittenTo). Otherwise
	/// defines the expression @a _expression by reading the value from @a _lvalue.
	void setLValue(Expression const& _expression, IRLValue _lvalue);
	void generateLoop(
		Statement const& _body,
		Expression const* _conditionExpression,
		Statement const*  _initExpression = nullptr,
		ExpressionStatement const* _loopExpression = nullptr,
		bool _isDoWhile = false
	);

	static Type const& type(Expression const& _expression);

	void setLocation(ASTNode const& _node);

	std::string linkerSymbol(ContractDefinition const& _library) const;

	std::ostringstream m_code;
	IRGenerationContext& m_context;
	YulUtilFunctions& m_utils;
	std::optional<IRLValue> m_currentLValue;
	langutil::SourceLocation m_currentLocation;
};

}
