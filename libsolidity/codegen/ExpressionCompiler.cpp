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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler for expressions.
 */

#include <libsolidity/codegen/ExpressionCompiler.h>

#include <libsolidity/codegen/ReturnInfo.h>
#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/LValue.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libevmasm/GasMeter.h>
#include <libsolutil/Common.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/Whiskers.h>
#include <libsolutil/StackTooDeepString.h>

#include <boost/algorithm/string/replace.hpp>
#include <numeric>
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;

namespace
{

Type const* closestType(Type const* _type, Type const* _targetType, bool _isShiftOp)
{
	if (_isShiftOp)
		return _type->mobileType();
	else if (auto const* tupleType = dynamic_cast<TupleType const*>(_type))
	{
		solAssert(_targetType, "");
		TypePointers const& targetComponents = dynamic_cast<TupleType const&>(*_targetType).components();
		solAssert(tupleType->components().size() == targetComponents.size(), "");
		TypePointers tempComponents(targetComponents.size());
		for (size_t i = 0; i < targetComponents.size(); ++i)
		{
			if (tupleType->components()[i] && targetComponents[i])
			{
				tempComponents[i] = closestType(tupleType->components()[i], targetComponents[i], _isShiftOp);
				solAssert(tempComponents[i], "");
			}
		}
		return TypeProvider::tuple(move(tempComponents));
	}
	else
		return _targetType->dataStoredIn(DataLocation::Storage) ? _type->mobileType() : _targetType;
}

}

void ExpressionCompiler::compile(Expression const& _expression)
{
	_expression.accept(*this);
}

void ExpressionCompiler::appendStateVariableInitialization(VariableDeclaration const& _varDecl)
{
	if (!_varDecl.value())
		return;
	Type const* type = _varDecl.value()->annotation().type;
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
	if (_varDecl.immutable())
		ImmutableItem(m_context, _varDecl).storeValue(*type, _varDecl.location(), true);
	else
		StorageItem(m_context, _varDecl).storeValue(*type, _varDecl.location(), true);
}

void ExpressionCompiler::appendConstStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "");
	acceptAndConvert(*_varDecl.value(), *_varDecl.annotation().type);

	// append return
	m_context << dupInstruction(_varDecl.annotation().type->sizeOnStack() + 1);
	m_context.appendJump(evmasm::AssemblyItem::JumpType::OutOfFunction);
}

void ExpressionCompiler::appendStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	solAssert(!_varDecl.isConstant(), "");
	CompilerContext::LocationSetter locationSetter(m_context, _varDecl);
	FunctionType accessorType(_varDecl);

	TypePointers paramTypes = accessorType.parameterTypes();
	if (_varDecl.immutable())
		solAssert(paramTypes.empty(), "");

	m_context.adjustStackOffset(static_cast<int>(1 + CompilerUtils::sizeOnStack(paramTypes)));

	if (!_varDecl.immutable())
	{
		// retrieve the position of the variable
		auto const& location = m_context.storageLocationOfVariable(_varDecl);
		m_context << location.first << u256(location.second);
	}

	Type const* returnType = _varDecl.annotation().type;

	for (size_t i = 0; i < paramTypes.size(); ++i)
	{
		if (auto mappingType = dynamic_cast<MappingType const*>(returnType))
		{
			solAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");

			// pop offset
			m_context << Instruction::POP;
			if (paramTypes[i]->isDynamicallySized())
			{
				solAssert(
					dynamic_cast<ArrayType const&>(*paramTypes[i]).isByteArrayOrString(),
					"Expected string or byte array for mapping key type"
				);

				// stack: <keys..> <slot position>

				// copy key[i] to top.
				utils().copyToStackTop(static_cast<unsigned>(paramTypes.size() - i + 1), 1);

				m_context.appendInlineAssembly(R"({
					let key_len := mload(key_ptr)
					// Temp. use the memory after the array data for the slot
					// position
					let post_data_ptr := add(key_ptr, add(key_len, 0x20))
					let orig_data := mload(post_data_ptr)
					mstore(post_data_ptr, slot_pos)
					let hash := keccak256(add(key_ptr, 0x20), add(key_len, 0x20))
					mstore(post_data_ptr, orig_data)
					slot_pos := hash
				})", {"slot_pos", "key_ptr"});

				m_context << Instruction::POP;
			}
			else
			{
				solAssert(paramTypes[i]->isValueType(), "Expected value type for mapping key");

				// move storage offset to memory.
				utils().storeInMemory(32);

				// move key to memory.
				utils().copyToStackTop(static_cast<unsigned>(paramTypes.size() - i), 1);
				utils().storeInMemory(0);
				m_context << u256(64) << u256(0);
				m_context << Instruction::KECCAK256;
			}

			// push offset
			m_context << u256(0);
			returnType = mappingType->valueType();
		}
		else if (auto arrayType = dynamic_cast<ArrayType const*>(returnType))
		{
			// pop offset
			m_context << Instruction::POP;
			utils().copyToStackTop(static_cast<unsigned>(paramTypes.size() - i + 1), 1);

			ArrayUtils(m_context).retrieveLength(*arrayType, 1);
			// Stack: ref [length] index length
			// check out-of-bounds access
			m_context << Instruction::DUP2 << Instruction::LT;
			auto tag = m_context.appendConditionalJump();
			m_context << u256(0) << Instruction::DUP1 << Instruction::REVERT;
			m_context << tag;

			ArrayUtils(m_context).accessIndex(*arrayType, false);
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
		m_context << swapInstruction(static_cast<unsigned>(paramTypes.size()));
		m_context << Instruction::POP;
		m_context << swapInstruction(static_cast<unsigned>(paramTypes.size()));
		utils().popStackSlots(paramTypes.size() - 1);
	}
	unsigned retSizeOnStack = 0;
	auto returnTypes = accessorType.returnParameterTypes();
	solAssert(returnTypes.size() >= 1, "");
	if (StructType const* structType = dynamic_cast<StructType const*>(returnType))
	{
		solAssert(!_varDecl.immutable(), "");
		// remove offset
		m_context << Instruction::POP;
		auto const& names = accessorType.returnParameterNames();
		// struct
		for (size_t i = 0; i < names.size(); ++i)
		{
			if (returnTypes[i]->category() == Type::Category::Mapping)
				continue;
			if (auto arrayType = dynamic_cast<ArrayType const*>(returnTypes[i]))
				if (!arrayType->isByteArrayOrString())
					continue;
			pair<u256, unsigned> const& offsets = structType->storageOffsetsOfMember(names[i]);
			m_context << Instruction::DUP1 << u256(offsets.first) << Instruction::ADD << u256(offsets.second);
			Type const* memberType = structType->memberType(names[i]);
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
		if (_varDecl.immutable())
			ImmutableItem(m_context, _varDecl).retrieveValue(SourceLocation());
		else
			StorageItem(m_context, *returnType).retrieveValue(SourceLocation(), true);
		utils().convertType(*returnType, *returnTypes.front());
		retSizeOnStack = returnTypes.front()->sizeOnStack();
	}
	solAssert(retSizeOnStack == utils().sizeOnStack(returnTypes), "");
	if (retSizeOnStack > 15)
		BOOST_THROW_EXCEPTION(
			StackTooDeepError() <<
			errinfo_sourceLocation(_varDecl.location()) <<
			util::errinfo_comment(util::stackTooDeepString)
		);
	m_context << dupInstruction(retSizeOnStack + 1);
	m_context.appendJump(evmasm::AssemblyItem::JumpType::OutOfFunction);
}

bool ExpressionCompiler::visit(Conditional const& _condition)
{
	CompilerContext::LocationSetter locationSetter(m_context, _condition);
	_condition.condition().accept(*this);
	evmasm::AssemblyItem trueTag = m_context.appendConditionalJump();
	acceptAndConvert(_condition.falseExpression(), *_condition.annotation().type);
	evmasm::AssemblyItem endTag = m_context.appendJumpToNew();
	m_context << trueTag;
	int offset = static_cast<int>(_condition.annotation().type->sizeOnStack());
	m_context.adjustStackOffset(-offset);
	acceptAndConvert(_condition.trueExpression(), *_condition.annotation().type);
	m_context << endTag;
	return false;
}

bool ExpressionCompiler::visit(Assignment const& _assignment)
{
	CompilerContext::LocationSetter locationSetter(m_context, _assignment);
	Token op = _assignment.assignmentOperator();
	Token binOp = op == Token::Assign ? op : TokenTraits::AssignmentToBinaryOp(op);
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
		cleanupNeeded = cleanupNeededForOp(leftType.category(), binOp, m_context.arithmetic());
	_assignment.rightHandSide().accept(*this);
	// Perform some conversion already. This will convert storage types to memory and literals
	// to their actual type, but will not convert e.g. memory to storage.
	Type const* rightIntermediateType = closestType(
		_assignment.rightHandSide().annotation().type,
		_assignment.leftHandSide().annotation().type,
		op != Token::Assign && TokenTraits::isShiftOp(binOp)
	);

	solAssert(rightIntermediateType, "");
	utils().convertType(*_assignment.rightHandSide().annotation().type, *rightIntermediateType, cleanupNeeded);

	_assignment.leftHandSide().accept(*this);
	solAssert(!!m_currentLValue, "LValue not retrieved.");

	if (op == Token::Assign)
		m_currentLValue->storeValue(*rightIntermediateType, _assignment.location());
	else  // compound assignment
	{
		solAssert(binOp != Token::Exp, "Compound exp is not possible.");
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

		if (TokenTraits::isShiftOp(binOp))
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
					StackTooDeepError() <<
					errinfo_sourceLocation(_assignment.location()) <<
					util::errinfo_comment(util::stackTooDeepString)
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
		utils().allocateMemory(max(u256(32u), arrayType.memoryDataSize()));
		m_context << Instruction::DUP1;

		for (auto const& component: _tuple.components())
		{
			acceptAndConvert(*component, *arrayType.baseType(), true);
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
				if (_tuple.annotation().willBeWrittenTo)
				{
					solAssert(!!m_currentLValue, "");
					lvalues.push_back(move(m_currentLValue));
				}
			}
			else if (_tuple.annotation().willBeWrittenTo)
				lvalues.push_back(unique_ptr<LValue>());
		if (_tuple.annotation().willBeWrittenTo)
		{
			if (_tuple.components().size() == 1)
				m_currentLValue = move(lvalues[0]);
			else
				m_currentLValue = make_unique<TupleObject>(m_context, move(lvalues));
		}
	}
	return false;
}

bool ExpressionCompiler::visit(UnaryOperation const& _unaryOperation)
{
	CompilerContext::LocationSetter locationSetter(m_context, _unaryOperation);
	Type const& type = *_unaryOperation.annotation().type;
	if (type.category() == Type::Category::RationalNumber)
	{
		m_context << type.literalValue(nullptr);
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
			type.category() != Type::Category::FixedPoint,
			"Not yet implemented - FixedPointType."
		);
		m_currentLValue->retrieveValue(_unaryOperation.location());
		if (!_unaryOperation.isPrefixOperation())
		{
			// store value for later
			solUnimplementedAssert(type.sizeOnStack() == 1, "Stack size != 1 not implemented.");
			m_context << Instruction::DUP1;
			if (m_currentLValue->sizeOnStack() > 0)
				for (unsigned i = 1 + m_currentLValue->sizeOnStack(); i > 0; --i)
					m_context << swapInstruction(i);
		}
		if (_unaryOperation.getOperator() == Token::Inc)
		{
			if (m_context.arithmetic() == Arithmetic::Checked)
				m_context.callYulFunction(m_context.utilFunctions().incrementCheckedFunction(type), 1, 1);
			else
			{
				m_context << u256(1);
				m_context << Instruction::ADD;
			}
		}
		else
		{
			if (m_context.arithmetic() == Arithmetic::Checked)
				m_context.callYulFunction(m_context.utilFunctions().decrementCheckedFunction(type), 1, 1);
			else
			{
				m_context << u256(1);
				m_context << Instruction::SWAP1 << Instruction::SUB;
			}
		}
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
		solUnimplementedAssert(
			type.category() != Type::Category::FixedPoint,
			"Not yet implemented - FixedPointType."
		);
		if (m_context.arithmetic() == Arithmetic::Checked)
			m_context.callYulFunction(m_context.utilFunctions().negateNumberCheckedFunction(type), 1, 1);
		else
			m_context << u256(0) << Instruction::SUB;
		break;
	default:
		solAssert(false, "Invalid unary operator: " + string(TokenTraits::toString(_unaryOperation.getOperator())));
	}
	return false;
}

bool ExpressionCompiler::visit(BinaryOperation const& _binaryOperation)
{
	CompilerContext::LocationSetter locationSetter(m_context, _binaryOperation);
	Expression const& leftExpression = _binaryOperation.leftExpression();
	Expression const& rightExpression = _binaryOperation.rightExpression();
	solAssert(!!_binaryOperation.annotation().commonType, "");
	Type const* commonType = _binaryOperation.annotation().commonType;
	Token const c_op = _binaryOperation.getOperator();

	if (c_op == Token::And || c_op == Token::Or) // special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	else if (commonType->category() == Type::Category::RationalNumber)
		m_context << commonType->literalValue(nullptr);
	else
	{
		bool cleanupNeeded = cleanupNeededForOp(commonType->category(), c_op, m_context.arithmetic());

		Type const* leftTargetType = commonType;
		Type const* rightTargetType =
			TokenTraits::isShiftOp(c_op) || c_op == Token::Exp ?
			rightExpression.annotation().type->mobileType() :
			commonType;
		solAssert(rightTargetType, "");

		// for commutative operators, push the literal as late as possible to allow improved optimization
		auto isLiteral = [](Expression const& _e)
		{
			return dynamic_cast<Literal const*>(&_e) || _e.annotation().type->category() == Type::Category::RationalNumber;
		};
		bool swap = m_optimiseOrderLiterals && TokenTraits::isCommutativeOp(c_op) && isLiteral(rightExpression) && !isLiteral(leftExpression);
		if (swap)
		{
			acceptAndConvert(leftExpression, *leftTargetType, cleanupNeeded);
			acceptAndConvert(rightExpression, *rightTargetType, cleanupNeeded);
		}
		else
		{
			acceptAndConvert(rightExpression, *rightTargetType, cleanupNeeded);
			acceptAndConvert(leftExpression, *leftTargetType, cleanupNeeded);
		}
		if (TokenTraits::isShiftOp(c_op))
			// shift only cares about the signedness of both sides
			appendShiftOperatorCode(c_op, *leftTargetType, *rightTargetType);
		else if (c_op == Token::Exp)
			appendExpOperatorCode(*leftTargetType, *rightTargetType);
		else if (TokenTraits::isCompareOp(c_op))
			appendCompareOperatorCode(c_op, *commonType);
		else
			appendOrdinaryBinaryOperatorCode(c_op, *commonType);
	}

	// do not visit the child nodes, we already did that explicitly
	return false;
}

bool ExpressionCompiler::visit(FunctionCall const& _functionCall)
{
	auto functionCallKind = *_functionCall.annotation().kind;

	CompilerContext::LocationSetter locationSetter(m_context, _functionCall);
	if (functionCallKind == FunctionCallKind::TypeConversion)
	{
		solAssert(_functionCall.arguments().size() == 1, "");
		solAssert(_functionCall.names().empty(), "");
		auto const& expression = *_functionCall.arguments().front();
		auto const& targetType = *_functionCall.annotation().type;
		if (auto const* typeType = dynamic_cast<TypeType const*>(expression.annotation().type))
			if (auto const* addressType = dynamic_cast<AddressType const*>(&targetType))
			{
				auto const* contractType = dynamic_cast<ContractType const*>(typeType->actualType());
				solAssert(
					contractType &&
					contractType->contractDefinition().isLibrary() &&
					addressType->stateMutability() == StateMutability::NonPayable,
					""
				);
				m_context.appendLibraryAddress(contractType->contractDefinition().fullyQualifiedName());
				return false;
			}
		acceptAndConvert(expression, targetType);
		return false;
	}

	FunctionTypePointer functionType;
	if (functionCallKind == FunctionCallKind::StructConstructorCall)
	{
		auto const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());
		functionType = structType.constructorType();
	}
	else
		functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);

	TypePointers parameterTypes = functionType->parameterTypes();

	vector<ASTPointer<Expression const>> const& arguments = _functionCall.sortedArguments();

	if (functionCallKind == FunctionCallKind::StructConstructorCall)
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());

		utils().allocateMemory(max(u256(32u), structType.memoryDataSize()));
		m_context << Instruction::DUP1;

		for (unsigned i = 0; i < arguments.size(); ++i)
		{
			acceptAndConvert(*arguments[i], *functionType->parameterTypes()[i]);
			utils().storeInMemoryDynamic(*functionType->parameterTypes()[i]);
		}
		m_context << Instruction::POP;
	}
	else
	{
		FunctionType const& function = *functionType;
		if (function.bound())
			solAssert(
				function.kind() == FunctionType::Kind::DelegateCall ||
				function.kind() == FunctionType::Kind::Internal ||
				function.kind() == FunctionType::Kind::ArrayPush ||
				function.kind() == FunctionType::Kind::ArrayPop,
			"");
		switch (function.kind())
		{
		case FunctionType::Kind::Declaration:
			solAssert(false, "Attempted to generate code for calling a function definition.");
			break;
		case FunctionType::Kind::Internal:
		{
			// Calling convention: Caller pushes return address and arguments
			// Callee removes them and pushes return values

			evmasm::AssemblyItem returnLabel = m_context.pushNewTag();
			for (unsigned i = 0; i < arguments.size(); ++i)
				acceptAndConvert(*arguments[i], *function.parameterTypes()[i]);

			{
				bool shortcutTaken = false;
				if (auto identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
				{
					solAssert(!function.bound(), "");
					if (auto functionDef = dynamic_cast<FunctionDefinition const*>(identifier->annotation().referencedDeclaration))
					{
						// Do not directly visit the identifier, because this way, we can avoid
						// the runtime entry label to be created at the creation time context.
						CompilerContext::LocationSetter locationSetter2(m_context, *identifier);
						solAssert(*identifier->annotation().requiredLookup == VirtualLookup::Virtual, "");
						utils().pushCombinedFunctionEntryLabel(
							functionDef->resolveVirtual(m_context.mostDerivedContract()),
							false
						);
						shortcutTaken = true;
					}
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

			m_context.appendJump(evmasm::AssemblyItem::JumpType::IntoFunction);
			m_context << returnLabel;

			unsigned returnParametersSize = CompilerUtils::sizeOnStack(function.returnParameterTypes());
			// callee adds return parameters, but removes arguments and return label
			m_context.adjustStackOffset(static_cast<int>(returnParametersSize - parameterSize) - 1);
			break;
		}
		case FunctionType::Kind::BareCall:
		case FunctionType::Kind::BareDelegateCall:
		case FunctionType::Kind::BareStaticCall:
			solAssert(!_functionCall.annotation().tryCall, "");
			[[fallthrough]];
		case FunctionType::Kind::External:
		case FunctionType::Kind::DelegateCall:
			_functionCall.expression().accept(*this);
			appendExternalFunctionCall(function, arguments, _functionCall.annotation().tryCall);
			break;
		case FunctionType::Kind::BareCallCode:
			solAssert(false, "Callcode has been removed.");
		case FunctionType::Kind::Creation:
		{
			_functionCall.expression().accept(*this);
			// Stack: [salt], [value]

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
			utils().fetchFreeMemoryPointer();
			utils().copyContractCodeToMemory(*contract, true);
			utils().abiEncode(argumentTypes, function.parameterTypes());
			// now on stack: [salt], [value], memory_end_ptr
			// need: [salt], size, offset, value

			if (function.saltSet())
			{
				m_context << dupInstruction(2 + (function.valueSet() ? 1 : 0));
				m_context << Instruction::SWAP1;
			}

			// now: [salt], [value], [salt], memory_end_ptr
			utils().toSizeAfterFreeMemoryPointer();

			// now: [salt], [value], [salt], size, offset
			if (function.valueSet())
				m_context << dupInstruction(3 + (function.saltSet() ? 1 : 0));
			else
				m_context << u256(0);

			// now: [salt], [value], [salt], size, offset, value
			if (function.saltSet())
				m_context << Instruction::CREATE2;
			else
				m_context << Instruction::CREATE;

			// now: [salt], [value], address

			if (function.valueSet())
				m_context << swapInstruction(1) << Instruction::POP;
			if (function.saltSet())
				m_context << swapInstruction(1) << Instruction::POP;

			// Check if zero (reverted)
			m_context << Instruction::DUP1 << Instruction::ISZERO;
			if (_functionCall.annotation().tryCall)
			{
				// If this is a try call, return "<address> 1" in the success case and
				// "0" in the error case.
				AssemblyItem errorCase = m_context.appendConditionalJump();
				m_context << u256(1);
				m_context << errorCase;
			}
			else
				m_context.appendConditionalRevert(true);
			break;
		}
		case FunctionType::Kind::SetGas:
		{
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.expression().accept(*this);

			acceptAndConvert(*arguments.front(), *TypeProvider::uint256(), true);
			// Note that function is not the original function, but the ".gas" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			unsigned stackDepth = (function.gasSet() ? 1u : 0u) + (function.valueSet() ? 1u : 0u);
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
		{
			_functionCall.expression().accept(*this);
			// Provide the gas stipend manually at first because we may send zero ether.
			// Will be zeroed if we send more than zero ether.
			m_context << u256(evmasm::GasCosts::callStipend);
			acceptAndConvert(*arguments.front(), *function.parameterTypes().front(), true);
			// gas <- gas * !value
			m_context << Instruction::SWAP1 << Instruction::DUP2;
			m_context << Instruction::ISZERO << Instruction::MUL << Instruction::SWAP1;
			FunctionType::Options callOptions;
			callOptions.valueSet = true;
			callOptions.gasSet = true;
			appendExternalFunctionCall(
				FunctionType(
					TypePointers{},
					TypePointers{},
					strings(),
					strings(),
					FunctionType::Kind::BareCall,
					StateMutability::NonPayable,
					nullptr,
					callOptions
				),
				{},
				false
			);
			if (function.kind() == FunctionType::Kind::Transfer)
			{
				// Check if zero (out of stack or not enough balance).
				m_context << Instruction::ISZERO;
				// Revert message bubbles up.
				m_context.appendConditionalRevert(true);
			}
			break;
		}
		case FunctionType::Kind::Selfdestruct:
			acceptAndConvert(*arguments.front(), *function.parameterTypes().front(), true);
			m_context << Instruction::SELFDESTRUCT;
			break;
		case FunctionType::Kind::Revert:
		{
			if (arguments.empty())
				m_context.appendRevert();
			else
			{
				// function-sel(Error(string)) + encoding
				solAssert(arguments.size() == 1, "");
				solAssert(function.parameterTypes().size() == 1, "");
				if (m_context.revertStrings() == RevertStrings::Strip)
				{
					if (!*arguments.front()->annotation().isPure)
					{
						arguments.front()->accept(*this);
						utils().popStackElement(*arguments.front()->annotation().type);
					}
					m_context.appendRevert();
				}
				else
				{
					arguments.front()->accept(*this);
					utils().revertWithStringData(*arguments.front()->annotation().type);
				}
			}
			break;
		}
		case FunctionType::Kind::KECCAK256:
		{
			solAssert(arguments.size() == 1, "");
			solAssert(!function.padArguments(), "");
			Type const* argType = arguments.front()->annotation().type;
			solAssert(argType, "");
			arguments.front()->accept(*this);
			if (auto const* stringLiteral = dynamic_cast<StringLiteralType const*>(argType))
				// Optimization: Compute keccak256 on string literals at compile-time.
				m_context << u256(keccak256(stringLiteral->value()));
			else if (*argType == *TypeProvider::bytesMemory() || *argType == *TypeProvider::stringMemory())
			{
				// Optimization: If type is bytes or string, then do not encode,
				// but directly compute keccak256 on memory.
				ArrayUtils(m_context).retrieveLength(*TypeProvider::bytesMemory());
				m_context << Instruction::SWAP1 << u256(0x20) << Instruction::ADD;
				m_context << Instruction::KECCAK256;
			}
			else
			{
				utils().fetchFreeMemoryPointer();
				utils().packedEncode({argType}, TypePointers());
				utils().toSizeAfterFreeMemoryPointer();
				m_context << Instruction::KECCAK256;
			}
			break;
		}
		case FunctionType::Kind::Event:
		{
			_functionCall.expression().accept(*this);
			auto const& event = dynamic_cast<EventDefinition const&>(function.declaration());
			unsigned numIndexed = 0;
			TypePointers paramTypes = function.parameterTypes();
			// All indexed arguments go to the stack
			for (size_t arg = arguments.size(); arg > 0; --arg)
				if (event.parameters()[arg - 1]->isIndexed())
				{
					++numIndexed;
					arguments[arg - 1]->accept(*this);
					if (auto const& referenceType = dynamic_cast<ReferenceType const*>(paramTypes[arg - 1]))
					{
						utils().fetchFreeMemoryPointer();
						utils().packedEncode(
							{arguments[arg - 1]->annotation().type},
							{referenceType}
						);
						utils().toSizeAfterFreeMemoryPointer();
						m_context << Instruction::KECCAK256;
					}
					else
					{
						solAssert(paramTypes[arg - 1]->isValueType(), "");
						if (auto functionType =	dynamic_cast<FunctionType const*>(paramTypes[arg - 1]))
						{
							auto argumentType =
								dynamic_cast<FunctionType const*>(arguments[arg-1]->annotation().type);
							solAssert(
								argumentType &&
								functionType->kind() == FunctionType::Kind::External &&
								argumentType->kind() == FunctionType::Kind::External &&
								!argumentType->bound(),
								""
							);

							utils().combineExternalFunctionType(true);
						}
						else
							utils().convertType(
								*arguments[arg - 1]->annotation().type,
								*paramTypes[arg - 1],
								true
							);
					}
				}
			if (!event.isAnonymous())
			{
				m_context << u256(h256::Arith(keccak256(function.externalSignature())));
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
					nonIndexedParamTypes.push_back(paramTypes[arg]);
				}
			utils().fetchFreeMemoryPointer();
			utils().abiEncode(nonIndexedArgTypes, nonIndexedParamTypes);
			// need: topic1 ... topicn memsize memstart
			utils().toSizeAfterFreeMemoryPointer();
			m_context << logInstruction(numIndexed);
			break;
		}
		case FunctionType::Kind::Error:
		{
			_functionCall.expression().accept(*this);
			vector<Type const*> argumentTypes;
			for (ASTPointer<Expression const> const& arg: _functionCall.sortedArguments())
			{
				arg->accept(*this);
				argumentTypes.push_back(arg->annotation().type);
			}
			solAssert(dynamic_cast<ErrorDefinition const*>(&function.declaration()), "");
			utils().revertWithError(
				function.externalSignature(),
				function.parameterTypes(),
				argumentTypes
			);
			break;
		}
		case FunctionType::Kind::Wrap:
		case FunctionType::Kind::Unwrap:
		{
			solAssert(arguments.size() == 1, "");
			Type const* argumentType = arguments.at(0)->annotation().type;
			Type const* functionCallType = _functionCall.annotation().type;
			solAssert(argumentType, "");
			solAssert(functionCallType, "");
			FunctionType::Kind kind = functionType->kind();
			if (kind == FunctionType::Kind::Wrap)
			{
				solAssert(
					argumentType->isImplicitlyConvertibleTo(
						dynamic_cast<UserDefinedValueType const&>(*functionCallType).underlyingType()
					),
					""
				);
				solAssert(argumentType->isImplicitlyConvertibleTo(*function.parameterTypes()[0]), "");
			}
			else
				solAssert(
					dynamic_cast<UserDefinedValueType const&>(*argumentType) ==
					dynamic_cast<UserDefinedValueType const&>(*function.parameterTypes()[0]),
					""
				);

			acceptAndConvert(*arguments[0], *function.parameterTypes()[0]);
			break;
		}
		case FunctionType::Kind::BlockHash:
		{
			acceptAndConvert(*arguments[0], *function.parameterTypes()[0], true);
			m_context << Instruction::BLOCKHASH;
			break;
		}
		case FunctionType::Kind::AddMod:
		case FunctionType::Kind::MulMod:
		{
			acceptAndConvert(*arguments[2], *TypeProvider::uint256());
			m_context << Instruction::DUP1 << Instruction::ISZERO;
			m_context.appendConditionalPanic(util::PanicCode::DivisionByZero);
			for (unsigned i = 1; i < 3; i ++)
				acceptAndConvert(*arguments[2 - i], *TypeProvider::uint256());
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
			static map<FunctionType::Kind, u256> const contractAddresses{
				{FunctionType::Kind::ECRecover, 1},
				{FunctionType::Kind::SHA256, 2},
				{FunctionType::Kind::RIPEMD160, 3}
			};
			m_context << contractAddresses.at(function.kind());
			for (unsigned i = function.sizeOnStack(); i > 0; --i)
				m_context << swapInstruction(i);
			solAssert(!_functionCall.annotation().tryCall, "");
			appendExternalFunctionCall(function, arguments, false);
			break;
		}
		case FunctionType::Kind::ArrayPush:
		{
			solAssert(function.bound(), "");
			_functionCall.expression().accept(*this);

			if (function.parameterTypes().size() == 0)
			{
				auto paramType = function.returnParameterTypes().at(0);
				solAssert(paramType, "");

				ArrayType const* arrayType = dynamic_cast<ArrayType const*>(function.selfType());
				solAssert(arrayType, "");

				// stack: ArrayReference
				m_context << u256(1) << Instruction::DUP2;
				ArrayUtils(m_context).incrementDynamicArraySize(*arrayType);
				// stack: ArrayReference 1 newLength
				m_context << Instruction::SUB;
				// stack: ArrayReference (newLength-1)
				ArrayUtils(m_context).accessIndex(*arrayType, false);

				if (arrayType->isByteArrayOrString())
					setLValue<StorageByteArrayElement>(_functionCall);
				else
					setLValueToStorageItem(_functionCall);
			}
			else
			{
				solAssert(function.parameterTypes().size() == 1, "");
				solAssert(!!function.parameterTypes()[0], "");
				Type const* paramType = function.parameterTypes()[0];
				ArrayType const* arrayType = dynamic_cast<ArrayType const*>(function.selfType());
				solAssert(arrayType, "");

				// stack: ArrayReference
				arguments[0]->accept(*this);
				Type const* argType = arguments[0]->annotation().type;
				// stack: ArrayReference argValue
				utils().moveToStackTop(argType->sizeOnStack(), 1);
				// stack: argValue ArrayReference
				m_context << Instruction::DUP1;
				ArrayUtils(m_context).incrementDynamicArraySize(*arrayType);
				// stack: argValue ArrayReference newLength
				m_context << u256(1) << Instruction::SWAP1 << Instruction::SUB;
				// stack: argValue ArrayReference (newLength-1)
				ArrayUtils(m_context).accessIndex(*arrayType, false);
				// stack: argValue storageSlot slotOffset
				utils().moveToStackTop(2, argType->sizeOnStack());
				// stack: storageSlot slotOffset argValue
				Type const* type =
					arrayType->baseType()->dataStoredIn(DataLocation::Storage) ?
					arguments[0]->annotation().type->mobileType() :
					arrayType->baseType();
				solAssert(type, "");
				utils().convertType(*argType, *type);
				utils().moveToStackTop(1 + type->sizeOnStack());
				utils().moveToStackTop(1 + type->sizeOnStack());
				// stack: argValue storageSlot slotOffset
				if (!arrayType->isByteArrayOrString())
					StorageItem(m_context, *paramType).storeValue(*type, _functionCall.location(), true);
				else
					StorageByteArrayElement(m_context).storeValue(*type, _functionCall.location(), true);
			}
			break;
		}
		case FunctionType::Kind::ArrayPop:
		{
			_functionCall.expression().accept(*this);
			solAssert(function.bound(), "");
			solAssert(function.parameterTypes().empty(), "");
			ArrayType const* arrayType = dynamic_cast<ArrayType const*>(function.selfType());
			solAssert(arrayType && arrayType->dataStoredIn(DataLocation::Storage), "");
			ArrayUtils(m_context).popStorageArrayElement(*arrayType);
			break;
		}
		case FunctionType::Kind::StringConcat:
		case FunctionType::Kind::BytesConcat:
		{
			_functionCall.expression().accept(*this);
			vector<Type const*> argumentTypes;
			vector<Type const*> targetTypes;
			for (auto const& argument: arguments)
			{
				argument->accept(*this);
				solAssert(argument->annotation().type, "");
				argumentTypes.emplace_back(argument->annotation().type);
				if (argument->annotation().type->category() == Type::Category::FixedBytes)
					targetTypes.emplace_back(argument->annotation().type);
				else if (
					auto const* literalType = dynamic_cast<StringLiteralType const*>(argument->annotation().type);
					literalType && !literalType->value().empty() && literalType->value().size() <= 32
				)
					targetTypes.emplace_back(TypeProvider::fixedBytes(static_cast<unsigned>(literalType->value().size())));
				else
				{
					solAssert(!dynamic_cast<RationalNumberType const*>(argument->annotation().type), "");
					if (function.kind() == FunctionType::Kind::StringConcat)
					{
						solAssert(argument->annotation().type->isImplicitlyConvertibleTo(*TypeProvider::stringMemory()), "");
						targetTypes.emplace_back(TypeProvider::stringMemory());
					}
					else if (function.kind() == FunctionType::Kind::BytesConcat)
					{
						solAssert(argument->annotation().type->isImplicitlyConvertibleTo(*TypeProvider::bytesMemory()), "");
						targetTypes.emplace_back(TypeProvider::bytesMemory());
					}
				}
			}
			utils().fetchFreeMemoryPointer();
			// stack: <arg1> <arg2> ... <argn> <free mem>
			m_context << u256(32) << Instruction::ADD;
			utils().packedEncode(argumentTypes, targetTypes);
			utils().fetchFreeMemoryPointer();
			m_context.appendInlineAssembly(R"({
				mstore(mem_ptr, sub(sub(mem_end, mem_ptr), 0x20))
			})", {"mem_end", "mem_ptr"});
			m_context << Instruction::SWAP1;
			utils().storeFreeMemoryPointer();

			break;
		}
		case FunctionType::Kind::ObjectCreation:
		{
			ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*_functionCall.annotation().type);
			_functionCall.expression().accept(*this);
			solAssert(arguments.size() == 1, "");

			// Fetch requested length.
			acceptAndConvert(*arguments[0], *TypeProvider::uint256());

			// Make sure we can allocate memory without overflow
			m_context << u256(0xffffffffffffffff);
			m_context << Instruction::DUP2;
			m_context << Instruction::GT;
			m_context.appendConditionalPanic(PanicCode::ResourceError);

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
			if (arrayType.isByteArrayOrString())
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
			acceptAndConvert(*arguments.front(), *function.parameterTypes().front(), false);

			bool haveReasonString = arguments.size() > 1 && m_context.revertStrings() != RevertStrings::Strip;

			if (arguments.size() > 1)
			{
				// Users probably expect the second argument to be evaluated
				// even if the condition is false, as would be the case for an actual
				// function call.
				solAssert(arguments.size() == 2, "");
				solAssert(function.kind() == FunctionType::Kind::Require, "");
				if (m_context.revertStrings() == RevertStrings::Strip)
				{
					if (!*arguments.at(1)->annotation().isPure)
					{
						arguments.at(1)->accept(*this);
						utils().popStackElement(*arguments.at(1)->annotation().type);
					}
				}
				else
				{
					arguments.at(1)->accept(*this);
					utils().moveIntoStack(1, arguments.at(1)->annotation().type->sizeOnStack());
				}
			}
			// Stack: <error string (unconverted)> <condition>
			// jump if condition was met
			m_context << Instruction::ISZERO << Instruction::ISZERO;
			auto success = m_context.appendConditionalJump();
			if (function.kind() == FunctionType::Kind::Assert)
				// condition was not met, flag an error
				m_context.appendPanic(util::PanicCode::Assert);
			else if (haveReasonString)
			{
				utils().revertWithStringData(*arguments.at(1)->annotation().type);
				// Here, the argument is consumed, but in the other branch, it is still there.
				m_context.adjustStackOffset(static_cast<int>(arguments.at(1)->annotation().type->sizeOnStack()));
			}
			else
				m_context.appendRevert();
			// the success branch
			m_context << success;
			if (haveReasonString)
				utils().popStackElement(*arguments.at(1)->annotation().type);
			break;
		}
		case FunctionType::Kind::ABIEncode:
		case FunctionType::Kind::ABIEncodePacked:
		case FunctionType::Kind::ABIEncodeWithSelector:
		case FunctionType::Kind::ABIEncodeCall:
		case FunctionType::Kind::ABIEncodeWithSignature:
		{
			bool const isPacked = function.kind() == FunctionType::Kind::ABIEncodePacked;
			bool const hasSelectorOrSignature =
				function.kind() == FunctionType::Kind::ABIEncodeWithSelector ||
				function.kind() == FunctionType::Kind::ABIEncodeCall ||
				function.kind() == FunctionType::Kind::ABIEncodeWithSignature;

			TypePointers argumentTypes;
			TypePointers targetTypes;

			ASTNode::listAccept(arguments, *this);

			if (function.kind() == FunctionType::Kind::ABIEncodeCall)
			{
				solAssert(arguments.size() == 2);

				// Account for tuples with one component which become that component
				if (auto const tupleType = dynamic_cast<TupleType const*>(arguments[1]->annotation().type))
					argumentTypes = tupleType->components();
				else
					argumentTypes.emplace_back(arguments[1]->annotation().type);

				auto functionPtr = dynamic_cast<FunctionTypePointer>(arguments[0]->annotation().type);
				solAssert(functionPtr);
				functionPtr = functionPtr->asExternallyCallableFunction(false);
				solAssert(functionPtr);
				targetTypes = functionPtr->parameterTypes();
			}
			else
				for (unsigned i = 0; i < arguments.size(); ++i)
				{
					// Do not keep the selector as part of the ABI encoded args
					if (!hasSelectorOrSignature || i > 0)
						argumentTypes.push_back(arguments[i]->annotation().type);
				}

			utils().fetchFreeMemoryPointer();
			// stack now: [<selector/functionPointer/signature>] <arg1> .. <argN> <free_mem>

			// adjust by 32(+4) bytes to accommodate the length(+selector)
			m_context << u256(32 + (hasSelectorOrSignature ? 4 : 0)) << Instruction::ADD;
			// stack now: [<selector/functionPointer/signature>] <arg1> .. <argN> <data_encoding_area_start>

			if (isPacked)
			{
				solAssert(!function.padArguments(), "");
				utils().packedEncode(argumentTypes, targetTypes);
			}
			else
			{
				solAssert(function.padArguments(), "");
				utils().abiEncode(argumentTypes, targetTypes);
			}
			utils().fetchFreeMemoryPointer();
			// stack: [<selector/functionPointer/signature>] <data_encoding_area_end> <bytes_memory_ptr>

			// size is end minus start minus length slot
			m_context.appendInlineAssembly(R"({
				mstore(mem_ptr, sub(sub(mem_end, mem_ptr), 0x20))
			})", {"mem_end", "mem_ptr"});
			m_context << Instruction::SWAP1;
			utils().storeFreeMemoryPointer();
			// stack: [<selector/functionPointer/signature>] <memory ptr>

			if (hasSelectorOrSignature)
			{
				// stack: <selector/functionPointer/signature> <memory pointer>
				solAssert(arguments.size() >= 1, "");
				Type const* selectorType = arguments[0]->annotation().type;
				utils().moveIntoStack(selectorType->sizeOnStack());
				Type const* dataOnStack = selectorType;

				// stack: <memory pointer> <selector/functionPointer/signature>
				if (function.kind() == FunctionType::Kind::ABIEncodeWithSignature)
				{
					// hash the signature
					if (auto const* stringType = dynamic_cast<StringLiteralType const*>(selectorType))
					{
						m_context << util::selectorFromSignature(stringType->value());
						dataOnStack = TypeProvider::fixedBytes(4);
					}
					else
					{
						utils().fetchFreeMemoryPointer();
						// stack: <memory pointer> <signature> <free mem ptr>
						utils().packedEncode(TypePointers{selectorType}, TypePointers());
						utils().toSizeAfterFreeMemoryPointer();
						m_context << Instruction::KECCAK256;
						// stack: <memory pointer> <hash>

						dataOnStack = TypeProvider::fixedBytes(32);
					}
				}
				else if (function.kind() == FunctionType::Kind::ABIEncodeCall)
				{
					auto const& funType = dynamic_cast<FunctionType const&>(*selectorType);
					if (funType.kind() == FunctionType::Kind::Declaration)
					{
						solAssert(funType.hasDeclaration());
						solAssert(selectorType->sizeOnStack() == 0);
						m_context << funType.externalIdentifier();
					}
					else
					{
						solAssert(selectorType->sizeOnStack() == 2);
						// stack: <memory pointer> <functionPointer>
						// Extract selector from the stack
						m_context << Instruction::SWAP1 << Instruction::POP;
					}
					// Conversion will be done below
					dataOnStack = TypeProvider::uint(32);
				}
				else
					solAssert(function.kind() == FunctionType::Kind::ABIEncodeWithSelector, "");

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
			Type const* firstArgType = arguments.front()->annotation().type;
			TypePointers targetTypes;
			if (TupleType const* targetTupleType = dynamic_cast<TupleType const*>(_functionCall.annotation().type))
				targetTypes = targetTupleType->components();
			else
				targetTypes = TypePointers{_functionCall.annotation().type};
			if (
				auto referenceType = dynamic_cast<ReferenceType const*>(firstArgType);
				referenceType && referenceType->dataStoredIn(DataLocation::CallData)
			)
			{
				solAssert(referenceType->isImplicitlyConvertibleTo(*TypeProvider::bytesCalldata()), "");
				utils().convertType(*referenceType, *TypeProvider::bytesCalldata());
				utils().abiDecode(targetTypes, false);
			}
			else
			{
				utils().convertType(*firstArgType, *TypeProvider::bytesMemory());
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
		case FunctionType::Kind::MetaType:
			// No code to generate.
			break;
		}
	}
	return false;
}

bool ExpressionCompiler::visit(FunctionCallOptions const& _functionCallOptions)
{
	_functionCallOptions.expression().accept(*this);

	// Desired Stack: [salt], [gas], [value]
	enum Option { Salt, Gas, Value };

	vector<Option> presentOptions;
	FunctionType const& funType = dynamic_cast<FunctionType const&>(
		*_functionCallOptions.expression().annotation().type
	);
	if (funType.saltSet()) presentOptions.emplace_back(Salt);
	if (funType.gasSet()) presentOptions.emplace_back(Gas);
	if (funType.valueSet()) presentOptions.emplace_back(Value);

	for (size_t i = 0; i < _functionCallOptions.options().size(); ++i)
	{
		string const& name = *_functionCallOptions.names()[i];
		Type const* requiredType = TypeProvider::uint256();
		Option newOption;
		if (name == "salt")
		{
			newOption = Salt;
			requiredType = TypeProvider::fixedBytes(32);
		}
		else if (name == "gas")
			newOption = Gas;
		else if (name == "value")
			newOption = Value;
		else
			solAssert(false, "Unexpected option name!");
		acceptAndConvert(*_functionCallOptions.options()[i], *requiredType);

		solAssert(!util::contains(presentOptions, newOption), "");
		ptrdiff_t insertPos = presentOptions.end() - lower_bound(presentOptions.begin(), presentOptions.end(), newOption);

		utils().moveIntoStack(static_cast<unsigned>(insertPos), 1);
		presentOptions.insert(presentOptions.end() - insertPos, newOption);
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
	if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type))
		if (funType->bound())
		{
			acceptAndConvert(_memberAccess.expression(), *funType->selfType(), true);
			if (funType->kind() == FunctionType::Kind::Internal)
			{
				FunctionDefinition const& funDef = dynamic_cast<decltype(funDef)>(funType->declaration());
				solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");
				utils().pushCombinedFunctionEntryLabel(funDef);
				utils().moveIntoStack(funType->selfType()->sizeOnStack(), 1);
			}
			else if (
				funType->kind() == FunctionType::Kind::ArrayPop ||
				funType->kind() == FunctionType::Kind::ArrayPush
			)
			{
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
	if (TypeType const* type = dynamic_cast<TypeType const*>(_memberAccess.expression().annotation().type))
	{
		if (auto contractType = dynamic_cast<ContractType const*>(type->actualType()))
		{
			solAssert(_memberAccess.annotation().type, "_memberAccess has no type");
			if (contractType->isSuper())
			{
				_memberAccess.expression().accept(*this);
				solAssert(_memberAccess.annotation().referencedDeclaration, "Referenced declaration not resolved.");
				solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Super, "");
				utils().pushCombinedFunctionEntryLabel(m_context.superFunction(
					dynamic_cast<FunctionDefinition const&>(*_memberAccess.annotation().referencedDeclaration),
					contractType->contractDefinition()
				));
			}
			else
			{
				if (auto variable = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
					appendVariable(*variable, static_cast<Expression const&>(_memberAccess));
				else if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type))
				{
					switch (funType->kind())
					{
					case FunctionType::Kind::Declaration:
						break;
					case FunctionType::Kind::Internal:
						// We do not visit the expression here on purpose, because in the case of an
						// internal library function call, this would push the library address forcing
						// us to link against it although we actually do not need it.
						if (auto const* function = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration))
						{
							solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");
							utils().pushCombinedFunctionEntryLabel(*function);
						}
						else
							solAssert(false, "Function not found in member access");
						break;
					case FunctionType::Kind::Event:
						if (!dynamic_cast<EventDefinition const*>(_memberAccess.annotation().referencedDeclaration))
							solAssert(false, "event not found");
						// no-op, because the parent node will do the job
						break;
					case FunctionType::Kind::Error:
						if (!dynamic_cast<ErrorDefinition const*>(_memberAccess.annotation().referencedDeclaration))
							solAssert(false, "error not found");
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
					case FunctionType::Kind::ECRecover:
					case FunctionType::Kind::SHA256:
					case FunctionType::Kind::RIPEMD160:
					default:
						solAssert(false, "unsupported member function");
					}
				}
				else if (dynamic_cast<TypeType const*>(_memberAccess.annotation().type))
				{
					// no-op
				}
				else
					_memberAccess.expression().accept(*this);
			}
		}
		else if (auto enumType = dynamic_cast<EnumType const*>(type->actualType()))
		{
			_memberAccess.expression().accept(*this);
			m_context << enumType->memberValue(_memberAccess.memberName());
		}
		else
			_memberAccess.expression().accept(*this);
		return false;
	}
	// Another special case for `this.f.selector` and for ``C.f.selector`` which do not need the address.
	// There are other uses of `.selector` which do need the address, but we want these
	// specific uses to be pure expressions.
	if (
		auto const* functionType = dynamic_cast<FunctionType const*>(_memberAccess.expression().annotation().type);
		functionType && member == "selector"
	)
	{
		if (functionType->hasDeclaration())
		{
			if (functionType->kind() == FunctionType::Kind::Event)
				m_context << u256(h256::Arith(util::keccak256(functionType->externalSignature())));
			else
			{
				m_context << functionType->externalIdentifier();
				/// need to store it as bytes4
				utils().leftShiftNumberOnStack(224);
			}
			return false;
		}
		else if (auto const* expr = dynamic_cast<MemberAccess const*>(&_memberAccess.expression()))
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
						/// need to store it as bytes4
						utils().leftShiftNumberOnStack(224);
						return false;
					}
	}
	// Another special case for `address(this).balance`. Post-Istanbul, we can use the selfbalance
	// opcode.
	if (
		m_context.evmVersion().hasSelfBalance() &&
		member == "balance" &&
		_memberAccess.expression().annotation().type->category() == Type::Category::Address
	)
		if (FunctionCall const* funCall = dynamic_cast<FunctionCall const*>(&_memberAccess.expression()))
			if (auto const* addr = dynamic_cast<ElementaryTypeNameExpression const*>(&funCall->expression()))
				if (
					addr->type().typeName().token() == Token::Address &&
					funCall->arguments().size() == 1
				)
					if (auto arg = dynamic_cast<Identifier const*>( funCall->arguments().front().get()))
						if (
							arg->name() == "this" &&
							dynamic_cast<MagicVariableDeclaration const*>(arg->annotation().referencedDeclaration)
						)
						{
							m_context << Instruction::SELFBALANCE;
							return false;
						}

	// Another special case for `address.code.length`, which should simply call extcodesize
	if (
		auto innerExpression = dynamic_cast<MemberAccess const*>(&_memberAccess.expression());
		member == "length" &&
		innerExpression &&
		innerExpression->memberName() == "code" &&
		innerExpression->expression().annotation().type->category() == Type::Category::Address
	)
	{
		solAssert(innerExpression->annotation().type->category() == Type::Category::Array, "");

		innerExpression->expression().accept(*this);

		utils().convertType(
			*innerExpression->expression().annotation().type,
			*TypeProvider::address(),
			true
		);
		m_context << Instruction::EXTCODESIZE;

		return false;
	}

	_memberAccess.expression().accept(*this);
	switch (_memberAccess.expression().annotation().type->category())
	{
	case Type::Category::Contract:
	{
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.expression().annotation().type);
		// ordinary contract type
		if (Declaration const* declaration = _memberAccess.annotation().referencedDeclaration)
		{
			u256 identifier;
			if (auto const* variable = dynamic_cast<VariableDeclaration const*>(declaration))
				identifier = FunctionType(*variable).externalIdentifier();
			else if (auto const* function = dynamic_cast<FunctionDefinition const*>(declaration))
				identifier = FunctionType(*function).externalIdentifier();
			else
				solAssert(false, "Contract member is neither variable nor function.");
			utils().convertType(type, type.isPayable() ? *TypeProvider::payableAddress() : *TypeProvider::address(), true);
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
				*TypeProvider::address(),
				true
			);
			m_context << Instruction::BALANCE;
		}
		else if (member == "code")
		{
			// Stack: <address>
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				*TypeProvider::address(),
				true
			);

			m_context << Instruction::DUP1 << Instruction::EXTCODESIZE;
			// Stack post: <address> <size>

			m_context << Instruction::DUP1;
			// Account for the size field of `bytes memory`
			m_context << u256(32) << Instruction::ADD;
			utils().allocateMemory();
			// Stack post: <address> <size> <mem_offset>

			// Store size at mem_offset
			m_context << Instruction::DUP2 << Instruction::DUP2 << Instruction::MSTORE;

			m_context << u256(0) << Instruction::SWAP1 << Instruction::DUP1;
			// Stack post: <address> <size> 0 <mem_offset> <mem_offset>

			m_context << u256(32) << Instruction::ADD << Instruction::SWAP1;
			// Stack post: <address> <size> 0 <mem_offset_adjusted> <mem_offset>

			m_context << Instruction::SWAP4;
			// Stack post: <mem_offset> <size> 0 <mem_offset_adjusted> <address>

			m_context << Instruction::EXTCODECOPY;
			// Stack post: <mem_offset>
		}
		else if (member == "codehash")
		{
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				*TypeProvider::address(),
				true
			);
			m_context << Instruction::EXTCODEHASH;
		}
		else if ((set<string>{"send", "transfer"}).count(member))
		{
			solAssert(dynamic_cast<AddressType const&>(*_memberAccess.expression().annotation().type).stateMutability() == StateMutability::Payable, "");
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				AddressType(StateMutability::Payable),
				true
			);
		}
		else if ((set<string>{"call", "callcode", "delegatecall", "staticcall"}).count(member))
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				*TypeProvider::address(),
				true
			);
		else
			solAssert(false, "Invalid member access to address");
		break;
	}
	case Type::Category::Function:
		if (member == "selector")
		{
			auto const& functionType = dynamic_cast<FunctionType const&>(*_memberAccess.expression().annotation().type);
			// all events should have already been caught by this stage
			solAssert(!(functionType.kind() == FunctionType::Kind::Event));

			if (functionType.kind() == FunctionType::Kind::External)
				CompilerUtils(m_context).popStackSlots(functionType.sizeOnStack() - 2);
			m_context << Instruction::SWAP1 << Instruction::POP;

			/// need to store it as bytes4
			utils().leftShiftNumberOnStack(224);
		}
		else if (member == "address")
		{
			auto const& functionType = dynamic_cast<FunctionType const&>(*_memberAccess.expression().annotation().type);
			solAssert(functionType.kind() == FunctionType::Kind::External, "");
			CompilerUtils(m_context).popStackSlots(functionType.sizeOnStack() - 1);
		}
		else
			solAssert(
				!!_memberAccess.expression().annotation().type->memberType(member),
				"Invalid member access to function."
			);
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
		else if (member == "gasprice")
			m_context << Instruction::GASPRICE;
		else if (member == "chainid")
			m_context << Instruction::CHAINID;
		else if (member == "basefee")
			m_context << Instruction::BASEFEE;
		else if (member == "data")
			m_context << u256(0) << Instruction::CALLDATASIZE;
		else if (member == "sig")
			m_context << u256(0) << Instruction::CALLDATALOAD
				<< (u256(0xffffffff) << (256 - 32)) << Instruction::AND;
		else if (member == "gas")
			solAssert(false, "Gas has been removed.");
		else if (member == "blockhash")
			solAssert(false, "Blockhash has been removed.");
		else if (member == "creationCode" || member == "runtimeCode")
		{
			Type const* arg = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type).typeArgument();
			auto const& contractType = dynamic_cast<ContractType const&>(*arg);
			solAssert(!contractType.isSuper(), "");
			ContractDefinition const& contract = contractType.contractDefinition();
			utils().fetchFreeMemoryPointer();
			m_context << Instruction::DUP1 << u256(32) << Instruction::ADD;
			utils().copyContractCodeToMemory(contract, member == "creationCode");
			// Stack: start end
			m_context.appendInlineAssembly(
				Whiskers(R"({
					mstore(start, sub(end, add(start, 0x20)))
					mstore(<free>, and(add(end, 31), not(31)))
				})")("free", to_string(CompilerUtils::freeMemoryPointer)).render(),
				{"start", "end"}
			);
			m_context << Instruction::POP;
		}
		else if (member == "name")
		{
			Type const* arg = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type).typeArgument();
			auto const& contractType = dynamic_cast<ContractType const&>(*arg);
			ContractDefinition const& contract = contractType.isSuper() ?
				*contractType.contractDefinition().superContract(m_context.mostDerivedContract()) :
				dynamic_cast<ContractType const&>(*arg).contractDefinition();
			utils().allocateMemory(((contract.name().length() + 31) / 32) * 32 + 32);
			// store string length
			m_context << u256(contract.name().length()) << Instruction::DUP2 << Instruction::MSTORE;
			// adjust pointer
			m_context << Instruction::DUP1 << u256(32) << Instruction::ADD;
			utils().storeStringData(contract.name());
		}
		else if (member == "interfaceId")
		{
			Type const* arg = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type).typeArgument();
			ContractDefinition const& contract = dynamic_cast<ContractType const&>(*arg).contractDefinition();
			m_context << (u256{contract.interfaceId()} << (256 - 32));
		}
		else if (member == "min" || member == "max")
		{
			MagicType const* arg = dynamic_cast<MagicType const*>(_memberAccess.expression().annotation().type);
			if (IntegerType const* integerType = dynamic_cast<IntegerType const*>(arg->typeArgument()))
				m_context << (member == "min" ? integerType->min() : integerType->max());
			else if (EnumType const* enumType = dynamic_cast<EnumType const*>(arg->typeArgument()))
				m_context << (member == "min" ? enumType->minValue() : enumType->maxValue());
			else
				solAssert(false, "min/max not available for the given type.");

		}
		else if ((set<string>{"encode", "encodePacked", "encodeWithSelector", "encodeWithSignature", "decode"}).count(member))
		{
			// no-op
		}
		else
			solAssert(false, "Unknown magic member.");
		break;
	case Type::Category::Struct:
	{
		StructType const& type = dynamic_cast<StructType const&>(*_memberAccess.expression().annotation().type);
		Type const* memberType = _memberAccess.annotation().type;
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
			setLValue<MemoryItem>(_memberAccess, *memberType);
			break;
		}
		case DataLocation::CallData:
		{
			if (_memberAccess.annotation().type->isDynamicallyEncoded())
			{
				m_context << Instruction::DUP1;
				m_context << type.calldataOffsetOfMember(member) << Instruction::ADD;
				CompilerUtils(m_context).accessCalldataTail(*memberType);
			}
			else
			{
				m_context << type.calldataOffsetOfMember(member) << Instruction::ADD;
				// For non-value types the calldata offset is returned directly.
				if (memberType->isValueType())
				{
					solAssert(memberType->calldataEncodedSize() > 0, "");
					solAssert(memberType->storageBytes() <= 32, "");
					if (memberType->storageBytes() < 32 && m_context.useABICoderV2())
					{
						m_context << u256(32);
						CompilerUtils(m_context).abiDecodeV2({memberType}, false);
					}
					else
						CompilerUtils(m_context).loadFromMemoryDynamic(*memberType, true, true, false);
				}
				else
					solAssert(
						memberType->category() == Type::Category::Array ||
						memberType->category() == Type::Category::Struct,
						""
					);
			}
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
					ArrayUtils(m_context).retrieveLength(type);
					m_context << Instruction::SWAP1 << Instruction::POP;
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
	case Type::Category::Module:
	{
		Type::Category category = _memberAccess.annotation().type->category();
		solAssert(
			dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration) ||
			dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration) ||
			dynamic_cast<ErrorDefinition const*>(_memberAccess.annotation().referencedDeclaration) ||
			category == Type::Category::TypeType ||
			category == Type::Category::Module,
			""
		);
		if (auto variable = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
		{
			solAssert(variable->isConstant(), "");
			appendVariable(*variable, static_cast<Expression const&>(_memberAccess));
		}
		else if (auto const* function = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration))
		{
			auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type);
			solAssert(function && function->isFree(), "");
			solAssert(funType->kind() == FunctionType::Kind::Internal, "");
			solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");
			utils().pushCombinedFunctionEntryLabel(*function);
		}
		else if (auto const* contract = dynamic_cast<ContractDefinition const*>(_memberAccess.annotation().referencedDeclaration))
		{
			if (contract->isLibrary())
				m_context.appendLibraryAddress(contract->fullyQualifiedName());
		}
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

	switch (baseType.category())
	{
		case Type::Category::Mapping:
		{
			// stack: storage_base_ref
			Type const* keyType = dynamic_cast<MappingType const&>(baseType).keyType();
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
				utils().storeInMemoryDynamic(*TypeProvider::uint256());
				utils().toSizeAfterFreeMemoryPointer();
			}
			else
			{
				m_context << u256(0); // memory position
				appendExpressionCopyToMemory(*keyType, *_indexAccess.indexExpression());
				m_context << Instruction::SWAP1;
				solAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");
				utils().storeInMemoryDynamic(*TypeProvider::uint256());
				m_context << u256(0);
			}
			m_context << Instruction::KECCAK256;
			m_context << u256(0);
			setLValueToStorageItem(_indexAccess);
			break;
		}
		case Type::Category::ArraySlice:
		{
			auto const& arrayType = dynamic_cast<ArraySliceType const&>(baseType).arrayType();
			solAssert(
				arrayType.location() == DataLocation::CallData &&
				arrayType.isDynamicallySized() &&
				!arrayType.baseType()->isDynamicallyEncoded(),
				""
			);
			solAssert(_indexAccess.indexExpression(), "Index expression expected.");

			acceptAndConvert(*_indexAccess.indexExpression(), *TypeProvider::uint256(), true);
			ArrayUtils(m_context).accessCallDataArrayElement(arrayType);
			break;

		}
		case Type::Category::Array:
		{
			ArrayType const& arrayType = dynamic_cast<ArrayType const&>(baseType);
			solAssert(_indexAccess.indexExpression(), "Index expression expected.");

			acceptAndConvert(*_indexAccess.indexExpression(), *TypeProvider::uint256(), true);
			// stack layout: <base_ref> [<length>] <index>
			switch (arrayType.location())
			{
				case DataLocation::Storage:
					ArrayUtils(m_context).accessIndex(arrayType);
					if (arrayType.isByteArrayOrString())
					{
						solAssert(!arrayType.isString(), "Index access to string is not allowed.");
						setLValue<StorageByteArrayElement>(_indexAccess);
					}
					else
						setLValueToStorageItem(_indexAccess);
					break;
				case DataLocation::Memory:
					ArrayUtils(m_context).accessIndex(arrayType);
					setLValue<MemoryItem>(_indexAccess, *_indexAccess.annotation().type, !arrayType.isByteArrayOrString());
					break;
				case DataLocation::CallData:
					ArrayUtils(m_context).accessCallDataArrayElement(arrayType);
					break;
			}
			break;
		}
		case Type::Category::FixedBytes:
		{
			FixedBytesType const& fixedBytesType = dynamic_cast<FixedBytesType const&>(baseType);
			solAssert(_indexAccess.indexExpression(), "Index expression expected.");

			acceptAndConvert(*_indexAccess.indexExpression(), *TypeProvider::uint256(), true);
			// stack layout: <value> <index>
			// check out-of-bounds access
			m_context << u256(fixedBytesType.numBytes());
			m_context << Instruction::DUP2 << Instruction::LT << Instruction::ISZERO;
			// out-of-bounds access throws exception
			m_context.appendConditionalPanic(util::PanicCode::ArrayOutOfBounds);

			m_context << Instruction::BYTE;
			utils().leftShiftNumberOnStack(256 - 8);
			break;
		}
		case Type::Category::TypeType:
		{
			solAssert(baseType.sizeOnStack() == 0, "");
			solAssert(_indexAccess.annotation().type->sizeOnStack() == 0, "");
			// no-op - this seems to be a lone array type (`structType[];`)
			break;
		}
		default:
			solAssert(false, "Index access only allowed for mappings or arrays.");
			break;
	}

	return false;
}

bool ExpressionCompiler::visit(IndexRangeAccess const& _indexAccess)
{
	CompilerContext::LocationSetter locationSetter(m_context, _indexAccess);
	_indexAccess.baseExpression().accept(*this);
	// stack: offset length

	Type const& baseType = *_indexAccess.baseExpression().annotation().type;

	ArrayType const *arrayType = dynamic_cast<ArrayType const*>(&baseType);
	if (!arrayType)
		if (ArraySliceType const* sliceType = dynamic_cast<ArraySliceType const*>(&baseType))
			arrayType = &sliceType->arrayType();

	solAssert(arrayType, "");
	solUnimplementedAssert(
		arrayType->location() == DataLocation::CallData &&
		arrayType->isDynamicallySized() &&
		!arrayType->baseType()->isDynamicallyEncoded()
	);

	if (_indexAccess.startExpression())
		acceptAndConvert(*_indexAccess.startExpression(), *TypeProvider::uint256());
	else
		m_context << u256(0);
	// stack: offset length sliceStart

	m_context << Instruction::SWAP1;
	// stack: offset sliceStart length

	if (_indexAccess.endExpression())
		acceptAndConvert(*_indexAccess.endExpression(), *TypeProvider::uint256());
	else
		m_context << Instruction::DUP1;
	// stack: offset sliceStart length sliceEnd

	m_context << Instruction::SWAP3;
	// stack: sliceEnd sliceStart length offset

	m_context.callYulFunction(m_context.utilFunctions().calldataArrayIndexRangeAccess(*arrayType), 4, 2);

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
			if (dynamic_cast<ContractType const*>(magicVar->type()))
			{
				solAssert(_identifier.name() == "this", "");
				m_context << Instruction::ADDRESS;
			}
			break;
		default:
			break;
		}
	}
	else if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
	{
		// If the identifier is called right away, this code is executed in visit(FunctionCall...), because
		// we want to avoid having a reference to the runtime function entry point in the
		// constructor context, since this would force the compiler to include unreferenced
		// internal functions in the runtime context.
		solAssert(*_identifier.annotation().requiredLookup == VirtualLookup::Virtual, "");
		utils().pushCombinedFunctionEntryLabel(functionDef->resolveVirtual(m_context.mostDerivedContract()));
	}
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
	else if (dynamic_cast<ErrorDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<EnumDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<UserDefinedValueTypeDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<StructDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<ImportDirective const*>(declaration))
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

	if (auto identifierPath = get_if<ASTPointer<IdentifierPath>>(&_literal.suffix()))
	{
		FunctionDefinition const& function = dynamic_cast<FunctionDefinition const&>(
			*(*identifierPath)->annotation().referencedDeclaration
		);
		FunctionType const& functionType = *function.functionType(true);

		evmasm::AssemblyItem returnLabel = m_context.pushNewTag();

		// TODO this is actually not always the right one.
		auto type = TypeProvider::forLiteral(_literal);

		if (dynamic_cast<RationalNumberType const*>(type) && dynamic_cast<RationalNumberType const*>(type)->isFractional())
		{
			auto&& [mantissa, exponent] = dynamic_cast<RationalNumberType const*>(type)->mantissaExponent();
			m_context << mantissa->literalValue(nullptr);
			utils().convertType(*mantissa, *functionType.parameterTypes().at(0));
			m_context << exponent->literalValue(nullptr);
			utils().convertType(*exponent, *functionType.parameterTypes().at(1));
		}
		else
		{
			// TODO reuse the code below
			if (type->category() != Type::Category::StringLiteral)
				m_context << type->literalValue(&_literal);
			utils().convertType(*type, *functionType.parameterTypes().at(0));
		}
		m_context << m_context.functionEntryLabel(function).pushTag();
		m_context.appendJump(evmasm::AssemblyItem::JumpType::IntoFunction);
		m_context << returnLabel;

		// callee adds return parameters, but removes arguments and return label
		m_context.adjustStackOffset(static_cast<int>(
			CompilerUtils::sizeOnStack(functionType.returnParameterTypes()) -
			CompilerUtils::sizeOnStack(functionType.parameterTypes()) -
			1
		));
	}
	else
	{
		Type const* type = _literal.annotation().type;


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
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation const& _binaryOperation)
{
	Token const c_op = _binaryOperation.getOperator();
	solAssert(c_op == Token::Or || c_op == Token::And, "");

	_binaryOperation.leftExpression().accept(*this);
	m_context << Instruction::DUP1;
	if (c_op == Token::And)
		m_context << Instruction::ISZERO;
	evmasm::AssemblyItem endLabel = m_context.appendConditionalJump();
	m_context << Instruction::POP;
	_binaryOperation.rightExpression().accept(*this);
	m_context << endLabel;
}

void ExpressionCompiler::appendCompareOperatorCode(Token _operator, Type const& _type)
{
	if (_operator == Token::Equal || _operator == Token::NotEqual)
	{
		FunctionType const* functionType = dynamic_cast<decltype(functionType)>(&_type);
		if (functionType && functionType->kind() == FunctionType::Kind::External)
		{
			solUnimplementedAssert(functionType->sizeOnStack() == 2, "");
			m_context << Instruction::SWAP3;

			m_context << ((u256(1) << 160) - 1) << Instruction::AND;
			m_context << Instruction::SWAP1;
			m_context << ((u256(1) << 160) - 1) << Instruction::AND;
			m_context << Instruction::EQ;
			m_context << Instruction::SWAP2;
			m_context << ((u256(1) << 32) - 1) << Instruction::AND;
			m_context << Instruction::SWAP1;
			m_context << ((u256(1) << 32) - 1) << Instruction::AND;
			m_context << Instruction::EQ;
			m_context << Instruction::AND;
		}
		else
		{
			solAssert(_type.sizeOnStack() == 1, "Comparison of multi-slot types.");
			if (functionType && functionType->kind() == FunctionType::Kind::Internal)
			{
				// We have to remove the upper bits (construction time value) because they might
				// be "unknown" in one of the operands and not in the other.
				m_context << ((u256(1) << 32) - 1) << Instruction::AND;
				m_context << Instruction::SWAP1;
				m_context << ((u256(1) << 32) - 1) << Instruction::AND;
			}
			m_context << Instruction::EQ;
		}
		if (_operator == Token::NotEqual)
			m_context << Instruction::ISZERO;
	}
	else
	{
		solAssert(_type.sizeOnStack() == 1, "Comparison of multi-slot types.");
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

void ExpressionCompiler::appendOrdinaryBinaryOperatorCode(Token _operator, Type const& _type)
{
	if (TokenTraits::isArithmeticOp(_operator))
		appendArithmeticOperatorCode(_operator, _type);
	else if (TokenTraits::isBitOp(_operator))
		appendBitOperatorCode(_operator);
	else
		solAssert(false, "Unknown binary operator.");
}

void ExpressionCompiler::appendArithmeticOperatorCode(Token _operator, Type const& _type)
{
	if (_type.category() == Type::Category::FixedPoint)
		solUnimplemented("Not yet implemented - FixedPointType.");

	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	if (m_context.arithmetic() == Arithmetic::Checked)
	{
		string functionName;
		switch (_operator)
		{
		case Token::Add:
			functionName = m_context.utilFunctions().overflowCheckedIntAddFunction(type);
			break;
		case Token::Sub:
			functionName = m_context.utilFunctions().overflowCheckedIntSubFunction(type);
			break;
		case Token::Mul:
			functionName = m_context.utilFunctions().overflowCheckedIntMulFunction(type);
			break;
		case Token::Div:
			functionName = m_context.utilFunctions().overflowCheckedIntDivFunction(type);
			break;
		case Token::Mod:
			functionName = m_context.utilFunctions().intModFunction(type);
			break;
		case Token::Exp:
			// EXP is handled in a different function.
		default:
			solAssert(false, "Unknown arithmetic operator.");
		}
		// TODO Maybe we want to force-inline this?
		m_context.callYulFunction(functionName, 2, 1);
	}
	else
	{
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
			m_context.appendConditionalPanic(util::PanicCode::DivisionByZero);

			if (_operator == Token::Div)
				m_context << (c_isSigned ? Instruction::SDIV : Instruction::DIV);
			else
				m_context << (c_isSigned ? Instruction::SMOD : Instruction::MOD);
			break;
		}
		default:
			solAssert(false, "Unknown arithmetic operator.");
		}
	}
}

void ExpressionCompiler::appendBitOperatorCode(Token _operator)
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

void ExpressionCompiler::appendShiftOperatorCode(Token _operator, Type const& _valueType, Type const& _shiftAmountType)
{
	// stack: shift_amount value_to_shift

	bool c_valueSigned = false;
	if (auto valueType = dynamic_cast<IntegerType const*>(&_valueType))
		c_valueSigned = valueType->isSigned();
	else
		solAssert(dynamic_cast<FixedBytesType const*>(&_valueType), "Only integer and fixed bytes type supported for shifts.");

	// The amount can be a RationalNumberType too.
	if (auto amountType = dynamic_cast<RationalNumberType const*>(&_shiftAmountType))
	{
		// This should be handled by the type checker.
		solAssert(amountType->integerType(), "");
		solAssert(!amountType->integerType()->isSigned(), "");
	}
	else if (auto amountType = dynamic_cast<IntegerType const*>(&_shiftAmountType))
		solAssert(!amountType->isSigned(), "");
	else
		solAssert(false, "Invalid shift amount type.");

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

void ExpressionCompiler::appendExpOperatorCode(Type const& _valueType, Type const& _exponentType)
{
	solAssert(_valueType.category() == Type::Category::Integer, "");
	solAssert(!dynamic_cast<IntegerType const&>(_exponentType).isSigned(), "");


	if (m_context.arithmetic() == Arithmetic::Checked)
		m_context.callYulFunction(m_context.utilFunctions().overflowCheckedIntExpFunction(
			dynamic_cast<IntegerType const&>(_valueType),
			dynamic_cast<IntegerType const&>(_exponentType)
		), 2, 1);
	else
		m_context << Instruction::EXP;
}

void ExpressionCompiler::appendExternalFunctionCall(
	FunctionType const& _functionType,
	vector<ASTPointer<Expression const>> const& _arguments,
	bool _tryCall
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
	unsigned gasValueSize = (_functionType.gasSet() ? 1u : 0u) + (_functionType.valueSet() ? 1u : 0u);
	unsigned contractStackPos = m_context.currentToBaseStackOffset(1 + gasValueSize + selfSize + (_functionType.isBareCall() ? 0 : 1));
	unsigned gasStackPos = m_context.currentToBaseStackOffset(gasValueSize);
	unsigned valueStackPos = m_context.currentToBaseStackOffset(1);

	// move self object to top
	if (_functionType.bound())
		utils().moveToStackTop(gasValueSize, _functionType.selfType()->sizeOnStack());

	auto funKind = _functionType.kind();

	solAssert(funKind != FunctionType::Kind::BareStaticCall || m_context.evmVersion().hasStaticCall(), "");

	solAssert(funKind != FunctionType::Kind::BareCallCode, "Callcode has been removed.");

	bool returnSuccessConditionAndReturndata = funKind == FunctionType::Kind::BareCall || funKind == FunctionType::Kind::BareDelegateCall || funKind == FunctionType::Kind::BareStaticCall;
	bool isDelegateCall = funKind == FunctionType::Kind::BareDelegateCall || funKind == FunctionType::Kind::DelegateCall;
	bool useStaticCall = funKind == FunctionType::Kind::BareStaticCall || (_functionType.stateMutability() <= StateMutability::View && m_context.evmVersion().hasStaticCall());

	if (_tryCall)
	{
		solAssert(!returnSuccessConditionAndReturndata, "");
		solAssert(!_functionType.isBareCall(), "");
	}

	ReturnInfo const returnInfo{m_context.evmVersion(), _functionType};
	bool const haveReturndatacopy = m_context.evmVersion().supportsReturndata();
	unsigned const retSize = returnInfo.estimatedReturnSize;
	bool const dynamicReturnSize = returnInfo.dynamicReturnSize;
	TypePointers const& returnTypes = returnInfo.returnTypes;

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
	bool encodeInPlace = _functionType.takesArbitraryParameters() || _functionType.isBareCall();
	if (_functionType.kind() == FunctionType::Kind::ECRecover)
		// This would be the only combination of padding and in-place encoding,
		// but all parameters of ecrecover are value types anyway.
		encodeInPlace = false;
	bool encodeForLibraryCall = funKind == FunctionType::Kind::DelegateCall;
	utils().encodeToMemory(
		argumentTypes,
		parameterTypes,
		_functionType.padArguments(),
		encodeInPlace,
		encodeForLibraryCall
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
	// Check the target contract exists (has code) for non-low-level calls.
	if (funKind == FunctionType::Kind::External || funKind == FunctionType::Kind::DelegateCall)
	{
		size_t encodedHeadSize = 0;
		for (auto const& t: returnTypes)
			encodedHeadSize += t->decodingType()->calldataHeadSize();
		// We do not need to check extcodesize if we expect return data, since if there is no
		// code, the call will return empty data and the ABI decoder will revert.
		if (
			encodedHeadSize == 0 ||
			!haveReturndatacopy ||
			m_context.revertStrings() >= RevertStrings::Debug
		)
		{
			m_context << Instruction::DUP1 << Instruction::EXTCODESIZE << Instruction::ISZERO;
			m_context.appendConditionalRevert(false, "Target contract does not contain code");
			existenceChecked = true;
		}
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
		u256 gasNeededByCaller = evmasm::GasCosts::callGas(m_context.evmVersion()) + 10;
		if (_functionType.valueSet())
			gasNeededByCaller += evmasm::GasCosts::callValueTransferGas;
		if (!existenceChecked)
			gasNeededByCaller += evmasm::GasCosts::callNewAccountGas; // we never know
		m_context << gasNeededByCaller << Instruction::GAS << Instruction::SUB;
	}
	// Order is important here, STATICCALL might overlap with DELEGATECALL.
	if (isDelegateCall)
		m_context << Instruction::DELEGATECALL;
	else if (useStaticCall)
		m_context << Instruction::STATICCALL;
	else
		m_context << Instruction::CALL;

	unsigned remainsSize =
		2u + // contract address, input_memory_end
		(_functionType.valueSet() ? 1 : 0) +
		(_functionType.gasSet() ? 1 : 0) +
		(!_functionType.isBareCall() ? 1 : 0);

	evmasm::AssemblyItem endTag = m_context.newTag();

	if (!returnSuccessConditionAndReturndata && !_tryCall)
	{
		// Propagate error condition (if CALL pushes 0 on stack).
		m_context << Instruction::ISZERO;
		m_context.appendConditionalRevert(true);
	}
	else
		m_context << swapInstruction(remainsSize);
	utils().popStackSlots(remainsSize);

	// Only success flag is remaining on stack.

	if (_tryCall)
	{
		m_context << Instruction::DUP1 << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(endTag);
		m_context << Instruction::POP;
	}

	if (returnSuccessConditionAndReturndata)
	{
		// success condition is already there
		// The return parameter types can be empty, when this function is used as
		// an internal helper function e.g. for ``send`` and ``transfer``. In that
		// case we're only interested in the success condition, not the return data.
		if (!_functionType.returnParameterTypes().empty())
			utils().returnDataToArray();
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
		if (dynamicReturnSize || m_context.useABICoderV2())
			needToUpdateFreeMemoryPtr = true;
		else
			for (auto const& retType: returnTypes)
				if (dynamic_cast<ReferenceType const*>(retType))
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
		m_context << (haveReturndatacopy ? evmasm::AssemblyItem(Instruction::RETURNDATASIZE) : u256(retSize));
		// Stack: return_data_start return_data_size
		if (needToUpdateFreeMemoryPtr)
			m_context.appendInlineAssembly(R"({
				// round size to the next multiple of 32
				let newMem := add(start, and(add(size, 0x1f), not(0x1f)))
				mstore(0x40, newMem)
			})", {"start", "size"});

		utils().abiDecode(returnTypes, true);
	}

	if (_tryCall)
	{
		// Success branch will reach this, failure branch will directly jump to endTag.
		m_context << u256(1);
		m_context << endTag;
	}
}

void ExpressionCompiler::appendExpressionCopyToMemory(Type const& _expectedType, Expression const& _expression)
{
	solUnimplementedAssert(_expectedType.isValueType(), "Not implemented for non-value types.");
	acceptAndConvert(_expression, _expectedType, true);
	utils().storeInMemoryDynamic(_expectedType);
}

void ExpressionCompiler::appendVariable(VariableDeclaration const& _variable, Expression const& _expression)
{
	if (_variable.isConstant())
		acceptAndConvert(*_variable.value(), *_variable.annotation().type);
	else if (_variable.immutable())
		setLValue<ImmutableItem>(_expression, _variable);
	else
		setLValueFromDeclaration(_variable, _expression);
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
			<< util::errinfo_comment("Identifier type not supported or identifier not found."));
}

void ExpressionCompiler::setLValueToStorageItem(Expression const& _expression)
{
	setLValue<StorageItem>(_expression, *_expression.annotation().type);
}

bool ExpressionCompiler::cleanupNeededForOp(Type::Category _type, Token _op, Arithmetic _arithmetic)
{
	if (TokenTraits::isCompareOp(_op) || TokenTraits::isShiftOp(_op))
		return true;
	else if (
		_arithmetic == Arithmetic::Wrapping &&
		_type == Type::Category::Integer &&
		(_op == Token::Div || _op == Token::Mod || _op == Token::Exp)
	)
		// We need cleanup for EXP because 0**0 == 1, but 0**0x100 == 0
		// It would suffice to clean the exponent, though.
		return true;
	else
		return false;
}

void ExpressionCompiler::acceptAndConvert(Expression const& _expression, Type const& _type, bool _cleanupNeeded)
{
	_expression.accept(*this);
	utils().convertType(*_expression.annotation().type, _type, _cleanupNeeded);
}

CompilerUtils ExpressionCompiler::utils()
{
	return CompilerUtils(m_context);
}
