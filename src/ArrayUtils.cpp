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
 * @date 2015
 * Code generation utils that handle arrays.
 */

#include <libsolidity/ArrayUtils.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/CompilerContext.h>
#include <libsolidity/CompilerUtils.h>
#include <libsolidity/Types.h>
#include <libsolidity/Utils.h>
#include <libsolidity/LValue.h>

using namespace std;
using namespace dev;
using namespace solidity;

void ArrayUtils::copyArrayToStorage(ArrayType const& _targetType, ArrayType const& _sourceType) const
{
	// this copies source to target and also clears target if it was larger
	// need to leave "target_ref target_byte_off" on the stack at the end

	// stack layout: [source_ref] [source length] target_ref (top)
	solAssert(_targetType.location() == DataLocation::Storage, "");

	IntegerType uint256(256);
	Type const* targetBaseType = _targetType.isByteArray() ? &uint256 : &(*_targetType.getBaseType());
	Type const* sourceBaseType = _sourceType.isByteArray() ? &uint256 : &(*_sourceType.getBaseType());

	// TODO unroll loop for small sizes

	bool sourceIsStorage = _sourceType.location() == DataLocation::Storage;
	bool fromCalldata = _sourceType.location() == DataLocation::CallData;
	bool directCopy = sourceIsStorage && sourceBaseType->isValueType() && *sourceBaseType == *targetBaseType;
	bool haveByteOffsetSource = !directCopy && sourceIsStorage && sourceBaseType->getStorageBytes() <= 16;
	bool haveByteOffsetTarget = !directCopy && targetBaseType->getStorageBytes() <= 16;
	unsigned byteOffsetSize = (haveByteOffsetSource ? 1 : 0) + (haveByteOffsetTarget ? 1 : 0);

	// stack: source_ref [source_length] target_ref
	// store target_ref
	for (unsigned i = _sourceType.getSizeOnStack(); i > 0; --i)
		m_context << eth::swapInstruction(i);
	// stack: target_ref source_ref [source_length]
	// stack: target_ref source_ref [source_length]
	// retrieve source length
	if (_sourceType.location() != DataLocation::CallData || !_sourceType.isDynamicallySized())
		retrieveLength(_sourceType); // otherwise, length is already there
	if (_sourceType.location() == DataLocation::Memory && _sourceType.isDynamicallySized())
	{
		// increment source pointer to point to data
		m_context << eth::Instruction::SWAP1 << u256(0x20);
		m_context << eth::Instruction::ADD << eth::Instruction::SWAP1;
	}

	// stack: target_ref source_ref source_length
	m_context << eth::Instruction::DUP3;
	// stack: target_ref source_ref source_length target_ref
	retrieveLength(_targetType);
	// stack: target_ref source_ref source_length target_ref target_length
	if (_targetType.isDynamicallySized())
		// store new target length
		m_context << eth::Instruction::DUP3 << eth::Instruction::DUP3 << eth::Instruction::SSTORE;
	if (sourceBaseType->getCategory() == Type::Category::Mapping)
	{
		solAssert(targetBaseType->getCategory() == Type::Category::Mapping, "");
		solAssert(_sourceType.location() == DataLocation::Storage, "");
		// nothing to copy
		m_context
			<< eth::Instruction::POP << eth::Instruction::POP
			<< eth::Instruction::POP << eth::Instruction::POP;
		return;
	}
	// compute hashes (data positions)
	m_context << eth::Instruction::SWAP1;
	if (_targetType.isDynamicallySized())
		CompilerUtils(m_context).computeHashStatic();
	// stack: target_ref source_ref source_length target_length target_data_pos
	m_context << eth::Instruction::SWAP1;
	convertLengthToSize(_targetType);
	m_context << eth::Instruction::DUP2 << eth::Instruction::ADD;
	// stack: target_ref source_ref source_length target_data_pos target_data_end
	m_context << eth::Instruction::SWAP3;
	// stack: target_ref target_data_end source_length target_data_pos source_ref
	// skip copying if source length is zero
	m_context << eth::Instruction::DUP3 << eth::Instruction::ISZERO;
	eth::AssemblyItem copyLoopEndWithoutByteOffset = m_context.newTag();
	m_context.appendConditionalJumpTo(copyLoopEndWithoutByteOffset);

	if (_sourceType.location() == DataLocation::Storage && _sourceType.isDynamicallySized())
		CompilerUtils(m_context).computeHashStatic();
	// stack: target_ref target_data_end source_length target_data_pos source_data_pos
	m_context << eth::Instruction::SWAP2;
	convertLengthToSize(_sourceType);
	m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end
	if (haveByteOffsetTarget)
		m_context << u256(0);
	if (haveByteOffsetSource)
		m_context << u256(0);
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
	eth::AssemblyItem copyLoopStart = m_context.newTag();
	m_context << copyLoopStart;
	// check for loop condition
	m_context
		<< eth::dupInstruction(3 + byteOffsetSize) << eth::dupInstruction(2 + byteOffsetSize)
		<< eth::Instruction::GT << eth::Instruction::ISZERO;
	eth::AssemblyItem copyLoopEnd = m_context.newTag();
	m_context.appendConditionalJumpTo(copyLoopEnd);
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
	// copy
	if (sourceBaseType->getCategory() == Type::Category::Array)
	{
		solAssert(byteOffsetSize == 0, "Byte offset for array as base type.");
		auto const& sourceBaseArrayType = dynamic_cast<ArrayType const&>(*sourceBaseType);
		m_context << eth::Instruction::DUP3;
		if (sourceBaseArrayType.location() == DataLocation::Memory)
			m_context << eth::Instruction::MLOAD;
		m_context << eth::Instruction::DUP3;
		copyArrayToStorage(dynamic_cast<ArrayType const&>(*targetBaseType), sourceBaseArrayType);
		m_context << eth::Instruction::POP;
	}
	else if (directCopy)
	{
		solAssert(byteOffsetSize == 0, "Byte offset for direct copy.");
		m_context
			<< eth::Instruction::DUP3 << eth::Instruction::SLOAD
			<< eth::Instruction::DUP3 << eth::Instruction::SSTORE;
	}
	else
	{
		// Note that we have to copy each element on its own in case conversion is involved.
		// We might copy too much if there is padding at the last element, but this way end
		// checking is easier.
		// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
		m_context << eth::dupInstruction(3 + byteOffsetSize);
		if (_sourceType.location() == DataLocation::Storage)
		{
			if (haveByteOffsetSource)
				m_context << eth::Instruction::DUP2;
			else
				m_context << u256(0);
			StorageItem(m_context, *sourceBaseType).retrieveValue(SourceLocation(), true);
		}
		else if (sourceBaseType->isValueType())
			CompilerUtils(m_context).loadFromMemoryDynamic(*sourceBaseType, fromCalldata, true, false);
		else
			solAssert(false, "Copying of type " + _sourceType.toString(false) + " to storage not yet supported.");
		// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset] <source_value>...
		solAssert(
			2 + byteOffsetSize + sourceBaseType->getSizeOnStack() <= 16,
			"Stack too deep, try removing local variables."
		);
		// fetch target storage reference
		m_context << eth::dupInstruction(2 + byteOffsetSize + sourceBaseType->getSizeOnStack());
		if (haveByteOffsetTarget)
			m_context << eth::dupInstruction(1 + byteOffsetSize + sourceBaseType->getSizeOnStack());
		else
			m_context << u256(0);
		StorageItem(m_context, *targetBaseType).storeValue(*sourceBaseType, SourceLocation(), true);
	}
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
	// increment source
	if (haveByteOffsetSource)
		incrementByteOffset(sourceBaseType->getStorageBytes(), 1, haveByteOffsetTarget ? 5 : 4);
	else
	{
		m_context << eth::swapInstruction(2 + byteOffsetSize);
		if (sourceIsStorage)
			m_context << sourceBaseType->getStorageSize();
		else if (_sourceType.location() == DataLocation::Memory)
			m_context << sourceBaseType->memoryHeadSize();
		else
			m_context << sourceBaseType->getCalldataEncodedSize(true);
		m_context
			<< eth::Instruction::ADD
			<< eth::swapInstruction(2 + byteOffsetSize);
	}
	// increment target
	if (haveByteOffsetTarget)
		incrementByteOffset(targetBaseType->getStorageBytes(), byteOffsetSize, byteOffsetSize + 2);
	else
		m_context
			<< eth::swapInstruction(1 + byteOffsetSize)
			<< targetBaseType->getStorageSize()
			<< eth::Instruction::ADD
			<< eth::swapInstruction(1 + byteOffsetSize);
	m_context.appendJumpTo(copyLoopStart);
	m_context << copyLoopEnd;
	if (haveByteOffsetTarget)
	{
		// clear elements that might be left over in the current slot in target
		// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end target_byte_offset [source_byte_offset]
		m_context << eth::dupInstruction(byteOffsetSize) << eth::Instruction::ISZERO;
		eth::AssemblyItem copyCleanupLoopEnd = m_context.appendConditionalJump();
		m_context << eth::dupInstruction(2 + byteOffsetSize) << eth::dupInstruction(1 + byteOffsetSize);
		StorageItem(m_context, *targetBaseType).setToZero(SourceLocation(), true);
		incrementByteOffset(targetBaseType->getStorageBytes(), byteOffsetSize, byteOffsetSize + 2);
		m_context.appendJumpTo(copyLoopEnd);

		m_context << copyCleanupLoopEnd;
		m_context << eth::Instruction::POP; // might pop the source, but then target is popped next
	}
	if (haveByteOffsetSource)
		m_context << eth::Instruction::POP;
	m_context << copyLoopEndWithoutByteOffset;

	// zero-out leftovers in target
	// stack: target_ref target_data_end source_data_pos target_data_pos_updated source_data_end
	m_context << eth::Instruction::POP << eth::Instruction::SWAP1 << eth::Instruction::POP;
	// stack: target_ref target_data_end target_data_pos_updated
	clearStorageLoop(*targetBaseType);
	m_context << eth::Instruction::POP;
}

void ArrayUtils::copyArrayToMemory(const ArrayType& _sourceType, bool _padToWordBoundaries) const
{
	solAssert(
		_sourceType.getBaseType()->getCalldataEncodedSize() > 0,
		"Nested dynamic arrays not implemented here."
	);
	CompilerUtils utils(m_context);
	unsigned baseSize = 1;
	if (!_sourceType.isByteArray())
		// We always pad the elements, regardless of _padToWordBoundaries.
		baseSize = _sourceType.getBaseType()->getCalldataEncodedSize();

	if (_sourceType.location() == DataLocation::CallData)
	{
		if (!_sourceType.isDynamicallySized())
			m_context << _sourceType.getLength();
		if (baseSize > 1)
			m_context << u256(baseSize) << eth::Instruction::MUL;
		// stack: target source_offset source_len
		m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3 << eth::Instruction::DUP5;
		// stack: target source_offset source_len source_len source_offset target
		m_context << eth::Instruction::CALLDATACOPY;
		m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
		m_context << eth::Instruction::SWAP2 << eth::Instruction::POP << eth::Instruction::POP;
	}
	else if (_sourceType.location() == DataLocation::Memory)
	{
		retrieveLength(_sourceType);
		// stack: target source length
		if (!_sourceType.getBaseType()->isValueType())
		{
			// copy using a loop
			m_context << u256(0) << eth::Instruction::SWAP3;
			// stack: counter source length target
			auto repeat = m_context.newTag();
			m_context << repeat;
			m_context << eth::Instruction::DUP2 << eth::Instruction::DUP5;
			m_context << eth::Instruction::LT << eth::Instruction::ISZERO;
			auto loopEnd = m_context.appendConditionalJump();
			m_context << eth::Instruction::DUP3 << eth::Instruction::DUP5;
			accessIndex(_sourceType, false);
			MemoryItem(m_context, *_sourceType.getBaseType(), true).retrieveValue(SourceLocation(), true);
			if (auto baseArray = dynamic_cast<ArrayType const*>(_sourceType.getBaseType().get()))
				copyArrayToMemory(*baseArray, _padToWordBoundaries);
			else
				utils.storeInMemoryDynamic(*_sourceType.getBaseType());
			m_context << eth::Instruction::SWAP3 << u256(1) << eth::Instruction::ADD;
			m_context << eth::Instruction::SWAP3;
			m_context.appendJumpTo(repeat);
			m_context << loopEnd;
			m_context << eth::Instruction::SWAP3;
			utils.popStackSlots(3);
			// stack: updated_target_pos
			return;
		}

		// memcpy using the built-in contract
		if (_sourceType.isDynamicallySized())
		{
			// change pointer to data part
			m_context << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD;
			m_context << eth::Instruction::SWAP1;
		}
		// convert length to size
		if (baseSize > 1)
			m_context << u256(baseSize) << eth::Instruction::MUL;
		// stack: <target> <source> <size>
		//@TODO do not use ::CALL if less than 32 bytes?
		m_context << eth::Instruction::DUP1 << eth::Instruction::DUP4 << eth::Instruction::DUP4;
		utils.memoryCopy();

		m_context << eth::Instruction::SWAP1 << eth::Instruction::POP;
		// stack: <target> <size>

		bool paddingNeeded = false;
		if (_sourceType.isDynamicallySized())
			paddingNeeded = _padToWordBoundaries && ((baseSize % 32) != 0);
		else
			paddingNeeded = _padToWordBoundaries && (((_sourceType.getLength() * baseSize) % 32) != 0);
		if (paddingNeeded)
		{
			// stack: <target> <size>
			m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2 << eth::Instruction::ADD;
			// stack: <length> <target + size>
			m_context << eth::Instruction::SWAP1 << u256(31) << eth::Instruction::AND;
			// stack: <target + size> <remainder = size % 32>
			eth::AssemblyItem skip = m_context.newTag();
			if (_sourceType.isDynamicallySized())
			{
				m_context << eth::Instruction::DUP1 << eth::Instruction::ISZERO;
				m_context.appendConditionalJumpTo(skip);
			}
			// round off, load from there.
			// stack <target + size> <remainder = size % 32>
			m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3;
			m_context << eth::Instruction::SUB;
			// stack: target+size remainder <target + size - remainder>
			m_context << eth::Instruction::DUP1 << eth::Instruction::MLOAD;
			// Now we AND it with ~(2**(8 * (32 - remainder)) - 1)
			m_context << u256(1);
			m_context << eth::Instruction::DUP4 << u256(32) << eth::Instruction::SUB;
			// stack: ...<v> 1 <32 - remainder>
			m_context << u256(0x100) << eth::Instruction::EXP << eth::Instruction::SUB;
			m_context << eth::Instruction::NOT << eth::Instruction::AND;
			// stack: target+size remainder target+size-remainder <v & ...>
			m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
			// stack: target+size remainder target+size-remainder
			m_context << u256(32) << eth::Instruction::ADD;
			// stack: target+size remainder <new_padded_end>
			m_context << eth::Instruction::SWAP2 << eth::Instruction::POP;

			if (_sourceType.isDynamicallySized())
				m_context << skip.tag();
			// stack <target + "size"> <remainder = size % 32>
			m_context << eth::Instruction::POP;
		}
		else
			// stack: <target> <size>
			m_context << eth::Instruction::ADD;
	}
	else
	{
		solAssert(_sourceType.location() == DataLocation::Storage, "");
		unsigned storageBytes = _sourceType.getBaseType()->getStorageBytes();
		u256 storageSize = _sourceType.getBaseType()->getStorageSize();
		solAssert(storageSize > 1 || (storageSize == 1 && storageBytes > 0), "");

		retrieveLength(_sourceType);
		// stack here: memory_offset storage_offset length
		// jump to end if length is zero
		m_context << eth::Instruction::DUP1 << eth::Instruction::ISZERO;
		eth::AssemblyItem loopEnd = m_context.newTag();
		m_context.appendConditionalJumpTo(loopEnd);
		// compute memory end offset
		if (baseSize > 1)
			// convert length to memory size
			m_context << u256(baseSize) << eth::Instruction::MUL;
		m_context << eth::Instruction::DUP3 << eth::Instruction::ADD << eth::Instruction::SWAP2;
		if (_sourceType.isDynamicallySized())
		{
			// actual array data is stored at SHA3(storage_offset)
			m_context << eth::Instruction::SWAP1;
			utils.computeHashStatic();
			m_context << eth::Instruction::SWAP1;
		}

		// stack here: memory_end_offset storage_data_offset memory_offset
		bool haveByteOffset = !_sourceType.isByteArray() && storageBytes <= 16;
		if (haveByteOffset)
			m_context << u256(0) << eth::Instruction::SWAP1;
		// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
		eth::AssemblyItem loopStart = m_context.newTag();
		m_context << loopStart;
		// load and store
		if (_sourceType.isByteArray())
		{
			// Packed both in storage and memory.
			m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
			m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
			// increment storage_data_offset by 1
			m_context << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::ADD;
			// increment memory offset by 32
			m_context << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD;
		}
		else
		{
			// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
			if (haveByteOffset)
				m_context << eth::Instruction::DUP3 << eth::Instruction::DUP3;
			else
				m_context << eth::Instruction::DUP2 << u256(0);
			StorageItem(m_context, *_sourceType.getBaseType()).retrieveValue(SourceLocation(), true);
			if (auto baseArray = dynamic_cast<ArrayType const*>(_sourceType.getBaseType().get()))
				copyArrayToMemory(*baseArray, _padToWordBoundaries);
			else
				utils.storeInMemoryDynamic(*_sourceType.getBaseType());
			// increment storage_data_offset and byte offset
			if (haveByteOffset)
				incrementByteOffset(storageBytes, 2, 3);
			else
			{
				m_context << eth::Instruction::SWAP1;
				m_context << storageSize << eth::Instruction::ADD;
				m_context << eth::Instruction::SWAP1;
			}
		}
		// check for loop condition
		m_context << eth::Instruction::DUP1 << eth::dupInstruction(haveByteOffset ? 5 : 4);
		m_context << eth::Instruction::GT;
		m_context.appendConditionalJumpTo(loopStart);
		// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
		if (haveByteOffset)
			m_context << eth::Instruction::SWAP1 << eth::Instruction::POP;
		if (_padToWordBoundaries && baseSize % 32 != 0)
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

void ArrayUtils::clearArray(ArrayType const& _type) const
{
	unsigned stackHeightStart = m_context.getStackHeight();
	solAssert(_type.location() == DataLocation::Storage, "");
	if (_type.getBaseType()->getStorageBytes() < 32)
	{
		solAssert(_type.getBaseType()->isValueType(), "Invalid storage size for non-value type.");
		solAssert(_type.getBaseType()->getStorageSize() <= 1, "Invalid storage size for type.");
	}
	if (_type.getBaseType()->isValueType())
		solAssert(_type.getBaseType()->getStorageSize() <= 1, "Invalid size for value type.");

	m_context << eth::Instruction::POP; // remove byte offset
	if (_type.isDynamicallySized())
		clearDynamicArray(_type);
	else if (_type.getLength() == 0 || _type.getBaseType()->getCategory() == Type::Category::Mapping)
		m_context << eth::Instruction::POP;
	else if (_type.getBaseType()->isValueType() && _type.getStorageSize() <= 5)
	{
		// unroll loop for small arrays @todo choose a good value
		// Note that we loop over storage slots here, not elements.
		for (unsigned i = 1; i < _type.getStorageSize(); ++i)
			m_context
				<< u256(0) << eth::Instruction::DUP2 << eth::Instruction::SSTORE
				<< u256(1) << eth::Instruction::ADD;
		m_context << u256(0) << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
	}
	else if (!_type.getBaseType()->isValueType() && _type.getLength() <= 4)
	{
		// unroll loop for small arrays @todo choose a good value
		solAssert(_type.getBaseType()->getStorageBytes() >= 32, "Invalid storage size.");
		for (unsigned i = 1; i < _type.getLength(); ++i)
		{
			m_context << u256(0);
			StorageItem(m_context, *_type.getBaseType()).setToZero(SourceLocation(), false);
			m_context
				<< eth::Instruction::POP
				<< u256(_type.getBaseType()->getStorageSize()) << eth::Instruction::ADD;
		}
		m_context << u256(0);
		StorageItem(m_context, *_type.getBaseType()).setToZero(SourceLocation(), true);
	}
	else
	{
		m_context << eth::Instruction::DUP1 << _type.getLength();
		convertLengthToSize(_type);
		m_context << eth::Instruction::ADD << eth::Instruction::SWAP1;
		if (_type.getBaseType()->getStorageBytes() < 32)
			clearStorageLoop(IntegerType(256));
		else
			clearStorageLoop(*_type.getBaseType());
		m_context << eth::Instruction::POP;
	}
	solAssert(m_context.getStackHeight() == stackHeightStart - 2, "");
}

void ArrayUtils::clearDynamicArray(ArrayType const& _type) const
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");

	unsigned stackHeightStart = m_context.getStackHeight();
	// fetch length
	m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
	// set length to zero
	m_context << u256(0) << eth::Instruction::DUP3 << eth::Instruction::SSTORE;
	// stack: ref old_length
	convertLengthToSize(_type);
	// compute data positions
	m_context << eth::Instruction::SWAP1;
	CompilerUtils(m_context).computeHashStatic();
	// stack: len data_pos
	m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2 << eth::Instruction::ADD
		<< eth::Instruction::SWAP1;
	// stack: data_pos_end data_pos
	if (_type.isByteArray() || _type.getBaseType()->getStorageBytes() < 32)
		clearStorageLoop(IntegerType(256));
	else
		clearStorageLoop(*_type.getBaseType());
	// cleanup
	m_context << eth::Instruction::POP;
	solAssert(m_context.getStackHeight() == stackHeightStart - 1, "");
}

void ArrayUtils::resizeDynamicArray(const ArrayType& _type) const
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	if (!_type.isByteArray() && _type.getBaseType()->getStorageBytes() < 32)
		solAssert(_type.getBaseType()->isValueType(), "Invalid storage size for non-value type.");

	unsigned stackHeightStart = m_context.getStackHeight();
	eth::AssemblyItem resizeEnd = m_context.newTag();

	// stack: ref new_length
	// fetch old length
	m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
	// stack: ref new_length old_length
	// store new length
	m_context << eth::Instruction::DUP2 << eth::Instruction::DUP4 << eth::Instruction::SSTORE;
	// skip if size is not reduced
	m_context << eth::Instruction::DUP2 << eth::Instruction::DUP2
		<< eth::Instruction::ISZERO << eth::Instruction::GT;
	m_context.appendConditionalJumpTo(resizeEnd);

	// size reduced, clear the end of the array
	// stack: ref new_length old_length
	convertLengthToSize(_type);
	m_context << eth::Instruction::DUP2;
	convertLengthToSize(_type);
	// stack: ref new_length old_size new_size
	// compute data positions
	m_context << eth::Instruction::DUP4;
	CompilerUtils(m_context).computeHashStatic();
	// stack: ref new_length old_size new_size data_pos
	m_context << eth::Instruction::SWAP2 << eth::Instruction::DUP3 << eth::Instruction::ADD;
	// stack: ref new_length data_pos new_size delete_end
	m_context << eth::Instruction::SWAP2 << eth::Instruction::ADD;
	// stack: ref new_length delete_end delete_start
	if (_type.isByteArray() || _type.getBaseType()->getStorageBytes() < 32)
		clearStorageLoop(IntegerType(256));
	else
		clearStorageLoop(*_type.getBaseType());

	m_context << resizeEnd;
	// cleanup
	m_context << eth::Instruction::POP << eth::Instruction::POP << eth::Instruction::POP;
	solAssert(m_context.getStackHeight() == stackHeightStart - 2, "");
}

void ArrayUtils::clearStorageLoop(Type const& _type) const
{
	unsigned stackHeightStart = m_context.getStackHeight();
	if (_type.getCategory() == Type::Category::Mapping)
	{
		m_context << eth::Instruction::POP;
		return;
	}
	// stack: end_pos pos

	// jump to and return from the loop to allow for duplicate code removal
	eth::AssemblyItem returnTag = m_context.pushNewTag();
	m_context << eth::Instruction::SWAP2 << eth::Instruction::SWAP1;

	// stack: <return tag> end_pos pos
	eth::AssemblyItem loopStart = m_context.appendJumpToNew();
	m_context << loopStart;
	// check for loop condition
	m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3
			   << eth::Instruction::GT << eth::Instruction::ISZERO;
	eth::AssemblyItem zeroLoopEnd = m_context.newTag();
	m_context.appendConditionalJumpTo(zeroLoopEnd);
	// delete
	m_context << u256(0);
	StorageItem(m_context, _type).setToZero(SourceLocation(), false);
	m_context << eth::Instruction::POP;
	// increment
	m_context << u256(1) << eth::Instruction::ADD;
	m_context.appendJumpTo(loopStart);
	// cleanup
	m_context << zeroLoopEnd;
	m_context << eth::Instruction::POP << eth::Instruction::SWAP1;
	// "return"
	m_context << eth::Instruction::JUMP;

	m_context << returnTag;
	solAssert(m_context.getStackHeight() == stackHeightStart - 1, "");
}

void ArrayUtils::convertLengthToSize(ArrayType const& _arrayType, bool _pad) const
{
	if (_arrayType.location() == DataLocation::Storage)
	{
		if (_arrayType.getBaseType()->getStorageSize() <= 1)
		{
			unsigned baseBytes = _arrayType.getBaseType()->getStorageBytes();
			if (baseBytes == 0)
				m_context << eth::Instruction::POP << u256(1);
			else if (baseBytes <= 16)
			{
				unsigned itemsPerSlot = 32 / baseBytes;
				m_context
					<< u256(itemsPerSlot - 1) << eth::Instruction::ADD
					<< u256(itemsPerSlot) << eth::Instruction::SWAP1 << eth::Instruction::DIV;
			}
		}
		else
			m_context << _arrayType.getBaseType()->getStorageSize() << eth::Instruction::MUL;
	}
	else
	{
		if (!_arrayType.isByteArray())
		{
			if (_arrayType.location() == DataLocation::Memory)
				m_context << _arrayType.getBaseType()->memoryHeadSize();
			else
				m_context << _arrayType.getBaseType()->getCalldataEncodedSize();
			m_context << eth::Instruction::MUL;
		}
		else if (_pad)
			m_context << u256(31) << eth::Instruction::ADD
				<< u256(32) << eth::Instruction::DUP1
				<< eth::Instruction::SWAP2 << eth::Instruction::DIV << eth::Instruction::MUL;
	}
}

void ArrayUtils::retrieveLength(ArrayType const& _arrayType) const
{
	if (!_arrayType.isDynamicallySized())
		m_context << _arrayType.getLength();
	else
	{
		m_context << eth::Instruction::DUP1;
		switch (_arrayType.location())
		{
		case DataLocation::CallData:
			// length is stored on the stack
			break;
		case DataLocation::Memory:
			m_context << eth::Instruction::MLOAD;
			break;
		case DataLocation::Storage:
			m_context << eth::Instruction::SLOAD;
			break;
		}
	}
}

void ArrayUtils::accessIndex(ArrayType const& _arrayType, bool _doBoundsCheck) const
{
	DataLocation location = _arrayType.location();
	eth::Instruction load =
		location == DataLocation::Storage ? eth::Instruction::SLOAD :
		location == DataLocation::Memory ? eth::Instruction::MLOAD :
		eth::Instruction::CALLDATALOAD;

	if (_doBoundsCheck)
	{
		// retrieve length
		if (!_arrayType.isDynamicallySized())
			m_context << _arrayType.getLength();
		else if (location == DataLocation::CallData)
			// length is stored on the stack
			m_context << eth::Instruction::SWAP1;
		else
			m_context << eth::Instruction::DUP2 << load;
		// stack: <base_ref> <index> <length>
		// check out-of-bounds access
		m_context << eth::Instruction::DUP2 << eth::Instruction::LT << eth::Instruction::ISZERO;
		// out-of-bounds access throws exception
		m_context.appendConditionalJumpTo(m_context.errorTag());
	}
	else if (location == DataLocation::CallData && _arrayType.isDynamicallySized())
		// remove length if present
		m_context << eth::Instruction::SWAP1 << eth::Instruction::POP;

	// stack: <base_ref> <index>
	m_context << eth::Instruction::SWAP1;
	if (_arrayType.isDynamicallySized())
	{
		if (location == DataLocation::Storage)
			CompilerUtils(m_context).computeHashStatic();
		else if (location == DataLocation::Memory)
			m_context << u256(32) << eth::Instruction::ADD;
	}
	// stack: <index> <data_ref>
	switch (location)
	{
	case DataLocation::CallData:
	case DataLocation::Memory:
		if (!_arrayType.isByteArray())
		{
			m_context << eth::Instruction::SWAP1;
			if (location == DataLocation::CallData)
				m_context << _arrayType.getBaseType()->getCalldataEncodedSize();
			else
				m_context << u256(_arrayType.memoryHeadSize());
			m_context << eth::Instruction::MUL;
		}
		m_context << eth::Instruction::ADD;
		break;
	case DataLocation::Storage:
		m_context << eth::Instruction::SWAP1;
		if (_arrayType.getBaseType()->getStorageBytes() <= 16)
		{
			// stack: <data_ref> <index>
			// goal:
			// <ref> <byte_number> = <base_ref + index / itemsPerSlot> <(index % itemsPerSlot) * byteSize>
			unsigned byteSize = _arrayType.getBaseType()->getStorageBytes();
			solAssert(byteSize != 0, "");
			unsigned itemsPerSlot = 32 / byteSize;
			m_context << u256(itemsPerSlot) << eth::Instruction::SWAP2;
			// stack: itemsPerSlot index data_ref
			m_context
				<< eth::Instruction::DUP3 << eth::Instruction::DUP3
				<< eth::Instruction::DIV << eth::Instruction::ADD
			// stack: itemsPerSlot index (data_ref + index / itemsPerSlot)
				<< eth::Instruction::SWAP2 << eth::Instruction::SWAP1
				<< eth::Instruction::MOD;
			if (byteSize != 1)
				m_context << u256(byteSize) << eth::Instruction::MUL;
		}
		else
		{
			if (_arrayType.getBaseType()->getStorageSize() != 1)
				m_context << _arrayType.getBaseType()->getStorageSize() << eth::Instruction::MUL;
			m_context << eth::Instruction::ADD << u256(0);
		}
		break;
	}
}

void ArrayUtils::incrementByteOffset(unsigned _byteSize, unsigned _byteOffsetPosition, unsigned _storageOffsetPosition) const
{
	solAssert(_byteSize < 32, "");
	solAssert(_byteSize != 0, "");
	// We do the following, but avoiding jumps:
	// byteOffset += byteSize
	// if (byteOffset + byteSize > 32)
	// {
	//     storageOffset++;
	//     byteOffset = 0;
	// }
	if (_byteOffsetPosition > 1)
		m_context << eth::swapInstruction(_byteOffsetPosition - 1);
	m_context << u256(_byteSize) << eth::Instruction::ADD;
	if (_byteOffsetPosition > 1)
		m_context << eth::swapInstruction(_byteOffsetPosition - 1);
	// compute, X := (byteOffset + byteSize - 1) / 32, should be 1 iff byteOffset + bytesize > 32
	m_context
		<< u256(32) << eth::dupInstruction(1 + _byteOffsetPosition) << u256(_byteSize - 1)
		<< eth::Instruction::ADD << eth::Instruction::DIV;
	// increment storage offset if X == 1 (just add X to it)
	// stack: X
	m_context
		<< eth::swapInstruction(_storageOffsetPosition) << eth::dupInstruction(_storageOffsetPosition + 1)
		<< eth::Instruction::ADD << eth::swapInstruction(_storageOffsetPosition);
	// stack: X
	// set source_byte_offset to zero if X == 1 (using source_byte_offset *= 1 - X)
	m_context << u256(1) << eth::Instruction::SUB;
	// stack: 1 - X
	if (_byteOffsetPosition == 1)
		m_context << eth::Instruction::MUL;
	else
		m_context
			<< eth::dupInstruction(_byteOffsetPosition + 1) << eth::Instruction::MUL
			<< eth::swapInstruction(_byteOffsetPosition) << eth::Instruction::POP;
}
