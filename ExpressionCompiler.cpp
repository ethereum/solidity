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
#include <boost/range/adaptor/reversed.hpp>
#include <libdevcore/Common.h>
#include <libdevcrypto/SHA3.h>
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

void ExpressionCompiler::appendStateVariableAccessor(CompilerContext& _context, VariableDeclaration const& _varDecl, bool _optimize)
{
	ExpressionCompiler compiler(_context, _optimize);
	compiler.appendStateVariableAccessor(_varDecl);
}

bool ExpressionCompiler::visit(Assignment const& _assignment)
{
	_assignment.getRightHandSide().accept(*this);
	if (_assignment.getType()->isValueType())
		appendTypeConversion(*_assignment.getRightHandSide().getType(), *_assignment.getType());
	_assignment.getLeftHandSide().accept(*this);
	solAssert(m_currentLValue.isValid(), "LValue not retrieved.");

	Token::Value op = _assignment.getAssignmentOperator();
	if (op != Token::Assign) // compound assignment
	{
		solAssert(_assignment.getType()->isValueType(), "Compound operators not implemented for non-value types.");
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
		m_currentLValue.retrieveValue(_assignment.getType(), _assignment.getLocation(), true);
		appendOrdinaryBinaryOperatorCode(Token::AssignmentToBinaryOp(op), *_assignment.getType());
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1;
	}
	m_currentLValue.storeValue(_assignment, *_assignment.getRightHandSide().getType());
	m_currentLValue.reset();

	return false;
}

bool ExpressionCompiler::visit(UnaryOperation const& _unaryOperation)
{
	//@todo type checking and creating code for an operator should be in the same place:
	// the operator should know how to convert itself and to which types it applies, so
	// put this code together with "Type::acceptsBinary/UnaryOperator" into a class that
	// represents the operator
	if (_unaryOperation.getType()->getCategory() == Type::Category::IntegerConstant)
	{
		m_context << _unaryOperation.getType()->literalValue(nullptr);
		return false;
	}

	_unaryOperation.getSubExpression().accept(*this);

	switch (_unaryOperation.getOperator())
	{
	case Token::Not: // !
		m_context << eth::Instruction::ISZERO;
		break;
	case Token::BitNot: // ~
		m_context << eth::Instruction::NOT;
		break;
	case Token::Delete: // delete
		solAssert(m_currentLValue.isValid(), "LValue not retrieved.");
		m_currentLValue.setToZero(_unaryOperation);
		m_currentLValue.reset();
		break;
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
		solAssert(m_currentLValue.isValid(), "LValue not retrieved.");
		m_currentLValue.retrieveValue(_unaryOperation.getType(), _unaryOperation.getLocation());
		if (!_unaryOperation.isPrefixOperation())
		{
			if (m_currentLValue.storesReferenceOnStack())
				m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
			else
				m_context << eth::Instruction::DUP1;
		}
		m_context << u256(1);
		if (_unaryOperation.getOperator() == Token::Inc)
			m_context << eth::Instruction::ADD;
		else
			m_context << eth::Instruction::SWAP1 << eth::Instruction::SUB; // @todo avoid the swap
		// Stack for prefix: [ref] (*ref)+-1
		// Stack for postfix: *ref [ref] (*ref)+-1
		if (m_currentLValue.storesReferenceOnStack())
			m_context << eth::Instruction::SWAP1;
		m_currentLValue.storeValue(_unaryOperation, *_unaryOperation.getType(), !_unaryOperation.isPrefixOperation());
		m_currentLValue.reset();
		break;
	case Token::Add: // +
		// unary add, so basically no-op
		break;
	case Token::Sub: // -
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

	if (c_op == Token::And || c_op == Token::Or) // special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	else if (commonType.getCategory() == Type::Category::IntegerConstant)
		m_context << commonType.literalValue(nullptr);
	else
	{
		bool cleanupNeeded = commonType.getCategory() == Type::Category::Integer &&
								(Token::isCompareOp(c_op) || c_op == Token::Div || c_op == Token::Mod);

		// for commutative operators, push the literal as late as possible to allow improved optimization
		auto isLiteral = [](Expression const& _e)
		{
			return dynamic_cast<Literal const*>(&_e) || _e.getType()->getCategory() == Type::Category::IntegerConstant;
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
		solAssert(_functionCall.getNames().empty(), "");
		Expression const& firstArgument = *_functionCall.getArguments().front();
		firstArgument.accept(*this);
		appendTypeConversion(*firstArgument.getType(), *_functionCall.getType());
	}
	else
	{
		FunctionType const& function = dynamic_cast<FunctionType const&>(*_functionCall.getExpression().getType());
		TypePointers const& parameterTypes = function.getParameterTypes();
		vector<ASTPointer<Expression const>> const& callArguments = _functionCall.getArguments();
		vector<ASTPointer<ASTString>> const& callArgumentNames = _functionCall.getNames();
		if (!function.takesArbitraryParameters())
			solAssert(callArguments.size() == parameterTypes.size(), "");

		vector<ASTPointer<Expression const>> arguments;
		if (callArgumentNames.empty())
			// normal arguments
			arguments = callArguments;
		else
			// named arguments
			for (auto const& parameterName: function.getParameterNames())
			{
				bool found = false;
				for (size_t j = 0; j < callArgumentNames.size() && !found; j++)
					if ((found = (parameterName == *callArgumentNames[j])))
						// we found the actual parameter position
						arguments.push_back(callArguments[j]);
				solAssert(found, "");
			}

		switch (function.getLocation())
		{
		case Location::Internal:
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
		case Location::External:
		case Location::Bare:
			_functionCall.getExpression().accept(*this);
			appendExternalFunctionCall(function, arguments, function.getLocation() == Location::Bare);
			break;
		case Location::Creation:
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

			m_context << u256(bytecode.size());
			appendArgumentsCopyToMemory(arguments, function.getParameterTypes());
			// size, offset, endowment
			m_context << u256(0);
			if (function.valueSet())
				m_context << eth::dupInstruction(3);
			else
				m_context << u256(0);
			m_context << eth::Instruction::CREATE;
			if (function.valueSet())
				m_context << eth::swapInstruction(1) << eth::Instruction::POP;
			break;
		}
		case Location::SetGas:
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
		case Location::SetValue:
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.getExpression().accept(*this);
			// Note that function is not the original function, but the ".value" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			if (function.valueSet())
				m_context << eth::Instruction::POP;
			arguments.front()->accept(*this);
			break;
		case Location::Send:
			_functionCall.getExpression().accept(*this);
			m_context << u256(0); // 0 gas, we do not want to execute code
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(),
								 *function.getParameterTypes().front(), true);
			appendExternalFunctionCall(FunctionType(TypePointers{}, TypePointers{},
													Location::External, false, true, true), {}, true);
			break;
		case Location::Suicide:
			arguments.front()->accept(*this);
			appendTypeConversion(*arguments.front()->getType(), *function.getParameterTypes().front(), true);
			m_context << eth::Instruction::SUICIDE;
			break;
		case Location::SHA3:
		{
			m_context << u256(0);
			appendArgumentsCopyToMemory(arguments, TypePointers(), function.padArguments());
			m_context << u256(0) << eth::Instruction::SHA3;
			break;
		}
		case Location::Log0:
		case Location::Log1:
		case Location::Log2:
		case Location::Log3:
		case Location::Log4:
		{
			unsigned logNumber = int(function.getLocation()) - int(Location::Log0);
			for (unsigned arg = logNumber; arg > 0; --arg)
			{
				arguments[arg]->accept(*this);
				appendTypeConversion(*arguments[arg]->getType(), *function.getParameterTypes()[arg], true);
			}
			m_context << u256(0);
			appendExpressionCopyToMemory(*function.getParameterTypes().front(), *arguments.front());
			m_context << u256(0) << eth::logInstruction(logNumber);
			break;
		}
		case Location::Event:
		{
			_functionCall.getExpression().accept(*this);
			auto const& event = dynamic_cast<EventDefinition const&>(function.getDeclaration());
			unsigned numIndexed = 0;
			// All indexed arguments go to the stack
			for (unsigned arg = arguments.size(); arg > 0; --arg)
				if (event.getParameters()[arg - 1]->isIndexed())
				{
					++numIndexed;
					arguments[arg - 1]->accept(*this);
					appendTypeConversion(*arguments[arg - 1]->getType(),
										 *function.getParameterTypes()[arg - 1], true);
				}
			m_context << u256(h256::Arith(dev::sha3(function.getCanonicalSignature(event.getName()))));
			++numIndexed;
			solAssert(numIndexed <= 4, "Too many indexed arguments.");
			// Copy all non-indexed arguments to memory (data)
			m_context << u256(0);
			for (unsigned arg = 0; arg < arguments.size(); ++arg)
				if (!event.getParameters()[arg]->isIndexed())
					appendExpressionCopyToMemory(*function.getParameterTypes()[arg], *arguments[arg]);
			m_context << u256(0) << eth::logInstruction(numIndexed);
			break;
		}
		case Location::BlockHash:
		{
			arguments[0]->accept(*this);
			appendTypeConversion(*arguments[0]->getType(), *function.getParameterTypes()[0], true);
			m_context << eth::Instruction::BLOCKHASH;
			break;
		}
		case Location::ECRecover:
		case Location::SHA256:
		case Location::RIPEMD160:
		{
			static const map<Location, u256> contractAddresses{{Location::ECRecover, 1},
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

bool ExpressionCompiler::visit(NewExpression const&)
{
	// code is created for the function call (CREATION) only
	return false;
}

void ExpressionCompiler::endVisit(MemberAccess const& _memberAccess)
{
	ASTString const& member = _memberAccess.getMemberName();
	switch (_memberAccess.getExpression().getType()->getCategory())
	{
	case Type::Category::Contract:
	{
		bool alsoSearchInteger = false;
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.getExpression().getType());
		if (type.isSuper())
			m_context << m_context.getSuperFunctionEntryLabel(member, type.getContractDefinition()).pushTag();
		else
		{
			// ordinary contract type
			u256 identifier = type.getFunctionIdentifier(member);
			if (identifier != Invalid256)
			{
				appendTypeConversion(type, IntegerType(0, IntegerType::Modifier::Address), true);
				m_context << identifier;
			}
			else
				// not found in contract, search in members inherited from address
				alsoSearchInteger = true;
		}
		if (!alsoSearchInteger)
			break;
	}
	case Type::Category::Integer:
		if (member == "balance")
		{
			appendTypeConversion(*_memberAccess.getExpression().getType(),
								 IntegerType(0, IntegerType::Modifier::Address), true);
			m_context << eth::Instruction::BALANCE;
		}
		else if (member == "send" || member.substr(0, min<size_t>(member.size(), 4)) == "call")
			appendTypeConversion(*_memberAccess.getExpression().getType(),
								 IntegerType(0, IntegerType::Modifier::Address), true);
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid member access to integer."));
		break;
	case Type::Category::Function:
		solAssert(!!_memberAccess.getExpression().getType()->getMemberType(member),
				 "Invalid member access to function.");
		break;
	case Type::Category::Magic:
		// we can ignore the kind of magic and only look at the name of the member
		if (member == "coinbase")
			m_context << eth::Instruction::COINBASE;
		else if (member == "timestamp")
			m_context << eth::Instruction::TIMESTAMP;
		else if (member == "difficulty")
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
		else if (member == "data")
		{
			// nothing to store on the stack
		}
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown magic member."));
		break;
	case Type::Category::Struct:
	{
		StructType const& type = dynamic_cast<StructType const&>(*_memberAccess.getExpression().getType());
		m_context << type.getStorageOffsetOfMember(member) << eth::Instruction::ADD;
		m_currentLValue = LValue(m_context, LValue::LValueType::Storage, *_memberAccess.getType());
		m_currentLValue.retrieveValueIfLValueNotRequested(_memberAccess);
		break;
	}
	case Type::Category::TypeType:
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_memberAccess.getExpression().getType());
		if (type.getMembers().getMemberType(member))
		{
			ContractDefinition const& contract = dynamic_cast<ContractType const&>(*type.getActualType())
													.getContractDefinition();
			for (ASTPointer<FunctionDefinition> const& function: contract.getDefinedFunctions())
				if (function->getName() == member)
				{
					m_context << m_context.getFunctionEntryLabel(*function).pushTag();
					return;
				}
		}
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid member access to " + type.toString()));
	}
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Member access to unknown type."));
	}
}

bool ExpressionCompiler::visit(IndexAccess const& _indexAccess)
{
	_indexAccess.getBaseExpression().accept(*this);

	Type const& baseType = *_indexAccess.getBaseExpression().getType();
	solAssert(baseType.getCategory() == Type::Category::Mapping, "");
	Type const& keyType = *dynamic_cast<MappingType const&>(baseType).getKeyType();
	m_context << u256(0);
	appendExpressionCopyToMemory(keyType, _indexAccess.getIndexExpression());
	solAssert(baseType.getSizeOnStack() == 1,
			  "Unexpected: Not exactly one stack slot taken by subscriptable expression.");
	m_context << eth::Instruction::SWAP1;
	appendTypeMoveToMemory(IntegerType(256));
	m_context << u256(0) << eth::Instruction::SHA3;

	m_currentLValue = LValue(m_context, LValue::LValueType::Storage, *_indexAccess.getType());
	m_currentLValue.retrieveValueIfLValueNotRequested(_indexAccess);

	return false;
}

void ExpressionCompiler::endVisit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.getReferencedDeclaration();
	if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		if (magicVar->getType()->getCategory() == Type::Category::Contract)
			// "this" or "super"
			if (!dynamic_cast<ContractType const&>(*magicVar->getType()).isSuper())
				m_context << eth::Instruction::ADDRESS;
	}
	else if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
		m_context << m_context.getVirtualFunctionEntryLabel(*functionDef).pushTag();
	else if (dynamic_cast<VariableDeclaration const*>(declaration))
	{
		m_currentLValue.fromIdentifier(_identifier, *declaration);
		m_currentLValue.retrieveValueIfLValueNotRequested(_identifier);
	}
	else if (dynamic_cast<ContractDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<EventDefinition const*>(declaration))
	{
		// no-op
	}
	else
	{
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Identifier type not expected in expression context."));
	}
}

void ExpressionCompiler::endVisit(Literal const& _literal)
{
	switch (_literal.getType()->getCategory())
	{
	case Type::Category::IntegerConstant:
	case Type::Category::Bool:
	case Type::Category::String:
		m_context << _literal.getType()->literalValue(&_literal);
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Only integer, boolean and string literals implemented for now."));
	}
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation const& _binaryOperation)
{
	Token::Value const c_op = _binaryOperation.getOperator();
	solAssert(c_op == Token::Or || c_op == Token::And, "");

	_binaryOperation.getLeftExpression().accept(*this);
	m_context << eth::Instruction::DUP1;
	if (c_op == Token::And)
		m_context << eth::Instruction::ISZERO;
	eth::AssemblyItem endLabel = m_context.appendConditionalJump();
	m_context << eth::Instruction::POP;
	_binaryOperation.getRightExpression().accept(*this);
	m_context << endLabel;
}

void ExpressionCompiler::appendCompareOperatorCode(Token::Value _operator, Type const& _type)
{
	if (_operator == Token::Equal || _operator == Token::NotEqual)
	{
		m_context << eth::Instruction::EQ;
		if (_operator == Token::NotEqual)
			m_context << eth::Instruction::ISZERO;
	}
	else
	{
		IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
		bool const c_isSigned = type.isSigned();

		switch (_operator)
		{
		case Token::GreaterThanOrEqual:
			m_context << (c_isSigned ? eth::Instruction::SLT : eth::Instruction::LT)
					  << eth::Instruction::ISZERO;
			break;
		case Token::LessThanOrEqual:
			m_context << (c_isSigned ? eth::Instruction::SGT : eth::Instruction::GT)
					  << eth::Instruction::ISZERO;
			break;
		case Token::GreaterThan:
			m_context << (c_isSigned ? eth::Instruction::SGT : eth::Instruction::GT);
			break;
		case Token::LessThan:
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
	case Token::Add:
		m_context << eth::Instruction::ADD;
		break;
	case Token::Sub:
		m_context << eth::Instruction::SUB;
		break;
	case Token::Mul:
		m_context << eth::Instruction::MUL;
		break;
	case Token::Div:
		m_context  << (c_isSigned ? eth::Instruction::SDIV : eth::Instruction::DIV);
		break;
	case Token::Mod:
		m_context << (c_isSigned ? eth::Instruction::SMOD : eth::Instruction::MOD);
		break;
	case Token::Exp:
		m_context << eth::Instruction::EXP;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown arithmetic operator."));
	}
}

void ExpressionCompiler::appendBitOperatorCode(Token::Value _operator)
{
	switch (_operator)
	{
	case Token::BitOr:
		m_context << eth::Instruction::OR;
		break;
	case Token::BitAnd:
		m_context << eth::Instruction::AND;
		break;
	case Token::BitXor:
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

	if (stackTypeCategory == Type::Category::String)
	{
		StaticStringType const& typeOnStack = dynamic_cast<StaticStringType const&>(_typeOnStack);
		if (targetTypeCategory == Type::Category::Integer)
		{
			// conversion from string to hash. no need to clean the high bit
			// only to shift right because of opposite alignment
			IntegerType const& targetIntegerType = dynamic_cast<IntegerType const&>(_targetType);
			solAssert(targetIntegerType.isHash(), "Only conversion between String and Hash is allowed.");
			solAssert(targetIntegerType.getNumBits() == typeOnStack.getNumBytes() * 8, "The size should be the same.");
			m_context << (u256(1) << (256 - typeOnStack.getNumBytes() * 8)) << eth::Instruction::SWAP1 << eth::Instruction::DIV;
		}
		else
		{
			// clear lower-order bytes for conversion to shorter strings - we always clean
			solAssert(targetTypeCategory == Type::Category::String, "Invalid type conversion requested.");
			StaticStringType const& targetType = dynamic_cast<StaticStringType const&>(_targetType);
			if (targetType.getNumBytes() < typeOnStack.getNumBytes())
			{
				if (targetType.getNumBytes() == 0)
					m_context << eth::Instruction::DUP1 << eth::Instruction::XOR;
				else
					m_context << (u256(1) << (256 - targetType.getNumBytes() * 8))
							  << eth::Instruction::DUP1 << eth::Instruction::SWAP2
							  << eth::Instruction::DIV << eth::Instruction::MUL;
			}
		}
	}
	else if (stackTypeCategory == Type::Category::Integer || stackTypeCategory == Type::Category::Contract ||
			 stackTypeCategory == Type::Category::IntegerConstant)
	{
		if (targetTypeCategory == Type::Category::String && stackTypeCategory == Type::Category::Integer)
		{
			// conversion from hash to string. no need to clean the high bit
			// only to shift left because of opposite alignment
			StaticStringType const& targetStringType = dynamic_cast<StaticStringType const&>(_targetType);
			IntegerType const& typeOnStack = dynamic_cast<IntegerType const&>(_typeOnStack);
			solAssert(typeOnStack.isHash(), "Only conversion between String and Hash is allowed.");
			solAssert(typeOnStack.getNumBits() == targetStringType.getNumBytes() * 8, "The size should be the same.");
			m_context << (u256(1) << (256 - typeOnStack.getNumBits())) << eth::Instruction::MUL;
		}
		else
		{
			solAssert(targetTypeCategory == Type::Category::Integer || targetTypeCategory == Type::Category::Contract, "");
			IntegerType addressType(0, IntegerType::Modifier::Address);
			IntegerType const& targetType = targetTypeCategory == Type::Category::Integer
											? dynamic_cast<IntegerType const&>(_targetType) : addressType;
			if (stackTypeCategory == Type::Category::IntegerConstant)
			{
				IntegerConstantType const& constType = dynamic_cast<IntegerConstantType const&>(_typeOnStack);
				// We know that the stack is clean, we only have to clean for a narrowing conversion
				// where cleanup is forced.
				if (targetType.getNumBits() < constType.getIntegerType()->getNumBits() && _cleanupNeeded)
					appendHighBitsCleanup(targetType);
			}
			else
			{
				IntegerType const& typeOnStack = stackTypeCategory == Type::Category::Integer
												? dynamic_cast<IntegerType const&>(_typeOnStack) : addressType;
				// Widening: clean up according to source type width
				// Non-widening and force: clean up according to target type bits
				if (targetType.getNumBits() > typeOnStack.getNumBits())
					appendHighBitsCleanup(typeOnStack);
				else if (_cleanupNeeded)
					appendHighBitsCleanup(targetType);
			}
		}
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
	solAssert(_functionType.takesArbitraryParameters() ||
			  _arguments.size() == _functionType.getParameterTypes().size(), "");

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

	//@todo only return the first return value for now
	Type const* firstType = _functionType.getReturnParameterTypes().empty() ? nullptr :
							_functionType.getReturnParameterTypes().front().get();
	unsigned retSize = firstType ? CompilerUtils::getPaddedSize(firstType->getCalldataEncodedSize()) : 0;
	m_context << u256(retSize) << u256(0);

	if (bare)
		m_context << u256(0);
	else
	{
		// copy function identifier
		m_context << eth::dupInstruction(gasValueSize + 3);
		CompilerUtils(m_context).storeInMemory(0, IntegerType(CompilerUtils::dataStartOffset * 8));
		m_context << u256(CompilerUtils::dataStartOffset);
	}

	// For bare call, activate "4 byte pad exception": If the first argument has exactly 4 bytes,
	// do not pad it to 32 bytes.
	appendArgumentsCopyToMemory(_arguments, _functionType.getParameterTypes(),
								_functionType.padArguments(), bare);

	// CALL arguments: outSize, outOff, inSize, (already present up to here)
	// inOff, value, addr, gas (stack top)
	m_context << u256(0);
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
		bool const c_leftAligned = firstType->getCategory() == Type::Category::String;
		CompilerUtils(m_context).loadFromMemory(0, retSize, c_leftAligned, false, true);
	}
}

void ExpressionCompiler::appendArgumentsCopyToMemory(vector<ASTPointer<Expression const>> const& _arguments,
													 TypePointers const& _types,
													 bool _padToWordBoundaries,
													 bool _padExceptionIfFourBytes)
{
	solAssert(_types.empty() || _types.size() == _arguments.size(), "");
	for (size_t i = 0; i < _arguments.size(); ++i)
	{
		_arguments[i]->accept(*this);
		TypePointer const& expectedType = _types.empty() ? _arguments[i]->getType()->getRealType() : _types[i];
		appendTypeConversion(*_arguments[i]->getType(), *expectedType, true);
		bool pad = _padToWordBoundaries;
		// Do not pad if the first argument has exactly four bytes
		if (i == 0 && pad && _padExceptionIfFourBytes && expectedType->getCalldataEncodedSize() == 4)
			pad = false;
		appendTypeMoveToMemory(*expectedType, pad);
	}
}

void ExpressionCompiler::appendTypeMoveToMemory(Type const& _type, bool _padToWordBoundaries)
{
	// @TODO move this to CompilerUtils!

	if (_type.getCategory() == Type::Category::ByteArray)
	{
		auto const& type = dynamic_cast<ByteArrayType const&>(_type);
		solAssert(type.getLocation() == ByteArrayType::Location::Storage, "Non-storage byte arrays not yet implemented.");

		m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
		// stack here: memory_offset storage_offset length_bytes
		// jump to end if length is zero
		m_context << eth::Instruction::DUP1 << eth::Instruction::ISZERO;
		eth::AssemblyItem loopEnd = m_context.newTag();
		m_context.appendConditionalJumpTo(loopEnd);
		// compute memory end offset
		m_context << eth::Instruction::DUP3 << eth::Instruction::ADD << eth::Instruction::SWAP2;
		// actual array data is stored at SHA3(storage_offset)
		m_context << eth::Instruction::SWAP1;
		CompilerUtils(m_context).computeHashStatic();
		m_context << eth::Instruction::SWAP1;

		// stack here: memory_end_offset storage_data_offset memory_offset
		eth::AssemblyItem loopStart = m_context.newTag();
		m_context << loopStart
				  // load and store
				  << eth::Instruction::DUP2 << eth::Instruction::SLOAD
				  << eth::Instruction::DUP2 << eth::Instruction::MSTORE
				  // increment storage_data_offset by 1
				  << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::ADD
				  // increment memory offset by 32
				  << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD
				  // check for loop condition
				  << eth::Instruction::DUP1 << eth::Instruction::DUP4 << eth::Instruction::GT;
		m_context.appendConditionalJumpTo(loopStart);
		m_context << loopEnd << eth::Instruction::POP << eth::Instruction::POP;
	}
	else
		CompilerUtils(m_context).storeInMemoryDynamic(_type, _padToWordBoundaries);
}

void ExpressionCompiler::appendExpressionCopyToMemory(Type const& _expectedType, Expression const& _expression)
{
	_expression.accept(*this);
	appendTypeConversion(*_expression.getType(), _expectedType, true);
	appendTypeMoveToMemory(_expectedType);
}

void ExpressionCompiler::appendStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	FunctionType accessorType(_varDecl);

	unsigned length = 0;
	TypePointers const& paramTypes = accessorType.getParameterTypes();
	// move arguments to memory
	for (TypePointer const& paramType: boost::adaptors::reverse(paramTypes))
		length += CompilerUtils(m_context).storeInMemory(length, *paramType, true);

	// retrieve the position of the variable
	m_context << m_context.getStorageLocationOfVariable(_varDecl);
	TypePointer returnType = _varDecl.getType();

	for (TypePointer const& paramType: paramTypes)
	{
		// move offset to memory
		CompilerUtils(m_context).storeInMemory(length);
		unsigned argLen = CompilerUtils::getPaddedSize(paramType->getCalldataEncodedSize());
		length -= argLen;
		m_context << u256(argLen + 32) << u256(length) << eth::Instruction::SHA3;

		returnType = dynamic_cast<MappingType const&>(*returnType).getValueType();
	}

	unsigned retSizeOnStack = 0;
	solAssert(accessorType.getReturnParameterTypes().size() >= 1, "");
	if (StructType const* structType = dynamic_cast<StructType const*>(returnType.get()))
	{
		auto const& names = accessorType.getReturnParameterNames();
		auto const& types = accessorType.getReturnParameterTypes();
		// struct
		for (size_t i = 0; i < names.size(); ++i)
		{
			m_context << eth::Instruction::DUP1
					  << structType->getStorageOffsetOfMember(names[i])
					  << eth::Instruction::ADD;
			m_currentLValue = LValue(m_context, LValue::LValueType::Storage, *types[i]);
			m_currentLValue.retrieveValue(types[i], Location(), true);
			solAssert(types[i]->getSizeOnStack() == 1, "Returning struct elements with stack size != 1 not yet implemented.");
			m_context << eth::Instruction::SWAP1;
			retSizeOnStack += types[i]->getSizeOnStack();
		}
		m_context << eth::Instruction::POP;
	}
	else
	{
		// simple value
		solAssert(accessorType.getReturnParameterTypes().size() == 1, "");
		m_currentLValue = LValue(m_context, LValue::LValueType::Storage, *returnType);
		m_currentLValue.retrieveValue(returnType, Location(), true);
		retSizeOnStack = returnType->getSizeOnStack();
	}
	solAssert(retSizeOnStack <= 15, "Stack too deep.");
	m_context << eth::dupInstruction(retSizeOnStack + 1) << eth::Instruction::JUMP;
}

ExpressionCompiler::LValue::LValue(CompilerContext& _compilerContext, LValueType _type, Type const& _dataType,
								   unsigned _baseStackOffset):
	m_context(&_compilerContext), m_type(_type), m_baseStackOffset(_baseStackOffset)
{
	//@todo change the type cast for arrays
	solAssert(_dataType.getStorageSize() <= numeric_limits<unsigned>::max(), "The storage size of " +_dataType.toString() + " should fit in unsigned");
	if (m_type == LValueType::Storage)
		m_size = unsigned(_dataType.getStorageSize());
	else
		m_size = unsigned(_dataType.getSizeOnStack());
}

void ExpressionCompiler::LValue::retrieveValue(TypePointer const& _type, Location const& _location, bool _remove) const
{
	switch (m_type)
	{
	case LValueType::Stack:
	{
		unsigned stackPos = m_context->baseToCurrentStackOffset(unsigned(m_baseStackOffset));
		if (stackPos >= 15) //@todo correct this by fetching earlier or moving to memory
			BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_location)
												  << errinfo_comment("Stack too deep."));
		for (unsigned i = 0; i < m_size; ++i)
			*m_context << eth::dupInstruction(stackPos + 1);
		break;
	}
	case LValueType::Storage:
		retrieveValueFromStorage(_type, _remove);
		break;
	case LValueType::Memory:
		if (!_type->isValueType())
			break; // no distinction between value and reference for non-value types
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_location)
													  << errinfo_comment("Location type not yet implemented."));
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_location)
													  << errinfo_comment("Unsupported location type."));
		break;
	}
}

void ExpressionCompiler::LValue::retrieveValueFromStorage(TypePointer const& _type, bool _remove) const
{
	if (!_type->isValueType())
		return; // no distinction between value and reference for non-value types
	if (!_remove)
		*m_context << eth::Instruction::DUP1;
	if (m_size == 1)
		*m_context << eth::Instruction::SLOAD;
	else
		for (unsigned i = 0; i < m_size; ++i)
		{
			*m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD << eth::Instruction::SWAP1;
			if (i + 1 < m_size)
				*m_context << u256(1) << eth::Instruction::ADD;
			else
				*m_context << eth::Instruction::POP;
		}
}

void ExpressionCompiler::LValue::storeValue(Expression const& _expression, Type const& _sourceType, bool _move) const
{
	switch (m_type)
	{
	case LValueType::Stack:
	{
		unsigned stackDiff = m_context->baseToCurrentStackOffset(unsigned(m_baseStackOffset)) - m_size + 1;
		if (stackDiff > 16)
			BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
												  << errinfo_comment("Stack too deep."));
		else if (stackDiff > 0)
			for (unsigned i = 0; i < m_size; ++i)
				*m_context << eth::swapInstruction(stackDiff) << eth::Instruction::POP;
		if (!_move)
			retrieveValue(_expression.getType(), _expression.getLocation());
		break;
	}
	case LValueType::Storage:
		// stack layout: value value ... value ref
		if (_expression.getType()->isValueType())
		{
			if (!_move) // copy values
			{
				if (m_size + 1 > 16)
					BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
														  << errinfo_comment("Stack too deep."));
				for (unsigned i = 0; i < m_size; ++i)
					*m_context << eth::dupInstruction(m_size + 1) << eth::Instruction::SWAP1;
			}
			if (m_size > 0) // store high index value first
				*m_context << u256(m_size - 1) << eth::Instruction::ADD;
			for (unsigned i = 0; i < m_size; ++i)
			{
				if (i + 1 >= m_size)
					*m_context << eth::Instruction::SSTORE;
				else
					// v v ... v v r+x
					*m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2
							   << eth::Instruction::SSTORE
							   << u256(1) << eth::Instruction::SWAP1 << eth::Instruction::SUB;
			}
		}
		else
		{
			solAssert(!_move, "Move assign for non-value types not implemented.");
			solAssert(_sourceType.getCategory() == _expression.getType()->getCategory(), "");
			if (_expression.getType()->getCategory() == Type::Category::ByteArray)
				copyByteArrayToStorage(dynamic_cast<ByteArrayType const&>(*_expression.getType()),
									   dynamic_cast<ByteArrayType const&>(_sourceType));
			else if (_expression.getType()->getCategory() == Type::Category::Struct)
			{
				//@todo
				solAssert(false, "Struct copy not yet implemented.");
			}
			else
				BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_expression.getLocation())
															  << errinfo_comment("Invalid non-value type for assignment."));
		}
		break;
	case LValueType::Memory:
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

void ExpressionCompiler::LValue::setToZero(Expression const& _expression) const
{
	switch (m_type)
	{
	case LValueType::Stack:
	{
		unsigned stackDiff = m_context->baseToCurrentStackOffset(unsigned(m_baseStackOffset));
		if (stackDiff > 16)
			BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_expression.getLocation())
												  << errinfo_comment("Stack too deep."));
		solAssert(stackDiff >= m_size - 1, "");
		for (unsigned i = 0; i < m_size; ++i)
			*m_context << u256(0) << eth::swapInstruction(stackDiff + 1 - i)
						<< eth::Instruction::POP;
		break;
	}
	case LValueType::Storage:
		if (m_size == 0)
			*m_context << eth::Instruction::POP;
		for (unsigned i = 0; i < m_size; ++i)
		{
			if (i + 1 >= m_size)
				*m_context << u256(0) << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
			else
				*m_context << u256(0) << eth::Instruction::DUP2 << eth::Instruction::SSTORE
							<< u256(1) << eth::Instruction::ADD;
		}
		break;
	case LValueType::Memory:
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
		retrieveValue(_expression.getType(), _expression.getLocation(), true);
		reset();
	}
}

void ExpressionCompiler::LValue::fromStateVariable(Declaration const& _varDecl, TypePointer const& _type)
{
	m_type = LValueType::Storage;
	solAssert(_type->getStorageSize() <= numeric_limits<unsigned>::max(), "The storage size of " + _type->toString() + " should fit in an unsigned");
	*m_context << m_context->getStorageLocationOfVariable(_varDecl);
	m_size = unsigned(_type->getStorageSize());
}

void ExpressionCompiler::LValue::fromIdentifier(Identifier const& _identifier, Declaration const& _declaration)
{
	if (m_context->isLocalVariable(&_declaration))
	{
		m_type = LValueType::Stack;
		m_size = _identifier.getType()->getSizeOnStack();
		m_baseStackOffset = m_context->getBaseStackOffsetOfVariable(_declaration);
	}
	else if (m_context->isStateVariable(&_declaration))
	{
		fromStateVariable(_declaration, _identifier.getType());
	}
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_sourceLocation(_identifier.getLocation())
													  << errinfo_comment("Identifier type not supported or identifier not found."));
}

void ExpressionCompiler::LValue::copyByteArrayToStorage(ByteArrayType const& _targetType,
														ByteArrayType const& _sourceType) const
{
	// stack layout: [source_ref] target_ref (head)
	// need to leave target_ref on the stack at the end
	solAssert(m_type == LValueType::Storage, "");
	solAssert(_targetType.getLocation() == ByteArrayType::Location::Storage, "");

	// @TODO move this to CompilerUtils!

	switch (_sourceType.getLocation())
	{
	case ByteArrayType::Location::CallData:
	{
		// @todo this does not take length into account. It also assumes that after "CALLDATALENGTH" we only have zeros.
		// add some useful constants
		*m_context << u256(32) << u256(1);
		// stack here: target_ref 32 1
		// store length (in bytes)
		if (_sourceType.getOffset() == 0)
			*m_context << eth::Instruction::CALLDATASIZE;
		else
			*m_context << _sourceType.getOffset() << eth::Instruction::CALLDATASIZE << eth::Instruction::SUB;
		*m_context << eth::Instruction::DUP1 << eth::Instruction::DUP5 << eth::Instruction::SSTORE;
		// jump to end if length is zero
		*m_context << eth::Instruction::ISZERO;
		eth::AssemblyItem loopEnd = m_context->newTag();
		m_context->appendConditionalJumpTo(loopEnd);
		// actual array data is stored at SHA3(storage_offset)
		*m_context << eth::Instruction::DUP3;
		CompilerUtils(*m_context).computeHashStatic();

		*m_context << _sourceType.getOffset();
		// stack now: target_ref 32 1 target_data_ref calldata_offset
		eth::AssemblyItem loopStart = m_context->newTag();
		*m_context << loopStart
				   // copy from calldata and store
				   << eth::Instruction::DUP1 << eth::Instruction::CALLDATALOAD
				   << eth::Instruction::DUP3 << eth::Instruction::SSTORE
				   // increment target_data_ref by 1
				   << eth::Instruction::SWAP1 << eth::Instruction::DUP3 << eth::Instruction::ADD
				   // increment calldata_offset by 32
				   << eth::Instruction::SWAP1 << eth::Instruction::DUP4 << eth::Instruction::ADD
				   // check for loop condition
				   << eth::Instruction::DUP1 << eth::Instruction::CALLDATASIZE << eth::Instruction::GT;
		m_context->appendConditionalJumpTo(loopStart);
		*m_context << eth::Instruction::POP << eth::Instruction::POP;
		*m_context << loopEnd << eth::Instruction::POP << eth::Instruction::POP;
		break;
	}
	case ByteArrayType::Location::Storage:
	{
		// this copies source to target and also clears target if it was larger

		// stack: source_ref target_ref
		// fetch lengthes
		*m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD << eth::Instruction::SWAP2
				   << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
		// stack: target_len_bytes target_ref source_ref source_len_bytes
		// store new target length
		*m_context << eth::Instruction::DUP1 << eth::Instruction::DUP4 << eth::Instruction::SSTORE;
		// compute hashes (data positions)
		*m_context << eth::Instruction::SWAP2;
		CompilerUtils(*m_context).computeHashStatic();
		*m_context << eth::Instruction::SWAP1;
		CompilerUtils(*m_context).computeHashStatic();
		// stack: target_len_bytes source_len_bytes target_data_pos source_data_pos
		// convert lengthes from bytes to storage slots
		*m_context << u256(31) << u256(32) << eth::Instruction::DUP1 << eth::Instruction::DUP3
				   << eth::Instruction::DUP8 << eth::Instruction::ADD << eth::Instruction::DIV
				   << eth::Instruction::SWAP2
				   << eth::Instruction::DUP6 << eth::Instruction::ADD << eth::Instruction::DIV;
		// stack: target_len_bytes source_len_bytes target_data_pos source_data_pos target_len source_len
		// @todo we might be able to go without a third counter
		*m_context << u256(0);
		// stack: target_len_bytes source_len_bytes target_data_pos source_data_pos target_len source_len counter
		eth::AssemblyItem copyLoopStart = m_context->newTag();
		*m_context << copyLoopStart;
		// check for loop condition
		*m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3
				   << eth::Instruction::GT << eth::Instruction::ISZERO;
		eth::AssemblyItem copyLoopEnd = m_context->newTag();
		m_context->appendConditionalJumpTo(copyLoopEnd);
		// copy
		*m_context << eth::Instruction::DUP4 << eth::Instruction::DUP2 << eth::Instruction::ADD
				   << eth::Instruction::SLOAD
				   << eth::Instruction::DUP6 << eth::Instruction::DUP3 << eth::Instruction::ADD
				   << eth::Instruction::SSTORE;
		// increment
		*m_context << u256(1) << eth::Instruction::ADD;
		m_context->appendJumpTo(copyLoopStart);
		*m_context << copyLoopEnd;
		// zero-out leftovers in target
		eth::AssemblyItem zeroLoopStart = m_context->newTag();
		*m_context << zeroLoopStart;
		// check for loop condition
		*m_context << eth::Instruction::DUP1 << eth::Instruction::DUP4
				   << eth::Instruction::GT << eth::Instruction::ISZERO;
		eth::AssemblyItem zeroLoopEnd = m_context->newTag();
		m_context->appendConditionalJumpTo(zeroLoopEnd);
		// zero out
		*m_context << u256(0)
				   << eth::Instruction::DUP6 << eth::Instruction::DUP3 << eth::Instruction::ADD
				   << eth::Instruction::SSTORE;
		// increment
		*m_context << u256(1) << eth::Instruction::ADD;
		m_context->appendJumpTo(zeroLoopStart);
		// cleanup
		*m_context << zeroLoopEnd;
		*m_context << eth::Instruction::POP << eth::Instruction::POP << eth::Instruction::POP
				   << eth::Instruction::POP << eth::Instruction::POP << eth::Instruction::POP;
		break;
	}
	default:
		solAssert(false, "Given byte array location not implemented.");
	}
}

}
}
