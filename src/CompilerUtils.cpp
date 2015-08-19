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
#include <libsolidity/LValue.h>

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

void CompilerUtils::allocateMemory()
{
	fetchFreeMemoryPointer();
	m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2 << eth::Instruction::ADD;
	storeFreeMemoryPointer();
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

void CompilerUtils::storeInMemory(unsigned _offset)
{
	unsigned numBytes = prepareMemoryStore(IntegerType(256), true);
	if (numBytes > 0)
		m_context << u256(_offset) << eth::Instruction::MSTORE;
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
		m_context << eth::Instruction::DUP1;
		storeStringData(bytesConstRef(str->value()));
		if (_padToWordBoundaries)
			m_context << u256(((str->value().size() + 31) / 32) * 32);
		else
			m_context << u256(str->value().size());
		m_context << eth::Instruction::ADD;
	}
	else
	{
		unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
		if (numBytes > 0)
		{
			solAssert(
				_type.getSizeOnStack() == 1,
				"Memory store of types with stack size != 1 not implemented."
			);
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
			if (
				_givenTypes[i]->dataStoredIn(DataLocation::Storage) ||
				_givenTypes[i]->dataStoredIn(DataLocation::CallData) ||
				_givenTypes[i]->getCategory() == Type::Category::StringLiteral
			)
				type = _givenTypes[i]; // delay conversion
			else
				convertType(*_givenTypes[i], *targetType, true);
			if (auto arrayType = dynamic_cast<ArrayType const*>(type.get()))
				ArrayUtils(m_context).copyArrayToMemory(*arrayType, _padToWordBoundaries);
			else
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
			// copy tail pointer (=mem_end - mem_start) to memory
			m_context << eth::dupInstruction(2 + dynPointers) << eth::Instruction::DUP2;
			m_context << eth::Instruction::SUB;
			m_context << eth::dupInstruction(2 + dynPointers - thisDynPointer);
			m_context << eth::Instruction::MSTORE;
			// stack: ... <end_of_mem>
			if (_givenTypes[i]->getCategory() == Type::Category::StringLiteral)
			{
				auto const& strType = dynamic_cast<StringLiteralType const&>(*_givenTypes[i]);
				m_context << u256(strType.value().size());
				storeInMemoryDynamic(IntegerType(256), true);
				// stack: ... <end_of_mem'>
				storeInMemoryDynamic(strType, _padToWordBoundaries);
			}
			else
			{
				solAssert(_givenTypes[i]->getCategory() == Type::Category::Array, "Unknown dynamic type.");
				auto const& arrayType = dynamic_cast<ArrayType const&>(*_givenTypes[i]);
				// now copy the array
				copyToStackTop(argSize - stackPos + dynPointers + 2, arrayType.getSizeOnStack());
				// stack: ... <end_of_mem> <value...>
				// copy length to memory
				m_context << eth::dupInstruction(1 + arrayType.getSizeOnStack());
				if (arrayType.location() == DataLocation::CallData)
					m_context << eth::Instruction::DUP2; // length is on stack
				else if (arrayType.location() == DataLocation::Storage)
					m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
				else
				{
					solAssert(arrayType.location() == DataLocation::Memory, "");
					m_context << eth::Instruction::DUP2 << eth::Instruction::MLOAD;
				}
				// stack: ... <end_of_mem> <value...> <end_of_mem'> <length>
				storeInMemoryDynamic(IntegerType(256), true);
				// stack: ... <end_of_mem> <value...> <end_of_mem''>
				// copy the new memory pointer
				m_context << eth::swapInstruction(arrayType.getSizeOnStack() + 1) << eth::Instruction::POP;
				// stack: ... <end_of_mem''> <value...>
				// copy data part
				ArrayUtils(m_context).copyArrayToMemory(arrayType, _padToWordBoundaries);
				// stack: ... <end_of_mem'''>
			}

			thisDynPointer++;
		}
		stackPos += _givenTypes[i]->getSizeOnStack();
	}

	// remove unneeded stack elements (and retain memory pointer)
	m_context << eth::swapInstruction(argSize + dynPointers + 1);
	popStackSlots(argSize + dynPointers + 1);
}

void CompilerUtils::memoryCopy()
{
	// Stack here: size target source
	// stack for call: outsize target size source value contract gas
	//@TODO do not use ::CALL if less than 32 bytes?
	m_context << eth::Instruction::DUP3 << eth::Instruction::SWAP1;
	m_context << u256(0) << u256(identityContractAddress);
	// compute gas costs
	m_context << u256(32) << eth::Instruction::DUP5 << u256(31) << eth::Instruction::ADD;
	m_context << eth::Instruction::DIV << u256(eth::c_identityWordGas) << eth::Instruction::MUL;
	m_context << u256(eth::c_identityGas) << eth::Instruction::ADD;
	m_context << eth::Instruction::CALL;
	m_context << eth::Instruction::POP; // ignore return value
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
			m_context << (u256(1) << (256 - typeOnStack.numBytes() * 8)) << eth::Instruction::SWAP1 << eth::Instruction::DIV;
			if (targetIntegerType.getNumBits() < typeOnStack.numBytes() * 8)
				convertType(IntegerType(typeOnStack.numBytes() * 8), _targetType, _cleanupNeeded);
		}
		else
		{
			// clear lower-order bytes for conversion to shorter bytes - we always clean
			solAssert(targetTypeCategory == Type::Category::FixedBytes, "Invalid type conversion requested.");
			FixedBytesType const& targetType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (targetType.numBytes() < typeOnStack.numBytes())
			{
				if (targetType.numBytes() == 0)
					m_context << eth::Instruction::DUP1 << eth::Instruction::XOR;
				else
				{
					m_context << (u256(1) << (256 - targetType.numBytes() * 8));
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
				if (targetBytesType.numBytes() * 8 > typeOnStack->getNumBits())
					cleanHigherOrderBits(*typeOnStack);
			m_context << (u256(1) << (256 - targetBytesType.numBytes() * 8)) << eth::Instruction::MUL;
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
			m_context << eth::Instruction::DUP1 << u256(data.size());
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
				unsigned stackSize = typeOnStack.getSizeOnStack();
				ArrayUtils(m_context).retrieveLength(typeOnStack);

				// allocate memory
				// stack: <source ref> (variably sized) <length>
				m_context << eth::Instruction::DUP1;
				ArrayUtils(m_context).convertLengthToSize(targetType, true);
				// stack: <source ref> (variably sized) <length> <size>
				if (targetType.isDynamicallySized())
					m_context << u256(0x20) << eth::Instruction::ADD;
				allocateMemory();
				// stack: <source ref> (variably sized) <length> <mem start>
				m_context << eth::Instruction::DUP1;
				moveIntoStack(2 + stackSize);
				if (targetType.isDynamicallySized())
				{
					m_context << eth::Instruction::DUP2;
					storeInMemoryDynamic(IntegerType(256));
				}
				// stack: <mem start> <source ref> (variably sized) <length> <mem data pos>
				if (targetType.getBaseType()->isValueType())
				{
					solAssert(typeOnStack.getBaseType()->isValueType(), "");
					copyToStackTop(2 + stackSize, stackSize);
					ArrayUtils(m_context).copyArrayToMemory(typeOnStack);
				}
				else
				{
					m_context << u256(0) << eth::Instruction::SWAP1;
					// stack: <mem start> <source ref> (variably sized) <length> <counter> <mem data pos>
					auto repeat = m_context.newTag();
					m_context << repeat;
					m_context << eth::Instruction::DUP3 << eth::Instruction::DUP3;
					m_context << eth::Instruction::LT << eth::Instruction::ISZERO;
					auto loopEnd = m_context.appendConditionalJump();
					copyToStackTop(3 + stackSize, stackSize);
					copyToStackTop(2 + stackSize, 1);
					ArrayUtils(m_context).accessIndex(typeOnStack, false);
					if (typeOnStack.location() == DataLocation::Storage)
						StorageItem(m_context, *typeOnStack.getBaseType()).retrieveValue(SourceLocation(), true);
					convertType(*typeOnStack.getBaseType(), *targetType.getBaseType(), _cleanupNeeded);
					storeInMemoryDynamic(*targetType.getBaseType(), true);
					m_context << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::ADD;
					m_context << eth::Instruction::SWAP1;
					m_context.appendJumpTo(repeat);
					m_context << loopEnd;
					m_context << eth::Instruction::POP;
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
				m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
				// stack: <memory ptr> <source ref> <memory ptr>
				for (auto const& member: typeOnStack.getMembers())
				{
					if (!member.type->canLiveOutsideStorage())
						continue;
					pair<u256, unsigned> const& offsets = typeOnStack.getStorageOffsetsOfMember(member.name);
					m_context << offsets.first << eth::Instruction::DUP3 << eth::Instruction::ADD;
					m_context << u256(offsets.second);
					StorageItem(m_context, *member.type).retrieveValue(SourceLocation(), true);
					TypePointer targetMemberType = targetType.getMemberType(member.name);
					solAssert(!!targetMemberType, "Member not found in target type.");
					convertType(*member.type, *targetMemberType, true);
					storeInMemoryDynamic(*targetMemberType, true);
				}
				m_context << eth::Instruction::POP << eth::Instruction::POP;
			}
			break;
		case DataLocation::CallData:
			solAssert(false, "Invalid type conversion target location CallData.");
			break;
		}
		break;
	}
	default:
		// All other types should not be convertible to non-equal types.
		solAssert(_typeOnStack == _targetType, "Invalid type conversion requested.");
		break;
	}
}

void CompilerUtils::pushZeroValue(const Type& _type)
{
	auto const* referenceType = dynamic_cast<ReferenceType const*>(&_type);
	if (!referenceType || referenceType->location() == DataLocation::Storage)
	{
		for (size_t i = 0; i < _type.getSizeOnStack(); ++i)
			m_context << u256(0);
		return;
	}
	solAssert(referenceType->location() == DataLocation::Memory, "");

	m_context << u256(max(32u, _type.getCalldataEncodedSize()));
	allocateMemory();
	m_context << eth::Instruction::DUP1;

	if (auto structType = dynamic_cast<StructType const*>(&_type))
		for (auto const& member: structType->getMembers())
		{
			pushZeroValue(*member.type);
			storeInMemoryDynamic(*member.type);
		}
	else if (auto arrayType = dynamic_cast<ArrayType const*>(&_type))
	{
		if (arrayType->isDynamicallySized())
		{
			// zero length
			m_context << u256(0);
			storeInMemoryDynamic(IntegerType(256));
		}
		else if (arrayType->getLength() > 0)
		{
			m_context << arrayType->getLength() << eth::Instruction::SWAP1;
			// stack: items_to_do memory_pos
			auto repeat = m_context.newTag();
			m_context << repeat;
			pushZeroValue(*arrayType->getBaseType());
			storeInMemoryDynamic(*arrayType->getBaseType());
			m_context << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::SWAP1;
			m_context << eth::Instruction::SUB << eth::Instruction::SWAP1;
			m_context << eth::Instruction::DUP2;
			m_context.appendConditionalJumpTo(repeat);
			m_context << eth::Instruction::SWAP1 << eth::Instruction::POP;
		}
	}
	else
		solAssert(false, "Requested initialisation for unknown type: " + _type.toString());

	// remove the updated memory pointer
	m_context << eth::Instruction::POP;
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

void CompilerUtils::computeHashStatic()
{
	storeInMemory(0);
	m_context << u256(32) << u256(0) << eth::Instruction::SHA3;
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
		m_context << eth::Instruction::POP;
	}
	else
	{
		// stack: mempos mempos_data
		m_context.appendData(_data.toBytes());
		m_context << u256(_data.size()) << eth::Instruction::SWAP2;
		m_context << eth::Instruction::CODECOPY;
	}
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
