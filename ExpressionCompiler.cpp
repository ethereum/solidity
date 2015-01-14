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

#include <utility>
#include <numeric>
#include <libdevcore/Common.h>
#include <libsolidity/AST.h>
#include <libsolidity/ExpressionCompiler.h>
#include <libsolidity/CompilerContext.h>
#include <libsolidity/CompilerUtils.h>

using namespace std;

namespace dev
{
namespace solidity
{

void ExpressionCompiler::compileExpression(CompilerContext& _context, Expression const& _expression, bool _optimize)
{
	ExpressionCompiler compiler(_context, _optimize);
	_expression.accept(compiler);
}

void ExpressionCompiler::appendTypeConversion(CompilerContext& _context, Type const& _typeOnStack,
											  Type const& _targetType, bool _cleanupNeeded)
{
	ExpressionCompiler compiler(_context);
	compiler.appendTypeConversion(_typeOnStack, _targetType, _cleanupNeeded);
}

bool ExpressionCompiler::visit(Assignment const& _assignment)
{
	_assignment.getRightHandSide().accept(*this);
	appendTypeConversion(*_assignment.getRightHandSide().getType(), *_assignment.getType());
	_assignment.getLeftHandSide().accept(*this);
	solAssert(m_currentLValue.isValid(), "LValue not retrieved.");

	Token::Value op = _assignment.getAssignmentOperator();
	if (op != Token::ASSIGN) // compound assignment
	{
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
		m_currentLValue.retrieveValue(_assignment, true);
		appendOrdinaryBinaryOperatorCode(Token::AssignmentToBinaryOp(op), *_assignment.getType());
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1;
	}
	m_currentLValue.storeValue(_assignment);
	m_currentLValue.reset();

	return false;
}

bool ExpressionCompiler::visit(UnaryOperation const& _unaryOperation)
{
	//@todo type checking and creating code for an operator should be in the same place:
	// the operator should know how to convert itself and to which types it applies, so
	// put this code together with "Type::acceptsBinary/UnaryOperator" into a class that
	// represents the operator
	if (_unaryOperation.getType()->getCategory() == Type::Category::INTEGER_CONSTANT)
	{
		m_context << _unaryOperation.getType()->literalValue(nullptr);
		return false;
	}

	_unaryOperation.getSubExpression().accept(*this);

	switch (_unaryOperation.getOperator())
	{
	case Token::NOT: // !
		m_context << eth::Instruction::ISZERO;
		break;
	case Token::BIT_NOT: // ~
		m_context << eth::Instruction::NOT;
		break;
	case Token::DELETE: // delete
		// @todo semantics change for complex types
		solAssert(m_currentLValue.isValid(), "LValue not retrieved.");

		m_context << u256(0);
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1;
		m_currentLValue.storeValue(_unaryOperation);
		m_currentLValue.reset();
		break;
	case Token::INC: // ++ (pre- or postfix)
	case Token::DEC: // -- (pre- or postfix)
		solAssert(m_currentLValue.isValid(), "LValue not retrieved.");
		m_currentLValue.retrieveValue(_unaryOperation);
		if (!_unaryOperation.isPrefixOperation())
		{
			if (m_currentLValue.storesReferenceOnStack())
				m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
			else
				m_context << eth::Instruction::DUP1;
		}
		m_context << u256(1);
		if (_unaryOperation.getOperator() == Token::INC)
			m_context << eth::Instruction::ADD;
		else
			m_context << eth::Instruction::SWAP1 << eth::Instruction::SUB; // @todo avoid the swap
		// Stack for prefix: [ref] (*ref)+-1
		// Stack for postfix: *ref [ref] (*ref)+-1
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1;
		m_currentLValue.storeValue(_unaryOperation, !_unaryOperation.isPrefixOperation());
		m_currentLValue.reset();
		break;
	case Token::ADD: // +
		// unary add, so basically no-op
		break;
	case Token::SUB: // -
		m_context << u256(0) << eth::Instruction::SUB;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid unary operator: " +
																		 string(Token::toString(_unaryOperation.getOperator()))));
	}
	return false;
}

bool ExpressionCompiler::visit(BinaryOperation const& _binaryOperation)
{
	Expression const& leftExpression = _binaryOperation.getLeftExpression();
	Expression const& rightExpression = _binaryOperation.getRightExpression();
	Type const& commonType = _binaryOperation.getCommonType();
	Token::Value const c_op = _binaryOperation.getOperator();

	if (c_op == Token::AND || c_op == Token::OR) // special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	else if (commonType.getCategory() == Type::Category::INTEGER_CONSTANT)
		m_context << commonType.literalValue(nullptr);
	else
	{
		bool cleanupNeeded = commonType.getCategory() == Type::Category::INTEGER &&
								(Token::isCompareOp(c_op) || c_op == Token::DIV || c_op == Token::MOD);

		// for commutative operators, push the literal as late as possible to allow improved optimization
		auto isLiteral = [](Expression const& _e)
		{
			return dynamic_cast<Literal const*>(&_e) || _e.getType()->getCategory() == Type::Category::INTEGER_CONSTANT;
		};
		bool swap = m_optimize && Token::isCommutativeOp(c_op) && isLiteral(rightExpression) && !isLiteral(leftExpression);
		if (swap)
		{
			leftExpression.accept(*this);
			appendTypeConversion(*leftExpression.getType(), commonType, cleanupNeeded);
			rightExpression.accept(*this);
			appendTypeConversion(*rightExpression.getType(), commonType, cleanupNeeded);
		}
		else
		{
			rightExpression.accept(*this);
			appendTypeConversion(*rightExpression.getType(), commonType, cleanupNeeded);
			leftExpression.accept(*this);
			appendTypeConversion(*leftExpression.getType(), commonType, cleanupNeeded);
		}
		if (Token::isCompareOp(c_op))
			appendCompareOperatorCode(c_op, commonType);
		else
			appendOrdinaryBinaryOperatorCode(c_op, commonType);
	}

	// do not visit the child nodes, we already did that explicitly
	return false;
}

bool ExpressionCompiler::visit(FunctionCall const& _functionCall)
{
	using Location = FunctionType::Location;
	if (_functionCall.isTypeConversion())
	{
		//@todo struct construction
		solAssert(_functionCall.getArguments().size() == 1, "");
		Expression const& firstArgument = *_functionCall.getArguments().front();
		firstArgument.accept(*this);
		appendTypeConversion(*firstArgument.getType(), *_functionCall.getType());
	}
	else
	{
		FunctionType const& function = dynamic_cast<FunctionType const&>(*_functionCall.getExpression().getType());
		vector<ASTPointer<Expression const>> arguments = _functionCall.getArguments();
		solAssert(arguments.size() == function.getParameterTypes().size(), "");

		switch (function.getLocation())
		{
		case Location::INTERNAL:
		{
			// Calling convention: Caller pushes return address and arguments
			// Callee removes them and pushes return values

			eth::AssemblyItem returnLabel = m_context.pushNewTag();
			for (unsigned i = 0; i < arguments.size(); ++i)
			{
				arguments[i]->accept(*this);
				appendTypeConversion(*arguments[i]->getType(), *function.getParameterTypes()[i]);
			}
			_functionCall.getExpression().accept(*this);

			m_context.appendJump();
			m_context << returnLabel;

			unsigned returnParametersSize = CompilerUtils::getSizeOnStack(function.getReturnParameterTypes());
			// callee adds return parameters, but removes arguments and return label
			m_context.adjustStackOffset(returnParametersSize - CompilerUtils::getSizeOnStack(function.getParameterTypes()) - 1);

			// @todo for now, the return value of a function is its first return value, so remove
			// all others
			for (unsigned i = 1; i < function.getReturnParameterTypes().size(); ++i)
				CompilerUtils(m_context).popStackElement(*function.getReturnParameterTypes()[i]);
			break;
		}
		case Location::EXTERNAL:
		case Location::BARE:
			_functionCall.getExpression().accept(*this);
			appendExternalFunctionCall(function, arguments, function.getLocation() == Location::BARE);
			break;
		case Location::CREATION:
		{
			_functionCall.getExpression().accept(*this);
			solAssert(!function.gasSet(), "Gas limit set for contract creation.");
			solAssert(function.getReturnParameterTypes().size() == 1, "");
			ContractDefinition const& contract = dynamic_cast<ContractType const&>(
							*function.getReturnParameterTypes().front()).getContractDefinition();
			// copy the contract's code into memory
			bytes const& bytecode = m_context.getCompiledContract(contract);
			m_context << u256(bytecode.size());
			//@todo could be done by actually appending the Assembly, but then we probably need to compile
			// multiple times. Will revisit once external fuctions are inlined.
			m_context.appendData(bytecode);
			//@todo copy to memory position 0, shift as soon as we use memory
			m_context << u256(0) << eth::Instruction::CODECOPY;

			unsigned length = bytecode.size();
			length += appendArgumentCopyToMemory(function.getParameterTypes(), arguments, length);
			// size, offset, endowment
			m_context << u256(length) << u256(0);
			if (function.valueSet())
				m_context << eth::dupInstruction(3);
			else
				m_context << u256(0);
			m_context << eth::Instruction::CREATE;
			if (function.valueSet())
				m_context << eth::swapInstruction(1) << eth::Instruction::POP;
			break;
		}
		case Location::SET_GAS:
		{
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.getExpression().accept(*this);
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(), IntegerType(256), true);
			// Note that function is not the original function, but the ".gas" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			unsigned stackDepth = (function.gasSet() ? 1 : 0) + (function.valueSet() ? 1 : 0);
			if (stackDepth > 0)
				m_context << eth::swapInstruction(stackDepth);
			if (function.gasSet())
				m_context << eth::Instruction::POP;
			break;
		}
		case Location::SET_VALUE:
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.getExpression().accept(*this);
			// Note that function is not the original function, but the ".value" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			if (function.valueSet())
				m_context << eth::Instruction::POP;
			arguments.front()->accept(*this);
			break;
		case Location::SEND:
			_functionCall.getExpression().accept(*this);
			m_context << u256(0); // 0 gas, we do not want to execute code
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(),
								 *function.getParameterTypes().front(), true);
			appendExternalFunctionCall(FunctionType(TypePointers{}, TypePointers{},
													Location::EXTERNAL, true, true), {}, true);
			break;
		case Location::SUICIDE:
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(), *function.getParameterTypes().front(), true);
			m_context << eth::Instruction::SUICIDE;
			break;
		case Location::SHA3:
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(), *function.getParameterTypes().front(), true);
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::Instruction::SHA3;
			break;
		case Location::LOG0:
		case Location::LOG1:
		case Location::LOG2:
		case Location::LOG3:
		case Location::LOG4:
		{
			unsigned logNumber = int(function.getLocation()) - int(Location::LOG0);
			for (int arg = logNumber; arg >= 0; --arg)
			{
				arguments[arg]->accept(*this);
				appendTypeConversion(*arguments[arg]->getType(), *function.getParameterTypes()[arg], true);
			}
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::logInstruction(logNumber);
			break;
		}
		case Location::ECRECOVER:
		case Location::SHA256:
		case Location::RIPEMD160:
		{
			static const map<Location, u256> contractAddresses{{Location::ECRECOVER, 1},
															   {Location::SHA256, 2},
															   {Location::RIPEMD160, 3}};
			m_context << contractAddresses.find(function.getLocation())->second;
			appendExternalFunctionCall(function, arguments, true);
			break;
		}
		default:
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid function type."));
		}
	}
	return false;
}

bool ExpressionCompiler::visit(NewExpression const& _newExpression)
{
	// code is created for the function call (CREATION) only
	return false;
}

void ExpressionCompiler::endVisit(MemberAccess const& _memberAccess)
{
	ASTString const& member = _memberAccess.getMemberName();
	switch (_memberAccess.getExpression().getType()->getCategory())
	{
	case Type::Category::CONTRACT:
	{
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.getExpression().getType());
		u256 identifier = type.getFunctionIdentifier(member);
		if (identifier != Invalid256)
		{
			appendTypeConversion(type, IntegerType(0, IntegerType::Modifier::ADDRESS), true);
			m_context << identifier;
			break;
		}
		// fall-through to "integer" otherwise (address)
	}
	case Type::Category::INTEGER:
		if (member == "balance")
		{
			appendTypeConversion(*_memberAccess.getExpression().getType(),
								 IntegerType(0, IntegerType::Modifier::ADDRESS), true);
			m_context << eth::Instruction::BALANCE;
		}
		else if (member == "send" || member.substr(0, min<size_t>(member.size(), 4)) == "call")
			appendTypeConversion(*_memberAccess.getExpression().getType(),
								 IntegerType(0, IntegerType::Modifier::ADDRESS), true);
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid member access to integer."));
		break;
	case Type::Category::FUNCTION:
		solAssert(!!_memberAccess.getExpression().getType()->getMemberType(member),
				 "Invalid member access to function.");
		break;
	case Type::Category::MAGIC:
		// we can ignore the kind of magic and only look at the name of the member
		if (member == "coinbase")
			m_context << eth::Instruction::COINBASE;
		else if (member == "timestamp")
			m_context << eth::Instruction::TIMESTAMP;
/*		else if (member == "blockhash")
			m_context << eth::Instruction::BLOCKHASH;
*/		else if (member == "difficulty")
			m_context << eth::Instruction::DIFFICULTY;
		else if (member == "number")
			m_context << eth::Instruction::NUMBER;
		else if (member == "gaslimit")
			m_context << eth::Instruction::GASLIMIT;
		else if (member == "sender")
			m_context << eth::Instruction::CALLER;
		else if (member == "value")
			m_context << eth::Instruction::CALLVALUE;
		else if (member == "origin")
			m_context << eth::Instruction::ORIGIN;
		else if (member == "gas")
			m_context << eth::Instruction::GAS;
		else if (member == "gasprice")
			m_context << eth::Instruction::GASPRICE;
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown magic member."));
		break;
	case Type::Category::STRUCT:
	{
		StructType const& type = dynamic_cast<StructType const&>(*_memberAccess.getExpression().getType());
		m_context << type.getStorageOffsetOfMember(member) << eth::Instruction::ADD;
		m_currentLValue = LValue(m_context, LValue::STORAGE, *_memberAccess.getType());
		m_currentLValue.retrieveValueIfLValueNotRequested(_memberAccess);
		break;
	}
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Member access to unknown type."));
	}
}

bool ExpressionCompiler::visit(IndexAccess const& _indexAccess)
{
	_indexAccess.getBaseExpression().accept(*this);
	_indexAccess.getIndexExpression().accept(*this);
	appendTypeConversion(*_indexAccess.getIndexExpression().getType(),
						 *dynamic_cast<MappingType const&>(*_indexAccess.getBaseExpression().getType()).getKeyType(),
						 true);
	// @todo move this once we actually use memory
	CompilerUtils(m_context).storeInMemory(0);
	CompilerUtils(m_context).storeInMemory(32);
	m_context << u256(64) << u256(0) << eth::Instruction::SHA3;

	m_currentLValue = LValue(m_context, LValue::STORAGE, *_indexAccess.getType());
	m_currentLValue.retrieveValueIfLValueNotRequested(_indexAccess);

	return false;
}

void ExpressionCompiler::endVisit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.getReferencedDeclaration();
	if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		if (magicVar->getType()->getCategory() == Type::Category::CONTRACT) // must be "this"
			m_context << eth::Instruction::ADDRESS;
		return;
	}
	if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
	{
		m_context << m_context.getFunctionEntryLabel(*functionDef).pushTag();
		return;
	}
	if (dynamic_cast<VariableDeclaration const*>(declaration))
	{
		m_currentLValue.fromIdentifier(_identifier, *declaration);
		m_currentLValue.retrieveValueIfLValueNotRequested(_identifier);
		return;
	}
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Identifier type not expected in expression context."));
}

void ExpressionCompiler::endVisit(Literal const& _literal)
{
	switch (_literal.getType()->getCategory())
	{
	case Type::Category::INTEGER_CONSTANT:
	case Type::Category::BOOL:
	case Type::Category::STRING:
		m_context << _literal.getType()->literalValue(&_literal);
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Only integer, boolean and string literals implemented for now."));
	}
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation const& _binaryOperation)
{
	Token::Value const c_op = _binaryOperation.getOperator();
	solAssert(c_op == Token::OR || c_op == Token::AND, "");

	_binaryOperation.getLeftExpression().accept(*this);
	m_context << eth::Instruction::DUP1;
	if (c_op == Token::AND)
		m_context << eth::Instruction::ISZERO;
	eth::AssemblyItem endLabel = m_context.appendConditionalJump();
	m_context << eth::Instruction::POP;
	_binaryOperation.getRightExpression().accept(*this);
	m_context << endLabel;
}

void ExpressionCompiler::appendCompareOperatorCode(Token::Value _operator, Type const& _type)
{
	if (_operator == Token::EQ || _operator == Token::NE)
	{
		m_context << eth::Instruction::EQ;
		if (_operator == Token::NE)
			m_context << eth::Instruction::ISZERO;
	}
	else
	{
		IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
		bool const c_isSigned = type.isSigned();

		switch (_operator)
		{
		case Token::GTE:
			m_context << (c_isSigned ? eth::Instruction::SLT : eth::Instruction::LT)
					  << eth::Instruction::ISZERO;
			break;
		case Token::LTE:
			m_context << (c_isSigned ? eth::Instruction::SGT : eth::Instruction::GT)
					  << eth::Instruction::ISZERO;
			break;
		case Token::GT:
			m_context << (c_isSigned ? eth::Instruction::SGT : eth::Instruction::GT);
			break;
		case Token::LT:
			m_context << (c_isSigned ? eth::Instruction::SLT : eth::Instruction::LT);
			break;
		default:
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown comparison operator."));
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
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown binary operator."));
}

void ExpressionCompiler::appendArithmeticOperatorCode(Token::Value _operator, Type const& _type)
{
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	bool const c_isSigned = type.isSigned();

	switch (_operator)
	{
	case Token::ADD:
		m_context << eth::Instruction::ADD;
		break;
	case Token::SUB:
		m_context << eth::Instruction::SUB;
		break;
	case Token::MUL:
		m_context << eth::Instruction::MUL;
		break;
	case Token::DIV:
		m_context  << (c_isSigned ? eth::Instruction::SDIV : eth::Instruction::DIV);
		break;
	case Token::MOD:
		m_context << (c_isSigned ? eth::Instruction::SMOD : eth::Instruction::MOD);
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown arithmetic operator."));
	}
}

void ExpressionCompiler::appendBitOperatorCode(Token::Value _operator)
{
	switch (_operator)
	{
	case Token::BIT_OR:
		m_context << eth::Instruction::OR;
		break;
	case Token::BIT_AND:
		m_context << eth::Instruction::AND;
		break;
	case Token::BIT_XOR:
		m_context << eth::Instruction::XOR;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown bit operator."));
	}
}

void ExpressionCompiler::appendShiftOperatorCode(Token::Value _operator)
{
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Shift operators not yet implemented."));
	switch (_operator)
	{
	case Token::SHL:
		break;
	case Token::SAR:
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown shift operator."));
	}
}

void ExpressionCompiler::appendTypeConversion(Type const& _typeOnStack, Type const& _targetType, bool _cleanupNeeded)
{
	// For a type extension, we need to remove all higher-order bits that we might have ignored in
	// previous operations.
	// @todo: store in the AST whether the operand might have "dirty" higher order bits

	if (_typeOnStack == _targetType && !_cleanupNeeded)
		return;
	Type::Category stackTypeCategory = _typeOnStack.getCategory();
	Type::Category targetTypeCategory = _targetType.getCategory();
	if (stackTypeCategory == Type::Category::INTEGER || stackTypeCategory == Type::Category::CONTRACT ||
			 stackTypeCategory == Type::Category::INTEGER_CONSTANT)
	{
		solAssert(targetTypeCategory == Type::Category::INTEGER || targetTypeCategory == Type::Category::CONTRACT, "");
		IntegerType addressType(0, IntegerType::Modifier::ADDRESS);
		IntegerType const& targetType = targetTypeCategory == Type::Category::INTEGER
			? dynamic_cast<IntegerType const&>(_targetType) : addressType;
		if (stackTypeCategory == Type::Category::INTEGER_CONSTANT)
		{
			IntegerConstantType const& constType = dynamic_cast<IntegerConstantType const&>(_typeOnStack);
			// We know that the stack is clean, we only have to clean for a narrowing conversion
			// where cleanup is forced.
			if (targetType.getNumBits() < constType.getIntegerType()->getNumBits() && _cleanupNeeded)
				appendHighBitsCleanup(targetType);
		}
		else
		{
			IntegerType const& typeOnStack = stackTypeCategory == Type::Category::INTEGER
				? dynamic_cast<IntegerType const&>(_typeOnStack) : addressType;
			// Widening: clean up according to source type width
			// Non-widening and force: clean up according to target type bits
			if (targetType.getNumBits() > typeOnStack.getNumBits())
				appendHighBitsCleanup(typeOnStack);
			else if (_cleanupNeeded)
				appendHighBitsCleanup(targetType);
		}
	}
	else if (stackTypeCategory == Type::Category::STRING)
	{
		solAssert(targetTypeCategory == Type::Category::STRING, "");
		// nothing to do, strings are high-order-bit-aligned
		//@todo clear lower-order bytes if we allow explicit conversion to shorter strings
	}
	else if (_typeOnStack != _targetType)
		// All other types should not be convertible to non-equal types.
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid type conversion requested."));
}

void ExpressionCompiler::appendHighBitsCleanup(IntegerType const& _typeOnStack)
{
	if (_typeOnStack.getNumBits() == 256)
		return;
	else if (_typeOnStack.isSigned())
		m_context << u256(_typeOnStack.getNumBits() / 8 - 1) << eth::Instruction::SIGNEXTEND;
	else
		m_context << ((u256(1) << _typeOnStack.getNumBits()) - 1) << eth::Instruction::AND;
}

void ExpressionCompiler::appendExternalFunctionCall(FunctionType const& _functionType,
													vector<ASTPointer<Expression const>> const& _arguments,
													bool bare)
{
	solAssert(_arguments.size() == _functionType.getParameterTypes().size(), "");

	// Assumed stack content here:
	// <stack top>
	// value [if _functionType.valueSet()]
	// gas [if _functionType.gasSet()]
	// function identifier [unless bare]
	// contract address

	unsigned gasValueSize = (_functionType.gasSet() ? 1 : 0) + (_functionType.valueSet() ? 1 : 0);

	unsigned contractStackPos = m_context.currentToBaseStackOffset(1 + gasValueSize + (bare ? 0 : 1));
	unsigned gasStackPos = m_context.currentToBaseStackOffset(gasValueSize);
	unsigned valueStackPos = m_context.currentToBaseStackOffset(1);

	if (!bare)
	{
		// copy function identifier
		m_context << eth::dupInstruction(gasValueSize + 1);
		CompilerUtils(m_context).storeInMemory(0, CompilerUtils::dataStartOffset);
	}

	// reserve space for the function identifier
	unsigned dataOffset = bare ? 0 : CompilerUtils::dataStartOffset;
	dataOffset += appendArgumentCopyToMemory(_functionType.getParameterTypes(), _arguments, dataOffset);

	//@todo only return the first return value for now
	Type const* firstType = _functionType.getReturnParameterTypes().empty() ? nullptr :
							_functionType.getReturnParameterTypes().front().get();
	unsigned retSize = firstType ? CompilerUtils::getPaddedSize(firstType->getCalldataEncodedSize()) : 0;
	// CALL arguments: outSize, outOff, inSize, inOff, value, addr, gas (stack top)
	m_context << u256(retSize) << u256(0) << u256(dataOffset) << u256(0);
	if (_functionType.valueSet())
		m_context << eth::dupInstruction(m_context.baseToCurrentStackOffset(valueStackPos));
	else
		m_context << u256(0);
	m_context << eth::dupInstruction(m_context.baseToCurrentStackOffset(contractStackPos));

	if (_functionType.gasSet())
		m_context << eth::dupInstruction(m_context.baseToCurrentStackOffset(gasStackPos));
	else
		// send all gas except for the 21 needed to execute "SUB" and "CALL"
		m_context << u256(21) << eth::Instruction::GAS << eth::Instruction::SUB;
	m_context << eth::Instruction::CALL
			  << eth::Instruction::POP; // @todo do not ignore failure indicator
	if (_functionType.valueSet())
		m_context << eth::Instruction::POP;
	if (_functionType.gasSet())
		m_context << eth::Instruction::POP;
	if (!bare)
		m_context << eth::Instruction::POP;
	m_context << eth::Instruction::POP; // pop contract address

	if (retSize > 0)
	{
		bool const c_leftAligned = firstType->getCategory() == Type::Category::STRING;
		CompilerUtils(m_context).loadFromMemory(0, retSize, c_leftAligned, false, true);
	}
}

unsigned ExpressionCompiler::appendArgumentCopyToMemory(TypePointers const& _types,
														vector<ASTPointer<Expression const>> const& _arguments,
														unsigned _memoryOffset)
{
	unsigned length = 0;
	for (unsigned i = 0; i < _arguments.size(); ++i)
	{
		_arguments[i]->accept(*this);
		appendTypeConversion(*_arguments[i]->getType(), *_types[i], true);
		unsigned const c_numBytes = _types[i]->getCalldataEncodedSize();
		if (c_numBytes == 0 || c_numBytes > 32)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(_arguments[i]->getLocation())
								  << errinfo_comment("Type " + _types[i]->toString() + " not yet supported."));
		bool const c_leftAligned = _types[i]->getCategory() == Type::Category::STRING;
		bool const c_padToWords = true;
		length += CompilerUtils(m_context).storeInMemory(_memoryOffset + length, c_numBytes,
														 c_leftAligned, c_padToWords);
	}
	return length;
}

ExpressionCompiler::LValue::LValue(CompilerContext& _compilerContext, LValueType _type, Type const& _dataType,
								   unsigned _baseStackOffset):
	m_context(&_compilerContext), m_type(_type), m_baseStackOffset(_baseStackOffset),
	m_stackSize(_dataType.getSizeOnStack())
{
}

void ExpressionCompiler::LValue::retrieveValue(Expression const& _expression, bool _remove) const
{
	switch (m_type)
	{
	case STACK:
	{
		unsigned stackPos = m_context->baseToCurrentStackOffset(unsigned(m_baseStackOffset));
		if (stackPos >= 15) //@todo correct this by fetching earlier or moving to memory
			BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
												  << errinfo_comment("Stack too deep."));
		for (unsigned i = 0; i < m_stackSize; ++i)
			*m_context << eth::dupInstruction(stackPos + 1);
		break;
	}
	case STORAGE:
		if (!_expression.getType()->isValueType())
			break; // no distinction between value and reference for non-value types
		if (!_remove)
			*m_context << eth::Instruction::DUP1;
		if (m_stackSize == 1)
			*m_context << eth::Instruction::SLOAD;
		else
			for (unsigned i = 0; i < m_stackSize; ++i)
			{
				*m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD << eth::Instruction::SWAP1;
				if (i + 1 < m_stackSize)
					 *m_context << u256(1) << eth::Instruction::ADD;
				else
					*m_context << eth::Instruction::POP;
			}
		break;
	case MEMORY:
		if (!_expression.getType()->isValueType())
			break; // no distinction between value and reference for non-value types
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_expression.getLocation())
													  << errinfo_comment("Location type not yet implemented."));
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_expression.getLocation())
													  << errinfo_comment("Unsupported location type."));
		break;
	}
}

void ExpressionCompiler::LValue::storeValue(Expression const& _expression, bool _move) const
{
	switch (m_type)
	{
	case STACK:
	{
		unsigned stackDiff = m_context->baseToCurrentStackOffset(unsigned(m_baseStackOffset)) - m_stackSize + 1;
		if (stackDiff > 16)
			BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
												  << errinfo_comment("Stack too deep."));
		else if (stackDiff > 0)
			for (unsigned i = 0; i < m_stackSize; ++i)
				*m_context << eth::swapInstruction(stackDiff) << eth::Instruction::POP;
		if (!_move)
			retrieveValue(_expression);
		break;
	}
	case LValue::STORAGE:
		if (!_expression.getType()->isValueType())
			break; // no distinction between value and reference for non-value types
		// stack layout: value value ... value ref
		if (!_move) // copy values
		{
			if (m_stackSize + 1 > 16)
				BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
													  << errinfo_comment("Stack too deep."));
			for (unsigned i = 0; i < m_stackSize; ++i)
				*m_context << eth::dupInstruction(m_stackSize + 1) << eth::Instruction::SWAP1;
		}
		if (m_stackSize > 0) // store high index value first
			*m_context << u256(m_stackSize - 1) << eth::Instruction::ADD;
		for (unsigned i = 0; i < m_stackSize; ++i)
		{
			if (i + 1 >= m_stackSize)
				*m_context << eth::Instruction::SSTORE;
			else
				// v v ... v v r+x
				*m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2
						   << eth::Instruction::SSTORE
						   << u256(1) << eth::Instruction::SWAP1 << eth::Instruction::SUB;
		}
		break;
	case LValue::MEMORY:
		if (!_expression.getType()->isValueType())
			break; // no distinction between value and reference for non-value types
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_expression.getLocation())
													  << errinfo_comment("Location type not yet implemented."));
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_expression.getLocation())
													  << errinfo_comment("Unsupported location type."));
		break;
	}
}

void ExpressionCompiler::LValue::retrieveValueIfLValueNotRequested(Expression const& _expression)
{
	if (!_expression.lvalueRequested())
	{
		retrieveValue(_expression, true);
		reset();
	}
}

void ExpressionCompiler::LValue::fromIdentifier(Identifier const& _identifier, Declaration const& _declaration)
{
	m_stackSize = _identifier.getType()->getSizeOnStack();
	if (m_context->isLocalVariable(&_declaration))
	{
		m_type = STACK;
		m_baseStackOffset = m_context->getBaseStackOffsetOfVariable(_declaration);
	}
	else if (m_context->isStateVariable(&_declaration))
	{
		m_type = STORAGE;
		*m_context << m_context->getStorageLocationOfVariable(_declaration);
	}
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_identifier.getLocation())
													  << errinfo_comment("Identifier type not supported or identifier not found."));
}

}
}
