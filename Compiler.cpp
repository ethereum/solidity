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

#include <cassert>
#include <utility>
#include <libsolidity/AST.h>
#include <libsolidity/Compiler.h>


namespace dev {
namespace solidity {


void CompilerContext::setLabelPosition(uint32_t _label, uint32_t _position)
{
	assert(m_labelPositions.find(_label) == m_labelPositions.end());
	m_labelPositions[_label] = _position;
}

uint32_t CompilerContext::getLabelPosition(uint32_t _label) const
{
	auto iter = m_labelPositions.find(_label);
	assert(iter != m_labelPositions.end());
	return iter->second;
}

void ExpressionCompiler::compile(Expression& _expression)
{
	m_assemblyItems.clear();
	_expression.accept(*this);
}

bytes ExpressionCompiler::getAssembledBytecode() const
{
	bytes assembled;
	assembled.reserve(m_assemblyItems.size());

	// resolve label references
	for (uint32_t pos = 0; pos < m_assemblyItems.size(); ++pos)
	{
		AssemblyItem const& item = m_assemblyItems[pos];
		if (item.getType() == AssemblyItem::Type::LABEL)
			m_context.setLabelPosition(item.getLabel(), pos + 1);
	}

	for (AssemblyItem const& item: m_assemblyItems)
	{
		if (item.getType() == AssemblyItem::Type::LABELREF)
			assembled.push_back(m_context.getLabelPosition(item.getLabel()));
		else
			assembled.push_back(item.getData());
	}

	return assembled;
}

AssemblyItems ExpressionCompiler::compileExpression(CompilerContext& _context,
													Expression& _expression)
{
	ExpressionCompiler compiler(_context);
	compiler.compile(_expression);
	return compiler.getAssemblyItems();
}

bool ExpressionCompiler::visit(Assignment& _assignment)
{
	m_currentLValue = nullptr;
	_assignment.getLeftHandSide().accept(*this);

	Expression& rightHandSide = _assignment.getRightHandSide();
	Token::Value op = _assignment.getAssignmentOperator();
	if (op != Token::ASSIGN)
	{
		// compound assignment
		rightHandSide.accept(*this);
		Type const& resultType = *_assignment.getType();
		cleanHigherOrderBitsIfNeeded(*rightHandSide.getType(), resultType);
		appendOrdinaryBinaryOperatorCode(Token::AssignmentToBinaryOp(op), resultType);
	}
	else
	{
		append(eth::Instruction::POP); //@todo do not retrieve the value in the first place
		rightHandSide.accept(*this);
	}

	storeInLValue(_assignment);
	return false;
}

void ExpressionCompiler::endVisit(UnaryOperation& _unaryOperation)
{
	//@todo type checking and creating code for an operator should be in the same place:
	// the operator should know how to convert itself and to which types it applies, so
	// put this code together with "Type::acceptsBinary/UnaryOperator" into a class that
	// represents the operator
	switch (_unaryOperation.getOperator())
	{
	case Token::NOT: // !
		append(eth::Instruction::NOT);
		break;
	case Token::BIT_NOT: // ~
		append(eth::Instruction::BNOT);
		break;
	case Token::DELETE: // delete
	{
		// a -> a xor a (= 0).
		// @todo semantics change for complex types
		append(eth::Instruction::DUP1);
		append(eth::Instruction::XOR);
		storeInLValue(_unaryOperation);
		break;
	}
	case Token::INC: // ++ (pre- or postfix)
	case Token::DEC: // -- (pre- or postfix)
		if (!_unaryOperation.isPrefixOperation())
			append(eth::Instruction::DUP1);
		append(eth::Instruction::PUSH1);
		append(1);
		if (_unaryOperation.getOperator() == Token::INC)
			append(eth::Instruction::ADD);
		else
		{
			append(eth::Instruction::SWAP1); //@todo avoid this
			append(eth::Instruction::SUB);
		}
		if (_unaryOperation.isPrefixOperation())
			storeInLValue(_unaryOperation);
		else
			moveToLValue(_unaryOperation);
		break;
	case Token::ADD: // +
		// unary add, so basically no-op
		break;
	case Token::SUB: // -
		append(eth::Instruction::PUSH1);
		append(0);
		append(eth::Instruction::SUB);
		break;
	default:
		assert(false); // invalid operation
	}
}

bool ExpressionCompiler::visit(BinaryOperation& _binaryOperation)
{
	Expression& leftExpression = _binaryOperation.getLeftExpression();
	Expression& rightExpression = _binaryOperation.getRightExpression();
	Type const& resultType = *_binaryOperation.getType();
	Token::Value const op = _binaryOperation.getOperator();

	if (op == Token::AND || op == Token::OR)
	{
		// special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	}
	else if (Token::isCompareOp(op))
	{
		leftExpression.accept(*this);
		rightExpression.accept(*this);

		// the types to compare have to be the same, but the resulting type is always bool
		assert(*leftExpression.getType() == *rightExpression.getType());
		appendCompareOperatorCode(op, *leftExpression.getType());
	}
	else
	{
		leftExpression.accept(*this);
		cleanHigherOrderBitsIfNeeded(*leftExpression.getType(), resultType);
		rightExpression.accept(*this);
		cleanHigherOrderBitsIfNeeded(*rightExpression.getType(), resultType);
		appendOrdinaryBinaryOperatorCode(op, resultType);
	}

	// do not visit the child nodes, we already did that explicitly
	return false;
}

void ExpressionCompiler::endVisit(FunctionCall& _functionCall)
{
	if (_functionCall.isTypeConversion())
	{
		//@todo we only have integers and bools for now which cannot be explicitly converted
		assert(_functionCall.getArguments().size() == 1);
		cleanHigherOrderBitsIfNeeded(*_functionCall.getArguments().front()->getType(),
									 *_functionCall.getType());
	}
	else
	{
		//@todo: arguments are already on the stack
		// push return label (below arguments?)
		// jump to function label
		// discard all but the first function return argument
	}
}

void ExpressionCompiler::endVisit(MemberAccess&)
{

}

void ExpressionCompiler::endVisit(IndexAccess&)
{

}

void ExpressionCompiler::endVisit(Identifier& _identifier)
{
	m_currentLValue = _identifier.getReferencedDeclaration();
	unsigned stackPos = stackPositionOfLValue();
	if (stackPos >= 15) //@todo correct this by fetching earlier or moving to memory
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_identifier.getLocation())
											  << errinfo_comment("Stack too deep."));
	appendDup(stackPos + 1);
}

void ExpressionCompiler::endVisit(Literal& _literal)
{
	switch (_literal.getType()->getCategory())
	{
	case Type::Category::INTEGER:
	case Type::Category::BOOL:
	{
		bytes value = _literal.getType()->literalToBigEndian(_literal);
		assert(value.size() <= 32);
		assert(!value.empty());
		appendPush(value.size());
		append(value);
		break;
	}
	default:
		assert(false); // @todo
	}
}

void ExpressionCompiler::cleanHigherOrderBitsIfNeeded(const Type& _typeOnStack, const Type& _targetType)
{
	// If the type of one of the operands is extended, we need to remove all
	// higher-order bits that we might have ignored in previous operations.
	// @todo: store in the AST whether the operand might have "dirty" higher
	// order bits

	if (_typeOnStack == _targetType)
		return;
	if (_typeOnStack.getCategory() == Type::Category::INTEGER &&
			_targetType.getCategory() == Type::Category::INTEGER)
	{
		//@todo
	}
	else
	{
		// If we get here, there is either an implementation missing to clean higher oder bits
		// for non-integer types that are explicitly convertible or we got here in error.
		assert(!_typeOnStack.isExplicitlyConvertibleTo(_targetType));
		assert(false); // these types should not be convertible.
	}
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation& _binaryOperation)
{
	Token::Value const op = _binaryOperation.getOperator();
	assert(op == Token::OR || op == Token::AND);

	_binaryOperation.getLeftExpression().accept(*this);
	append(eth::Instruction::DUP1);
	if (op == Token::AND)
		append(eth::Instruction::NOT);
	uint32_t endLabel = appendConditionalJump();
	_binaryOperation.getRightExpression().accept(*this);
	appendLabel(endLabel);
}

void ExpressionCompiler::appendCompareOperatorCode(Token::Value _operator, Type const& _type)
{
	if (_operator == Token::EQ || _operator == Token::NE)
	{
		append(eth::Instruction::EQ);
		if (_operator == Token::NE)
			append(eth::Instruction::NOT);
	}
	else
	{
		IntegerType const* type = dynamic_cast<IntegerType const*>(&_type);
		assert(type);
		bool const isSigned = type->isSigned();

		// note that EVM opcodes compare like "stack[0] < stack[1]",
		// but our left value is at stack[1], so everyhing is reversed.
		switch (_operator)
		{
		case Token::GTE:
			append(isSigned ? eth::Instruction::SGT : eth::Instruction::GT);
			append(eth::Instruction::NOT);
			break;
		case Token::LTE:
			append(isSigned ? eth::Instruction::SLT : eth::Instruction::LT);
			append(eth::Instruction::NOT);
			break;
		case Token::GT:
			append(isSigned ? eth::Instruction::SLT : eth::Instruction::LT);
			break;
		case Token::LT:
			append(isSigned ? eth::Instruction::SGT : eth::Instruction::GT);
			break;
		default:
			assert(false);
		}
	}
}

void ExpressionCompiler::appendOrdinaryBinaryOperatorCode(Token::Value _operator, Type const& _type)
{
	if (Token::isArithmeticOp(_operator))
		appendArithmeticOperatorCode(_operator, _type);
	else if (Token::isBitOp(_operator))
		appendBitOperatorCode(_operator);
	else if (Token::isShiftOp(_operator))
		appendShiftOperatorCode(_operator);
	else
		assert(false); // unknown binary operator
}

void ExpressionCompiler::appendArithmeticOperatorCode(Token::Value _operator, Type const& _type)
{
	IntegerType const* type = dynamic_cast<IntegerType const*>(&_type);
	assert(type);
	bool const isSigned = type->isSigned();

	switch (_operator)
	{
	case Token::ADD:
		append(eth::Instruction::ADD);
		break;
	case Token::SUB:
		append(eth::Instruction::SWAP1);
		append(eth::Instruction::SUB);
		break;
	case Token::MUL:
		append(eth::Instruction::MUL);
		break;
	case Token::DIV:
		append(isSigned ? eth::Instruction::SDIV : eth::Instruction::DIV);
		break;
	case Token::MOD:
		append(isSigned ? eth::Instruction::SMOD : eth::Instruction::MOD);
		break;
	default:
		assert(false);
	}
}

void ExpressionCompiler::appendBitOperatorCode(Token::Value _operator)
{
	switch (_operator)
	{
	case Token::BIT_OR:
		append(eth::Instruction::OR);
		break;
	case Token::BIT_AND:
		append(eth::Instruction::AND);
		break;
	case Token::BIT_XOR:
		append(eth::Instruction::XOR);
		break;
	default:
		assert(false);
	}
}

void ExpressionCompiler::appendShiftOperatorCode(Token::Value _operator)
{
	switch (_operator)
	{
	case Token::SHL:
		assert(false); //@todo
		break;
	case Token::SAR:
		assert(false); //@todo
		break;
	default:
		assert(false);
	}
}

uint32_t ExpressionCompiler::appendConditionalJump()
{
	uint32_t label = m_context.dispenseNewLabel();
	append(eth::Instruction::PUSH1);
	appendLabelref(label);
	append(eth::Instruction::JUMPI);
	return label;
}

void ExpressionCompiler::appendPush(unsigned _number)
{
	assert(1 <= _number && _number <= 32);
	append(eth::Instruction(unsigned(eth::Instruction::PUSH1) + _number - 1));
}

void ExpressionCompiler::appendDup(unsigned _number)
{
	assert(1 <= _number && _number <= 16);
	append(eth::Instruction(unsigned(eth::Instruction::DUP1) + _number - 1));
}

void ExpressionCompiler::appendSwap(unsigned _number)
{
	assert(1 <= _number && _number <= 16);
	append(eth::Instruction(unsigned(eth::Instruction::SWAP1) + _number - 1));
}

void ExpressionCompiler::append(bytes const& _data)
{
	m_assemblyItems.reserve(m_assemblyItems.size() + _data.size());
	for (byte b: _data)
		append(b);
}

void ExpressionCompiler::storeInLValue(Expression const& _expression)
{
	assert(m_currentLValue);
	moveToLValue(_expression);
	unsigned stackPos = stackPositionOfLValue();
	if (stackPos > 16)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
											  << errinfo_comment("Stack too deep."));
	if (stackPos >= 1)
		appendDup(stackPos);
}

void ExpressionCompiler::moveToLValue(Expression const& _expression)
{
	assert(m_currentLValue);
	unsigned stackPos = stackPositionOfLValue();
	if (stackPos > 16)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
											  << errinfo_comment("Stack too deep."));
	else if (stackPos > 0)
	{
		appendSwap(stackPos);
		append(eth::Instruction::POP);
	}
}

unsigned ExpressionCompiler::stackPositionOfLValue() const
{
	return 8; // @todo ask the context and track stack changes due to m_assemblyItems
}



}
}
