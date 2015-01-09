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

void ExpressionCompiler::appendTypeConversion(CompilerContext& _context,
											  Type const& _typeOnStack, Type const& _targetType)
{
	ExpressionCompiler compiler(_context);
	compiler.appendTypeConversion(_typeOnStack, _targetType);
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

void ExpressionCompiler::endVisit(UnaryOperation const& _unaryOperation)
{
	//@todo type checking and creating code for an operator should be in the same place:
	// the operator should know how to convert itself and to which types it applies, so
	// put this code together with "Type::acceptsBinary/UnaryOperator" into a class that
	// represents the operator
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
}

bool ExpressionCompiler::visit(BinaryOperation const& _binaryOperation)
{
	Expression const& leftExpression = _binaryOperation.getLeftExpression();
	Expression const& rightExpression = _binaryOperation.getRightExpression();
	Type const& commonType = _binaryOperation.getCommonType();
	Token::Value const op = _binaryOperation.getOperator();

	if (op == Token::AND || op == Token::OR) // special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	else
	{
		bool cleanupNeeded = false;
		if (commonType.getCategory() == Type::Category::INTEGER)
			if (Token::isCompareOp(op) || op == Token::DIV || op == Token::MOD)
				cleanupNeeded = true;

		// for commutative operators, push the literal as late as possible to allow improved optimization
		//@todo this has to be extended for literal expressions
		bool swap = (m_optimize && Token::isCommutativeOp(op) && dynamic_cast<Literal const*>(&rightExpression)
					 && !dynamic_cast<Literal const*>(&leftExpression));
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
		if (Token::isCompareOp(op))
			appendCompareOperatorCode(op, commonType);
		else
			appendOrdinaryBinaryOperatorCode(op, commonType);
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
		if (firstArgument.getType()->getCategory() == Type::Category::CONTRACT &&
				_functionCall.getType()->getCategory() == Type::Category::INTEGER)
		{
			// explicit type conversion contract -> address, nothing to do.
		}
		else
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
		{
			FunctionCallOptions options;
			options.bare = function.getLocation() == Location::BARE;
			options.obtainAddress = [&]() { _functionCall.getExpression().accept(*this); };
			appendExternalFunctionCall(function, arguments, options);
			break;
		}
		case Location::SEND:
		{
			FunctionCallOptions options;
			options.bare = true;
			options.obtainAddress = [&]() { _functionCall.getExpression().accept(*this); };
			options.obtainValue = [&]() { arguments.front()->accept(*this); };
			appendExternalFunctionCall(FunctionType({}, {}, Location::EXTERNAL), {}, options);
			break;
		}
		case Location::SUICIDE:
			arguments.front()->accept(*this);
			//@todo might not be necessary
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
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(), *function.getParameterTypes().front(), true);
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::Instruction::LOG0;
			break;
		case Location::LOG1:
			arguments[1]->accept(*this);
			arguments[0]->accept(*this);
			appendTypeConversion(*arguments[1]->getType(), *function.getParameterTypes()[1], true);
			appendTypeConversion(*arguments[0]->getType(), *function.getParameterTypes()[0], true);
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::Instruction::LOG1;
			break;
		case Location::LOG2:
			arguments[2]->accept(*this);
			arguments[1]->accept(*this);
			arguments[0]->accept(*this);
			appendTypeConversion(*arguments[2]->getType(), *function.getParameterTypes()[2], true);
			appendTypeConversion(*arguments[1]->getType(), *function.getParameterTypes()[1], true);
			appendTypeConversion(*arguments[0]->getType(), *function.getParameterTypes()[0], true);
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::Instruction::LOG2;
			break;
		case Location::LOG3:
			arguments[3]->accept(*this);
			arguments[2]->accept(*this);
			arguments[1]->accept(*this);
			arguments[0]->accept(*this);
			appendTypeConversion(*arguments[3]->getType(), *function.getParameterTypes()[3], true);
			appendTypeConversion(*arguments[2]->getType(), *function.getParameterTypes()[2], true);
			appendTypeConversion(*arguments[1]->getType(), *function.getParameterTypes()[1], true);
			appendTypeConversion(*arguments[0]->getType(), *function.getParameterTypes()[0], true);
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::Instruction::LOG3;
			break;
		case Location::LOG4:
			arguments[4]->accept(*this);
			arguments[3]->accept(*this);
			arguments[2]->accept(*this);
			arguments[1]->accept(*this);
			arguments[0]->accept(*this);
			appendTypeConversion(*arguments[4]->getType(), *function.getParameterTypes()[4], true);
			appendTypeConversion(*arguments[3]->getType(), *function.getParameterTypes()[3], true);
			appendTypeConversion(*arguments[2]->getType(), *function.getParameterTypes()[2], true);
			appendTypeConversion(*arguments[1]->getType(), *function.getParameterTypes()[1], true);
			appendTypeConversion(*arguments[0]->getType(), *function.getParameterTypes()[0], true);
			// @todo move this once we actually use memory
			CompilerUtils(m_context).storeInMemory(0);
			m_context << u256(32) << u256(0) << eth::Instruction::LOG4;
			break;
		case Location::ECRECOVER:
		case Location::SHA256:
		case Location::RIPEMD160:
		{
			static const map<Location, u256> contractAddresses{{Location::ECRECOVER, 1},
															   {Location::SHA256, 2},
															   {Location::RIPEMD160, 3}};
			u256 contractAddress = contractAddresses.find(function.getLocation())->second;
			FunctionCallOptions options;
			options.bare = true;
			options.obtainAddress = [&]() { m_context << contractAddress; };
			options.packDensely = false;
			appendExternalFunctionCall(function, arguments, options);
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
	ContractType const* type = dynamic_cast<ContractType const*>(_newExpression.getType().get());
	solAssert(type, "");
	TypePointers const& types = type->getConstructorType()->getParameterTypes();
	vector<ASTPointer<Expression const>> arguments = _newExpression.getArguments();
	solAssert(arguments.size() == types.size(), "");

	// copy the contracts code into memory
	bytes const& bytecode = m_context.getCompiledContract(*_newExpression.getContract());
	m_context << u256(bytecode.size());
	//@todo could be done by actually appending the Assembly, but then we probably need to compile
	// multiple times. Will revisit once external fuctions are inlined.
	m_context.appendData(bytecode);
	//@todo copy to memory position 0, shift as soon as we use memory
	m_context << u256(0) << eth::Instruction::CODECOPY;

	unsigned dataOffset = bytecode.size();
	for (unsigned i = 0; i < arguments.size(); ++i)
	{
		arguments[i]->accept(*this);
		appendTypeConversion(*arguments[i]->getType(), *types[i]);
		unsigned const numBytes = types[i]->getCalldataEncodedSize();
		if (numBytes > 32)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(arguments[i]->getLocation())
								  << errinfo_comment("Type " + types[i]->toString() + " not yet supported."));
		bool const leftAligned = types[i]->getCategory() == Type::Category::STRING;
		CompilerUtils(m_context).storeInMemory(dataOffset, numBytes, leftAligned);
		dataOffset += numBytes;
	}
	// size, offset, endowment
	m_context << u256(dataOffset) << u256(0) << u256(0) << eth::Instruction::CREATE;
	return false;
}

void ExpressionCompiler::endVisit(MemberAccess const& _memberAccess)
{
	ASTString const& member = _memberAccess.getMemberName();
	switch (_memberAccess.getExpression().getType()->getCategory())
	{
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
	case Type::Category::CONTRACT:
	{
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.getExpression().getType());
		m_context << type.getFunctionIdentifier(member);
		break;
	}
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
	case Type::Category::INTEGER:
	case Type::Category::BOOL:
	case Type::Category::STRING:
		m_context << _literal.getType()->literalValue(_literal);
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Only integer, boolean and string literals implemented for now."));
	}
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation const& _binaryOperation)
{
	Token::Value const op = _binaryOperation.getOperator();
	solAssert(op == Token::OR || op == Token::AND, "");

	_binaryOperation.getLeftExpression().accept(*this);
	m_context << eth::Instruction::DUP1;
	if (op == Token::AND)
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
		bool const isSigned = type.isSigned();

		switch (_operator)
		{
		case Token::GTE:
			m_context << (isSigned ? eth::Instruction::SLT : eth::Instruction::LT)
					  << eth::Instruction::ISZERO;
			break;
		case Token::LTE:
			m_context << (isSigned ? eth::Instruction::SGT : eth::Instruction::GT)
					  << eth::Instruction::ISZERO;
			break;
		case Token::GT:
			m_context << (isSigned ? eth::Instruction::SGT : eth::Instruction::GT);
			break;
		case Token::LT:
			m_context << (isSigned ? eth::Instruction::SLT : eth::Instruction::LT);
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
	bool const isSigned = type.isSigned();

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
		m_context  << (isSigned ? eth::Instruction::SDIV : eth::Instruction::DIV);
		break;
	case Token::MOD:
		m_context << (isSigned ? eth::Instruction::SMOD : eth::Instruction::MOD);
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
	if (_typeOnStack.getCategory() == Type::Category::INTEGER)
		appendHighBitsCleanup(dynamic_cast<IntegerType const&>(_typeOnStack));
	else if (_typeOnStack.getCategory() == Type::Category::STRING)
	{
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
													FunctionCallOptions const& _options)
{
	solAssert(_arguments.size() == _functionType.getParameterTypes().size(), "");

	_options.obtainAddress();
	if (!_options.bare)
		CompilerUtils(m_context).storeInMemory(0, CompilerUtils::dataStartOffset);

	unsigned dataOffset = _options.bare ? 0 : CompilerUtils::dataStartOffset; // reserve 4 bytes for the function's hash identifier
	for (unsigned i = 0; i < _arguments.size(); ++i)
	{
		_arguments[i]->accept(*this);
		Type const& type = *_functionType.getParameterTypes()[i];
		appendTypeConversion(*_arguments[i]->getType(), type);
		unsigned const numBytes = _options.packDensely ? type.getCalldataEncodedSize() : 32;
		if (numBytes == 0 || numBytes > 32)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(_arguments[i]->getLocation())
								  << errinfo_comment("Type " + type.toString() + " not yet supported."));
		bool const leftAligned = type.getCategory() == Type::Category::STRING;
		CompilerUtils(m_context).storeInMemory(dataOffset, numBytes, leftAligned);
		dataOffset += numBytes;
	}
	//@todo only return the first return value for now
	Type const* firstType = _functionType.getReturnParameterTypes().empty() ? nullptr :
							_functionType.getReturnParameterTypes().front().get();
	unsigned retSize = firstType ? firstType->getCalldataEncodedSize() : 0;
	if (!_options.packDensely && retSize > 0)
		retSize = 32;
	// CALL arguments: outSize, outOff, inSize, inOff, value, addr, gas (stack top)
	m_context << u256(retSize) << u256(0) << u256(dataOffset) << u256(0);
	if (_options.obtainValue)
		_options.obtainValue();
	else
		m_context << u256(0);
	m_context << eth::dupInstruction(6); //copy contract address

	m_context << u256(25) << eth::Instruction::GAS << eth::Instruction::SUB
			  << eth::Instruction::CALL
			  << eth::Instruction::POP // @todo do not ignore failure indicator
			  << eth::Instruction::POP; // pop contract address

	if (retSize > 0)
	{
		bool const leftAligned = firstType->getCategory() == Type::Category::STRING;
		CompilerUtils(m_context).loadFromMemory(0, retSize, leftAligned);
	}
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
