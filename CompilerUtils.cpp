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
 * Routines used by both the compiler and the expression compiler.
 */

#include <libsolidity/CompilerUtils.h>
#include <libsolidity/AST.h>
#include <libevmcore/Instruction.h>
#include <libevmcore/Params.h>
#include <libsolidity/ArrayUtils.h>

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
	m_context << u256(freeMemoryPointer) << eth::Instruction::MLOAD;
}

void CompilerUtils::storeFreeMemoryPointer()
{
	m_context << u256(freeMemoryPointer) << eth::Instruction::MSTORE;
}

void CompilerUtils::toSizeAfterFreeMemoryPointer()
{
	fetchFreeMemoryPointer();
	m_context << eth::Instruction::DUP1 << eth::Instruction::SWAP2 << eth::Instruction::SUB;
	m_context << eth::Instruction::SWAP1;
}

unsigned CompilerUtils::loadFromMemory(
	unsigned _offset,
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries
)
{
	solAssert(_type.getCategory() != Type::Category::Array, "Unable to statically load dynamic type.");
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
	solAssert(_type.getCategory() != Type::Category::Array, "Arrays not yet implemented.");
	if (_keepUpdatedMemoryOffset)
		m_context << eth::Instruction::DUP1;
	unsigned numBytes = loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
	if (_keepUpdatedMemoryOffset)
	{
		// update memory counter
		moveToStackTop(_type.getSizeOnStack());
		m_context << u256(numBytes) << eth::Instruction::ADD;
	}
}

unsigned CompilerUtils::storeInMemory(unsigned _offset, Type const& _type, bool _padToWordBoundaries)
{
	solAssert(_type.getCategory() != Type::Category::Array, "Unable to statically store dynamic type.");
	unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
	if (numBytes > 0)
		m_context << u256(_offset) << eth::Instruction::MSTORE;
	return numBytes;
}

void CompilerUtils::storeInMemoryDynamic(Type const& _type, bool _padToWordBoundaries)
{
	if (_type.getCategory() == Type::Category::Array)
	{
		auto const& type = dynamic_cast<ArrayType const&>(_type);
		solAssert(type.isByteArray(), "Non byte arrays not yet implemented here.");

		if (type.location() == ReferenceType::Location::CallData)
		{
			// stack: target source_offset source_len
			m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3 << eth::Instruction::DUP5;
				// stack: target source_offset source_len source_len source_offset target
			m_context << eth::Instruction::CALLDATACOPY;
			m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
			m_context << eth::Instruction::SWAP2 << eth::Instruction::POP << eth::Instruction::POP;
		}
		else if (type.location() == ReferenceType::Location::Memory)
		{
			// memcpy using the built-in contract
			ArrayUtils(m_context).retrieveLength(type);
			if (type.isDynamicallySized())
			{
				// change pointer to data part
				m_context << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD;
				m_context << eth::Instruction::SWAP1;
			}
			// stack: <target> <source> <length>
			// stack for call: outsize target size source value contract gas
			m_context << eth::Instruction::DUP1 << eth::Instruction::DUP4;
			m_context << eth::Instruction::DUP2 << eth::Instruction::DUP5;
			m_context << u256(0) << u256(identityContractAddress);
			//@TODO do not use ::CALL if less than 32 bytes?
			//@todo in production, we should not have to pair c_callNewAccountGas.
			m_context << u256(eth::c_callGas + 10 + eth::c_callNewAccountGas) << eth::Instruction::GAS;
			m_context << eth::Instruction::SUB << eth::Instruction::CALL;
			m_context << eth::Instruction::POP; // ignore return value

			m_context << eth::Instruction::SWAP1 << eth::Instruction::POP;
			// stack: <target> <length>

			if (_padToWordBoundaries && (type.isDynamicallySized() || (type.getLength()) % 32 != 0))
			{
				// stack: <target> <length>
				m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2 << eth::Instruction::ADD;
				// stack: <length> <target + length>
				m_context << eth::Instruction::SWAP1 << u256(31) << eth::Instruction::AND;
				// stack: <target + length> <remainder = length % 32>
				eth::AssemblyItem skip = m_context.newTag();
				if (type.isDynamicallySized())
				{
					m_context << eth::Instruction::DUP1 << eth::Instruction::ISZERO;
					m_context.appendConditionalJumpTo(skip);
				}
				// round off, load from there.
				// stack <target + length> <remainder = length % 32>
				m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3;
				m_context << eth::Instruction::SUB;
				// stack: target+length remainder <target + length - remainder>
				m_context << eth::Instruction::DUP1 << eth::Instruction::MLOAD;
				// Now we AND it with ~(2**(8 * (32 - remainder)) - 1)
				m_context << u256(1);
				m_context << eth::Instruction::DUP4 << u256(32) << eth::Instruction::SUB;
				// stack: ...<v> 1 <32 - remainder>
				m_context << u256(0x100) << eth::Instruction::EXP << eth::Instruction::SUB;
				m_context << eth::Instruction::NOT << eth::Instruction::AND;
				// stack: target+length remainder target+length-remainder <v & ...>
				m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
				// stack: target+length remainder target+length-remainder
				m_context << u256(32) << eth::Instruction::ADD;
				// stack: target+length remainder <new_padded_end>
				m_context << eth::Instruction::SWAP2 << eth::Instruction::POP;

				if (type.isDynamicallySized())
					m_context << skip.tag();
				// stack <target + "length"> <remainder = length % 32>
				m_context << eth::Instruction::POP;
			}
			else
				// stack: <target> <length>
				m_context << eth::Instruction::ADD;
		}
		else
		{
			solAssert(type.location() == ReferenceType::Location::Storage, "");
			m_context << eth::Instruction::POP; // remove offset, arrays always start new slot
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
			m_context << loopStart;
			// load and store
			m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
			m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
			// increment storage_data_offset by 1
			m_context << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::ADD;
			// increment memory offset by 32
			m_context << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD;
			// check for loop condition
			m_context << eth::Instruction::DUP1 << eth::Instruction::DUP4 << eth::Instruction::GT;
			m_context.appendConditionalJumpTo(loopStart);
			// stack here: memory_end_offset storage_data_offset memory_offset
			if (_padToWordBoundaries)
			{
				// memory_end_offset - start is the actual length (we want to compute the ceil of).
				// memory_offset - start is its next multiple of 32, but it might be off by 32.
				// so we compute: memory_end_offset += (memory_offset - memory_end_offest) & 31
				m_context << eth::Instruction::DUP3 << eth::Instruction::SWAP1 << eth::Instruction::SUB;
				m_context << u256(31) << eth::Instruction::AND;
				m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
				m_context << eth::Instruction::SWAP2;
			}
			m_context << loopEnd << eth::Instruction::POP << eth::Instruction::POP;
		}
	}
	else
	{
		unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
		if (numBytes > 0)
		{
			solAssert(_type.getSizeOnStack() == 1, "Memory store of types with stack size != 1 not implemented.");
			m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
			m_context << u256(numBytes) << eth::Instruction::ADD;
		}
	}
}

void CompilerUtils::encodeToMemory(
	TypePointers const& _givenTypes,
	TypePointers const& _targetTypes,
	bool _padToWordBoundaries,
	bool _copyDynamicDataInPlace
)
{
	// stack: <v1> <v2> ... <vn> <mem>
	TypePointers targetTypes = _targetTypes.empty() ? _givenTypes : _targetTypes;
	solAssert(targetTypes.size() == _givenTypes.size(), "");
	for (TypePointer& t: targetTypes)
		t = t->mobileType()->externalType();

	// Stack during operation:
	// <v1> <v2> ... <vn> <mem_start> <dyn_head_1> ... <dyn_head_r> <end_of_mem>
	// The values dyn_head_i are added during the first loop and they point to the head part
	// of the ith dynamic parameter, which is filled once the dynamic parts are processed.

	// store memory start pointer
	m_context << eth::Instruction::DUP1;

	unsigned argSize = CompilerUtils::getSizeOnStack(_givenTypes);
	unsigned stackPos = 0; // advances through the argument values
	unsigned dynPointers = 0; // number of dynamic head pointers on the stack
	for (size_t i = 0; i < _givenTypes.size(); ++i)
	{
		TypePointer targetType = targetTypes[i];
		solAssert(!!targetType, "Externalable type expected.");
		if (targetType->isDynamicallySized() && !_copyDynamicDataInPlace)
		{
			// leave end_of_mem as dyn head pointer
			m_context << eth::Instruction::DUP1 << u256(32) << eth::Instruction::ADD;
			dynPointers++;
		}
		else
		{
			copyToStackTop(argSize - stackPos + dynPointers + 2, _givenTypes[i]->getSizeOnStack());
			solAssert(!!targetType, "Externalable type expected.");
			TypePointer type = targetType;
			if (_givenTypes[i]->isInStorage())
				type = _givenTypes[i]; // delay conversion
			else
				convertType(*_givenTypes[i], *targetType, true);
			storeInMemoryDynamic(*type, _padToWordBoundaries);
		}
		stackPos += _givenTypes[i]->getSizeOnStack();
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
			solAssert(_givenTypes[i]->getCategory() == Type::Category::Array, "Unknown dynamic type.");
			auto const& arrayType = dynamic_cast<ArrayType const&>(*_givenTypes[i]);
			// copy tail pointer (=mem_end - mem_start) to memory
			m_context << eth::dupInstruction(2 + dynPointers) << eth::Instruction::DUP2;
			m_context << eth::Instruction::SUB;
			m_context << eth::dupInstruction(2 + dynPointers - thisDynPointer);
			m_context << eth::Instruction::MSTORE;
			// now copy the array
			copyToStackTop(argSize - stackPos + dynPointers + 2, arrayType.getSizeOnStack());
			// stack: ... <end_of_mem> <value...>
			// copy length to memory
			m_context << eth::dupInstruction(1 + arrayType.getSizeOnStack());
			if (arrayType.location() == ReferenceType::Location::CallData)
				m_context << eth::Instruction::DUP2; // length is on stack
			else if (arrayType.location() == ReferenceType::Location::Storage)
				m_context << eth::Instruction::DUP3 << eth::Instruction::SLOAD;
			else
			{
				solAssert(arrayType.location() == ReferenceType::Location::Memory, "");
				m_context << eth::Instruction::DUP2 << eth::Instruction::MLOAD;
			}
			// stack: ... <end_of_mem> <value...> <end_of_mem'> <length>
			storeInMemoryDynamic(IntegerType(256), true);
			// stack: ... <end_of_mem> <value...> <end_of_mem''>
			// copy the new memory pointer
			m_context << eth::swapInstruction(arrayType.getSizeOnStack() + 1) << eth::Instruction::POP;
			// stack: ... <end_of_mem''> <value...>
			// copy data part
			storeInMemoryDynamic(arrayType, true);
			// stack: ... <end_of_mem'''>

			thisDynPointer++;
		}
		stackPos += _givenTypes[i]->getSizeOnStack();
	}

	// remove unneeded stack elements (and retain memory pointer)
	m_context << eth::swapInstruction(argSize + dynPointers + 1);
	popStackSlots(argSize + dynPointers + 1);
}

void CompilerUtils::convertType(Type const& _typeOnStack, Type const& _targetType, bool _cleanupNeeded)
{
	// For a type extension, we need to remove all higher-order bits that we might have ignored in
	// previous operations.
	// @todo: store in the AST whether the operand might have "dirty" higher order bits

	if (_typeOnStack == _targetType && !_cleanupNeeded)
		return;
	Type::Category stackTypeCategory = _typeOnStack.getCategory();
	Type::Category targetTypeCategory = _targetType.getCategory();

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
			m_context << (u256(1) << (256 - typeOnStack.getNumBytes() * 8)) << eth::Instruction::SWAP1 << eth::Instruction::DIV;
			if (targetIntegerType.getNumBits() < typeOnStack.getNumBytes() * 8)
				convertType(IntegerType(typeOnStack.getNumBytes() * 8), _targetType, _cleanupNeeded);
		}
		else
		{
			// clear lower-order bytes for conversion to shorter bytes - we always clean
			solAssert(targetTypeCategory == Type::Category::FixedBytes, "Invalid type conversion requested.");
			FixedBytesType const& targetType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (targetType.getNumBytes() < typeOnStack.getNumBytes())
			{
				if (targetType.getNumBytes() == 0)
					m_context << eth::Instruction::DUP1 << eth::Instruction::XOR;
				else
				{
					m_context << (u256(1) << (256 - targetType.getNumBytes() * 8));
					m_context << eth::Instruction::DUP1 << eth::Instruction::SWAP2;
					m_context << eth::Instruction::DIV << eth::Instruction::MUL;
				}
			}
		}
	}
		break;
	case Type::Category::Enum:
		solAssert(targetTypeCategory == Type::Category::Integer || targetTypeCategory == Type::Category::Enum, "");
		break;
	case Type::Category::Integer:
	case Type::Category::Contract:
	case Type::Category::IntegerConstant:
		if (targetTypeCategory == Type::Category::FixedBytes)
		{
			solAssert(stackTypeCategory == Type::Category::Integer || stackTypeCategory == Type::Category::IntegerConstant,
				"Invalid conversion to FixedBytesType requested.");
			// conversion from bytes to string. no need to clean the high bit
			// only to shift left because of opposite alignment
			FixedBytesType const& targetBytesType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (auto typeOnStack = dynamic_cast<IntegerType const*>(&_typeOnStack))
				if (targetBytesType.getNumBytes() * 8 > typeOnStack->getNumBits())
					cleanHigherOrderBits(*typeOnStack);
			m_context << (u256(1) << (256 - targetBytesType.getNumBytes() * 8)) << eth::Instruction::MUL;
		}
		else if (targetTypeCategory == Type::Category::Enum)
			// just clean
			convertType(_typeOnStack, *_typeOnStack.mobileType(), true);
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
					cleanHigherOrderBits(targetType);
			}
			else
			{
				IntegerType const& typeOnStack = stackTypeCategory == Type::Category::Integer
					? dynamic_cast<IntegerType const&>(_typeOnStack) : addressType;
				// Widening: clean up according to source type width
				// Non-widening and force: clean up according to target type bits
				if (targetType.getNumBits() > typeOnStack.getNumBits())
					cleanHigherOrderBits(typeOnStack);
				else if (_cleanupNeeded)
					cleanHigherOrderBits(targetType);
			}
		}
		break;
	case Type::Category::Array:
	{
		solAssert(targetTypeCategory == stackTypeCategory, "");
		ArrayType const& typeOnStack = dynamic_cast<ArrayType const&>(_typeOnStack);
		ArrayType const& targetType = dynamic_cast<ArrayType const&>(_targetType);
		switch (targetType.location())
		{
		case ReferenceType::Location::Storage:
			// Other cases are done explicitly in LValue::storeValue, and only possible by assignment.
			solAssert(
				targetType.isPointer() &&
				typeOnStack.location() == ReferenceType::Location::Storage,
				"Invalid conversion to storage type."
			);
			break;
		case ReferenceType::Location::Memory:
		{
			// Copy the array to a free position in memory, unless it is already in memory.
			if (typeOnStack.location() != ReferenceType::Location::Memory)
			{
				// stack: <source ref> (variably sized)
				unsigned stackSize = typeOnStack.getSizeOnStack();
				fetchFreeMemoryPointer();
				moveIntoStack(stackSize);
				// stack: <mem start> <source ref> (variably sized)
				if (targetType.isDynamicallySized())
				{
					bool fromStorage = (typeOnStack.location() == ReferenceType::Location::Storage);
					// store length
					if (fromStorage)
					{
						stackSize--;
						// remove storage offset, as requested by ArrayUtils::retrieveLength
						m_context << eth::Instruction::POP;
					}
					ArrayUtils(m_context).retrieveLength(typeOnStack);
					// Stack: <mem start> <source ref> <length>
					m_context << eth::dupInstruction(2 + stackSize) << eth::Instruction::MSTORE;
					m_context << eth::dupInstruction(1 + stackSize) << u256(0x20);
					m_context << eth::Instruction::ADD;
					moveIntoStack(stackSize);
					if (fromStorage)
					{
						m_context << u256(0);
						stackSize++;
					}
				}
				else
				{
					m_context << eth::dupInstruction(1 + stackSize);
					moveIntoStack(stackSize);
				}
				// Stack: <mem start> <mem data start> <value>
				// Store data part.
				storeInMemoryDynamic(typeOnStack);
				// Stack <mem start> <mem end>
				storeFreeMemoryPointer();
			}
			else if (typeOnStack.location() == ReferenceType::Location::CallData)
			{
				// Stack: <offset> <length>
				//@todo
				solAssert(false, "Not yet implemented.");
			}
			// nothing to do for memory to memory
			break;
		}
		default:
			solAssert(false, "Invalid type conversion requested.");
		}
		break;
	}
	case Type::Category::Struct:
	{
		//@todo we can probably use some of the code for arrays here.
		solAssert(targetTypeCategory == stackTypeCategory, "");
		auto& targetType = dynamic_cast<StructType const&>(_targetType);
		auto& stackType = dynamic_cast<StructType const&>(_typeOnStack);
		solAssert(
			targetType.location() == ReferenceType::Location::Storage &&
				stackType.location() == ReferenceType::Location::Storage,
			"Non-storage structs not yet implemented."
		);
		solAssert(
			targetType.isPointer(),
			"Type conversion to non-pointer struct requested."
		);
		break;
	}
	default:
		// All other types should not be convertible to non-equal types.
		solAssert(_typeOnStack == _targetType, "Invalid type conversion requested.");
		break;
	}
}

void CompilerUtils::moveToStackVariable(VariableDeclaration const& _variable)
{
	unsigned const stackPosition = m_context.baseToCurrentStackOffset(m_context.getBaseStackOffsetOfVariable(_variable));
	unsigned const size = _variable.getType()->getSizeOnStack();
	solAssert(stackPosition >= size, "Variable size and position mismatch.");
	// move variable starting from its top end in the stack
	if (stackPosition - size + 1 > 16)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_variable.getLocation()) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	for (unsigned i = 0; i < size; ++i)
		m_context << eth::swapInstruction(stackPosition - size + 1) << eth::Instruction::POP;
}

void CompilerUtils::copyToStackTop(unsigned _stackDepth, unsigned _itemSize)
{
	solAssert(_stackDepth <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 0; i < _itemSize; ++i)
		m_context << eth::dupInstruction(_stackDepth);
}

void CompilerUtils::moveToStackTop(unsigned _stackDepth)
{
	solAssert(_stackDepth <= 15, "Stack too deep, try removing local variables.");
	for (unsigned i = 0; i < _stackDepth; ++i)
		m_context << eth::swapInstruction(1 + i);
}

void CompilerUtils::moveIntoStack(unsigned _stackDepth)
{
	solAssert(_stackDepth <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = _stackDepth; i > 0; --i)
		m_context << eth::swapInstruction(i);
}

void CompilerUtils::popStackElement(Type const& _type)
{
	popStackSlots(_type.getSizeOnStack());
}

void CompilerUtils::popStackSlots(size_t _amount)
{
	for (size_t i = 0; i < _amount; ++i)
		m_context << eth::Instruction::POP;
}

unsigned CompilerUtils::getSizeOnStack(vector<shared_ptr<Type const>> const& _variableTypes)
{
	unsigned size = 0;
	for (shared_ptr<Type const> const& type: _variableTypes)
		size += type->getSizeOnStack();
	return size;
}

void CompilerUtils::computeHashStatic(Type const& _type, bool _padToWordBoundaries)
{
	unsigned length = storeInMemory(0, _type, _padToWordBoundaries);
	solAssert(length <= CompilerUtils::freeMemoryPointer, "");
	m_context << u256(length) << u256(0) << eth::Instruction::SHA3;
}

unsigned CompilerUtils::loadFromMemoryHelper(Type const& _type, bool _fromCalldata, bool _padToWordBoundaries)
{
	unsigned numBytes = _type.getCalldataEncodedSize(_padToWordBoundaries);
	bool leftAligned = _type.getCategory() == Type::Category::FixedBytes;
	if (numBytes == 0)
		m_context << eth::Instruction::POP << u256(0);
	else
	{
		solAssert(numBytes <= 32, "Static memory load of more than 32 bytes requested.");
		m_context << (_fromCalldata ? eth::Instruction::CALLDATALOAD : eth::Instruction::MLOAD);
		if (numBytes != 32)
		{
			// add leading or trailing zeros by dividing/multiplying depending on alignment
			u256 shiftFactor = u256(1) << ((32 - numBytes) * 8);
			m_context << shiftFactor << eth::Instruction::SWAP1 << eth::Instruction::DIV;
			if (leftAligned)
				m_context << shiftFactor << eth::Instruction::MUL;
		}
	}

	return numBytes;
}

void CompilerUtils::cleanHigherOrderBits(IntegerType const& _typeOnStack)
{
	if (_typeOnStack.getNumBits() == 256)
		return;
	else if (_typeOnStack.isSigned())
		m_context << u256(_typeOnStack.getNumBits() / 8 - 1) << eth::Instruction::SIGNEXTEND;
	else
		m_context << ((u256(1) << _typeOnStack.getNumBits()) - 1) << eth::Instruction::AND;
}

unsigned CompilerUtils::prepareMemoryStore(Type const& _type, bool _padToWordBoundaries) const
{
	unsigned numBytes = _type.getCalldataEncodedSize(_padToWordBoundaries);
	bool leftAligned = _type.getCategory() == Type::Category::FixedBytes;
	if (numBytes == 0)
		m_context << eth::Instruction::POP;
	else
	{
		solAssert(numBytes <= 32, "Memory store of more than 32 bytes requested.");
		if (numBytes != 32 && !leftAligned && !_padToWordBoundaries)
			// shift the value accordingly before storing
			m_context << (u256(1) << ((32 - numBytes) * 8)) << eth::Instruction::MUL;
	}
	return numBytes;
}

}
}
