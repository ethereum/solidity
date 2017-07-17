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
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler for expressions.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/LValue.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>
#include <libdevcore/Common.h>

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

namespace dev {
namespace eth
{
class AssemblyItem; // forward
}
namespace solidity {

// forward declarations
class CompilerContext;
class CompilerUtils;
class Type;
class IntegerType;
class ArrayType;

/**
 * Compiler for expressions, i.e. converts an AST tree whose root is an Expression into a stream
 * of EVM instructions. It needs a compiler context that is the same for the whole compilation
 * unit.
 */
class ExpressionCompiler: private ASTConstVisitor
{
public:
	explicit ExpressionCompiler(CompilerContext& _compilerContext, bool _optimiseOrderLiterals = false):
		m_optimiseOrderLiterals(_optimiseOrderLiterals), m_context(_compilerContext) {}

	/// Compile the given @a _expression and leave its value on the stack.
	void compile(Expression const& _expression);

	/// Appends code to set a state variable to its initial value/expression.
	void appendStateVariableInitialization(VariableDeclaration const& _varDecl);

	/// Appends code for a State Variable accessor function
	void appendStateVariableAccessor(VariableDeclaration const& _varDecl);

	/// Appends code for a Constant State Variable accessor function
	void appendConstStateVariableAccessor(VariableDeclaration const& _varDecl);

private:
	bool visit(Conditional const& _condition) override;
	bool visit(Assignment const& _assignment) override;
	bool visit(TupleExpression const& _tuple) override;
	bool visit(UnaryOperation const& _unaryOperation) override;
	bool visit(BinaryOperation const& _binaryOperation) override;
	bool visit(FunctionCall const& _functionCall) override;
	bool visit(NewExpression const& _newExpression) override;
	bool visit(MemberAccess const& _memberAccess) override;
	bool visit(IndexAccess const& _indexAccess) override;
	void endVisit(Identifier const& _identifier) override;
	void endVisit(Literal const& _literal) override;

	///@{
	///@name Append code for various operator types
	void appendAndOrOperatorCode(BinaryOperation const& _binaryOperation);
	void appendCompareOperatorCode(Token _operator, Type const& _type);
	void appendOrdinaryBinaryOperatorCode(Token _operator, Type const& _type);

	void appendArithmeticOperatorCode(Token _operator, Type const& _type);
	void appendBitOperatorCode(Token _operator);
	void appendShiftOperatorCode(Token _operator, Type const& _valueType, Type const& _shiftAmountType);
	/// @}

	/// Appends code to call a function of the given type with the given arguments.
	void appendExternalFunctionCall(
		FunctionType const& _functionType,
		std::vector<ASTPointer<Expression const>> const& _arguments
	);
	/// Appends code that evaluates a single expression and moves the result to memory. The memory offset is
	/// expected to be on the stack and is updated by this call.
	void appendExpressionCopyToMemory(Type const& _expectedType, Expression const& _expression);

	/// Appends code for a variable that might be a constant or not
	void appendVariable(VariableDeclaration const& _variable, Expression const& _expression);
	/// Sets the current LValue to a new one (of the appropriate type) from the given declaration.
	/// Also retrieves the value if it was not requested by @a _expression.
	void setLValueFromDeclaration(Declaration const& _declaration, Expression const& _expression);
	/// Sets the current LValue to a StorageItem holding the type of @a _expression. The reference is assumed
	/// to be on the stack.
	/// Also retrieves the value if it was not requested by @a _expression.
	void setLValueToStorageItem(Expression const& _expression);
	/// Sets the current LValue to a new LValue constructed from the arguments.
	/// Also retrieves the value if it was not requested by @a _expression.
	template <class _LValueType, class... _Arguments>
	void setLValue(Expression const& _expression, _Arguments const&... _arguments);

	/// @returns true if the operator applied to the given type requires a cleanup prior to the
	/// operation.
	static bool cleanupNeededForOp(Type::Category _type, Token _op);

	/// @returns the CompilerUtils object containing the current context.
	CompilerUtils utils();

	bool m_optimiseOrderLiterals;
	CompilerContext& m_context;
	std::unique_ptr<LValue> m_currentLValue;

};

template <class _LValueType, class... _Arguments>
void ExpressionCompiler::setLValue(Expression const& _expression, _Arguments const&... _arguments)
{
	solAssert(!m_currentLValue, "Current LValue not reset before trying to set new one.");
	std::unique_ptr<_LValueType> lvalue(new _LValueType(m_context, _arguments...));
	if (_expression.annotation().lValueRequested)
		m_currentLValue = move(lvalue);
	else
		lvalue->retrieveValue(_expression.location(), true);
}

}
}
