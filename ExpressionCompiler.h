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
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler for expressions.
 */

#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>
#include <libdevcore/Common.h>
#include <libsolidity/BaseTypes.h>
#include <libsolidity/ASTVisitor.h>

namespace dev {
namespace eth
{
class AssemblyItem; // forward
}
namespace solidity {

// forward declarations
class CompilerContext;
class Type;
class IntegerType;
class ByteArrayType;
class StaticStringType;

/**
 * Compiler for expressions, i.e. converts an AST tree whose root is an Expression into a stream
 * of EVM instructions. It needs a compiler context that is the same for the whole compilation
 * unit.
 */
class ExpressionCompiler: private ASTConstVisitor
{
public:
	/// Compile the given @a _expression into the @a _context.
	static void compileExpression(CompilerContext& _context, Expression const& _expression, bool _optimize = false);

	/// Appends code to remove dirty higher order bits in case of an implicit promotion to a wider type.
	static void appendTypeConversion(CompilerContext& _context, Type const& _typeOnStack,
									 Type const& _targetType, bool _cleanupNeeded = false);
	/// Appends code for a State Variable accessor function
	static void appendStateVariableAccessor(CompilerContext& _context, VariableDeclaration const& _varDecl, bool _optimize = false);

private:
	explicit ExpressionCompiler(CompilerContext& _compilerContext, bool _optimize = false):
		m_optimize(_optimize), m_context(_compilerContext), m_currentLValue(m_context) {}

	virtual bool visit(Assignment const& _assignment) override;
	virtual bool visit(UnaryOperation const& _unaryOperation) override;
	virtual bool visit(BinaryOperation const& _binaryOperation) override;
	virtual bool visit(FunctionCall const& _functionCall) override;
	virtual bool visit(NewExpression const& _newExpression) override;
	virtual void endVisit(MemberAccess const& _memberAccess) override;
	virtual bool visit(IndexAccess const& _indexAccess) override;
	virtual void endVisit(Identifier const& _identifier) override;
	virtual void endVisit(Literal const& _literal) override;

	///@{
	///@name Append code for various operator types
	void appendAndOrOperatorCode(BinaryOperation const& _binaryOperation);
	void appendCompareOperatorCode(Token::Value _operator, Type const& _type);
	void appendOrdinaryBinaryOperatorCode(Token::Value _operator, Type const& _type);

	void appendArithmeticOperatorCode(Token::Value _operator, Type const& _type);
	void appendBitOperatorCode(Token::Value _operator);
	void appendShiftOperatorCode(Token::Value _operator);
	/// @}

	/// Appends an implicit or explicit type conversion. For now this comprises only erasing
	/// higher-order bits (@see appendHighBitCleanup) when widening integer.
	/// If @a _cleanupNeeded, high order bits cleanup is also done if no type conversion would be
	/// necessary.
	void appendTypeConversion(Type const& _typeOnStack, Type const& _targetType, bool _cleanupNeeded = false);
	//// Appends code that cleans higher-order bits for integer types.
	void appendHighBitsCleanup(IntegerType const& _typeOnStack);

	/// Appends code to call a function of the given type with the given arguments.
	void appendExternalFunctionCall(FunctionType const& _functionType, std::vector<ASTPointer<Expression const>> const& _arguments,
									bool bare = false);
	/// Appends code that evaluates the given arguments and moves the result to memory. The memory offset is
	/// expected to be on the stack and is updated by this call.
	void appendArgumentsCopyToMemory(std::vector<ASTPointer<Expression const>> const& _arguments,
									 TypePointers const& _types = {},
									 bool _padToWordBoundaries = true,
									 bool _padExceptionIfFourBytes = false);
	/// Appends code that moves a stack element of the given type to memory. The memory offset is
	/// expected below the stack element and is updated by this call.
	void appendTypeMoveToMemory(Type const& _type, bool _padToWordBoundaries = true);
	/// Appends code that evaluates a single expression and moves the result to memory. The memory offset is
	/// expected to be on the stack and is updated by this call.
	void appendExpressionCopyToMemory(Type const& _expectedType, Expression const& _expression);

	/// Appends code for a State Variable accessor function
	void appendStateVariableAccessor(VariableDeclaration const& _varDecl);

	/**
	 * Helper class to store and retrieve lvalues to and from various locations.
	 * All types except STACK store a reference in a slot on the stack, STACK just
	 * stores the base stack offset of the variable in @a m_baseStackOffset.
	 */
	class LValue
	{
	public:
		enum class LValueType { None, Stack, Memory, Storage };

		explicit LValue(CompilerContext& _compilerContext): m_context(&_compilerContext) { reset(); }
		LValue(CompilerContext& _compilerContext, LValueType _type,
			   std::shared_ptr<Type const> const& _dataType, unsigned _baseStackOffset = 0);

		/// Set type according to the declaration and retrieve the reference.
		/// @a _expression is the current expression
		void fromIdentifier(Identifier const& _identifier, Declaration const& _declaration);
		void reset() { m_type = LValueType::None; m_dataType.reset(); m_baseStackOffset = 0; m_size = 0; }

		bool isValid() const { return m_type != LValueType::None; }
		bool isInOnStack() const { return m_type == LValueType::Stack; }
		bool isInMemory() const { return m_type == LValueType::Memory; }
		bool isInStorage() const { return m_type == LValueType::Storage; }

		/// @returns true if this lvalue reference type occupies a slot on the stack.
		bool storesReferenceOnStack() const { return m_type == LValueType::Storage || m_type == LValueType::Memory; }

		/// Copies the value of the current lvalue to the top of the stack and, if @a _remove is true,
		/// also removes the reference from the stack (note that is does not reset the type to @a NONE).
		/// @a _location source location of the current expression, used for error reporting.
		void retrieveValue(Location const& _location, bool _remove = false) const;
		/// Moves a value from the stack to the lvalue. Removes the value if @a _move is true.
		/// @a _location is the source location of the expression that caused this operation.
		/// Stack pre: [lvalue_ref] value
		/// Stack post if !_move: value_of(lvalue_ref)
		void storeValue(Type const& _sourceType, Location const& _location = Location(), bool _move = false) const;
		/// Stores zero in the lvalue.
		/// @a _location is the source location of the requested operation
		void setToZero(Location const& _location = Location()) const;
		/// Convenience function to convert the stored reference to a value and reset type to NONE if
		/// the reference was not requested by @a _expression.
		void retrieveValueIfLValueNotRequested(Expression const& _expression);

	private:
		/// Convenience function to retrieve Value from Storage. Specific version of @ref retrieveValue
		void retrieveValueFromStorage(bool _remove = false) const;
		/// Copies from a byte array to a byte array in storage, both references on the stack.
		void copyByteArrayToStorage(ByteArrayType const& _targetType, ByteArrayType const& _sourceType) const;

		CompilerContext* m_context;
		LValueType m_type = LValueType::None;
		std::shared_ptr<Type const> m_dataType;
		/// If m_type is STACK, this is base stack offset (@see
		/// CompilerContext::getBaseStackOffsetOfVariable) of a local variable.
		unsigned m_baseStackOffset = 0;
		/// Size of the value of this lvalue on the stack or the storage.
		unsigned m_size = 0;
	};

	bool m_optimize;
	CompilerContext& m_context;
	LValue m_currentLValue;
};


}
}
