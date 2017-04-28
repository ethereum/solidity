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
 * Routines used by both the compiler and the expression compiler.
 */

#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/ast/AST.h>
#include <libevmasm/Instruction.h>
#include <libsolidity/codegen/ArrayUtils.h>
#include <libsolidity/codegen/LValue.h>

using namespace std;

namespace dev
{
namespace solidity
{

const unsigned CompilerUtils::dataStartOffset = 4;
const size_t CompilerUtils::freeMemoryPointer = 64;
const unsigned CompilerUtils::identityContractAddress = 4;

void CompilerUtils::initialiseFreeMemoryPointer()
{
	m_context << u256(freeMemoryPointer + 32);
	storeFreeMemoryPointer();
}

void CompilerUtils::fetchFreeMemoryPointer()
{
	m_context << u256(freeMemoryPointer) << Instruction::MLOAD;
}

void CompilerUtils::storeFreeMemoryPointer()
{
	m_context << u256(freeMemoryPointer) << Instruction::MSTORE;
}

void CompilerUtils::allocateMemory()
{
	fetchFreeMemoryPointer();
	m_context << Instruction::SWAP1 << Instruction::DUP2 << Instruction::ADD;
	storeFreeMemoryPointer();
}

void CompilerUtils::toSizeAfterFreeMemoryPointer()
{
	fetchFreeMemoryPointer();
	m_context << Instruction::DUP1 << Instruction::SWAP2 << Instruction::SUB;
	m_context << Instruction::SWAP1;
}

unsigned CompilerUtils::loadFromMemory(
	unsigned _offset,
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries
)
{
	solAssert(_type.category() != Type::Category::Array, "Unable to statically load dynamic type.");
	m_context << u256(_offset);
	return loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
}

void CompilerUtils::loadFromMemoryDynamic(
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries,
	bool _keepUpdatedMemoryOffset
)
{		
	if (_keepUpdatedMemoryOffset)
		m_context << Instruction::DUP1;

	if (auto arrayType = dynamic_cast<ArrayType const*>(&_type))
	{
		solAssert(!arrayType->isDynamicallySized(), "");
		solAssert(!_fromCalldata, "");
		solAssert(_padToWordBoundaries, "");
		if (_keepUpdatedMemoryOffset)
			m_context << arrayType->memorySize() << Instruction::ADD;
	}
	else
	{
		unsigned numBytes = loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
		if (_keepUpdatedMemoryOffset)
		{
			// update memory counter
			moveToStackTop(_type.sizeOnStack());
			m_context << u256(numBytes) << Instruction::ADD;
		}
	}
}

void CompilerUtils::storeInMemory(unsigned _offset)
{
	unsigned numBytes = prepareMemoryStore(IntegerType(256), true);
	if (numBytes > 0)
		m_context << u256(_offset) << Instruction::MSTORE;
}

void CompilerUtils::storeInMemoryDynamic(Type const& _type, bool _padToWordBoundaries)
{
	if (auto ref = dynamic_cast<ReferenceType const*>(&_type))
	{
		solAssert(ref->location() == DataLocation::Memory, "");
		storeInMemoryDynamic(IntegerType(256), _padToWordBoundaries);
	}
	else if (auto str = dynamic_cast<StringLiteralType const*>(&_type))
	{
		m_context << Instruction::DUP1;
		storeStringData(bytesConstRef(str->value()));
		if (_padToWordBoundaries)
			m_context << u256(((str->value().size() + 31) / 32) * 32);
		else
			m_context << u256(str->value().size());
		m_context << Instruction::ADD;
	}
	else if (
		_type.category() == Type::Category::Function &&
		dynamic_cast<FunctionType const&>(_type).kind() == FunctionType::Kind::External
	)
	{
		solUnimplementedAssert(_padToWordBoundaries, "Non-padded store for function not implemented.");
		combineExternalFunctionType(true);
		m_context << Instruction::DUP2 << Instruction::MSTORE;
		m_context << u256(_padToWordBoundaries ? 32 : 24) << Instruction::ADD;
	}
	else
	{
		unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
		if (numBytes > 0)
		{
			solUnimplementedAssert(
				_type.sizeOnStack() == 1,
				"Memory store of types with stack size != 1 not implemented."
			);
			m_context << Instruction::DUP2 << Instruction::MSTORE;
			m_context << u256(numBytes) << Instruction::ADD;
		}
	}
}

void CompilerUtils::encodeToMemory(
	TypePointers const& _givenTypes,
	TypePointers const& _targetTypes,
	bool _padToWordBoundaries,
	bool _copyDynamicDataInPlace,
	bool _encodeAsLibraryTypes
)
{
	// stack: <v1> <v2> ... <vn> <mem>
	TypePointers targetTypes = _targetTypes.empty() ? _givenTypes : _targetTypes;
	solAssert(targetTypes.size() == _givenTypes.size(), "");
	for (TypePointer& t: targetTypes)
	{
		solUnimplementedAssert(
			t->mobileType() &&
			t->mobileType()->interfaceType(_encodeAsLibraryTypes) &&
			t->mobileType()->interfaceType(_encodeAsLibraryTypes)->encodingType(),
			"Encoding type \"" + t->toString() + "\" not yet implemented."
		);
		t = t->mobileType()->interfaceType(_encodeAsLibraryTypes)->encodingType();
	}

	// Stack during operation:
	// <v1> <v2> ... <vn> <mem_start> <dyn_head_1> ... <dyn_head_r> <end_of_mem>
	// The values dyn_head_i are added during the first loop and they point to the head part
	// of the ith dynamic parameter, which is filled once the dynamic parts are processed.

	// store memory start pointer
	m_context << Instruction::DUP1;

	unsigned argSize = CompilerUtils::sizeOnStack(_givenTypes);
	unsigned stackPos = 0; // advances through the argument values
	unsigned dynPointers = 0; // number of dynamic head pointers on the stack
	for (size_t i = 0; i < _givenTypes.size(); ++i)
	{
		TypePointer targetType = targetTypes[i];
		solAssert(!!targetType, "Externalable type expected.");
		if (targetType->isDynamicallySized() && !_copyDynamicDataInPlace)
		{
			// leave end_of_mem as dyn head pointer
			m_context << Instruction::DUP1 << u256(32) << Instruction::ADD;
			dynPointers++;
			solAssert((argSize + dynPointers) < 16, "Stack too deep, try using less variables.");
		}
		else
		{
			copyToStackTop(argSize - stackPos + dynPointers + 2, _givenTypes[i]->sizeOnStack());
			solAssert(!!targetType, "Externalable type expected.");
			TypePointer type = targetType;
			if (_givenTypes[i]->dataStoredIn(DataLocation::Storage) && targetType->isValueType())
			{
				// special case: convert storage reference type to value type - this is only
				// possible for library calls where we just forward the storage reference
				solAssert(_encodeAsLibraryTypes, "");
				solAssert(_givenTypes[i]->sizeOnStack() == 1, "");
			}
			else if (
				_givenTypes[i]->dataStoredIn(DataLocation::Storage) ||
				_givenTypes[i]->dataStoredIn(DataLocation::CallData) ||
				_givenTypes[i]->category() == Type::Category::StringLiteral ||
				_givenTypes[i]->category() == Type::Category::Function
			)
				type = _givenTypes[i]; // delay conversion
			else
				convertType(*_givenTypes[i], *targetType, true);
			if (auto arrayType = dynamic_cast<ArrayType const*>(type.get()))
				ArrayUtils(m_context).copyArrayToMemory(*arrayType, _padToWordBoundaries);
			else
				storeInMemoryDynamic(*type, _padToWordBoundaries);
		}
		stackPos += _givenTypes[i]->sizeOnStack();
	}

	// now copy the dynamic part
	// Stack: <v1> <v2> ... <vn> <mem_start> <dyn_head_1> ... <dyn_head_r> <end_of_mem>
	stackPos = 0;
	unsigned thisDynPointer = 0;
	for (size_t i = 0; i < _givenTypes.size(); ++i)
	{
		TypePointer targetType = targetTypes[i];
		solAssert(!!targetType, "Externalable type expected.");
		if (targetType->isDynamicallySized() && !_copyDynamicDataInPlace)
		{
			// copy tail pointer (=mem_end - mem_start) to memory
			m_context << dupInstruction(2 + dynPointers) << Instruction::DUP2;
			m_context << Instruction::SUB;
			m_context << dupInstruction(2 + dynPointers - thisDynPointer);
			m_context << Instruction::MSTORE;
			// stack: ... <end_of_mem>
			if (_givenTypes[i]->category() == Type::Category::StringLiteral)
			{
				auto const& strType = dynamic_cast<StringLiteralType const&>(*_givenTypes[i]);
				m_context << u256(strType.value().size());
				storeInMemoryDynamic(IntegerType(256), true);
				// stack: ... <end_of_mem'>
				storeInMemoryDynamic(strType, _padToWordBoundaries);
			}
			else
			{
				solAssert(_givenTypes[i]->category() == Type::Category::Array, "Unknown dynamic type.");
				auto const& arrayType = dynamic_cast<ArrayType const&>(*_givenTypes[i]);
				// now copy the array
				copyToStackTop(argSize - stackPos + dynPointers + 2, arrayType.sizeOnStack());
				// stack: ... <end_of_mem> <value...>
				// copy length to memory
				m_context << dupInstruction(1 + arrayType.sizeOnStack());
				ArrayUtils(m_context).retrieveLength(arrayType, 1);
				// stack: ... <end_of_mem> <value...> <end_of_mem'> <length>
				storeInMemoryDynamic(IntegerType(256), true);
				// stack: ... <end_of_mem> <value...> <end_of_mem''>
				// copy the new memory pointer
				m_context << swapInstruction(arrayType.sizeOnStack() + 1) << Instruction::POP;
				// stack: ... <end_of_mem''> <value...>
				// copy data part
				ArrayUtils(m_context).copyArrayToMemory(arrayType, _padToWordBoundaries);
				// stack: ... <end_of_mem'''>
			}

			thisDynPointer++;
		}
		stackPos += _givenTypes[i]->sizeOnStack();
	}

	// remove unneeded stack elements (and retain memory pointer)
	m_context << swapInstruction(argSize + dynPointers + 1);
	popStackSlots(argSize + dynPointers + 1);
}

void CompilerUtils::zeroInitialiseMemoryArray(ArrayType const& _type)
{
	auto repeat = m_context.newTag();
	m_context << repeat;
	pushZeroValue(*_type.baseType());
	storeInMemoryDynamic(*_type.baseType());
	m_context << Instruction::SWAP1 << u256(1) << Instruction::SWAP1;
	m_context << Instruction::SUB << Instruction::SWAP1;
	m_context << Instruction::DUP2;
	m_context.appendConditionalJumpTo(repeat);
	m_context << Instruction::SWAP1 << Instruction::POP;
}

void CompilerUtils::memoryCopy32()
{
	// Stack here: size target source

	m_context.appendInlineAssembly(R"(
		{
		jumpi(end, eq(len, 0))
		start:
		mstore(dst, mload(src))
		jumpi(end, iszero(gt(len, 32)))
		dst := add(dst, 32)
		src := add(src, 32)
		len := sub(len, 32)
		jump(start)
		end:
		}
	)",
		{ "len", "dst", "src" }
	);
	m_context << Instruction::POP << Instruction::POP << Instruction::POP;
}

void CompilerUtils::memoryCopy()
{
	// Stack here: size target source

	m_context.appendInlineAssembly(R"(
		{
		// copy 32 bytes at once
		start32:
		jumpi(end32, lt(len, 32))
		mstore(dst, mload(src))
		dst := add(dst, 32)
		src := add(src, 32)
		len := sub(len, 32)
		jump(start32)
		end32:

		// copy the remainder (0 < len < 32)
		let mask := sub(exp(256, sub(32, len)), 1)
		let srcpart := and(mload(src), not(mask))
		let dstpart := and(mload(dst), mask)
		mstore(dst, or(srcpart, dstpart))
		}
	)",
		{ "len", "dst", "src" }
	);
	m_context << Instruction::POP << Instruction::POP << Instruction::POP;
}

void CompilerUtils::splitExternalFunctionType(bool _leftAligned)
{
	// We have to split the left-aligned <address><function identifier> into two stack slots:
	// address (right aligned), function identifier (right aligned)
	if (_leftAligned)
	{
		m_context << Instruction::DUP1 << (u256(1) << (64 + 32)) << Instruction::SWAP1 << Instruction::DIV;
		// <input> <address>
		m_context << Instruction::SWAP1 << (u256(1) << 64) << Instruction::SWAP1 << Instruction::DIV;
	}
	else
	{
		m_context << Instruction::DUP1 << (u256(1) << 32) << Instruction::SWAP1 << Instruction::DIV;
		m_context << ((u256(1) << 160) - 1) << Instruction::AND << Instruction::SWAP1;
	}
	m_context << u256(0xffffffffUL) << Instruction::AND;
}

void CompilerUtils::combineExternalFunctionType(bool _leftAligned)
{
	// <address> <function_id>
	m_context << u256(0xffffffffUL) << Instruction::AND << Instruction::SWAP1;
	if (!_leftAligned)
		m_context << ((u256(1) << 160) - 1) << Instruction::AND;
	m_context << (u256(1) << 32) << Instruction::MUL;
	m_context << Instruction::OR;
	if (_leftAligned)
		m_context << (u256(1) << 64) << Instruction::MUL;
}

void CompilerUtils::pushCombinedFunctionEntryLabel(Declaration const& _function)
{
	m_context << m_context.functionEntryLabel(_function).pushTag();
	// If there is a runtime context, we have to merge both labels into the same
	// stack slot in case we store it in storage.
	if (CompilerContext* rtc = m_context.runtimeContext())
		m_context <<
			(u256(1) << 32) <<
			Instruction::MUL <<
			rtc->functionEntryLabel(_function).toSubAssemblyTag(m_context.runtimeSub()) <<
			Instruction::OR;
}

void CompilerUtils::convertType(Type const& _typeOnStack, Type const& _targetType, bool _cleanupNeeded, bool _chopSignBits)
{
	// For a type extension, we need to remove all higher-order bits that we might have ignored in
	// previous operations.
	// @todo: store in the AST whether the operand might have "dirty" higher order bits

	if (_typeOnStack == _targetType && !_cleanupNeeded)
		return;
	Type::Category stackTypeCategory = _typeOnStack.category();
	Type::Category targetTypeCategory = _targetType.category();

	bool enumOverflowCheckPending = (targetTypeCategory == Type::Category::Enum || stackTypeCategory == Type::Category::Enum);
	bool chopSignBitsPending = _chopSignBits && targetTypeCategory == Type::Category::Integer;
	if (chopSignBitsPending)
	{
		const IntegerType& targetIntegerType = dynamic_cast<const IntegerType &>(_targetType);
		chopSignBitsPending = targetIntegerType.isSigned();
	}

	switch (stackTypeCategory)
	{
	case Type::Category::FixedBytes:
	{
		FixedBytesType const& typeOnStack = dynamic_cast<FixedBytesType const&>(_typeOnStack);
		if (targetTypeCategory == Type::Category::Integer)
		{
			// conversion from bytes to integer. no need to clean the high bit
			// only to shift right because of opposite alignment
			IntegerType const& targetIntegerType = dynamic_cast<IntegerType const&>(_targetType);
			m_context << (u256(1) << (256 - typeOnStack.numBytes() * 8)) << Instruction::SWAP1 << Instruction::DIV;
			if (targetIntegerType.numBits() < typeOnStack.numBytes() * 8)
				convertType(IntegerType(typeOnStack.numBytes() * 8), _targetType, _cleanupNeeded);
		}
		else
		{
			// clear for conversion to longer bytes
			solAssert(targetTypeCategory == Type::Category::FixedBytes, "Invalid type conversion requested.");
			FixedBytesType const& targetType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (targetType.numBytes() > typeOnStack.numBytes() || _cleanupNeeded)
			{
				if (typeOnStack.numBytes() == 0)
					m_context << Instruction::POP << u256(0);
				else
				{
					m_context << ((u256(1) << (256 - typeOnStack.numBytes() * 8)) - 1);
					m_context << Instruction::NOT << Instruction::AND;
				}
			}
		}
	}
		break;
	case Type::Category::Enum:
		solAssert(_targetType == _typeOnStack || targetTypeCategory == Type::Category::Integer, "");
		if (enumOverflowCheckPending)
		{
			EnumType const& enumType = dynamic_cast<decltype(enumType)>(_typeOnStack);
			solAssert(enumType.numberOfMembers() > 0, "empty enum should have caused a parser error.");
			m_context << u256(enumType.numberOfMembers() - 1) << Instruction::DUP2 << Instruction::GT;
			m_context.appendConditionalInvalid();
			enumOverflowCheckPending = false;
		}
		break;
	case Type::Category::FixedPoint:
		solUnimplemented("Not yet implemented - FixedPointType.");
	case Type::Category::Integer:
	case Type::Category::Contract:
	case Type::Category::RationalNumber:
		if (targetTypeCategory == Type::Category::FixedBytes)
		{
			solAssert(stackTypeCategory == Type::Category::Integer || stackTypeCategory == Type::Category::RationalNumber,
				"Invalid conversion to FixedBytesType requested.");
			// conversion from bytes to string. no need to clean the high bit
			// only to shift left because of opposite alignment
			FixedBytesType const& targetBytesType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (auto typeOnStack = dynamic_cast<IntegerType const*>(&_typeOnStack))
				if (targetBytesType.numBytes() * 8 > typeOnStack->numBits())
					cleanHigherOrderBits(*typeOnStack);
			m_context << (u256(1) << (256 - targetBytesType.numBytes() * 8)) << Instruction::MUL;
		}
		else if (targetTypeCategory == Type::Category::Enum)
		{
			solAssert(_typeOnStack.mobileType(), "");
			// just clean
			convertType(_typeOnStack, *_typeOnStack.mobileType(), true);
			EnumType const& enumType = dynamic_cast<decltype(enumType)>(_targetType);
			solAssert(enumType.numberOfMembers() > 0, "empty enum should have caused a parser error.");
			m_context << u256(enumType.numberOfMembers() - 1) << Instruction::DUP2 << Instruction::GT;
			m_context.appendConditionalInvalid();
			enumOverflowCheckPending = false;
		}
		else if (targetTypeCategory == Type::Category::FixedPoint)
		{
			solAssert(
				stackTypeCategory == Type::Category::Integer || 
				stackTypeCategory == Type::Category::RationalNumber ||
				stackTypeCategory == Type::Category::FixedPoint,
				"Invalid conversion to FixedMxNType requested."
			);
			//shift all integer bits onto the left side of the fixed type
			FixedPointType const& targetFixedPointType = dynamic_cast<FixedPointType const&>(_targetType);
			if (auto typeOnStack = dynamic_cast<IntegerType const*>(&_typeOnStack))
				if (targetFixedPointType.integerBits() > typeOnStack->numBits())
					cleanHigherOrderBits(*typeOnStack);
			solUnimplemented("Not yet implemented - FixedPointType.");
		}
		else
		{
			solAssert(targetTypeCategory == Type::Category::Integer || targetTypeCategory == Type::Category::Contract, "");
			IntegerType addressType(0, IntegerType::Modifier::Address);
			IntegerType const& targetType = targetTypeCategory == Type::Category::Integer
				? dynamic_cast<IntegerType const&>(_targetType) : addressType;
			if (stackTypeCategory == Type::Category::RationalNumber)
			{
				RationalNumberType const& constType = dynamic_cast<RationalNumberType const&>(_typeOnStack);
				// We know that the stack is clean, we only have to clean for a narrowing conversion
				// where cleanup is forced.
				solUnimplementedAssert(!constType.isFractional(), "Not yet implemented - FixedPointType.");
				if (targetType.numBits() < constType.integerType()->numBits() && _cleanupNeeded)
					cleanHigherOrderBits(targetType);
			}
			else
			{
				IntegerType const& typeOnStack = stackTypeCategory == Type::Category::Integer
					? dynamic_cast<IntegerType const&>(_typeOnStack) : addressType;
				// Widening: clean up according to source type width
				// Non-widening and force: clean up according to target type bits
				if (targetType.numBits() > typeOnStack.numBits())
					cleanHigherOrderBits(typeOnStack);
				else if (_cleanupNeeded)
					cleanHigherOrderBits(targetType);
				if (chopSignBitsPending)
				{
					if (typeOnStack.numBits() < 256)
						m_context
							<< ((u256(1) << typeOnStack.numBits()) - 1)
							<< Instruction::AND;
					chopSignBitsPending = false;
				}
			}
		}
		break;
	case Type::Category::StringLiteral:
	{
		auto const& literalType = dynamic_cast<StringLiteralType const&>(_typeOnStack);
		string const& value = literalType.value();
		bytesConstRef data(value);
		if (targetTypeCategory == Type::Category::FixedBytes)
		{
			solAssert(data.size() <= 32, "");
			m_context << h256::Arith(h256(data, h256::AlignLeft));
		}
		else if (targetTypeCategory == Type::Category::Array)
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(_targetType);
			solAssert(arrayType.isByteArray(), "");
			u256 storageSize(32 + ((data.size() + 31) / 32) * 32);
			m_context << storageSize;
			allocateMemory();
			// stack: mempos
			m_context << Instruction::DUP1 << u256(data.size());
			storeInMemoryDynamic(IntegerType(256));
			// stack: mempos datapos
			storeStringData(data);
			break;
		}
		else
			solAssert(
				false,
				"Invalid conversion from string literal to " + _targetType.toString(false) + " requested."
			);
		break;
	}
	case Type::Category::Array:
	{
		solAssert(targetTypeCategory == stackTypeCategory, "");
		ArrayType const& typeOnStack = dynamic_cast<ArrayType const&>(_typeOnStack);
		ArrayType const& targetType = dynamic_cast<ArrayType const&>(_targetType);
		switch (targetType.location())
		{
		case DataLocation::Storage:
			// Other cases are done explicitly in LValue::storeValue, and only possible by assignment.
			solAssert(
				(targetType.isPointer() || (typeOnStack.isByteArray() && targetType.isByteArray())) &&
				typeOnStack.location() == DataLocation::Storage,
				"Invalid conversion to storage type."
			);
			break;
		case DataLocation::Memory:
		{
			// Copy the array to a free position in memory, unless it is already in memory.
			if (typeOnStack.location() != DataLocation::Memory)
			{
				// stack: <source ref> (variably sized)
				unsigned stackSize = typeOnStack.sizeOnStack();
				ArrayUtils(m_context).retrieveLength(typeOnStack);

				// allocate memory
				// stack: <source ref> (variably sized) <length>
				m_context << Instruction::DUP1;
				ArrayUtils(m_context).convertLengthToSize(targetType, true);
				// stack: <source ref> (variably sized) <length> <size>
				if (targetType.isDynamicallySized())
					m_context << u256(0x20) << Instruction::ADD;
				allocateMemory();
				// stack: <source ref> (variably sized) <length> <mem start>
				m_context << Instruction::DUP1;
				moveIntoStack(2 + stackSize);
				if (targetType.isDynamicallySized())
				{
					m_context << Instruction::DUP2;
					storeInMemoryDynamic(IntegerType(256));
				}
				// stack: <mem start> <source ref> (variably sized) <length> <mem data pos>
				if (targetType.baseType()->isValueType())
				{
					solAssert(typeOnStack.baseType()->isValueType(), "");
					copyToStackTop(2 + stackSize, stackSize);
					ArrayUtils(m_context).copyArrayToMemory(typeOnStack);
				}
				else
				{
					m_context << u256(0) << Instruction::SWAP1;
					// stack: <mem start> <source ref> (variably sized) <length> <counter> <mem data pos>
					auto repeat = m_context.newTag();
					m_context << repeat;
					m_context << Instruction::DUP3 << Instruction::DUP3;
					m_context << Instruction::LT << Instruction::ISZERO;
					auto loopEnd = m_context.appendConditionalJump();
					copyToStackTop(3 + stackSize, stackSize);
					copyToStackTop(2 + stackSize, 1);
					ArrayUtils(m_context).accessIndex(typeOnStack, false);
					if (typeOnStack.location() == DataLocation::Storage)
						StorageItem(m_context, *typeOnStack.baseType()).retrieveValue(SourceLocation(), true);
					convertType(*typeOnStack.baseType(), *targetType.baseType(), _cleanupNeeded);
					storeInMemoryDynamic(*targetType.baseType(), true);
					m_context << Instruction::SWAP1 << u256(1) << Instruction::ADD;
					m_context << Instruction::SWAP1;
					m_context.appendJumpTo(repeat);
					m_context << loopEnd;
					m_context << Instruction::POP;
				}
				// stack: <mem start> <source ref> (variably sized) <length> <mem data pos updated>
				popStackSlots(2 + stackSize);
				// Stack: <mem start>
			}
			break;
		}
		case DataLocation::CallData:
			solAssert(
					targetType.isByteArray() &&
					typeOnStack.isByteArray() &&
					typeOnStack.location() == DataLocation::CallData,
				"Invalid conversion to calldata type.");
			break;
		default:
			solAssert(
				false,
				"Invalid type conversion " +
				_typeOnStack.toString(false) +
				" to " +
				_targetType.toString(false) +
				" requested."
			);
		}
		break;
	}
	case Type::Category::Struct:
	{
		solAssert(targetTypeCategory == stackTypeCategory, "");
		auto& targetType = dynamic_cast<StructType const&>(_targetType);
		auto& typeOnStack = dynamic_cast<StructType const&>(_typeOnStack);
		solAssert(
			targetType.location() != DataLocation::CallData &&
			typeOnStack.location() != DataLocation::CallData
		, "");
		switch (targetType.location())
		{
		case DataLocation::Storage:
			// Other cases are done explicitly in LValue::storeValue, and only possible by assignment.
			solAssert(
				targetType.isPointer() &&
				typeOnStack.location() == DataLocation::Storage,
				"Invalid conversion to storage type."
			);
			break;
		case DataLocation::Memory:
			// Copy the array to a free position in memory, unless it is already in memory.
			if (typeOnStack.location() != DataLocation::Memory)
			{
				solAssert(typeOnStack.location() == DataLocation::Storage, "");
				// stack: <source ref>
				m_context << typeOnStack.memorySize();
				allocateMemory();
				m_context << Instruction::SWAP1 << Instruction::DUP2;
				// stack: <memory ptr> <source ref> <memory ptr>
				for (auto const& member: typeOnStack.members(nullptr))
				{
					if (!member.type->canLiveOutsideStorage())
						continue;
					pair<u256, unsigned> const& offsets = typeOnStack.storageOffsetsOfMember(member.name);
					m_context << offsets.first << Instruction::DUP3 << Instruction::ADD;
					m_context << u256(offsets.second);
					StorageItem(m_context, *member.type).retrieveValue(SourceLocation(), true);
					TypePointer targetMemberType = targetType.memberType(member.name);
					solAssert(!!targetMemberType, "Member not found in target type.");
					convertType(*member.type, *targetMemberType, true);
					storeInMemoryDynamic(*targetMemberType, true);
				}
				m_context << Instruction::POP << Instruction::POP;
			}
			break;
		case DataLocation::CallData:
			solAssert(false, "Invalid type conversion target location CallData.");
			break;
		}
		break;
	}
	case Type::Category::Tuple:
	{
		TupleType const& sourceTuple = dynamic_cast<TupleType const&>(_typeOnStack);
		TupleType const& targetTuple = dynamic_cast<TupleType const&>(_targetType);
		// fillRight: remove excess values at right side, !fillRight: remove eccess values at left side
		bool fillRight = !targetTuple.components().empty() && (
			!targetTuple.components().back() ||
			targetTuple.components().front()
		);
		unsigned depth = sourceTuple.sizeOnStack();
		for (size_t i = 0; i < sourceTuple.components().size(); ++i)
		{
			TypePointer sourceType = sourceTuple.components()[i];
			TypePointer targetType;
			if (fillRight && i < targetTuple.components().size())
				targetType = targetTuple.components()[i];
			else if (!fillRight && targetTuple.components().size() + i >= sourceTuple.components().size())
				targetType = targetTuple.components()[targetTuple.components().size() - (sourceTuple.components().size() - i)];
			if (!sourceType)
			{
				solAssert(!targetType, "");
				continue;
			}
			unsigned sourceSize = sourceType->sizeOnStack();
			unsigned targetSize = targetType ? targetType->sizeOnStack() : 0;
			if (!targetType || *sourceType != *targetType || _cleanupNeeded)
			{
				if (targetType)
				{
					if (sourceSize > 0)
						copyToStackTop(depth, sourceSize);
					convertType(*sourceType, *targetType, _cleanupNeeded);
				}
				if (sourceSize > 0 || targetSize > 0)
				{
					// Move it back into its place.
					for (unsigned j = 0; j < min(sourceSize, targetSize); ++j)
						m_context <<
							swapInstruction(depth + targetSize - sourceSize) <<
							Instruction::POP;
					// Value shrank
					for (unsigned j = targetSize; j < sourceSize; ++j)
					{
						moveToStackTop(depth - 1, 1);
						m_context << Instruction::POP;
					}
					// Value grew
					if (targetSize > sourceSize)
						moveIntoStack(depth + targetSize - sourceSize - 1, targetSize - sourceSize);
				}
			}
			depth -= sourceSize;
		}
		break;
	}
	case Type::Category::Bool:
		solAssert(_targetType == _typeOnStack, "Invalid conversion for bool.");
		if (_cleanupNeeded)
			m_context << Instruction::ISZERO << Instruction::ISZERO;
		break;
	case Type::Category::Function:
	{
		if (targetTypeCategory == Type::Category::Integer)
		{
			IntegerType const& targetType = dynamic_cast<IntegerType const&>(_targetType);
			solAssert(targetType.isAddress(), "Function type can only be converted to address.");
			FunctionType const& typeOnStack = dynamic_cast<FunctionType const&>(_typeOnStack);
			solAssert(typeOnStack.kind() == FunctionType::Kind::External, "Only external function type can be converted.");

			// stack: <address> <function_id>
			m_context << Instruction::POP;
			break;
		}
	}
	default:
		// All other types should not be convertible to non-equal types.
		solAssert(_typeOnStack == _targetType, "Invalid type conversion requested.");
		if (_cleanupNeeded && _targetType.canBeStored() && _targetType.storageBytes() < 32)
				m_context
					<< ((u256(1) << (8 * _targetType.storageBytes())) - 1)
					<< Instruction::AND;
		break;
	}

	solAssert(!enumOverflowCheckPending, "enum overflow checking missing.");
	solAssert(!chopSignBitsPending, "forgot to chop the sign bits.");
}

void CompilerUtils::pushZeroValue(Type const& _type)
{
	if (auto const* funType = dynamic_cast<FunctionType const*>(&_type))
	{
		if (funType->kind() == FunctionType::Kind::Internal)
		{
			m_context << m_context.lowLevelFunctionTag("$invalidFunction", 0, 0, [](CompilerContext& _context) {
				_context.appendInvalid();
			});
			return;
		}
	}
	auto const* referenceType = dynamic_cast<ReferenceType const*>(&_type);
	if (!referenceType || referenceType->location() == DataLocation::Storage)
	{
		for (size_t i = 0; i < _type.sizeOnStack(); ++i)
			m_context << u256(0);
		return;
	}
	solAssert(referenceType->location() == DataLocation::Memory, "");

	TypePointer type = _type.shared_from_this();
	m_context.callLowLevelFunction(
		"$pushZeroValue_" + referenceType->identifier(),
		0,
		1,
		[type](CompilerContext& _context) {
			CompilerUtils utils(_context);
			_context << u256(max(32u, type->calldataEncodedSize()));
			utils.allocateMemory();
			_context << Instruction::DUP1;

			if (auto structType = dynamic_cast<StructType const*>(type.get()))
				for (auto const& member: structType->members(nullptr))
				{
					utils.pushZeroValue(*member.type);
					utils.storeInMemoryDynamic(*member.type);
				}
			else if (auto arrayType = dynamic_cast<ArrayType const*>(type.get()))
			{
				if (arrayType->isDynamicallySized())
				{
					// zero length
					_context << u256(0);
					utils.storeInMemoryDynamic(IntegerType(256));
				}
				else if (arrayType->length() > 0)
				{
					_context << arrayType->length() << Instruction::SWAP1;
					// stack: items_to_do memory_pos
					utils.zeroInitialiseMemoryArray(*arrayType);
					// stack: updated_memory_pos
				}
			}
			else
				solAssert(false, "Requested initialisation for unknown type: " + type->toString());

			// remove the updated memory pointer
			_context << Instruction::POP;
		}
	);
}

void CompilerUtils::moveToStackVariable(VariableDeclaration const& _variable)
{
	unsigned const stackPosition = m_context.baseToCurrentStackOffset(m_context.baseStackOffsetOfVariable(_variable));
	unsigned const size = _variable.annotation().type->sizeOnStack();
	solAssert(stackPosition >= size, "Variable size and position mismatch.");
	// move variable starting from its top end in the stack
	if (stackPosition - size + 1 > 16)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_variable.location()) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	for (unsigned i = 0; i < size; ++i)
		m_context << swapInstruction(stackPosition - size + 1) << Instruction::POP;
}

void CompilerUtils::copyToStackTop(unsigned _stackDepth, unsigned _itemSize)
{
	solAssert(_stackDepth <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 0; i < _itemSize; ++i)
		m_context << dupInstruction(_stackDepth);
}

void CompilerUtils::moveToStackTop(unsigned _stackDepth, unsigned _itemSize)
{
	moveIntoStack(_itemSize, _stackDepth);
}

void CompilerUtils::moveIntoStack(unsigned _stackDepth, unsigned _itemSize)
{
	if (_stackDepth <= _itemSize)
		for (unsigned i = 0; i < _stackDepth; ++i)
			rotateStackDown(_stackDepth + _itemSize);
	else
		for (unsigned i = 0; i < _itemSize; ++i)
			rotateStackUp(_stackDepth + _itemSize);
}

void CompilerUtils::rotateStackUp(unsigned _items)
{
	solAssert(_items - 1 <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 1; i < _items; ++i)
		m_context << swapInstruction(_items - i);
}

void CompilerUtils::rotateStackDown(unsigned _items)
{
	solAssert(_items - 1 <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 1; i < _items; ++i)
		m_context << swapInstruction(i);
}

void CompilerUtils::popStackElement(Type const& _type)
{
	popStackSlots(_type.sizeOnStack());
}

void CompilerUtils::popStackSlots(size_t _amount)
{
	for (size_t i = 0; i < _amount; ++i)
		m_context << Instruction::POP;
}

unsigned CompilerUtils::sizeOnStack(vector<shared_ptr<Type const>> const& _variableTypes)
{
	unsigned size = 0;
	for (shared_ptr<Type const> const& type: _variableTypes)
		size += type->sizeOnStack();
	return size;
}

void CompilerUtils::computeHashStatic()
{
	storeInMemory(0);
	m_context << u256(32) << u256(0) << Instruction::SHA3;
}

void CompilerUtils::storeStringData(bytesConstRef _data)
{
	//@todo provide both alternatives to the optimiser
	// stack: mempos
	if (_data.size() <= 128)
	{
		for (unsigned i = 0; i < _data.size(); i += 32)
		{
			m_context << h256::Arith(h256(_data.cropped(i), h256::AlignLeft));
			storeInMemoryDynamic(IntegerType(256));
		}
		m_context << Instruction::POP;
	}
	else
	{
		// stack: mempos mempos_data
		m_context.appendData(_data.toBytes());
		m_context << u256(_data.size()) << Instruction::SWAP2;
		m_context << Instruction::CODECOPY;
	}
}

unsigned CompilerUtils::loadFromMemoryHelper(Type const& _type, bool _fromCalldata, bool _padToWords)
{
	unsigned numBytes = _type.calldataEncodedSize(_padToWords);
	bool isExternalFunctionType = false;
	if (auto const* funType = dynamic_cast<FunctionType const*>(&_type))
		if (funType->kind() == FunctionType::Kind::External)
			isExternalFunctionType = true;
	if (numBytes == 0)
	{
		m_context << Instruction::POP << u256(0);
		return numBytes;
	}
	solAssert(numBytes <= 32, "Static memory load of more than 32 bytes requested.");
	m_context << (_fromCalldata ? Instruction::CALLDATALOAD : Instruction::MLOAD);
	if (isExternalFunctionType)
		splitExternalFunctionType(true);
	else if (numBytes != 32)
	{
		bool leftAligned = _type.category() == Type::Category::FixedBytes;
		// add leading or trailing zeros by dividing/multiplying depending on alignment
		u256 shiftFactor = u256(1) << ((32 - numBytes) * 8);
		m_context << shiftFactor << Instruction::SWAP1 << Instruction::DIV;
		if (leftAligned)
			m_context << shiftFactor << Instruction::MUL;
	}
	if (_fromCalldata)
		convertType(_type, _type, true);

	return numBytes;
}

void CompilerUtils::cleanHigherOrderBits(IntegerType const& _typeOnStack)
{
	if (_typeOnStack.numBits() == 256)
		return;
	else if (_typeOnStack.isSigned())
		m_context << u256(_typeOnStack.numBits() / 8 - 1) << Instruction::SIGNEXTEND;
	else
		m_context << ((u256(1) << _typeOnStack.numBits()) - 1) << Instruction::AND;
}

unsigned CompilerUtils::prepareMemoryStore(Type const& _type, bool _padToWords)
{
	unsigned numBytes = _type.calldataEncodedSize(_padToWords);
	bool leftAligned = _type.category() == Type::Category::FixedBytes;
	if (numBytes == 0)
		m_context << Instruction::POP;
	else
	{
		solAssert(numBytes <= 32, "Memory store of more than 32 bytes requested.");
		convertType(_type, _type, true);
		if (numBytes != 32 && !leftAligned && !_padToWords)
			// shift the value accordingly before storing
			m_context << (u256(1) << ((32 - numBytes) * 8)) << Instruction::MUL;
	}
	return numBytes;
}

}
}
