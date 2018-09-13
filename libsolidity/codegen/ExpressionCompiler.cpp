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
 * @date 2014
 * Solidity AST to EVM bytecode compiler for expressions.
 */

#include <utility>
#include <numeric>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/SHA3.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/ExpressionCompiler.h>
#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/LValue.h>
#include <libevmasm/GasMeter.h>

#include <libdevcore/Whiskers.h>

using namespace std;

namespace dev
{
namespace solidity
{

void ExpressionCompiler::compile(Expression const& _expression)
{
	_expression.accept(*this);
}

void ExpressionCompiler::appendStateVariableInitialization(VariableDeclaration const& _varDecl)
{
	if (!_varDecl.value())
		return;
	TypePointer type = _varDecl.value()->annotation().type;
	solAssert(!!type, "Type information not available.");
	CompilerContext::LocationSetter locationSetter(m_context, _varDecl);
	_varDecl.value()->accept(*this);

	if (_varDecl.annotation().type->dataStoredIn(DataLocation::Storage))
	{
		// reference type, only convert value to mobile type and do final conversion in storeValue.
		auto mt = type->mobileType();
		solAssert(mt, "");
		utils().convertType(*type, *mt);
		type = mt;
	}
	else
	{
		utils().convertType(*type, *_varDecl.annotation().type);
		type = _varDecl.annotation().type;
	}
	StorageItem(m_context, _varDecl).storeValue(*type, _varDecl.location(), true);
}

void ExpressionCompiler::appendConstStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "");
	_varDecl.value()->accept(*this);
	utils().convertType(*_varDecl.value()->annotation().type, *_varDecl.annotation().type);

	// append return
	m_context << dupInstruction(_varDecl.annotation().type->sizeOnStack() + 1);
	m_context.appendJump(eth::AssemblyItem::JumpType::OutOfFunction);
}

void ExpressionCompiler::appendStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	solAssert(!_varDecl.isConstant(), "");
	CompilerContext::LocationSetter locationSetter(m_context, _varDecl);
	FunctionType accessorType(_varDecl);

	TypePointers paramTypes = accessorType.parameterTypes();
	m_context.adjustStackOffset(1 + CompilerUtils::sizeOnStack(paramTypes));

	// retrieve the position of the variable
	auto const& location = m_context.storageLocationOfVariable(_varDecl);
	m_context << location.first << u256(location.second);

	TypePointer returnType = _varDecl.annotation().type;

	for (size_t i = 0; i < paramTypes.size(); ++i)
	{
		if (auto mappingType = dynamic_cast<MappingType const*>(returnType.get()))
		{
			solAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");
			solUnimplementedAssert(
				!paramTypes[i]->isDynamicallySized(),
				"Accessors for mapping with dynamically-sized keys not yet implemented."
			);
			// pop offset
			m_context << Instruction::POP;
			// move storage offset to memory.
			utils().storeInMemory(32);
			// move key to memory.
			utils().copyToStackTop(paramTypes.size() - i, 1);
			utils().storeInMemory(0);
			m_context << u256(64) << u256(0) << Instruction::KECCAK256;
			// push offset
			m_context << u256(0);
			returnType = mappingType->valueType();
		}
		else if (auto arrayType = dynamic_cast<ArrayType const*>(returnType.get()))
		{
			// pop offset
			m_context << Instruction::POP;
			utils().copyToStackTop(paramTypes.size() - i + 1, 1);
			ArrayUtils(m_context).accessIndex(*arrayType);
			returnType = arrayType->baseType();
		}
		else
			solAssert(false, "Index access is allowed only for \"mapping\" and \"array\" types.");
	}
	// remove index arguments.
	if (paramTypes.size() == 1)
		m_context << Instruction::SWAP2 << Instruction::POP << Instruction::SWAP1;
	else if (paramTypes.size() >= 2)
	{
		m_context << swapInstruction(paramTypes.size());
		m_context << Instruction::POP;
		m_context << swapInstruction(paramTypes.size());
		utils().popStackSlots(paramTypes.size() - 1);
	}
	unsigned retSizeOnStack = 0;
	auto returnTypes = accessorType.returnParameterTypes();
	solAssert(returnTypes.size() >= 1, "");
	if (StructType const* structType = dynamic_cast<StructType const*>(returnType.get()))
	{
		// remove offset
		m_context << Instruction::POP;
		auto const& names = accessorType.returnParameterNames();
		// struct
		for (size_t i = 0; i < names.size(); ++i)
		{
			if (returnTypes[i]->category() == Type::Category::Mapping)
				continue;
			if (auto arrayType = dynamic_cast<ArrayType const*>(returnTypes[i].get()))
				if (!arrayType->isByteArray())
					continue;
			pair<u256, unsigned> const& offsets = structType->storageOffsetsOfMember(names[i]);
			m_context << Instruction::DUP1 << u256(offsets.first) << Instruction::ADD << u256(offsets.second);
			TypePointer memberType = structType->memberType(names[i]);
			StorageItem(m_context, *memberType).retrieveValue(SourceLocation(), true);
			utils().convertType(*memberType, *returnTypes[i]);
			utils().moveToStackTop(returnTypes[i]->sizeOnStack());
			retSizeOnStack += returnTypes[i]->sizeOnStack();
		}
		// remove slot
		m_context << Instruction::POP;
	}
	else
	{
		// simple value or array
		solAssert(returnTypes.size() == 1, "");
		StorageItem(m_context, *returnType).retrieveValue(SourceLocation(), true);
		utils().convertType(*returnType, *returnTypes.front());
		retSizeOnStack = returnTypes.front()->sizeOnStack();
	}
	solAssert(retSizeOnStack == utils().sizeOnStack(returnTypes), "");
	if (retSizeOnStack > 15)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_varDecl.location()) <<
			errinfo_comment("Stack too deep.")
		);
	m_context << dupInstruction(retSizeOnStack + 1);
	m_context.appendJump(eth::AssemblyItem::JumpType::OutOfFunction);
}

bool ExpressionCompiler::visit(Conditional const& _condition)
{
	CompilerContext::LocationSetter locationSetter(m_context, _condition);
	_condition.condition().accept(*this);
	eth::AssemblyItem trueTag = m_context.appendConditionalJump();
	_condition.falseExpression().accept(*this);
	utils().convertType(*_condition.falseExpression().annotation().type, *_condition.annotation().type);
	eth::AssemblyItem endTag = m_context.appendJumpToNew();
	m_context << trueTag;
	int offset = _condition.annotation().type->sizeOnStack();
	m_context.adjustStackOffset(-offset);
	_condition.trueExpression().accept(*this);
	utils().convertType(*_condition.trueExpression().annotation().type, *_condition.annotation().type);
	m_context << endTag;
	return false;
}

bool ExpressionCompiler::visit(Assignment const& _assignment)
{
	CompilerContext::LocationSetter locationSetter(m_context, _assignment);
	Token::Value op = _assignment.assignmentOperator();
	Token::Value binOp = op == Token::Assign ? op : Token::AssignmentToBinaryOp(op);
	Type const& leftType = *_assignment.leftHandSide().annotation().type;
	if (leftType.category() == Type::Category::Tuple)
	{
		solAssert(*_assignment.annotation().type == TupleType(), "");
		solAssert(op == Token::Assign, "");
	}
	else
		solAssert(*_assignment.annotation().type == leftType, "");
	bool cleanupNeeded = false;
	if (op != Token::Assign)
		cleanupNeeded = cleanupNeededForOp(leftType.category(), binOp);
	_assignment.rightHandSide().accept(*this);
	// Perform some conversion already. This will convert storage types to memory and literals
	// to their actual type, but will not convert e.g. memory to storage.
	TypePointer rightIntermediateType;
	if (op != Token::Assign && Token::isShiftOp(binOp))
		rightIntermediateType = _assignment.rightHandSide().annotation().type->mobileType();
	else
		rightIntermediateType = _assignment.rightHandSide().annotation().type->closestTemporaryType(
			_assignment.leftHandSide().annotation().type
		);
	solAssert(rightIntermediateType, "");
	utils().convertType(*_assignment.rightHandSide().annotation().type, *rightIntermediateType, cleanupNeeded);

	_assignment.leftHandSide().accept(*this);
	solAssert(!!m_currentLValue, "LValue not retrieved.");

	if (op == Token::Assign)
		m_currentLValue->storeValue(*rightIntermediateType, _assignment.location());
	else  // compound assignment
	{
		solAssert(leftType.isValueType(), "Compound operators only available for value types.");
		unsigned lvalueSize = m_currentLValue->sizeOnStack();
		unsigned itemSize = _assignment.annotation().type->sizeOnStack();
		if (lvalueSize > 0)
		{
			utils().copyToStackTop(lvalueSize + itemSize, itemSize);
			utils().copyToStackTop(itemSize + lvalueSize, lvalueSize);
			// value lvalue_ref value lvalue_ref
		}
		m_currentLValue->retrieveValue(_assignment.location(), true);
		utils().convertType(leftType, leftType, cleanupNeeded);

		if (Token::isShiftOp(binOp))
			appendShiftOperatorCode(binOp, leftType, *rightIntermediateType);
		else
		{
			solAssert(leftType == *rightIntermediateType, "");
			appendOrdinaryBinaryOperatorCode(binOp, leftType);
		}
		if (lvalueSize > 0)
		{
			if (itemSize + lvalueSize > 16)
				BOOST_THROW_EXCEPTION(
					CompilerError() <<
					errinfo_sourceLocation(_assignment.location()) <<
					errinfo_comment("Stack too deep, try removing local variables.")
				);
			// value [lvalue_ref] updated_value
			for (unsigned i = 0; i < itemSize; ++i)
				m_context << swapInstruction(itemSize + lvalueSize) << Instruction::POP;
		}
		m_currentLValue->storeValue(*_assignment.annotation().type, _assignment.location());
	}
	m_currentLValue.reset();
	return false;
}

bool ExpressionCompiler::visit(TupleExpression const& _tuple)
{
	if (_tuple.isInlineArray())
	{
		ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*_tuple.annotation().type);

		solAssert(!arrayType.isDynamicallySized(), "Cannot create dynamically sized inline array.");
		m_context << max(u256(32u), arrayType.memorySize());
		utils().allocateMemory();
		m_context << Instruction::DUP1;

		for (auto const& component: _tuple.components())
		{
			component->accept(*this);
			utils().convertType(*component->annotation().type, *arrayType.baseType(), true);
			utils().storeInMemoryDynamic(*arrayType.baseType(), true);
		}

		m_context << Instruction::POP;
	}
	else
	{
		vector<unique_ptr<LValue>> lvalues;
		for (auto const& component: _tuple.components())
			if (component)
			{
				component->accept(*this);
				if (_tuple.annotation().lValueRequested)
				{
					solAssert(!!m_currentLValue, "");
					lvalues.push_back(move(m_currentLValue));
				}
			}
			else if (_tuple.annotation().lValueRequested)
				lvalues.push_back(unique_ptr<LValue>());
		if (_tuple.annotation().lValueRequested)
		{
			if (_tuple.components().size() == 1)
				m_currentLValue = move(lvalues[0]);
			else
				m_currentLValue.reset(new TupleObject(m_context, move(lvalues)));
		}
	}
	return false;
}

bool ExpressionCompiler::visit(UnaryOperation const& _unaryOperation)
{
	CompilerContext::LocationSetter locationSetter(m_context, _unaryOperation);
	if (_unaryOperation.annotation().type->category() == Type::Category::RationalNumber)
	{
		m_context << _unaryOperation.annotation().type->literalValue(nullptr);
		return false;
	}

	_unaryOperation.subExpression().accept(*this);

	switch (_unaryOperation.getOperator())
	{
	case Token::Not: // !
		m_context << Instruction::ISZERO;
		break;
	case Token::BitNot: // ~
		m_context << Instruction::NOT;
		break;
	case Token::Delete: // delete
		solAssert(!!m_currentLValue, "LValue not retrieved.");
		m_currentLValue->setToZero(_unaryOperation.location());
		m_currentLValue.reset();
		break;
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
		solAssert(!!m_currentLValue, "LValue not retrieved.");
		solUnimplementedAssert(
			_unaryOperation.annotation().type->category() != Type::Category::FixedPoint,
			"Not yet implemented - FixedPointType."
		);
		m_currentLValue->retrieveValue(_unaryOperation.location());
		if (!_unaryOperation.isPrefixOperation())
		{
			// store value for later
			solUnimplementedAssert(_unaryOperation.annotation().type->sizeOnStack() == 1, "Stack size != 1 not implemented.");
			m_context << Instruction::DUP1;
			if (m_currentLValue->sizeOnStack() > 0)
				for (unsigned i = 1 + m_currentLValue->sizeOnStack(); i > 0; --i)
					m_context << swapInstruction(i);
		}
		m_context << u256(1);
		if (_unaryOperation.getOperator() == Token::Inc)
			m_context << Instruction::ADD;
		else
			m_context << Instruction::SWAP1 << Instruction::SUB;
		// Stack for prefix: [ref...] (*ref)+-1
		// Stack for postfix: *ref [ref...] (*ref)+-1
		for (unsigned i = m_currentLValue->sizeOnStack(); i > 0; --i)
			m_context << swapInstruction(i);
		m_currentLValue->storeValue(
			*_unaryOperation.annotation().type, _unaryOperation.location(),
			!_unaryOperation.isPrefixOperation());
		m_currentLValue.reset();
		break;
	case Token::Add: // +
		// unary add, so basically no-op
		break;
	case Token::Sub: // -
		m_context << u256(0) << Instruction::SUB;
		break;
	default:
		solAssert(false, "Invalid unary operator: " + string(Token::toString(_unaryOperation.getOperator())));
	}
	return false;
}

bool ExpressionCompiler::visit(BinaryOperation const& _binaryOperation)
{
	CompilerContext::LocationSetter locationSetter(m_context, _binaryOperation);
	Expression const& leftExpression = _binaryOperation.leftExpression();
	Expression const& rightExpression = _binaryOperation.rightExpression();
	solAssert(!!_binaryOperation.annotation().commonType, "");
	TypePointer const& commonType = _binaryOperation.annotation().commonType;
	Token::Value const c_op = _binaryOperation.getOperator();

	if (c_op == Token::And || c_op == Token::Or) // special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	else if (commonType->category() == Type::Category::RationalNumber)
		m_context << commonType->literalValue(nullptr);
	else
	{
		bool cleanupNeeded = cleanupNeededForOp(commonType->category(), c_op);

		TypePointer leftTargetType = commonType;
		TypePointer rightTargetType = Token::isShiftOp(c_op) ? rightExpression.annotation().type->mobileType() : commonType;
		solAssert(rightTargetType, "");

		// for commutative operators, push the literal as late as possible to allow improved optimization
		auto isLiteral = [](Expression const& _e)
		{
			return dynamic_cast<Literal const*>(&_e) || _e.annotation().type->category() == Type::Category::RationalNumber;
		};
		bool swap = m_optimize && Token::isCommutativeOp(c_op) && isLiteral(rightExpression) && !isLiteral(leftExpression);
		if (swap)
		{
			leftExpression.accept(*this);
			utils().convertType(*leftExpression.annotation().type, *leftTargetType, cleanupNeeded);
			rightExpression.accept(*this);
			utils().convertType(*rightExpression.annotation().type, *rightTargetType, cleanupNeeded);
		}
		else
		{
			rightExpression.accept(*this);
			utils().convertType(*rightExpression.annotation().type, *rightTargetType, cleanupNeeded);
			leftExpression.accept(*this);
			utils().convertType(*leftExpression.annotation().type, *leftTargetType, cleanupNeeded);
		}
		if (Token::isShiftOp(c_op))
			// shift only cares about the signedness of both sides
			appendShiftOperatorCode(c_op, *leftTargetType, *rightTargetType);
		else if (Token::isCompareOp(c_op))
			appendCompareOperatorCode(c_op, *commonType);
		else
			appendOrdinaryBinaryOperatorCode(c_op, *commonType);
	}

	// do not visit the child nodes, we already did that explicitly
	return false;
}

bool ExpressionCompiler::visit(FunctionCall const& _functionCall)
{
	CompilerContext::LocationSetter locationSetter(m_context, _functionCall);
	if (_functionCall.annotation().kind == FunctionCallKind::TypeConversion)
	{
		solAssert(_functionCall.arguments().size() == 1, "");
		solAssert(_functionCall.names().empty(), "");
		Expression const& firstArgument = *_functionCall.arguments().front();
		firstArgument.accept(*this);
		utils().convertType(*firstArgument.annotation().type, *_functionCall.annotation().type);
		return false;
	}

	FunctionTypePointer functionType;
	if (_functionCall.annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		auto const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());
		functionType = structType.constructorType();
	}
	else
		functionType = dynamic_pointer_cast<FunctionType const>(_functionCall.expression().annotation().type);

	TypePointers parameterTypes = functionType->parameterTypes();
	vector<ASTPointer<Expression const>> const& callArguments = _functionCall.arguments();
	vector<ASTPointer<ASTString>> const& callArgumentNames = _functionCall.names();
	if (!functionType->takesArbitraryParameters())
		solAssert(callArguments.size() == parameterTypes.size(), "");

	vector<ASTPointer<Expression const>> arguments;
	if (callArgumentNames.empty())
		// normal arguments
		arguments = callArguments;
	else
		// named arguments
		for (auto const& parameterName: functionType->parameterNames())
		{
			bool found = false;
			for (size_t j = 0; j < callArgumentNames.size() && !found; j++)
				if ((found = (parameterName == *callArgumentNames[j])))
					// we found the actual parameter position
					arguments.push_back(callArguments[j]);
			solAssert(found, "");
		}

	if (_functionCall.annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());

		m_context << max(u256(32u), structType.memorySize());
		utils().allocateMemory();
		m_context << Instruction::DUP1;

		for (unsigned i = 0; i < arguments.size(); ++i)
		{
			arguments[i]->accept(*this);
			utils().convertType(*arguments[i]->annotation().type, *functionType->parameterTypes()[i]);
			utils().storeInMemoryDynamic(*functionType->parameterTypes()[i]);
		}
		m_context << Instruction::POP;
	}
	else
	{
		FunctionType const& function = *functionType;
		if (function.bound())
			// Only delegatecall and internal functions can be bound, this might be lifted later.
			solAssert(function.kind() == FunctionType::Kind::DelegateCall || function.kind() == FunctionType::Kind::Internal, "");
		switch (function.kind())
		{
		case FunctionType::Kind::Internal:
		{
			// Calling convention: Caller pushes return address and arguments
			// Callee removes them and pushes return values

			eth::AssemblyItem returnLabel = m_context.pushNewTag();
			for (unsigned i = 0; i < arguments.size(); ++i)
			{
				arguments[i]->accept(*this);
				utils().convertType(*arguments[i]->annotation().type, *function.parameterTypes()[i]);
			}

			{
				bool shortcutTaken = false;
				if (auto identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
					if (auto functionDef = dynamic_cast<FunctionDefinition const*>(identifier->annotation().referencedDeclaration))
					{
						// Do not directly visit the identifier, because this way, we can avoid
						// the runtime entry label to be created at the creation time context.
						CompilerContext::LocationSetter locationSetter2(m_context, *identifier);
						utils().pushCombinedFunctionEntryLabel(m_context.resolveVirtualFunction(*functionDef), false);
						shortcutTaken = true;
					}

				if (!shortcutTaken)
					_functionCall.expression().accept(*this);
			}

			unsigned parameterSize = CompilerUtils::sizeOnStack(function.parameterTypes());
			if (function.bound())
			{
				// stack: arg2, ..., argn, label, arg1
				unsigned depth = parameterSize + 1;
				utils().moveIntoStack(depth, function.selfType()->sizeOnStack());
				parameterSize += function.selfType()->sizeOnStack();
			}

			if (m_context.runtimeContext())
				// We have a runtime context, so we need the creation part.
				utils().rightShiftNumberOnStack(32);
			else
				// Extract the runtime part.
				m_context << ((u256(1) << 32) - 1) << Instruction::AND;

			m_context.appendJump(eth::AssemblyItem::JumpType::IntoFunction);
			m_context << returnLabel;

			unsigned returnParametersSize = CompilerUtils::sizeOnStack(function.returnParameterTypes());
			// callee adds return parameters, but removes arguments and return label
			m_context.adjustStackOffset(returnParametersSize - parameterSize - 1);
			break;
		}
		case FunctionType::Kind::External:
		case FunctionType::Kind::DelegateCall:
		case FunctionType::Kind::BareCall:
		case FunctionType::Kind::BareCallCode:
		case FunctionType::Kind::BareDelegateCall:
		case FunctionType::Kind::BareStaticCall:
			_functionCall.expression().accept(*this);
			appendExternalFunctionCall(function, arguments);
			break;
		case FunctionType::Kind::Creation:
		{
			_functionCall.expression().accept(*this);
			solAssert(!function.gasSet(), "Gas limit set for contract creation.");
			solAssert(function.returnParameterTypes().size() == 1, "");
			TypePointers argumentTypes;
			for (auto const& arg: arguments)
			{
				arg->accept(*this);
				argumentTypes.push_back(arg->annotation().type);
			}
			ContractDefinition const* contract =
				&dynamic_cast<ContractType const&>(*function.returnParameterTypes().front()).contractDefinition();
			m_context.callLowLevelFunction(
				"$copyContractCreationCodeToMemory_" + contract->type()->identifier(),
				0,
				1,
				[contract](CompilerContext& _context)
				{
					// copy the contract's code into memory
					eth::Assembly const& assembly = _context.compiledContract(*contract);
					CompilerUtils(_context).fetchFreeMemoryPointer();
					// pushes size
					auto subroutine = _context.addSubroutine(make_shared<eth::Assembly>(assembly));
					_context << Instruction::DUP1 << subroutine;
					_context << Instruction::DUP4 << Instruction::CODECOPY;
					_context << Instruction::ADD;
				}
			);
			utils().abiEncode(argumentTypes, function.parameterTypes());
			// now on stack: memory_end_ptr
			// need: size, offset, endowment
			utils().toSizeAfterFreeMemoryPointer();
			if (function.valueSet())
				m_context << dupInstruction(3);
			else
				m_context << u256(0);
			m_context << Instruction::CREATE;
			// Check if zero (out of stack or not enough balance).
			m_context << Instruction::DUP1 << Instruction::ISZERO;
			// TODO: Can we bubble up here? There might be different reasons for failure, I think.
			m_context.appendConditionalRevert(true);
			if (function.valueSet())
				m_context << swapInstruction(1) << Instruction::POP;
			break;
		}
		case FunctionType::Kind::SetGas:
		{
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.expression().accept(*this);

			arguments.front()->accept(*this);
			utils().convertType(*arguments.front()->annotation().type, IntegerType(256), true);
			// Note that function is not the original function, but the ".gas" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			unsigned stackDepth = (function.gasSet() ? 1 : 0) + (function.valueSet() ? 1 : 0);
			if (stackDepth > 0)
				m_context << swapInstruction(stackDepth);
			if (function.gasSet())
				m_context << Instruction::POP;
			break;
		}
		case FunctionType::Kind::SetValue:
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.expression().accept(*this);
			// Note that function is not the original function, but the ".value" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			if (function.valueSet())
				m_context << Instruction::POP;
			arguments.front()->accept(*this);
			break;
		case FunctionType::Kind::Send:
		case FunctionType::Kind::Transfer:
			_functionCall.expression().accept(*this);
			// Provide the gas stipend manually at first because we may send zero ether.
			// Will be zeroed if we send more than zero ether.
			m_context << u256(eth::GasCosts::callStipend);
			arguments.front()->accept(*this);
			utils().convertType(
				*arguments.front()->annotation().type,
				*function.parameterTypes().front(), true
			);
			// gas <- gas * !value
			m_context << Instruction::SWAP1 << Instruction::DUP2;
			m_context << Instruction::ISZERO << Instruction::MUL << Instruction::SWAP1;
			appendExternalFunctionCall(
				FunctionType(
					TypePointers{},
					TypePointers{},
					strings(),
					strings(),
					FunctionType::Kind::BareCall,
					false,
					StateMutability::NonPayable,
					nullptr,
					true,
					true
				),
				{}
			);
			if (function.kind() == FunctionType::Kind::Transfer)
			{
				// Check if zero (out of stack or not enough balance).
				// TODO: bubble up here, but might also be different error.
				m_context << Instruction::ISZERO;
				m_context.appendConditionalRevert(true);
			}
			break;
		case FunctionType::Kind::Selfdestruct:
			arguments.front()->accept(*this);
			utils().convertType(*arguments.front()->annotation().type, *function.parameterTypes().front(), true);
			m_context << Instruction::SELFDESTRUCT;
			break;
		case FunctionType::Kind::Revert:
		{
			if (!arguments.empty())
			{
				// function-sel(Error(string)) + encoding
				solAssert(arguments.size() == 1, "");
				solAssert(function.parameterTypes().size() == 1, "");
				arguments.front()->accept(*this);
				utils().revertWithStringData(*arguments.front()->annotation().type);
			}
			else
				m_context.appendRevert();
			break;
		}
		case FunctionType::Kind::KECCAK256:
		{
			solAssert(arguments.size() == 1, "");
			solAssert(!function.padArguments(), "");
			TypePointer const& argType = arguments.front()->annotation().type;
			solAssert(argType, "");
			arguments.front()->accept(*this);
			// Optimization: If type is bytes or string, then do not encode,
			// but directly compute keccak256 on memory.
			if (*argType == ArrayType(DataLocation::Memory) || *argType == ArrayType(DataLocation::Memory, true))
			{
				ArrayUtils(m_context).retrieveLength(ArrayType(DataLocation::Memory));
				m_context << Instruction::SWAP1 << u256(0x20) << Instruction::ADD;
			}
			else
			{
				utils().fetchFreeMemoryPointer();
				utils().packedEncode({argType}, TypePointers());
				utils().toSizeAfterFreeMemoryPointer();
			}
			m_context << Instruction::KECCAK256;
			break;
		}
		case FunctionType::Kind::Log0:
		case FunctionType::Kind::Log1:
		case FunctionType::Kind::Log2:
		case FunctionType::Kind::Log3:
		case FunctionType::Kind::Log4:
		{
			unsigned logNumber = int(function.kind()) - int(FunctionType::Kind::Log0);
			for (unsigned arg = logNumber; arg > 0; --arg)
			{
				arguments[arg]->accept(*this);
				utils().convertType(*arguments[arg]->annotation().type, *function.parameterTypes()[arg], true);
			}
			arguments.front()->accept(*this);
			utils().fetchFreeMemoryPointer();
			utils().packedEncode(
				{arguments.front()->annotation().type},
				{function.parameterTypes().front()}
			);
			utils().toSizeAfterFreeMemoryPointer();
			m_context << logInstruction(logNumber);
			break;
		}
		case FunctionType::Kind::Event:
		{
			_functionCall.expression().accept(*this);
			auto const& event = dynamic_cast<EventDefinition const&>(function.declaration());
			unsigned numIndexed = 0;
			// All indexed arguments go to the stack
			for (unsigned arg = arguments.size(); arg > 0; --arg)
				if (event.parameters()[arg - 1]->isIndexed())
				{
					++numIndexed;
					arguments[arg - 1]->accept(*this);
					if (auto const& arrayType = dynamic_pointer_cast<ArrayType const>(function.parameterTypes()[arg - 1]))
					{
						utils().fetchFreeMemoryPointer();
						utils().packedEncode(
							{arguments[arg - 1]->annotation().type},
							{arrayType}
						);
						utils().toSizeAfterFreeMemoryPointer();
						m_context << Instruction::KECCAK256;
					}
					else
						utils().convertType(
							*arguments[arg - 1]->annotation().type,
							*function.parameterTypes()[arg - 1],
							true
						);
				}
			if (!event.isAnonymous())
			{
				m_context << u256(h256::Arith(dev::keccak256(function.externalSignature())));
				++numIndexed;
			}
			solAssert(numIndexed <= 4, "Too many indexed arguments.");
			// Copy all non-indexed arguments to memory (data)
			// Memory position is only a hack and should be removed once we have free memory pointer.
			TypePointers nonIndexedArgTypes;
			TypePointers nonIndexedParamTypes;
			for (unsigned arg = 0; arg < arguments.size(); ++arg)
				if (!event.parameters()[arg]->isIndexed())
				{
					arguments[arg]->accept(*this);
					nonIndexedArgTypes.push_back(arguments[arg]->annotation().type);
					nonIndexedParamTypes.push_back(function.parameterTypes()[arg]);
				}
			utils().fetchFreeMemoryPointer();
			utils().abiEncode(nonIndexedArgTypes, nonIndexedParamTypes);
			// need: topic1 ... topicn memsize memstart
			utils().toSizeAfterFreeMemoryPointer();
			m_context << logInstruction(numIndexed);
			break;
		}
		case FunctionType::Kind::BlockHash:
		{
			arguments[0]->accept(*this);
			utils().convertType(*arguments[0]->annotation().type, *function.parameterTypes()[0], true);
			m_context << Instruction::BLOCKHASH;
			break;
		}
		case FunctionType::Kind::AddMod:
		case FunctionType::Kind::MulMod:
		{
			arguments[2]->accept(*this);
			utils().convertType(*arguments[2]->annotation().type, IntegerType(256));
			m_context << Instruction::DUP1 << Instruction::ISZERO;
			m_context.appendConditionalInvalid();
			for (unsigned i = 1; i < 3; i ++)
			{
				arguments[2 - i]->accept(*this);
				utils().convertType(*arguments[2 - i]->annotation().type, IntegerType(256));
			}
			if (function.kind() == FunctionType::Kind::AddMod)
				m_context << Instruction::ADDMOD;
			else
				m_context << Instruction::MULMOD;
			break;
		}
		case FunctionType::Kind::ECRecover:
		case FunctionType::Kind::SHA256:
		case FunctionType::Kind::RIPEMD160:
		{
			_functionCall.expression().accept(*this);
			static const map<FunctionType::Kind, u256> contractAddresses{{FunctionType::Kind::ECRecover, 1},
															   {FunctionType::Kind::SHA256, 2},
															   {FunctionType::Kind::RIPEMD160, 3}};
			m_context << contractAddresses.find(function.kind())->second;
			for (unsigned i = function.sizeOnStack(); i > 0; --i)
				m_context << swapInstruction(i);
			appendExternalFunctionCall(function, arguments);
			break;
		}
		case FunctionType::Kind::ByteArrayPush:
		case FunctionType::Kind::ArrayPush:
		{
			_functionCall.expression().accept(*this);
			solAssert(function.parameterTypes().size() == 1, "");
			solAssert(!!function.parameterTypes()[0], "");
			TypePointer paramType = function.parameterTypes()[0];
			shared_ptr<ArrayType> arrayType =
				function.kind() == FunctionType::Kind::ArrayPush ?
				make_shared<ArrayType>(DataLocation::Storage, paramType) :
				make_shared<ArrayType>(DataLocation::Storage);

			// stack: ArrayReference
			arguments[0]->accept(*this);
			TypePointer const& argType = arguments[0]->annotation().type;
			// stack: ArrayReference argValue
			utils().moveToStackTop(argType->sizeOnStack(), 1);
			// stack: argValue ArrayReference
			m_context << Instruction::DUP1;
			ArrayUtils(m_context).incrementDynamicArraySize(*arrayType);
			// stack: argValue ArrayReference newLength
			m_context << Instruction::SWAP1;
			// stack: argValue newLength ArrayReference
			m_context << u256(1) << Instruction::DUP3 << Instruction::SUB;
			// stack: argValue newLength ArrayReference (newLength-1)
			ArrayUtils(m_context).accessIndex(*arrayType, false);
			// stack: argValue newLength storageSlot slotOffset
			utils().moveToStackTop(3, argType->sizeOnStack());
			// stack: newLength storageSlot slotOffset argValue
			TypePointer type = arguments[0]->annotation().type->closestTemporaryType(arrayType->baseType());
			solAssert(type, "");
			utils().convertType(*argType, *type);
			utils().moveToStackTop(1 + type->sizeOnStack());
			utils().moveToStackTop(1 + type->sizeOnStack());
			// stack: newLength argValue storageSlot slotOffset
			if (function.kind() == FunctionType::Kind::ArrayPush)
				StorageItem(m_context, *paramType).storeValue(*type, _functionCall.location(), true);
			else
				StorageByteArrayElement(m_context).storeValue(*type, _functionCall.location(), true);
			break;
		}
		case FunctionType::Kind::ArrayPop:
		{
			_functionCall.expression().accept(*this);
			solAssert(function.parameterTypes().empty(), "");

			ArrayType const& arrayType = dynamic_cast<ArrayType const&>(
				*dynamic_cast<MemberAccess const&>(_functionCall.expression()).expression().annotation().type
			);
			solAssert(arrayType.dataStoredIn(DataLocation::Storage), "");

			ArrayUtils(m_context).popStorageArrayElement(arrayType);
			break;
		}
		case FunctionType::Kind::ObjectCreation:
		{
			ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*_functionCall.annotation().type);
			_functionCall.expression().accept(*this);
			solAssert(arguments.size() == 1, "");

			// Fetch requested length.
			arguments[0]->accept(*this);
			utils().convertType(*arguments[0]->annotation().type, IntegerType(256));

			// Stack: requested_length
			utils().fetchFreeMemoryPointer();

			// Stack: requested_length memptr
			m_context << Instruction::SWAP1;
			// Stack: memptr requested_length
			// store length
			m_context << Instruction::DUP1 << Instruction::DUP3 << Instruction::MSTORE;
			// Stack: memptr requested_length
			// update free memory pointer
			m_context << Instruction::DUP1;
			// Stack: memptr requested_length requested_length
			if (arrayType.isByteArray())
				// Round up to multiple of 32
				m_context << u256(31) << Instruction::ADD << u256(31) << Instruction::NOT << Instruction::AND;
			else
				m_context << arrayType.baseType()->memoryHeadSize() << Instruction::MUL;
			// stacK: memptr requested_length data_size
			m_context << u256(32) << Instruction::ADD;
			m_context << Instruction::DUP3 << Instruction::ADD;
			utils().storeFreeMemoryPointer();
			// Stack: memptr requested_length

			// Check if length is zero
			m_context << Instruction::DUP1 << Instruction::ISZERO;
			auto skipInit = m_context.appendConditionalJump();
			// Always initialize because the free memory pointer might point at
			// a dirty memory area.
			m_context << Instruction::DUP2 << u256(32) << Instruction::ADD;
			utils().zeroInitialiseMemoryArray(arrayType);
			m_context << skipInit;
			m_context << Instruction::POP;
			break;
		}
		case FunctionType::Kind::Assert:
		case FunctionType::Kind::Require:
		{
			arguments.front()->accept(*this);
			utils().convertType(*arguments.front()->annotation().type, *function.parameterTypes().front(), false);
			if (arguments.size() > 1)
			{
				// Users probably expect the second argument to be evaluated
				// even if the condition is false, as would be the case for an actual
				// function call.
				solAssert(arguments.size() == 2, "");
				solAssert(function.kind() == FunctionType::Kind::Require, "");
				arguments.at(1)->accept(*this);
				utils().moveIntoStack(1, arguments.at(1)->annotation().type->sizeOnStack());
			}
			// Stack: <error string (unconverted)> <condition>
			// jump if condition was met
			m_context << Instruction::ISZERO << Instruction::ISZERO;
			auto success = m_context.appendConditionalJump();
			if (function.kind() == FunctionType::Kind::Assert)
				// condition was not met, flag an error
				m_context.appendInvalid();
			else if (arguments.size() > 1)
			{
				utils().revertWithStringData(*arguments.at(1)->annotation().type);
				// Here, the argument is consumed, but in the other branch, it is still there.
				m_context.adjustStackOffset(arguments.at(1)->annotation().type->sizeOnStack());
			}
			else
				m_context.appendRevert();
			// the success branch
			m_context << success;
			if (arguments.size() > 1)
				utils().popStackElement(*arguments.at(1)->annotation().type);
			break;
		}
		case FunctionType::Kind::ABIEncode:
		case FunctionType::Kind::ABIEncodePacked:
		case FunctionType::Kind::ABIEncodeWithSelector:
		case FunctionType::Kind::ABIEncodeWithSignature:
		{
			bool const isPacked = function.kind() == FunctionType::Kind::ABIEncodePacked;
			bool const hasSelectorOrSignature =
				function.kind() == FunctionType::Kind::ABIEncodeWithSelector ||
				function.kind() == FunctionType::Kind::ABIEncodeWithSignature;

			TypePointers argumentTypes;
			TypePointers targetTypes;
			for (unsigned i = 0; i < arguments.size(); ++i)
			{
				arguments[i]->accept(*this);
				// Do not keep the selector as part of the ABI encoded args
				if (!hasSelectorOrSignature || i > 0)
					argumentTypes.push_back(arguments[i]->annotation().type);
			}
			utils().fetchFreeMemoryPointer();
			// stack now: [<selector>] <arg1> .. <argN> <free_mem>

			// adjust by 32(+4) bytes to accommodate the length(+selector)
			m_context << u256(32 + (hasSelectorOrSignature ? 4 : 0)) << Instruction::ADD;
			// stack now: [<selector>] <arg1> .. <argN> <data_encoding_area_start>

			if (isPacked)
			{
				solAssert(!function.padArguments(), "");
				utils().packedEncode(argumentTypes, TypePointers());
			}
			else
			{
				solAssert(function.padArguments(), "");
				utils().abiEncode(argumentTypes, TypePointers());
			}
			utils().fetchFreeMemoryPointer();
			// stack: [<selector>] <data_encoding_area_end> <bytes_memory_ptr>

			// size is end minus start minus length slot
			m_context.appendInlineAssembly(R"({
				mstore(mem_ptr, sub(sub(mem_end, mem_ptr), 0x20))
			})", {"mem_end", "mem_ptr"});
			m_context << Instruction::SWAP1;
			utils().storeFreeMemoryPointer();
			// stack: [<selector>] <memory ptr>

			if (hasSelectorOrSignature)
			{
				// stack: <selector> <memory pointer>
				solAssert(arguments.size() >= 1, "");
				TypePointer const& selectorType = arguments[0]->annotation().type;
				utils().moveIntoStack(selectorType->sizeOnStack());
				TypePointer dataOnStack = selectorType;
				// stack: <memory pointer> <selector>
				if (function.kind() == FunctionType::Kind::ABIEncodeWithSignature)
				{
					// hash the signature
					if (auto const* stringType = dynamic_cast<StringLiteralType const*>(selectorType.get()))
					{
						FixedHash<4> hash(dev::keccak256(stringType->value()));
						m_context << (u256(FixedHash<4>::Arith(hash)) << (256 - 32));
						dataOnStack = make_shared<FixedBytesType>(4);
					}
					else
					{
						utils().fetchFreeMemoryPointer();
						// stack: <memory pointer> <selector> <free mem ptr>
						utils().packedEncode(TypePointers{selectorType}, TypePointers());
						utils().toSizeAfterFreeMemoryPointer();
						m_context << Instruction::KECCAK256;
						// stack: <memory pointer> <hash>

						dataOnStack = make_shared<FixedBytesType>(32);
					}
				}
				else
				{
					solAssert(function.kind() == FunctionType::Kind::ABIEncodeWithSelector, "");
				}

				utils().convertType(*dataOnStack, FixedBytesType(4), true);

				// stack: <memory pointer> <selector>

				// load current memory, mask and combine the selector
				string mask = formatNumber((u256(-1) >> 32));
				m_context.appendInlineAssembly(R"({
					let data_start := add(mem_ptr, 0x20)
					let data := mload(data_start)
					let mask := )" + mask + R"(
					mstore(data_start, or(and(data, mask), selector))
				})", {"mem_ptr", "selector"});
				m_context << Instruction::POP;
			}

			// stack now: <memory pointer>
			break;
		}
		case FunctionType::Kind::ABIDecode:
		{
			arguments.front()->accept(*this);
			TypePointer firstArgType = arguments.front()->annotation().type;
			TypePointers targetTypes;
			if (TupleType const* targetTupleType = dynamic_cast<TupleType const*>(_functionCall.annotation().type.get()))
				targetTypes = targetTupleType->components();
			else
				targetTypes = TypePointers{_functionCall.annotation().type};
			if (
				*firstArgType == ArrayType(DataLocation::CallData) ||
				*firstArgType == ArrayType(DataLocation::CallData, true)
			)
				utils().abiDecode(targetTypes, false);
			else
			{
				utils().convertType(*firstArgType, ArrayType(DataLocation::Memory));
				m_context << Instruction::DUP1 << u256(32) << Instruction::ADD;
				m_context << Instruction::SWAP1 << Instruction::MLOAD;
				// stack now: <mem_pos> <length>

				utils().abiDecode(targetTypes, true);
			}
			break;
		}
		case FunctionType::Kind::GasLeft:
			m_context << Instruction::GAS;
			break;
		}
	}
	return false;
}

bool ExpressionCompiler::visit(NewExpression const&)
{
	// code is created for the function call (CREATION) only
	return false;
}

bool ExpressionCompiler::visit(MemberAccess const& _memberAccess)
{
	CompilerContext::LocationSetter locationSetter(m_context, _memberAccess);
	// Check whether the member is a bound function.
	ASTString const& member = _memberAccess.memberName();
	if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type.get()))
		if (funType->bound())
		{
			_memberAccess.expression().accept(*this);
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				*funType->selfType(),
				true
			);
			if (funType->kind() == FunctionType::Kind::Internal)
			{
				FunctionDefinition const& funDef = dynamic_cast<decltype(funDef)>(funType->declaration());
				utils().pushCombinedFunctionEntryLabel(funDef);
				utils().moveIntoStack(funType->selfType()->sizeOnStack(), 1);
			}
			else
			{
				solAssert(funType->kind() == FunctionType::Kind::DelegateCall, "");
				auto contract = dynamic_cast<ContractDefinition const*>(funType->declaration().scope());
				solAssert(contract && contract->isLibrary(), "");
				m_context.appendLibraryAddress(contract->fullyQualifiedName());
				m_context << funType->externalIdentifier();
				utils().moveIntoStack(funType->selfType()->sizeOnStack(), 2);
			}
			return false;
		}

	// Special processing for TypeType because we do not want to visit the library itself
	// for internal functions, or enum/struct definitions.
	if (TypeType const* type = dynamic_cast<TypeType const*>(_memberAccess.expression().annotation().type.get()))
	{
		if (dynamic_cast<ContractType const*>(type->actualType().get()))
		{
			solAssert(_memberAccess.annotation().type, "_memberAccess has no type");
			if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type.get()))
			{
				switch (funType->kind())
				{
				case FunctionType::Kind::Internal:
					// We do not visit the expression here on purpose, because in the case of an
					// internal library function call, this would push the library address forcing
					// us to link against it although we actually do not need it.
					if (auto const* function = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration))
						utils().pushCombinedFunctionEntryLabel(*function);
					else
						solAssert(false, "Function not found in member access");
					break;
				case FunctionType::Kind::Event:
					if (!dynamic_cast<EventDefinition const*>(_memberAccess.annotation().referencedDeclaration))
						solAssert(false, "event not found");
					// no-op, because the parent node will do the job
					break;
				case FunctionType::Kind::DelegateCall:
					_memberAccess.expression().accept(*this);
					m_context << funType->externalIdentifier();
					break;
				case FunctionType::Kind::External:
				case FunctionType::Kind::Creation:
				case FunctionType::Kind::Send:
				case FunctionType::Kind::BareCall:
				case FunctionType::Kind::BareCallCode:
				case FunctionType::Kind::BareDelegateCall:
				case FunctionType::Kind::BareStaticCall:
				case FunctionType::Kind::Transfer:
				case FunctionType::Kind::Log0:
				case FunctionType::Kind::Log1:
				case FunctionType::Kind::Log2:
				case FunctionType::Kind::Log3:
				case FunctionType::Kind::Log4:
				case FunctionType::Kind::ECRecover:
				case FunctionType::Kind::SHA256:
				case FunctionType::Kind::RIPEMD160:
				default:
					solAssert(false, "unsupported member function");
				}
			}
			else if (dynamic_cast<TypeType const*>(_memberAccess.annotation().type.get()))
			{
				// no-op
			}
			else if (auto variable = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
				appendVariable(*variable, static_cast<Expression const&>(_memberAccess));
			else
				_memberAccess.expression().accept(*this);
		}
		else if (auto enumType = dynamic_cast<EnumType const*>(type->actualType().get()))
		{
			_memberAccess.expression().accept(*this);
			m_context << enumType->memberValue(_memberAccess.memberName());
		}
		else
			_memberAccess.expression().accept(*this);
		return false;
	}
	// Another special case for `this.f.selector` which does not need the address.
	// There are other uses of `.selector` which do need the address, but we want this
	// specific use to be a pure expression.
	if (
		_memberAccess.expression().annotation().type->category() == Type::Category::Function &&
		member == "selector"
	)
		if (auto const* expr = dynamic_cast<MemberAccess const*>(&_memberAccess.expression()))
			if (auto const* exprInt = dynamic_cast<Identifier const*>(&expr->expression()))
				if (exprInt->name() == "this")
					if (Declaration const* declaration = expr->annotation().referencedDeclaration)
					{
						u256 identifier;
						if (auto const* variable = dynamic_cast<VariableDeclaration const*>(declaration))
							identifier = FunctionType(*variable).externalIdentifier();
						else if (auto const* function = dynamic_cast<FunctionDefinition const*>(declaration))
							identifier = FunctionType(*function).externalIdentifier();
						else
							solAssert(false, "Contract member is neither variable nor function.");
						m_context << identifier;
						/// need to store store it as bytes4
						utils().leftShiftNumberOnStack(224);
						return false;
					}

	_memberAccess.expression().accept(*this);
	switch (_memberAccess.expression().annotation().type->category())
	{
	case Type::Category::Contract:
	{
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.expression().annotation().type);
		if (type.isSuper())
		{
			solAssert(!!_memberAccess.annotation().referencedDeclaration, "Referenced declaration not resolved.");
			utils().pushCombinedFunctionEntryLabel(m_context.superFunction(
				dynamic_cast<FunctionDefinition const&>(*_memberAccess.annotation().referencedDeclaration),
				type.contractDefinition()
			));
		}
		// ordinary contract type
		else if (Declaration const* declaration = _memberAccess.annotation().referencedDeclaration)
		{
			u256 identifier;
			if (auto const* variable = dynamic_cast<VariableDeclaration const*>(declaration))
				identifier = FunctionType(*variable).externalIdentifier();
			else if (auto const* function = dynamic_cast<FunctionDefinition const*>(declaration))
				identifier = FunctionType(*function).externalIdentifier();
			else
				solAssert(false, "Contract member is neither variable nor function.");
			utils().convertType(type, AddressType(), true);
			m_context << identifier;
		}
		else
			solAssert(false, "Invalid member access in contract");
		break;
	}
	case Type::Category::Integer:
	{
		solAssert(false, "Invalid member access to integer");
		break;
	}
	case Type::Category::Address:
	{
		if (member == "balance")
		{
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				AddressType(),
				true
			);
			m_context << Instruction::BALANCE;
		}
		else if ((set<string>{"send", "transfer", "call", "callcode", "delegatecall", "staticcall"}).count(member))
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				AddressType(),
				true
			);
		else
			solAssert(false, "Invalid member access to address");
		break;
	}
	case Type::Category::Function:
		if (member == "selector")
		{
			m_context << Instruction::SWAP1 << Instruction::POP;
			/// need to store store it as bytes4
			utils().leftShiftNumberOnStack(224);
		}
		else
			solAssert(!!_memberAccess.expression().annotation().type->memberType(member),
				 "Invalid member access to function.");
		break;
	case Type::Category::Magic:
		// we can ignore the kind of magic and only look at the name of the member
		if (member == "coinbase")
			m_context << Instruction::COINBASE;
		else if (member == "timestamp")
			m_context << Instruction::TIMESTAMP;
		else if (member == "difficulty")
			m_context << Instruction::DIFFICULTY;
		else if (member == "number")
			m_context << Instruction::NUMBER;
		else if (member == "gaslimit")
			m_context << Instruction::GASLIMIT;
		else if (member == "sender")
			m_context << Instruction::CALLER;
		else if (member == "value")
			m_context << Instruction::CALLVALUE;
		else if (member == "origin")
			m_context << Instruction::ORIGIN;
		else if (member == "gas")
			m_context << Instruction::GAS;
		else if (member == "gasprice")
			m_context << Instruction::GASPRICE;
		else if (member == "data")
			m_context << u256(0) << Instruction::CALLDATASIZE;
		else if (member == "sig")
			m_context << u256(0) << Instruction::CALLDATALOAD
				<< (u256(0xffffffff) << (256 - 32)) << Instruction::AND;
		else if (member == "blockhash")
		{
		}
		else
			solAssert(false, "Unknown magic member.");
		break;
	case Type::Category::Struct:
	{
		StructType const& type = dynamic_cast<StructType const&>(*_memberAccess.expression().annotation().type);
		switch (type.location())
		{
		case DataLocation::Storage:
		{
			pair<u256, unsigned> const& offsets = type.storageOffsetsOfMember(member);
			m_context << offsets.first << Instruction::ADD << u256(offsets.second);
			setLValueToStorageItem(_memberAccess);
			break;
		}
		case DataLocation::Memory:
		{
			m_context << type.memoryOffsetOfMember(member) << Instruction::ADD;
			setLValue<MemoryItem>(_memberAccess, *_memberAccess.annotation().type);
			break;
		}
		default:
			solAssert(false, "Illegal data location for struct.");
		}
		break;
	}
	case Type::Category::Enum:
	{
		EnumType const& type = dynamic_cast<EnumType const&>(*_memberAccess.expression().annotation().type);
		m_context << type.memberValue(_memberAccess.memberName());
		break;
	}
	case Type::Category::Array:
	{
		auto const& type = dynamic_cast<ArrayType const&>(*_memberAccess.expression().annotation().type);
		if (member == "length")
		{
			if (!type.isDynamicallySized())
			{
				utils().popStackElement(type);
				m_context << type.length();
			}
			else
				switch (type.location())
				{
				case DataLocation::CallData:
					m_context << Instruction::SWAP1 << Instruction::POP;
					break;
				case DataLocation::Storage:
					setLValue<StorageArrayLength>(_memberAccess, type);
					break;
				case DataLocation::Memory:
					m_context << Instruction::MLOAD;
					break;
				}
		}
		else if (member == "push" || member == "pop")
		{
			solAssert(
				type.isDynamicallySized() &&
				type.location() == DataLocation::Storage &&
				type.category() == Type::Category::Array,
				"Tried to use ." + member + "() on a non-dynamically sized array"
			);
		}
		else
			solAssert(false, "Illegal array member.");
		break;
	}
	case Type::Category::FixedBytes:
	{
		auto const& type = dynamic_cast<FixedBytesType const&>(*_memberAccess.expression().annotation().type);
		utils().popStackElement(type);
		if (member == "length")
			m_context << u256(type.numBytes());
		else
			solAssert(false, "Illegal fixed bytes member.");
		break;
	}
	default:
		solAssert(false, "Member access to unknown type.");
	}
	return false;
}

bool ExpressionCompiler::visit(IndexAccess const& _indexAccess)
{
	CompilerContext::LocationSetter locationSetter(m_context, _indexAccess);
	_indexAccess.baseExpression().accept(*this);

	Type const& baseType = *_indexAccess.baseExpression().annotation().type;

	if (baseType.category() == Type::Category::Mapping)
	{
		// stack: storage_base_ref
		TypePointer keyType = dynamic_cast<MappingType const&>(baseType).keyType();
		solAssert(_indexAccess.indexExpression(), "Index expression expected.");
		if (keyType->isDynamicallySized())
		{
			_indexAccess.indexExpression()->accept(*this);
			utils().fetchFreeMemoryPointer();
			// stack: base index mem
			// note: the following operations must not allocate memory!
			utils().packedEncode(
				TypePointers{_indexAccess.indexExpression()->annotation().type},
				TypePointers{keyType}
			);
			m_context << Instruction::SWAP1;
			utils().storeInMemoryDynamic(IntegerType(256));
			utils().toSizeAfterFreeMemoryPointer();
		}
		else
		{
			m_context << u256(0); // memory position
			appendExpressionCopyToMemory(*keyType, *_indexAccess.indexExpression());
			m_context << Instruction::SWAP1;
			solAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");
			utils().storeInMemoryDynamic(IntegerType(256));
			m_context << u256(0);
		}
		m_context << Instruction::KECCAK256;
		m_context << u256(0);
		setLValueToStorageItem(_indexAccess);
	}
	else if (baseType.category() == Type::Category::Array)
	{
		ArrayType const& arrayType = dynamic_cast<ArrayType const&>(baseType);
		solAssert(_indexAccess.indexExpression(), "Index expression expected.");

		_indexAccess.indexExpression()->accept(*this);
		utils().convertType(*_indexAccess.indexExpression()->annotation().type, IntegerType(256), true);
		// stack layout: <base_ref> [<length>] <index>
		ArrayUtils(m_context).accessIndex(arrayType);
		switch (arrayType.location())
		{
		case DataLocation::Storage:
			if (arrayType.isByteArray())
			{
				solAssert(!arrayType.isString(), "Index access to string is not allowed.");
				setLValue<StorageByteArrayElement>(_indexAccess);
			}
			else
				setLValueToStorageItem(_indexAccess);
			break;
		case DataLocation::Memory:
			setLValue<MemoryItem>(_indexAccess, *_indexAccess.annotation().type, !arrayType.isByteArray());
			break;
		case DataLocation::CallData:
			//@todo if we implement this, the value in calldata has to be added to the base offset
			solUnimplementedAssert(!arrayType.baseType()->isDynamicallySized(), "Nested arrays not yet implemented.");
			if (arrayType.baseType()->isValueType())
				CompilerUtils(m_context).loadFromMemoryDynamic(
					*arrayType.baseType(),
					true,
					!arrayType.isByteArray(),
					false
				);
			break;
		}
	}
	else if (baseType.category() == Type::Category::FixedBytes)
	{
		FixedBytesType const& fixedBytesType = dynamic_cast<FixedBytesType const&>(baseType);
		solAssert(_indexAccess.indexExpression(), "Index expression expected.");

		_indexAccess.indexExpression()->accept(*this);
		utils().convertType(*_indexAccess.indexExpression()->annotation().type, IntegerType(256), true);
		// stack layout: <value> <index>
		// check out-of-bounds access
		m_context << u256(fixedBytesType.numBytes());
		m_context << Instruction::DUP2 << Instruction::LT << Instruction::ISZERO;
		// out-of-bounds access throws exception
		m_context.appendConditionalInvalid();

		m_context << Instruction::BYTE;
		utils().leftShiftNumberOnStack(256 - 8);
	}
	else if (baseType.category() == Type::Category::TypeType)
	{
		solAssert(baseType.sizeOnStack() == 0, "");
		solAssert(_indexAccess.annotation().type->sizeOnStack() == 0, "");
		// no-op - this seems to be a lone array type (`structType[];`)
	}
	else
		solAssert(false, "Index access only allowed for mappings or arrays.");

	return false;
}

void ExpressionCompiler::endVisit(Identifier const& _identifier)
{
	CompilerContext::LocationSetter locationSetter(m_context, _identifier);
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		switch (magicVar->type()->category())
		{
		case Type::Category::Contract:
			// "this" or "super"
			if (!dynamic_cast<ContractType const&>(*magicVar->type()).isSuper())
				m_context << Instruction::ADDRESS;
			break;
		case Type::Category::Integer:
			// "now"
			m_context << Instruction::TIMESTAMP;
			break;
		default:
			break;
		}
	}
	else if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
		// If the identifier is called right away, this code is executed in visit(FunctionCall...), because
		// we want to avoid having a reference to the runtime function entry point in the
		// constructor context, since this would force the compiler to include unreferenced
		// internal functions in the runtime contex.
		utils().pushCombinedFunctionEntryLabel(m_context.resolveVirtualFunction(*functionDef));
	else if (auto variable = dynamic_cast<VariableDeclaration const*>(declaration))
		appendVariable(*variable, static_cast<Expression const&>(_identifier));
	else if (auto contract = dynamic_cast<ContractDefinition const*>(declaration))
	{
		if (contract->isLibrary())
			m_context.appendLibraryAddress(contract->fullyQualifiedName());
	}
	else if (dynamic_cast<EventDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<EnumDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<StructDefinition const*>(declaration))
	{
		// no-op
	}
	else
	{
		solAssert(false, "Identifier type not expected in expression context.");
	}
}

void ExpressionCompiler::endVisit(Literal const& _literal)
{
	CompilerContext::LocationSetter locationSetter(m_context, _literal);
	TypePointer type = _literal.annotation().type;

	switch (type->category())
	{
	case Type::Category::RationalNumber:
	case Type::Category::Bool:
	case Type::Category::Address:
		m_context << type->literalValue(&_literal);
		break;
	case Type::Category::StringLiteral:
		break; // will be done during conversion
	default:
		solUnimplemented("Only integer, boolean and string literals implemented for now.");
	}
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation const& _binaryOperation)
{
	Token::Value const c_op = _binaryOperation.getOperator();
	solAssert(c_op == Token::Or || c_op == Token::And, "");

	_binaryOperation.leftExpression().accept(*this);
	m_context << Instruction::DUP1;
	if (c_op == Token::And)
		m_context << Instruction::ISZERO;
	eth::AssemblyItem endLabel = m_context.appendConditionalJump();
	m_context << Instruction::POP;
	_binaryOperation.rightExpression().accept(*this);
	m_context << endLabel;
}

void ExpressionCompiler::appendCompareOperatorCode(Token::Value _operator, Type const& _type)
{
	solAssert(_type.sizeOnStack() == 1, "Comparison of multi-slot types.");
	if (_operator == Token::Equal || _operator == Token::NotEqual)
	{
		if (FunctionType const* funType = dynamic_cast<decltype(funType)>(&_type))
		{
			if (funType->kind() == FunctionType::Kind::Internal)
			{
				// We have to remove the upper bits (construction time value) because they might
				// be "unknown" in one of the operands and not in the other.
				m_context << ((u256(1) << 32) - 1) << Instruction::AND;
				m_context << Instruction::SWAP1;
				m_context << ((u256(1) << 32) - 1) << Instruction::AND;
			}
		}
		m_context << Instruction::EQ;
		if (_operator == Token::NotEqual)
			m_context << Instruction::ISZERO;
	}
	else
	{
		bool isSigned = false;
		if (auto type = dynamic_cast<IntegerType const*>(&_type))
			isSigned = type->isSigned();

		switch (_operator)
		{
		case Token::GreaterThanOrEqual:
			m_context <<
				(isSigned ? Instruction::SLT : Instruction::LT) <<
				Instruction::ISZERO;
			break;
		case Token::LessThanOrEqual:
			m_context <<
				(isSigned ? Instruction::SGT : Instruction::GT) <<
				Instruction::ISZERO;
			break;
		case Token::GreaterThan:
			m_context << (isSigned ? Instruction::SGT : Instruction::GT);
			break;
		case Token::LessThan:
			m_context << (isSigned ? Instruction::SLT : Instruction::LT);
			break;
		default:
			solAssert(false, "Unknown comparison operator.");
		}
	}
}

void ExpressionCompiler::appendOrdinaryBinaryOperatorCode(Token::Value _operator, Type const& _type)
{
	if (Token::isArithmeticOp(_operator))
		appendArithmeticOperatorCode(_operator, _type);
	else if (Token::isBitOp(_operator))
		appendBitOperatorCode(_operator);
	else
		solAssert(false, "Unknown binary operator.");
}

void ExpressionCompiler::appendArithmeticOperatorCode(Token::Value _operator, Type const& _type)
{
	if (_type.category() == Type::Category::FixedPoint)
		solUnimplemented("Not yet implemented - FixedPointType.");

	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	bool const c_isSigned = type.isSigned();

	switch (_operator)
	{
	case Token::Add:
		m_context << Instruction::ADD;
		break;
	case Token::Sub:
		m_context << Instruction::SUB;
		break;
	case Token::Mul:
		m_context << Instruction::MUL;
		break;
	case Token::Div:
	case Token::Mod:
	{
		// Test for division by zero
		m_context << Instruction::DUP2 << Instruction::ISZERO;
		m_context.appendConditionalInvalid();

		if (_operator == Token::Div)
			m_context << (c_isSigned ? Instruction::SDIV : Instruction::DIV);
		else
			m_context << (c_isSigned ? Instruction::SMOD : Instruction::MOD);
		break;
	}
	case Token::Exp:
		m_context << Instruction::EXP;
		break;
	default:
		solAssert(false, "Unknown arithmetic operator.");
	}
}

void ExpressionCompiler::appendBitOperatorCode(Token::Value _operator)
{
	switch (_operator)
	{
	case Token::BitOr:
		m_context << Instruction::OR;
		break;
	case Token::BitAnd:
		m_context << Instruction::AND;
		break;
	case Token::BitXor:
		m_context << Instruction::XOR;
		break;
	default:
		solAssert(false, "Unknown bit operator.");
	}
}

void ExpressionCompiler::appendShiftOperatorCode(Token::Value _operator, Type const& _valueType, Type const& _shiftAmountType)
{
	// stack: shift_amount value_to_shift

	bool c_valueSigned = false;
	if (auto valueType = dynamic_cast<IntegerType const*>(&_valueType))
		c_valueSigned = valueType->isSigned();
	else
		solAssert(dynamic_cast<FixedBytesType const*>(&_valueType), "Only integer and fixed bytes type supported for shifts.");

	// The amount can be a RationalNumberType too.
	bool c_amountSigned = false;
	if (auto amountType = dynamic_cast<RationalNumberType const*>(&_shiftAmountType))
	{
		// This should be handled by the type checker.
		solAssert(amountType->integerType(), "");
		solAssert(!amountType->integerType()->isSigned(), "");
	}
	else if (auto amountType = dynamic_cast<IntegerType const*>(&_shiftAmountType))
		c_amountSigned = amountType->isSigned();
	else
		solAssert(false, "Invalid shift amount type.");

	// shift by negative amount throws exception
	if (c_amountSigned)
	{
		m_context << u256(0) << Instruction::DUP3 << Instruction::SLT;
		m_context.appendConditionalInvalid();
	}

	m_context << Instruction::SWAP1;
	// stack: value_to_shift shift_amount

	switch (_operator)
	{
	case Token::SHL:
		if (m_context.evmVersion().hasBitwiseShifting())
			m_context << Instruction::SHL;
		else
			m_context << u256(2) << Instruction::EXP << Instruction::MUL;
		break;
	case Token::SAR:
		if (m_context.evmVersion().hasBitwiseShifting())
			m_context << (c_valueSigned ? Instruction::SAR : Instruction::SHR);
		else
		{
			if (c_valueSigned)
				// In the following assembly snippet, xor_mask will be zero, if value_to_shift is positive.
				// Therefore xor'ing with xor_mask is the identity and the computation reduces to
				// div(value_to_shift, exp(2, shift_amount)), which is correct, since for positive values
				// arithmetic right shift is dividing by a power of two (which, as a bitwise operation, results
				// in discarding bits on the right and filling with zeros from the left).
				// For negative values arithmetic right shift, viewed as a bitwise operation, discards bits to the
				// right and fills in ones from the left. This is achieved as follows:
				// If value_to_shift is negative, then xor_mask will have all bits set, so xor'ing with xor_mask
				// will flip all bits. First all bits in value_to_shift are flipped. As for the positive case,
				// dividing by a power of two using integer arithmetic results in discarding bits to the right
				// and filling with zeros from the left. Flipping all bits in the result again, turns all zeros
				// on the left to ones and restores the non-discarded, shifted bits to their original value (they
				// have now been flipped twice). In summary we now have discarded bits to the right and filled with
				// ones from the left, i.e. we have performed an arithmetic right shift.
				m_context.appendInlineAssembly(R"({
					let xor_mask := sub(0, slt(value_to_shift, 0))
					value_to_shift := xor(div(xor(value_to_shift, xor_mask), exp(2, shift_amount)), xor_mask)
				})", {"value_to_shift", "shift_amount"});
			else
				m_context.appendInlineAssembly(R"({
					value_to_shift := div(value_to_shift, exp(2, shift_amount))
				})", {"value_to_shift", "shift_amount"});
			m_context << Instruction::POP;

		}
		break;
	case Token::SHR:
	default:
		solAssert(false, "Unknown shift operator.");
	}
}

void ExpressionCompiler::appendExternalFunctionCall(
	FunctionType const& _functionType,
	vector<ASTPointer<Expression const>> const& _arguments
)
{
	solAssert(
		_functionType.takesArbitraryParameters() ||
		_arguments.size() == _functionType.parameterTypes().size(), ""
	);

	// Assumed stack content here:
	// <stack top>
	// value [if _functionType.valueSet()]
	// gas [if _functionType.gasSet()]
	// self object [if bound - moved to top right away]
	// function identifier [unless bare]
	// contract address

	unsigned selfSize = _functionType.bound() ? _functionType.selfType()->sizeOnStack() : 0;
	unsigned gasValueSize = (_functionType.gasSet() ? 1 : 0) + (_functionType.valueSet() ? 1 : 0);
	unsigned contractStackPos = m_context.currentToBaseStackOffset(1 + gasValueSize + selfSize + (_functionType.isBareCall() ? 0 : 1));
	unsigned gasStackPos = m_context.currentToBaseStackOffset(gasValueSize);
	unsigned valueStackPos = m_context.currentToBaseStackOffset(1);

	// move self object to top
	if (_functionType.bound())
		utils().moveToStackTop(gasValueSize, _functionType.selfType()->sizeOnStack());

	auto funKind = _functionType.kind();

	solAssert(funKind != FunctionType::Kind::BareStaticCall || m_context.evmVersion().hasStaticCall(), "");

	bool returnSuccessConditionAndReturndata = funKind == FunctionType::Kind::BareCall || funKind == FunctionType::Kind::BareCallCode || funKind == FunctionType::Kind::BareDelegateCall || funKind == FunctionType::Kind::BareStaticCall;
	bool isCallCode = funKind == FunctionType::Kind::BareCallCode;
	bool isDelegateCall = funKind == FunctionType::Kind::BareDelegateCall || funKind == FunctionType::Kind::DelegateCall;
	bool useStaticCall = funKind == FunctionType::Kind::BareStaticCall || (_functionType.stateMutability() <= StateMutability::View && m_context.evmVersion().hasStaticCall());

	bool haveReturndatacopy = m_context.evmVersion().supportsReturndata();
	unsigned retSize = 0;
	bool dynamicReturnSize = false;
	TypePointers returnTypes;
	if (!returnSuccessConditionAndReturndata)
	{
		if (haveReturndatacopy)
			returnTypes = _functionType.returnParameterTypes();
		else
			returnTypes = _functionType.returnParameterTypesWithoutDynamicTypes();

		for (auto const& retType: returnTypes)
			if (retType->isDynamicallyEncoded())
			{
				solAssert(haveReturndatacopy, "");
				dynamicReturnSize = true;
				retSize = 0;
				break;
			}
			else
				retSize += retType->calldataEncodedSize();
	}

	// Evaluate arguments.
	TypePointers argumentTypes;
	TypePointers parameterTypes = _functionType.parameterTypes();
	if (_functionType.bound())
	{
		argumentTypes.push_back(_functionType.selfType());
		parameterTypes.insert(parameterTypes.begin(), _functionType.selfType());
	}
	for (size_t i = 0; i < _arguments.size(); ++i)
	{
		_arguments[i]->accept(*this);
		argumentTypes.push_back(_arguments[i]->annotation().type);
	}

	if (funKind == FunctionType::Kind::ECRecover)
	{
		// Clears 32 bytes of currently free memory and advances free memory pointer.
		// Output area will be "start of input area" - 32.
		// The reason is that a failing ECRecover cannot be detected, it will just return
		// zero bytes (which we cannot detect).
		solAssert(0 < retSize && retSize <= 32, "");
		utils().fetchFreeMemoryPointer();
		m_context << u256(0) << Instruction::DUP2 << Instruction::MSTORE;
		m_context << u256(32) << Instruction::ADD;
		utils().storeFreeMemoryPointer();
	}

	if (!m_context.evmVersion().canOverchargeGasForCall())
	{
		// Touch the end of the output area so that we do not pay for memory resize during the call
		// (which we would have to subtract from the gas left)
		// We could also just use MLOAD; POP right before the gas calculation, but the optimizer
		// would remove that, so we use MSTORE here.
		if (!_functionType.gasSet() && retSize > 0)
		{
			m_context << u256(0);
			utils().fetchFreeMemoryPointer();
			// This touches too much, but that way we save some rounding arithmetic
			m_context << u256(retSize) << Instruction::ADD << Instruction::MSTORE;
		}
	}

	// Copy function identifier to memory.
	utils().fetchFreeMemoryPointer();
	if (!_functionType.isBareCall())
	{
		m_context << dupInstruction(2 + gasValueSize + CompilerUtils::sizeOnStack(argumentTypes));
		utils().storeInMemoryDynamic(IntegerType(8 * CompilerUtils::dataStartOffset), false);
	}

	// If the function takes arbitrary parameters or is a bare call, copy dynamic length data in place.
	// Move arguments to memory, will not update the free memory pointer (but will update the memory
	// pointer on the stack).
	utils().encodeToMemory(
		argumentTypes,
		parameterTypes,
		_functionType.padArguments(),
		_functionType.takesArbitraryParameters() || _functionType.isBareCall(),
		isCallCode || isDelegateCall
	);

	// Stack now:
	// <stack top>
	// input_memory_end
	// value [if _functionType.valueSet()]
	// gas [if _functionType.gasSet()]
	// function identifier [unless bare]
	// contract address

	// Output data will replace input data, unless we have ECRecover (then, output
	// area will be 32 bytes just before input area).
	// put on stack: <size of output> <memory pos of output> <size of input> <memory pos of input>
	m_context << u256(retSize);
	utils().fetchFreeMemoryPointer(); // This is the start of input
	if (funKind == FunctionType::Kind::ECRecover)
	{
		// In this case, output is 32 bytes before input and has already been cleared.
		m_context << u256(32) << Instruction::DUP2 << Instruction::SUB << Instruction::SWAP1;
		// Here: <input end> <output size> <outpos> <input pos>
		m_context << Instruction::DUP1 << Instruction::DUP5 << Instruction::SUB;
		m_context << Instruction::SWAP1;
	}
	else
	{
		m_context << Instruction::DUP1 << Instruction::DUP4 << Instruction::SUB;
		m_context << Instruction::DUP2;
	}

	// CALL arguments: outSize, outOff, inSize, inOff (already present up to here)
	// [value,] addr, gas (stack top)
	if (isDelegateCall)
		solAssert(!_functionType.valueSet(), "Value set for delegatecall");
	else if (useStaticCall)
		solAssert(!_functionType.valueSet(), "Value set for staticcall");
	else if (_functionType.valueSet())
		m_context << dupInstruction(m_context.baseToCurrentStackOffset(valueStackPos));
	else
		m_context << u256(0);
	m_context << dupInstruction(m_context.baseToCurrentStackOffset(contractStackPos));

	bool existenceChecked = false;
	// Check the the target contract exists (has code) for non-low-level calls.
	if (funKind == FunctionType::Kind::External || funKind == FunctionType::Kind::DelegateCall)
	{
		m_context << Instruction::DUP1 << Instruction::EXTCODESIZE << Instruction::ISZERO;
		// TODO: error message?
		m_context.appendConditionalRevert();
		existenceChecked = true;
	}

	if (_functionType.gasSet())
		m_context << dupInstruction(m_context.baseToCurrentStackOffset(gasStackPos));
	else if (m_context.evmVersion().canOverchargeGasForCall())
		// Send all gas (requires tangerine whistle EVM)
		m_context << Instruction::GAS;
	else
	{
		// send all gas except the amount needed to execute "SUB" and "CALL"
		// @todo this retains too much gas for now, needs to be fine-tuned.
		u256 gasNeededByCaller = eth::GasCosts::callGas(m_context.evmVersion()) + 10;
		if (_functionType.valueSet())
			gasNeededByCaller += eth::GasCosts::callValueTransferGas;
		if (!existenceChecked)
			gasNeededByCaller += eth::GasCosts::callNewAccountGas; // we never know
		m_context << gasNeededByCaller << Instruction::GAS << Instruction::SUB;
	}
	// Order is important here, STATICCALL might overlap with DELEGATECALL.
	if (isDelegateCall)
		m_context << Instruction::DELEGATECALL;
	else if (isCallCode)
		m_context << Instruction::CALLCODE;
	else if (useStaticCall)
		m_context << Instruction::STATICCALL;
	else
		m_context << Instruction::CALL;

	unsigned remainsSize =
		2 + // contract address, input_memory_end
		(_functionType.valueSet() ? 1 : 0) +
		(_functionType.gasSet() ? 1 : 0) +
		(!_functionType.isBareCall() ? 1 : 0);

	if (returnSuccessConditionAndReturndata)
		m_context << swapInstruction(remainsSize);
	else
	{
		//Propagate error condition (if CALL pushes 0 on stack).
		m_context << Instruction::ISZERO;
		m_context.appendConditionalRevert(true);
	}

	utils().popStackSlots(remainsSize);

	if (returnSuccessConditionAndReturndata)
	{
		// success condition is already there
		// The return parameter types can be empty, when this function is used as
		// an internal helper function e.g. for ``send`` and ``transfer``. In that
		// case we're only interested in the success condition, not the return data.
		if (!_functionType.returnParameterTypes().empty())
		{
			if (haveReturndatacopy)
			{
				m_context << Instruction::RETURNDATASIZE;
				m_context.appendInlineAssembly(R"({
					switch v case 0 {
						v := 0x60
					} default {
						v := mload(0x40)
						mstore(0x40, add(v, and(add(returndatasize(), 0x3f), not(0x1f))))
						mstore(v, returndatasize())
						returndatacopy(add(v, 0x20), 0, returndatasize())
					}
			    })", {"v"});
			}
			else
				utils().pushZeroPointer();
		}
	}
	else if (funKind == FunctionType::Kind::RIPEMD160)
	{
		// fix: built-in contract returns right-aligned data
		utils().fetchFreeMemoryPointer();
		utils().loadFromMemoryDynamic(IntegerType(160), false, true, false);
		utils().convertType(IntegerType(160), FixedBytesType(20));
	}
	else if (funKind == FunctionType::Kind::ECRecover)
	{
		// Output is 32 bytes before input / free mem pointer.
		// Failing ecrecover cannot be detected, so we clear output before the call.
		m_context << u256(32);
		utils().fetchFreeMemoryPointer();
		m_context << Instruction::SUB << Instruction::MLOAD;
	}
	else if (!returnTypes.empty())
	{
		utils().fetchFreeMemoryPointer();
		// Stack: return_data_start

		// The old decoder did not allocate any memory (i.e. did not touch the free
		// memory pointer), but kept references to the return data for
		// (statically-sized) arrays
		bool needToUpdateFreeMemoryPtr = false;
		if (dynamicReturnSize || m_context.experimentalFeatureActive(ExperimentalFeature::ABIEncoderV2))
			needToUpdateFreeMemoryPtr = true;
		else
			for (auto const& retType: returnTypes)
				if (dynamic_cast<ReferenceType const*>(retType.get()))
					needToUpdateFreeMemoryPtr = true;

		// Stack: return_data_start
		if (dynamicReturnSize)
		{
			solAssert(haveReturndatacopy, "");
			m_context.appendInlineAssembly("{ returndatacopy(return_data_start, 0, returndatasize()) }", {"return_data_start"});
		}
		else
			solAssert(retSize > 0, "");
		// Always use the actual return length, and not our calculated expected length, if returndatacopy is supported.
		// This ensures it can catch badly formatted input from external calls.
		m_context << (haveReturndatacopy ? eth::AssemblyItem(Instruction::RETURNDATASIZE) : u256(retSize));
		// Stack: return_data_start return_data_size
		if (needToUpdateFreeMemoryPtr)
			m_context.appendInlineAssembly(R"({
				// round size to the next multiple of 32
				let newMem := add(start, and(add(size, 0x1f), not(0x1f)))
				mstore(0x40, newMem)
			})", {"start", "size"});

		utils().abiDecode(returnTypes, true);
	}
}

void ExpressionCompiler::appendExpressionCopyToMemory(Type const& _expectedType, Expression const& _expression)
{
	solUnimplementedAssert(_expectedType.isValueType(), "Not implemented for non-value types.");
	_expression.accept(*this);
	utils().convertType(*_expression.annotation().type, _expectedType, true);
	utils().storeInMemoryDynamic(_expectedType);
}

void ExpressionCompiler::appendVariable(VariableDeclaration const& _variable, Expression const& _expression)
{
	if (!_variable.isConstant())
		setLValueFromDeclaration(_variable, _expression);
	else
	{
		_variable.value()->accept(*this);
		utils().convertType(*_variable.value()->annotation().type, *_variable.annotation().type);
	}
}

void ExpressionCompiler::setLValueFromDeclaration(Declaration const& _declaration, Expression const& _expression)
{
	if (m_context.isLocalVariable(&_declaration))
		setLValue<StackVariable>(_expression, dynamic_cast<VariableDeclaration const&>(_declaration));
	else if (m_context.isStateVariable(&_declaration))
		setLValue<StorageItem>(_expression, dynamic_cast<VariableDeclaration const&>(_declaration));
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError()
			<< errinfo_sourceLocation(_expression.location())
			<< errinfo_comment("Identifier type not supported or identifier not found."));
}

void ExpressionCompiler::setLValueToStorageItem(Expression const& _expression)
{
	setLValue<StorageItem>(_expression, *_expression.annotation().type);
}

bool ExpressionCompiler::cleanupNeededForOp(Type::Category _type, Token::Value _op)
{
	if (Token::isCompareOp(_op) || Token::isShiftOp(_op))
		return true;
	else if (_type == Type::Category::Integer && (_op == Token::Div || _op == Token::Mod || _op == Token::Exp))
		// We need cleanup for EXP because 0**0 == 1, but 0**0x100 == 0
		// It would suffice to clean the exponent, though.
		return true;
	else
		return false;
}

CompilerUtils ExpressionCompiler::utils()
{
	return CompilerUtils(m_context);
}

}
}
