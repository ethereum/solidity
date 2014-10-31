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
 * Solidity AST to EVM bytecode compiler.
 */

#include <libevmface/Instruction.h>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/Types.h>
#include <libsolidity/Token.h>

namespace dev {
namespace solidity {

/**
 * A single item of compiled code that can be assembled to a single byte value in the final
 * bytecode. Its main purpose is to inject jump labels and label references into the opcode stream,
 * which can be resolved in the final step.
 */
class AssemblyItem
{
public:
	enum class Type
	{
		CODE,    ///< m_data is opcode, m_label is empty.
		DATA,    ///< m_data is actual data, m_label is empty
		LABEL,   ///< m_data is JUMPDEST opcode, m_label is id of label
		LABELREF ///< m_data is empty, m_label is id of label
	};

	explicit AssemblyItem(eth::Instruction _instruction) : m_type(Type::CODE), m_data(byte(_instruction)) {}
	explicit AssemblyItem(byte _data): m_type(Type::DATA), m_data(_data) {}

	/// Factory functions
	static AssemblyItem labelRef(uint32_t _label) { return AssemblyItem(Type::LABELREF, 0, _label); }
	static AssemblyItem label(uint32_t _label) { return AssemblyItem(Type::LABEL, byte(eth::Instruction::JUMPDEST), _label); }

	Type getType() const { return m_type; }
	byte getData() const { return m_data; }
	uint32_t getLabel() const { return m_label; }

private:
	AssemblyItem(Type _type, byte _data, uint32_t _label): m_type(_type), m_data(_data), m_label(_label) {}

	Type m_type;
	byte m_data; ///< data to be written to the bytecode stream (or filled by a label if this is a LABELREF)
	uint32_t m_label; ///< the id of a label either referenced or defined by this item
};

using AssemblyItems = std::vector<AssemblyItem>;


/**
 * Context to be shared by all units that compile the same contract. Its current usage only
 * concerns dispensing unique jump label IDs and storing their actual positions in the bytecode
 * stream.
 */
class CompilerContext
{
public:
	CompilerContext(): m_nextLabel(0) {}
	uint32_t dispenseNewLabel() { return m_nextLabel++; }
	void setLabelPosition(uint32_t _label, uint32_t _position);
	uint32_t getLabelPosition(uint32_t _label) const;

private:
	uint32_t m_nextLabel;

	std::map<uint32_t, uint32_t> m_labelPositions;
};

/**
 * Compiler for expressions, i.e. converts an AST tree whose root is an Expression into a stream
 * of EVM instructions. It needs a compiler context that is the same for the whole compilation
 * unit.
 */
class ExpressionCompiler: public ASTVisitor
{
public:
	ExpressionCompiler(CompilerContext& _compilerContext): m_context(_compilerContext) {}

	/// Compile the given expression and (re-)populate the assembly item list.
	void compile(Expression& _expression);
	AssemblyItems const& getAssemblyItems() const { return m_assemblyItems; }
	bytes getAssembledBytecode() const;

	/// Compile the given expression and return the assembly items right away.
	static AssemblyItems compileExpression(CompilerContext& _context, Expression& _expression);

private:
	virtual void endVisit(Assignment& _assignment) override;
	virtual void endVisit(UnaryOperation& _unaryOperation) override;
	virtual bool visit(BinaryOperation& _binaryOperation) override;
	virtual void endVisit(FunctionCall& _functionCall) override;
	virtual void endVisit(MemberAccess& _memberAccess) override;
	virtual void endVisit(IndexAccess& _indexAccess) override;
	virtual void endVisit(Identifier& _identifier) override;
	virtual void endVisit(Literal& _literal) override;

	/// Appends code to remove dirty higher order bits in case of an implicit promotion to a wider type.
	void cleanHigherOrderBitsIfNeeded(Type const& _typeOnStack, Type const& _targetType);

	///@{
	///@name Append code for various operator types
	void appendAndOrOperatorCode(BinaryOperation& _binaryOperation);
	void appendCompareOperatorCode(Token::Value _operator, Type const& _type);
	void appendOrdinaryBinaryOperatorCode(Token::Value _operator, Type const& _type);

	void appendArithmeticOperatorCode(Token::Value _operator, Type const& _type);
	void appendBitOperatorCode(Token::Value _operator);
	void appendShiftOperatorCode(Token::Value _operator);
	/// @}

	/// Appends a JUMPI instruction to a new label and returns the label
	uint32_t appendConditionalJump();

	/// Append elements to the current instruction list.
	void append(eth::Instruction const& _instruction) { m_assemblyItems.push_back(AssemblyItem(_instruction)); }
	void append(byte _value) { m_assemblyItems.push_back(AssemblyItem(_value)); }
	void append(bytes const& _data);
	void appendLabelref(byte _label) { m_assemblyItems.push_back(AssemblyItem::labelRef(_label)); }
	void appendLabel(byte _label) { m_assemblyItems.push_back(AssemblyItem::label(_label)); }

	AssemblyItems m_assemblyItems;
	CompilerContext& m_context;
};


}
}
