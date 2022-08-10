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
 * Base class for the statement generator.
 * Encapsulates access to the yul code stream and handles source code locations.
 */
class IRGeneratorForStatementsBase: public ASTConstVisitor
{
public:
	IRGeneratorForStatementsBase(IRGenerationContext& _context):
		m_context(_context)
	{}

	virtual std::string code() const;
	std::ostringstream& appendCode(bool _addLocationComment = true);
protected:
	void setLocation(ASTNode const& _node);
	langutil::SourceLocation m_currentLocation = {};
	langutil::SourceLocation m_lastLocation = {};
	IRGenerationContext& m_context;
private:
	std::ostringstream m_code;
};

/**
 * Component that translates Solidity's AST into Yul at statement level and below.
 * It is an AST visitor that appends to an internal string buffer.
 */
class IRGeneratorForStatements: public IRGeneratorForStatementsBase
{
public:
	IRGeneratorForStatements(
		IRGenerationContext& _context,
		YulUtilFunctions& _utils,
		std::function<std::string()> _placeholderCallback = {}
	):
		IRGeneratorForStatementsBase(_context),
		m_placeholderCallback(std::move(_placeholderCallback)),
		m_utils(_utils)
	{}

	std::string code() const override;

	/// Generate the code for the statements in the block;
	void generate(Block const& _block);

	/// Generates code to initialize the given state variable.
	void initializeStateVar(VariableDeclaration const& _varDecl);
	/// Generates code to initialize the given local variable.
	void initializeLocalVar(VariableDeclaration const& _varDecl);

	/// Calculates expression's value and returns variable where it was stored
	IRVariable evaluateExpression(Expression const& _expression, Type const& _to);

	/// Defines @a _var using the value of @a _value while performing type conversions, if required.
	void define(IRVariable const& _var, IRVariable const& _value)
	{
		bool _declare = true;
		declareAssign(_var, _value, _declare);
	}

	/// Defines @a _var using the value of @a _value while performing type conversions, if required.
	/// It also cleans the value of the variable.
	void defineAndCleanup(IRVariable const& _var, IRVariable const& _value)
	{
		bool _forceCleanup = true;
		bool _declare = true;
		declareAssign(_var, _value, _declare, _forceCleanup);
	}

	/// @returns the name of a function that computes the value of the given constant
	/// and also generates the function.
	std::string constantValueFunction(VariableDeclaration const& _constant);

	void endVisit(VariableDeclarationStatement const& _variableDeclaration) override;
	bool visit(Conditional const& _conditional) override;
	bool visit(Assignment const& _assignment) override;
	bool visit(TupleExpression const& _tuple) override;
	void endVisit(PlaceholderStatement const& _placeholder) override;
	bool visit(Block const& _block) override;
	void endVisit(Block const& _block) override;
	bool visit(IfStatement const& _ifStatement) override;
	bool visit(ForStatement const& _forStatement) override;
	bool visit(WhileStatement const& _whileStatement) override;
	bool visit(Continue const& _continueStatement) override;
	bool visit(Break const& _breakStatement) override;
	void endVisit(Return const& _return) override;
	bool visit(UnaryOperation const& _unaryOperation) override;
	bool visit(BinaryOperation const& _binOp) override;
	void endVisit(FunctionCall const& _funCall) override;
	void endVisit(FunctionCallOptions const& _funCallOptions) override;
	bool visit(MemberAccess const& _memberAccess) override;
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
	void handleCatchFallback(TryCatchClause const& _fallback);

	/// Generates code to revert with an error. The error arguments are assumed to
	/// be already evaluated and available in local IRVariables, but not yet
	/// converted.
	void revertWithError(
		std::string const& _signature,
		std::vector<Type const*> const& _parameterTypes,
		std::vector<ASTPointer<Expression const>> const& _errorArguments
	);

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

	/// Requests and assigns the internal ID of the referenced function to the referencing
	/// expression and adds the function to the internal dispatch.
	/// If the function is called right away, it does nothing.
	void assignInternalFunctionIDIfNotCalledDirectly(
		Expression const& _expression,
		FunctionDefinition const& _referencedFunction
	);

	/// Generates the required conversion code and @returns an IRVariable referring to the value of @a _variable
	IRVariable convert(IRVariable const& _variable, Type const& _to);

	/// Generates the required conversion code and @returns an IRVariable referring to the value of @a _variable
	/// It also cleans the value of the variable.
	IRVariable convertAndCleanup(IRVariable const& _from, Type const& _to);

	/// @returns a Yul expression representing the current value of @a _expression,
	/// converted to type @a _to if it does not yet have that type.
	std::string expressionAsType(Expression const& _expression, Type const& _to);

	/// @returns a Yul expression representing the current value of @a _expression,
	/// converted to type @a _to if it does not yet have that type.
	/// It also cleans the value, in case it already has type @a _to.
	std::string expressionAsCleanedType(Expression const& _expression, Type const& _to);

	/// @returns an output stream that can be used to define @a _var using a function call or
	/// single stack slot expression.
	std::ostream& define(IRVariable const& _var);

	/// Assigns @a _var to the value of @a _value while performing type conversions, if required.
	void assign(IRVariable const& _var, IRVariable const& _value) { declareAssign(_var, _value, false); }
	/// Declares variable @a _var.
	void declare(IRVariable const& _var);

	void declareAssign(IRVariable const& _var, IRVariable const& _value, bool _define, bool _forceCleanup = false);

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
		bool _isDoWhile = false,
		bool _isSimpleCounterLoop = false
	);

	static Type const& type(Expression const& _expression);

	std::string linkerSymbol(ContractDefinition const& _library) const;

	std::function<std::string()> m_placeholderCallback;
	YulUtilFunctions& m_utils;
	std::optional<IRLValue> m_currentLValue;
};

}
