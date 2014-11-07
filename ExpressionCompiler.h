/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler for expressions.
 */

#include <libsolidity/ASTVisitor.h>

namespace dev {
namespace solidity {

class CompilerContext; // forward
class Type; // forward
class IntegerType; // forward

/// Compiler for expressions, i.e. converts an AST tree whose root is an Expression into a stream
/// of EVM instructions. It needs a compiler context that is the same for the whole compilation
/// unit.
class ExpressionCompiler: private ASTVisitor
{
public:
	/// Compile the given @a _expression into the @a _context.
	static void compileExpression(CompilerContext& _context, Expression& _expression);

	/// Appends code to remove dirty higher order bits in case of an implicit promotion to a wider type.
	static void appendTypeConversion(CompilerContext& _context, Type const& _typeOnStack, Type const& _targetType);

private:
	ExpressionCompiler(CompilerContext& _compilerContext): m_currentLValue(nullptr), m_context(_compilerContext) {}

	virtual bool visit(Assignment& _assignment) override;
	virtual void endVisit(UnaryOperation& _unaryOperation) override;
	virtual bool visit(BinaryOperation& _binaryOperation) override;
	virtual bool visit(FunctionCall& _functionCall) override;
	virtual void endVisit(MemberAccess& _memberAccess) override;
	virtual void endVisit(IndexAccess& _indexAccess) override;
	virtual void endVisit(Identifier& _identifier) override;
	virtual void endVisit(Literal& _literal) override;

	///@{
	///@name Append code for various operator types
	void appendAndOrOperatorCode(BinaryOperation& _binaryOperation);
	void appendCompareOperatorCode(Token::Value _operator, Type const& _type);
	void appendOrdinaryBinaryOperatorCode(Token::Value _operator, Type const& _type);

	void appendArithmeticOperatorCode(Token::Value _operator, Type const& _type);
	void appendBitOperatorCode(Token::Value _operator);
	void appendShiftOperatorCode(Token::Value _operator);
	/// @}

	/// Appends an implicit or explicit type conversion. For now this comprises only erasing
	/// higher-order bits (@see appendHighBitCleanup) when widening integer types.
	/// If @a _cleanupNeeded, high order bits cleanup is also done if no type conversion would be
	/// necessary.
	void appendTypeConversion(Type const& _typeOnStack, Type const& _targetType, bool _cleanupNeeded = false);
	//// Appends code that cleans higher-order bits for integer types.
	void appendHighBitsCleanup(IntegerType const& _typeOnStack);

	/// Stores the value on top of the stack in the current lvalue and copies that value to the
	/// top of the stack again
	void storeInLValue(Expression const& _expression);
	/// The same as storeInLValue but do not again retrieve the value to the top of the stack.
	void moveToLValue(Expression const& _expression);
	/// Returns the position of @a m_currentLValue in the stack, where 0 is the top of the stack.
	unsigned stackPositionOfLValue() const;

	Declaration* m_currentLValue;
	CompilerContext& m_context;
};


}
}
