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
 * @date 2015
 * Code generation utils that handle arrays.
 */

#include <libsolidity/codegen/ArrayUtils.h>

#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/LValue.h>

#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Whiskers.h>
#include <libsolutil/StackTooDeepString.h>

#include <libevmasm/Instruction.h>
#include <liblangutil/Exceptions.h>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;

void ArrayUtils::copyArrayToStorage(ArrayType const& _targetType, ArrayType const& _sourceType) const
{
	// this copies source to target and also clears target if it was larger
	// need to leave "target_ref target_byte_off" on the stack at the end

	// stack layout: [source_ref] [source length] target_ref (top)
	solAssert(_targetType.location() == DataLocation::Storage, "");

	Type const* targetBaseType = _targetType.baseType();
	Type const* sourceBaseType = _sourceType.baseType();

	// TODO unroll loop for small sizes

	bool sourceIsStorage = _sourceType.location() == DataLocation::Storage;
	bool fromCalldata = _sourceType.location() == DataLocation::CallData;
	bool directCopy = sourceIsStorage && sourceBaseType->isValueType() && *sourceBaseType == *targetBaseType;
	bool haveByteOffsetSource = !directCopy && sourceIsStorage && sourceBaseType->storageBytes() <= 16;
	bool haveByteOffsetTarget = !directCopy && targetBaseType->storageBytes() <= 16;
	unsigned byteOffsetSize = (haveByteOffsetSource ? 1u : 0u) + (haveByteOffsetTarget ? 1u : 0u);

	// stack: source_ref [source_length] target_ref
	// store target_ref
	for (unsigned i = _sourceType.sizeOnStack(); i > 0; --i)
		m_context << swapInstruction(i);
	// stack: target_ref source_ref [source_length]

	if (_targetType.isByteArrayOrString())
	{
		// stack: target_ref source_ref [source_length]
		if (fromCalldata && _sourceType.isDynamicallySized())
		{
			// stack: target_ref source_ref source_length
			m_context << Instruction::SWAP1;
			// stack: target_ref source_length source_ref
			m_context << Instruction::DUP3;
			// stack: target_ref source_length source_ref target_ref
			m_context.callYulFunction(
				m_context.utilFunctions().copyByteArrayToStorageFunction(_sourceType, _targetType),
				3,
				0
			);
		}
		else
		{
			// stack: target_ref source_ref
			m_context << Instruction::DUP2;
			// stack: target_ref source_ref target_ref
			m_context.callYulFunction(
				m_context.utilFunctions().copyByteArrayToStorageFunction(_sourceType, _targetType),
				2,
				0
			);
		}
		// stack: target_ref
		return;
	}

	// retrieve source length
	if (_sourceType.location() != DataLocation::CallData || !_sourceType.isDynamicallySized())
		retrieveLength(_sourceType); // otherwise, length is already there
	if (_sourceType.location() == DataLocation::Memory && _sourceType.isDynamicallySized())
	{
		// increment source pointer to point to data
		m_context << Instruction::SWAP1 << u256(0x20);
		m_context << Instruction::ADD << Instruction::SWAP1;
	}

	// stack: target_ref source_ref source_length
	Type const* targetType = &_targetType;
	Type const* sourceType = &_sourceType;
	m_context.callLowLevelFunction(
		"$copyArrayToStorage_" + sourceType->identifier() + "_to_" + targetType->identifier(),
		3,
		1,
		[=](CompilerContext& _context)
		{
			ArrayUtils utils(_context);
			ArrayType const& _sourceType = dynamic_cast<ArrayType const&>(*sourceType);
			ArrayType const& _targetType = dynamic_cast<ArrayType const&>(*targetType);
			// stack: target_ref source_ref source_length
			_context << Instruction::DUP3;
			// stack: target_ref source_ref source_length target_ref
			utils.retrieveLength(_targetType);
			// stack: target_ref source_ref source_length target_ref target_length
			if (_targetType.isDynamicallySized())
			{
				// store new target length
				solAssert(!_targetType.isByteArrayOrString());
				_context << Instruction::DUP3 << Instruction::DUP3 << Instruction::SSTORE;
			}
			if (sourceBaseType->category() == Type::Category::Mapping)
			{
				solAssert(targetBaseType->category() == Type::Category::Mapping, "");
				solAssert(_sourceType.location() == DataLocation::Storage, "");
				// nothing to copy
				_context
					<< Instruction::POP << Instruction::POP
					<< Instruction::POP << Instruction::POP;
				return;
			}
			// stack: target_ref source_ref source_length target_ref target_length
			// compute hashes (data positions)
			_context << Instruction::SWAP1;
			if (_targetType.isDynamicallySized())
				CompilerUtils(_context).computeHashStatic();
			// stack: target_ref source_ref source_length target_length target_data_pos
			_context << Instruction::SWAP1;
			utils.convertLengthToSize(_targetType);
			_context << Instruction::DUP2 << Instruction::ADD;
			// stack: target_ref source_ref source_length target_data_pos target_data_end
			_context << Instruction::SWAP3;
			// stack: target_ref target_data_end source_length target_data_pos source_ref

			evmasm::AssemblyItem copyLoopEndWithoutByteOffset = _context.newTag();
			solAssert(!_targetType.isByteArrayOrString());
			// skip copying if source length is zero
			_context << Instruction::DUP3 << Instruction::ISZERO;
			_context.appendConditionalJumpTo(copyLoopEndWithoutByteOffset);

			if (_sourceType.location() == DataLocation::Storage && _sourceType.isDynamicallySized())
				CompilerUtils(_context).computeHashStatic();
			// stack: target_ref target_data_end source_length target_data_pos source_data_pos
			_context << Instruction::SWAP2;
			utils.convertLengthToSize(_sourceType);
			_context << Instruction::DUP3 << Instruction::ADD;
			// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end
			if (haveByteOffsetTarget)
				_context << u256(0);
			if (haveByteOffsetSource)
				_context << u256(0);
			// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
			evmasm::AssemblyItem copyLoopStart = _context.newTag();
			_context << copyLoopStart;
			// check for loop condition
			_context
				<< dupInstruction(3 + byteOffsetSize) << dupInstruction(2 + byteOffsetSize)
				<< Instruction::GT << Instruction::ISZERO;
			evmasm::AssemblyItem copyLoopEnd = _context.appendConditionalJump();
			// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
			// copy
			if (sourceBaseType->category() == Type::Category::Array)
			{
				solAssert(byteOffsetSize == 0, "Byte offset for array as base type.");
				auto const& sourceBaseArrayType = dynamic_cast<ArrayType const&>(*sourceBaseType);

				solUnimplementedAssert(
					_sourceType.location() != DataLocation::CallData ||
					!_sourceType.isDynamicallyEncoded() ||
					!sourceBaseArrayType.isDynamicallySized(),
					"Copying nested calldata dynamic arrays to storage is not implemented in the old code generator."
				);
				_context << Instruction::DUP3;
				if (sourceBaseArrayType.location() == DataLocation::Memory)
					_context << Instruction::MLOAD;
				_context << Instruction::DUP3;
				utils.copyArrayToStorage(dynamic_cast<ArrayType const&>(*targetBaseType), sourceBaseArrayType);
				_context << Instruction::POP;
			}
			else if (directCopy)
			{
				solAssert(byteOffsetSize == 0, "Byte offset for direct copy.");
				_context
					<< Instruction::DUP3 << Instruction::SLOAD
					<< Instruction::DUP3 << Instruction::SSTORE;
			}
			else
			{
				// Note that we have to copy each element on its own in case conversion is involved.
				// We might copy too much if there is padding at the last element, but this way end
				// checking is easier.
				// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
				_context << dupInstruction(3 + byteOffsetSize);
				if (_sourceType.location() == DataLocation::Storage)
				{
					if (haveByteOffsetSource)
						_context << Instruction::DUP2;
					else
						_context << u256(0);
					StorageItem(_context, *sourceBaseType).retrieveValue(SourceLocation(), true);
				}
				else if (sourceBaseType->isValueType())
					CompilerUtils(_context).loadFromMemoryDynamic(*sourceBaseType, fromCalldata, true, false);
				else
					solUnimplemented("Copying of type " + _sourceType.toString(false) + " to storage is not supported in legacy (only supported by the IR pipeline). Hint: try compiling with `--via-ir` (CLI) or the equivalent `viaIR: true` (Standard JSON)");
				// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset] <source_value>...
				assertThrow(
					2 + byteOffsetSize + sourceBaseType->sizeOnStack() <= 16,
					StackTooDeepError,
					util::stackTooDeepString
				);
				// fetch target storage reference
				_context << dupInstruction(2 + byteOffsetSize + sourceBaseType->sizeOnStack());
				if (haveByteOffsetTarget)
					_context << dupInstruction(1 + byteOffsetSize + sourceBaseType->sizeOnStack());
				else
					_context << u256(0);
				StorageItem(_context, *targetBaseType).storeValue(*sourceBaseType, SourceLocation(), true);
			}
			// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
			// increment source
			if (haveByteOffsetSource)
				utils.incrementByteOffset(sourceBaseType->storageBytes(), 1, haveByteOffsetTarget ? 5 : 4);
			else
			{
				_context << swapInstruction(2 + byteOffsetSize);
				if (sourceIsStorage)
					_context << sourceBaseType->storageSize();
				else if (_sourceType.location() == DataLocation::Memory)
					_context << sourceBaseType->memoryHeadSize();
				else
					_context << sourceBaseType->calldataHeadSize();
				_context
					<< Instruction::ADD
					<< swapInstruction(2 + byteOffsetSize);
			}
			// increment target
			if (haveByteOffsetTarget)
				utils.incrementByteOffset(targetBaseType->storageBytes(), byteOffsetSize, byteOffsetSize + 2);
			else
				_context
					<< swapInstruction(1 + byteOffsetSize)
					<< targetBaseType->storageSize()
					<< Instruction::ADD
					<< swapInstruction(1 + byteOffsetSize);
			_context.appendJumpTo(copyLoopStart);
			_context << copyLoopEnd;
			if (haveByteOffsetTarget)
			{
				// clear elements that might be left over in the current slot in target
				// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end target_byte_offset [source_byte_offset]
				_context << dupInstruction(byteOffsetSize) << Instruction::ISZERO;
				evmasm::AssemblyItem copyCleanupLoopEnd = _context.appendConditionalJump();
				_context << dupInstruction(2 + byteOffsetSize) << dupInstruction(1 + byteOffsetSize);
				StorageItem(_context, *targetBaseType).setToZero(SourceLocation(), true);
				utils.incrementByteOffset(targetBaseType->storageBytes(), byteOffsetSize, byteOffsetSize + 2);
				_context.appendJumpTo(copyLoopEnd);

				_context << copyCleanupLoopEnd;
				_context << Instruction::POP; // might pop the source, but then target is popped next
			}
			if (haveByteOffsetSource)
				_context << Instruction::POP;
			_context << copyLoopEndWithoutByteOffset;

			// zero-out leftovers in target
			// stack: target_ref target_data_end source_data_pos target_data_pos_updated source_data_end
			_context << Instruction::POP << Instruction::SWAP1 << Instruction::POP;
			// stack: target_ref target_data_end target_data_pos_updated
			if (targetBaseType->storageBytes() < 32)
				utils.clearStorageLoop(TypeProvider::uint256());
			else
				utils.clearStorageLoop(targetBaseType);
			_context << Instruction::POP;
		}
	);
}

void ArrayUtils::copyArrayToMemory(ArrayType const& _sourceType, bool _padToWordBoundaries) const
{
	solUnimplementedAssert(
		!_sourceType.baseType()->isDynamicallySized(),
		"Nested dynamic arrays not implemented here."
	);
	CompilerUtils utils(m_context);

	if (_sourceType.location() == DataLocation::CallData)
	{
		if (!_sourceType.isDynamicallySized())
			m_context << _sourceType.length();
		if (!_sourceType.isByteArrayOrString())
			convertLengthToSize(_sourceType);

		std::string routine = "calldatacopy(target, source, len)\n";
		if (_padToWordBoundaries)
			routine += R"(
				// Set padding suffix to zero
				mstore(add(target, len), 0)
				len := and(add(len, 0x1f), not(0x1f))
			)";
		routine += "target := add(target, len)\n";
		m_context.appendInlineAssembly("{" + routine + "}", {"target", "source", "len"});
		m_context << Instruction::POP << Instruction::POP;
	}
	else if (_sourceType.location() == DataLocation::Memory)
	{
		retrieveLength(_sourceType);
		// stack: target source length
		if (!_sourceType.baseType()->isValueType())
		{
			// copy using a loop
			m_context << u256(0) << Instruction::SWAP3;
			// stack: counter source length target
			auto repeat = m_context.newTag();
			m_context << repeat;
			m_context << Instruction::DUP2 << Instruction::DUP5;
			m_context << Instruction::LT << Instruction::ISZERO;
			auto loopEnd = m_context.appendConditionalJump();
			m_context << Instruction::DUP3 << Instruction::DUP5;
			accessIndex(_sourceType, false);
			MemoryItem(m_context, *_sourceType.baseType(), true).retrieveValue(SourceLocation(), true);
			if (auto baseArray = dynamic_cast<ArrayType const*>(_sourceType.baseType()))
				copyArrayToMemory(*baseArray, _padToWordBoundaries);
			else
				utils.storeInMemoryDynamic(*_sourceType.baseType());
			m_context << Instruction::SWAP3 << u256(1) << Instruction::ADD;
			m_context << Instruction::SWAP3;
			m_context.appendJumpTo(repeat);
			m_context << loopEnd;
			m_context << Instruction::SWAP3;
			utils.popStackSlots(3);
			// stack: updated_target_pos
			return;
		}

		// memcpy using the built-in contract
		if (_sourceType.isDynamicallySized())
		{
			// change pointer to data part
			m_context << Instruction::SWAP1 << u256(32) << Instruction::ADD;
			m_context << Instruction::SWAP1;
		}
		if (!_sourceType.isByteArrayOrString())
			convertLengthToSize(_sourceType);
		// stack: <target> <source> <size>
		m_context << Instruction::DUP1 << Instruction::DUP4 << Instruction::DUP4;
		// We can resort to copying full 32 bytes only if
		// - the length is known to be a multiple of 32 or
		// - we will pad to full 32 bytes later anyway.
		if (!_sourceType.isByteArrayOrString() || _padToWordBoundaries)
			utils.memoryCopy32();
		else
			utils.memoryCopy();

		m_context << Instruction::SWAP1 << Instruction::POP;
		// stack: <target> <size>

		bool paddingNeeded = _padToWordBoundaries && _sourceType.isByteArrayOrString();

		if (paddingNeeded)
		{
			// stack: <target> <size>
			m_context << Instruction::SWAP1 << Instruction::DUP2 << Instruction::ADD;
			// stack: <length> <target + size>
			m_context << Instruction::SWAP1 << u256(31) << Instruction::AND;
			// stack: <target + size> <remainder = size % 32>
			evmasm::AssemblyItem skip = m_context.newTag();
			if (_sourceType.isDynamicallySized())
			{
				m_context << Instruction::DUP1 << Instruction::ISZERO;
				m_context.appendConditionalJumpTo(skip);
			}
			// round off, load from there.
			// stack <target + size> <remainder = size % 32>
			m_context << Instruction::DUP1 << Instruction::DUP3;
			m_context << Instruction::SUB;
			// stack: target+size remainder <target + size - remainder>
			m_context << Instruction::DUP1 << Instruction::MLOAD;
			// Now we AND it with ~(2**(8 * (32 - remainder)) - 1)
			m_context << u256(1);
			m_context << Instruction::DUP4 << u256(32) << Instruction::SUB;
			// stack: ...<v> 1 <32 - remainder>
			m_context << u256(0x100) << Instruction::EXP << Instruction::SUB;
			m_context << Instruction::NOT << Instruction::AND;
			// stack: target+size remainder target+size-remainder <v & ...>
			m_context << Instruction::DUP2 << Instruction::MSTORE;
			// stack: target+size remainder target+size-remainder
			m_context << u256(32) << Instruction::ADD;
			// stack: target+size remainder <new_padded_end>
			m_context << Instruction::SWAP2 << Instruction::POP;

			if (_sourceType.isDynamicallySized())
				m_context << skip.tag();
			// stack <target + "size"> <remainder = size % 32>
			m_context << Instruction::POP;
		}
		else
			// stack: <target> <size>
			m_context << Instruction::ADD;
	}
	else
	{
		solAssert(_sourceType.location() == DataLocation::Storage, "");
		unsigned storageBytes = _sourceType.baseType()->storageBytes();
		u256 storageSize = _sourceType.baseType()->storageSize();
		solAssert(storageSize > 1 || (storageSize == 1 && storageBytes > 0), "");

		retrieveLength(_sourceType);
		// stack here: memory_offset storage_offset length
		// jump to end if length is zero
		m_context << Instruction::DUP1 << Instruction::ISZERO;
		evmasm::AssemblyItem loopEnd = m_context.appendConditionalJump();
		// Special case for tightly-stored byte arrays
		if (_sourceType.isByteArrayOrString())
		{
			// stack here: memory_offset storage_offset length
			m_context << Instruction::DUP1 << u256(31) << Instruction::LT;
			evmasm::AssemblyItem longByteArray = m_context.appendConditionalJump();
			// store the short byte array (discard lower-order byte)
			m_context << u256(0x100) << Instruction::DUP1;
			m_context << Instruction::DUP4 << Instruction::SLOAD;
			m_context << Instruction::DIV << Instruction::MUL;
			m_context << Instruction::DUP4 << Instruction::MSTORE;
			// stack here: memory_offset storage_offset length
			// add 32 or length to memory offset
			m_context << Instruction::SWAP2;
			if (_padToWordBoundaries)
				m_context << u256(32);
			else
				m_context << Instruction::DUP3;
			m_context << Instruction::ADD;
			m_context << Instruction::SWAP2;
			m_context.appendJumpTo(loopEnd);
			m_context << longByteArray;
		}
		else
			// convert length to memory size
			m_context << _sourceType.baseType()->memoryHeadSize() << Instruction::MUL;

		m_context << Instruction::DUP3 << Instruction::ADD << Instruction::SWAP2;
		if (_sourceType.isDynamicallySized())
		{
			// actual array data is stored at KECCAK256(storage_offset)
			m_context << Instruction::SWAP1;
			utils.computeHashStatic();
			m_context << Instruction::SWAP1;
		}

		// stack here: memory_end_offset storage_data_offset memory_offset
		bool haveByteOffset = !_sourceType.isByteArrayOrString() && storageBytes <= 16;
		if (haveByteOffset)
			m_context << u256(0) << Instruction::SWAP1;
		// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
		evmasm::AssemblyItem loopStart = m_context.newTag();
		m_context << loopStart;
		// load and store
		if (_sourceType.isByteArrayOrString())
		{
			// Packed both in storage and memory.
			m_context << Instruction::DUP2 << Instruction::SLOAD;
			m_context << Instruction::DUP2 << Instruction::MSTORE;
			// increment storage_data_offset by 1
			m_context << Instruction::SWAP1 << u256(1) << Instruction::ADD;
			// increment memory offset by 32
			m_context << Instruction::SWAP1 << u256(32) << Instruction::ADD;
		}
		else
		{
			// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
			if (haveByteOffset)
				m_context << Instruction::DUP3 << Instruction::DUP3;
			else
				m_context << Instruction::DUP2 << u256(0);
			StorageItem(m_context, *_sourceType.baseType()).retrieveValue(SourceLocation(), true);
			if (auto baseArray = dynamic_cast<ArrayType const*>(_sourceType.baseType()))
				copyArrayToMemory(*baseArray, _padToWordBoundaries);
			else
				utils.storeInMemoryDynamic(*_sourceType.baseType());
			// increment storage_data_offset and byte offset
			if (haveByteOffset)
				incrementByteOffset(storageBytes, 2, 3);
			else
			{
				m_context << Instruction::SWAP1;
				m_context << storageSize << Instruction::ADD;
				m_context << Instruction::SWAP1;
			}
		}
		// check for loop condition
		m_context << Instruction::DUP1 << dupInstruction(haveByteOffset ? 5 : 4);
		m_context << Instruction::GT;
		m_context.appendConditionalJumpTo(loopStart);
		// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
		if (haveByteOffset)
			m_context << Instruction::SWAP1 << Instruction::POP;
		if (!_sourceType.isByteArrayOrString())
		{
			solAssert(_sourceType.calldataStride() % 32 == 0, "");
			solAssert(_sourceType.memoryStride() % 32 == 0, "");
		}
		if (_padToWordBoundaries && _sourceType.isByteArrayOrString())
		{
			// memory_end_offset - start is the actual length (we want to compute the ceil of).
			// memory_offset - start is its next multiple of 32, but it might be off by 32.
			// so we compute: memory_end_offset += (memory_offset - memory_end_offset) & 31
			m_context << Instruction::DUP3 << Instruction::SWAP1 << Instruction::SUB;
			m_context << u256(31) << Instruction::AND;
			m_context << Instruction::DUP3 << Instruction::ADD;
			m_context << Instruction::SWAP2;
		}
		m_context << loopEnd << Instruction::POP << Instruction::POP;
	}
}

void ArrayUtils::clearArray(ArrayType const& _typeIn) const
{
	Type const* type = &_typeIn;
	m_context.callLowLevelFunction(
		"$clearArray_" + _typeIn.identifier(),
		2,
		0,
		[type](CompilerContext& _context)
		{
			ArrayType const& _type = dynamic_cast<ArrayType const&>(*type);
			unsigned stackHeightStart = _context.stackHeight();
			solAssert(_type.location() == DataLocation::Storage, "");
			if (_type.baseType()->storageBytes() < 32)
			{
				solAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");
				solAssert(_type.baseType()->storageSize() <= 1, "Invalid storage size for type.");
			}
			if (_type.baseType()->isValueType())
				solAssert(_type.baseType()->storageSize() <= 1, "Invalid size for value type.");

			_context << Instruction::POP; // remove byte offset
			if (_type.isDynamicallySized())
				ArrayUtils(_context).clearDynamicArray(_type);
			else if (_type.length() == 0 || _type.baseType()->category() == Type::Category::Mapping)
				_context << Instruction::POP;
			else if (_type.baseType()->isValueType() && _type.storageSize() <= 5)
			{
				// unroll loop for small arrays @todo choose a good value
				// Note that we loop over storage slots here, not elements.
				for (unsigned i = 1; i < _type.storageSize(); ++i)
					_context
						<< u256(0) << Instruction::DUP2 << Instruction::SSTORE
						<< u256(1) << Instruction::ADD;
				_context << u256(0) << Instruction::SWAP1 << Instruction::SSTORE;
			}
			else if (!_type.baseType()->isValueType() && _type.length() <= 4)
			{
				// unroll loop for small arrays @todo choose a good value
				solAssert(_type.baseType()->storageBytes() >= 32, "Invalid storage size.");
				for (unsigned i = 1; i < _type.length(); ++i)
				{
					_context << u256(0);
					StorageItem(_context, *_type.baseType()).setToZero(SourceLocation(), false);
					_context
						<< Instruction::POP
						<< u256(_type.baseType()->storageSize()) << Instruction::ADD;
				}
				_context << u256(0);
				StorageItem(_context, *_type.baseType()).setToZero(SourceLocation(), true);
			}
			else
			{
				_context << Instruction::DUP1 << _type.length();
				ArrayUtils(_context).convertLengthToSize(_type);
				_context << Instruction::ADD << Instruction::SWAP1;
				if (_type.baseType()->storageBytes() < 32)
					ArrayUtils(_context).clearStorageLoop(TypeProvider::uint256());
				else
					ArrayUtils(_context).clearStorageLoop(_type.baseType());
				_context << Instruction::POP;
			}
			solAssert(_context.stackHeight() == stackHeightStart - 2, "");
		}
	);
}

void ArrayUtils::clearDynamicArray(ArrayType const& _type) const
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");

	// fetch length
	retrieveLength(_type);
	// set length to zero
	m_context << u256(0) << Instruction::DUP3 << Instruction::SSTORE;
	// Special case: short byte arrays are stored togeher with their length
	evmasm::AssemblyItem endTag = m_context.newTag();
	if (_type.isByteArrayOrString())
	{
		// stack: ref old_length
		m_context << Instruction::DUP1 << u256(31) << Instruction::LT;
		evmasm::AssemblyItem longByteArray = m_context.appendConditionalJump();
		m_context << Instruction::POP;
		m_context.appendJumpTo(endTag);
		m_context.adjustStackOffset(1); // needed because of jump
		m_context << longByteArray;
	}
	// stack: ref old_length
	convertLengthToSize(_type);
	// compute data positions
	m_context << Instruction::SWAP1;
	CompilerUtils(m_context).computeHashStatic();
	// stack: len data_pos
	m_context << Instruction::SWAP1 << Instruction::DUP2 << Instruction::ADD
		<< Instruction::SWAP1;
	// stack: data_pos_end data_pos
	if (_type.storageStride() < 32)
		clearStorageLoop(TypeProvider::uint256());
	else
		clearStorageLoop(_type.baseType());
	// cleanup
	m_context << endTag;
	m_context << Instruction::POP;
}

void ArrayUtils::resizeDynamicArray(ArrayType const& _typeIn) const
{
	Type const* type = &_typeIn;
	m_context.callLowLevelFunction(
		"$resizeDynamicArray_" + _typeIn.identifier(),
		2,
		0,
		[type](CompilerContext& _context)
		{
			ArrayType const& _type = dynamic_cast<ArrayType const&>(*type);
			solAssert(_type.location() == DataLocation::Storage, "");
			solAssert(_type.isDynamicallySized(), "");
			if (!_type.isByteArrayOrString() && _type.baseType()->storageBytes() < 32)
				solAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");

			unsigned stackHeightStart = _context.stackHeight();
			evmasm::AssemblyItem resizeEnd = _context.newTag();

			// stack: ref new_length
			// fetch old length
			ArrayUtils(_context).retrieveLength(_type, 1);
			// stack: ref new_length old_length
			solAssert(_context.stackHeight() - stackHeightStart == 3 - 2, "2");

			// Special case for short byte arrays, they are stored together with their length
			if (_type.isByteArrayOrString())
			{
				evmasm::AssemblyItem regularPath = _context.newTag();
				// We start by a large case-distinction about the old and new length of the byte array.

				_context << Instruction::DUP3 << Instruction::SLOAD;
				// stack: ref new_length current_length ref_value

				solAssert(_context.stackHeight() - stackHeightStart == 4 - 2, "3");
				_context << Instruction::DUP2 << u256(31) << Instruction::LT;
				evmasm::AssemblyItem currentIsLong = _context.appendConditionalJump();
				_context << Instruction::DUP3 << u256(31) << Instruction::LT;
				evmasm::AssemblyItem newIsLong = _context.appendConditionalJump();

				// Here: short -> short

				// Compute 1 << (256 - 8 * new_size)
				evmasm::AssemblyItem shortToShort = _context.newTag();
				_context << shortToShort;
				_context << Instruction::DUP3 << u256(8) << Instruction::MUL;
				_context << u256(0x100) << Instruction::SUB;
				_context << u256(2) << Instruction::EXP;
				// Divide and multiply by that value, clearing bits.
				_context << Instruction::DUP1 << Instruction::SWAP2;
				_context << Instruction::DIV << Instruction::MUL;
				// Insert 2*length.
				_context << Instruction::DUP3 << Instruction::DUP1 << Instruction::ADD;
				_context << Instruction::OR;
				// Store.
				_context << Instruction::DUP4 << Instruction::SSTORE;
				solAssert(_context.stackHeight() - stackHeightStart == 3 - 2, "3");
				_context.appendJumpTo(resizeEnd);

				_context.adjustStackOffset(1); // we have to do that because of the jumps
				// Here: short -> long

				_context << newIsLong;
				// stack: ref new_length current_length ref_value
				solAssert(_context.stackHeight() - stackHeightStart == 4 - 2, "3");
				// Zero out lower-order byte.
				_context << u256(0xff) << Instruction::NOT << Instruction::AND;
				// Store at data location.
				_context << Instruction::DUP4;
				CompilerUtils(_context).computeHashStatic();
				_context << Instruction::SSTORE;
				// stack: ref new_length current_length
				// Store new length: Compule 2*length + 1 and store it.
				_context << Instruction::DUP2 << Instruction::DUP1 << Instruction::ADD;
				_context << u256(1) << Instruction::ADD;
				// stack: ref new_length current_length 2*new_length+1
				_context << Instruction::DUP4 << Instruction::SSTORE;
				solAssert(_context.stackHeight() - stackHeightStart == 3 - 2, "3");
				_context.appendJumpTo(resizeEnd);

				_context.adjustStackOffset(1); // we have to do that because of the jumps

				_context << currentIsLong;
				_context << Instruction::DUP3 << u256(31) << Instruction::LT;
				_context.appendConditionalJumpTo(regularPath);

				// Here: long -> short
				// Read the first word of the data and store it on the stack. Clear the data location and
				// then jump to the short -> short case.

				// stack: ref new_length current_length ref_value
				solAssert(_context.stackHeight() - stackHeightStart == 4 - 2, "3");
				_context << Instruction::POP << Instruction::DUP3;
				CompilerUtils(_context).computeHashStatic();
				_context << Instruction::DUP1 << Instruction::SLOAD << Instruction::SWAP1;
				// stack: ref new_length current_length first_word data_location
				_context << Instruction::DUP3;
				ArrayUtils(_context).convertLengthToSize(_type);
				_context << Instruction::DUP2 << Instruction::ADD << Instruction::SWAP1;
				// stack: ref new_length current_length first_word data_location_end data_location
				ArrayUtils(_context).clearStorageLoop(TypeProvider::uint256());
				_context << Instruction::POP;
				// stack: ref new_length current_length first_word
				solAssert(_context.stackHeight() - stackHeightStart == 4 - 2, "3");
				_context.appendJumpTo(shortToShort);

				_context << regularPath;
				// stack: ref new_length current_length ref_value
				_context << Instruction::POP;
			}

			// Change of length for a regular array (i.e. length at location, data at KECCAK256(location)).
			// stack: ref new_length old_length
			// store new length
			_context << Instruction::DUP2;
			if (_type.isByteArrayOrString())
				// For a "long" byte array, store length as 2*length+1
				_context << Instruction::DUP1 << Instruction::ADD << u256(1) << Instruction::ADD;
			_context << Instruction::DUP4 << Instruction::SSTORE;
			// skip if size is not reduced
			_context << Instruction::DUP2 << Instruction::DUP2
				<< Instruction::GT << Instruction::ISZERO;
			_context.appendConditionalJumpTo(resizeEnd);

			// size reduced, clear the end of the array
			// stack: ref new_length old_length
			ArrayUtils(_context).convertLengthToSize(_type);
			_context << Instruction::DUP2;
			ArrayUtils(_context).convertLengthToSize(_type);
			// stack: ref new_length old_size new_size
			// compute data positions
			_context << Instruction::DUP4;
			CompilerUtils(_context).computeHashStatic();
			// stack: ref new_length old_size new_size data_pos
			_context << Instruction::SWAP2 << Instruction::DUP3 << Instruction::ADD;
			// stack: ref new_length data_pos new_size delete_end
			_context << Instruction::SWAP2 << Instruction::ADD;
			// stack: ref new_length delete_end delete_start
			if (_type.storageStride() < 32)
				ArrayUtils(_context).clearStorageLoop(TypeProvider::uint256());
			else
				ArrayUtils(_context).clearStorageLoop(_type.baseType());

			_context << resizeEnd;
			// cleanup
			_context << Instruction::POP << Instruction::POP << Instruction::POP;
			solAssert(_context.stackHeight() == stackHeightStart - 2, "");
		}
	);
}

void ArrayUtils::incrementDynamicArraySize(ArrayType const& _type) const
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	if (!_type.isByteArrayOrString() && _type.baseType()->storageBytes() < 32)
		solAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");

	if (_type.isByteArrayOrString())
	{
		// We almost always just add 2 (length of byte arrays is shifted left by one)
		// except for the case where we transition from a short byte array
		// to a long byte array, there we have to copy.
		// This happens if the length is exactly 31, which means that the
		// lowest-order byte (we actually use a mask with fewer bits) must
		// be (31*2+0) = 62

		m_context << Instruction::DUP1 << Instruction::SLOAD << Instruction::DUP1;
		m_context.callYulFunction(m_context.utilFunctions().extractByteArrayLengthFunction(), 1, 1);
		m_context.appendInlineAssembly(R"({
			// We have to copy if length is exactly 31, because that marks
			// the transition between in-place and out-of-place storage.
			switch length
			case 31
			{
				mstore(0, ref)
				let data_area := keccak256(0, 0x20)
				sstore(data_area, and(data, not(0xff)))
				// Set old length in new format (31 * 2 + 1)
				data := 63
			}
			sstore(ref, add(data, 2))
			// return new length in ref
			ref := add(length, 1)
		})", {"ref", "data", "length"});
		m_context << Instruction::POP << Instruction::POP;
	}
	else
		m_context.appendInlineAssembly(R"({
			let new_length := add(sload(ref), 1)
			sstore(ref, new_length)
			ref := new_length
		})", {"ref"});
}

void ArrayUtils::popStorageArrayElement(ArrayType const& _type) const
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	if (!_type.isByteArrayOrString() && _type.baseType()->storageBytes() < 32)
		solAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");

	if (_type.isByteArrayOrString())
	{
		m_context << Instruction::DUP1 << Instruction::SLOAD << Instruction::DUP1;
		m_context.callYulFunction(m_context.utilFunctions().extractByteArrayLengthFunction(), 1, 1);
		util::Whiskers code(R"({
			if iszero(length) {
				mstore(0, <panicSelector>)
				mstore(4, <emptyArrayPop>)
				revert(0, 0x24)
			}
			switch gt(length, 31)
			case 0 {
				// short byte array
				// Zero-out the suffix including the least significant byte.
				let mask := sub(exp(0x100, sub(33, length)), 1)
				length := sub(length, 1)
				slot_value := or(and(not(mask), slot_value), mul(length, 2))
			}
			case 1 {
				// long byte array
				mstore(0, ref)
				let slot := keccak256(0, 0x20)
				switch length
				case 32
				{
					let data := sload(slot)
					sstore(slot, 0)
					data := and(data, not(0xff))
					slot_value := or(data, 62)
				}
				default
				{
					let offset_inside_slot := and(sub(length, 1), 0x1f)
					slot := add(slot, div(sub(length, 1), 32))
					let data := sload(slot)

					// Zero-out the suffix of the byte array by masking it.
					// ((1<<(8 * (32 - offset))) - 1)
					let mask := sub(exp(0x100, sub(32, offset_inside_slot)), 1)
					data := and(not(mask), data)
					sstore(slot, data)

					// Reduce the length by 1
					slot_value := sub(slot_value, 2)
				}
			}
			sstore(ref, slot_value)
		})");
		code("panicSelector", util::selectorFromSignatureU256("Panic(uint256)").str());
		code("emptyArrayPop", std::to_string(unsigned(util::PanicCode::EmptyArrayPop)));
		m_context.appendInlineAssembly(code.render(), {"ref", "slot_value", "length"});
		m_context << Instruction::POP << Instruction::POP << Instruction::POP;
	}
	else
	{
		// stack: ArrayReference
		retrieveLength(_type);
		// stack: ArrayReference oldLength
		m_context << Instruction::DUP1;
		// stack: ArrayReference oldLength oldLength
		m_context << Instruction::ISZERO;
		m_context.appendConditionalPanic(util::PanicCode::EmptyArrayPop);

		// Stack: ArrayReference oldLength
		m_context << u256(1) << Instruction::SWAP1 << Instruction::SUB;
		// Stack ArrayReference newLength

		if (_type.baseType()->category() != Type::Category::Mapping)
		{
			m_context << Instruction::DUP2 << Instruction::DUP2;
			// Stack ArrayReference newLength ArrayReference newLength;
			accessIndex(_type, false);
			// Stack: ArrayReference newLength storage_slot byte_offset
			StorageItem(m_context, *_type.baseType()).setToZero(SourceLocation(), true);
		}

		// Stack: ArrayReference newLength
		m_context << Instruction::SWAP1 << Instruction::SSTORE;
	}
}

void ArrayUtils::clearStorageLoop(Type const* _type) const
{
	solAssert(_type->storageBytes() >= 32, "");
	m_context.callLowLevelFunction(
		"$clearStorageLoop_" + _type->identifier(),
		2,
		1,
		[_type](CompilerContext& _context)
		{
			unsigned stackHeightStart = _context.stackHeight();
			if (_type->category() == Type::Category::Mapping)
			{
				_context << Instruction::POP;
				return;
			}
			// stack: end_pos pos

			evmasm::AssemblyItem loopStart = _context.appendJumpToNew();
			_context << loopStart;
			// check for loop condition
			_context <<
				Instruction::DUP1 <<
				Instruction::DUP3 <<
				Instruction::GT <<
				Instruction::ISZERO;
			evmasm::AssemblyItem zeroLoopEnd = _context.newTag();
			_context.appendConditionalJumpTo(zeroLoopEnd);
			// delete
			_context << u256(0);
			StorageItem(_context, *_type).setToZero(SourceLocation(), false);
			_context << Instruction::POP;
			// increment
			_context << _type->storageSize() << Instruction::ADD;
			_context.appendJumpTo(loopStart);
			// cleanup
			_context << zeroLoopEnd;
			_context << Instruction::POP;

			solAssert(_context.stackHeight() == stackHeightStart - 1, "");
		}
	);
}

void ArrayUtils::convertLengthToSize(ArrayType const& _arrayType, bool _pad) const
{
	if (_arrayType.location() == DataLocation::Storage)
	{
		if (_arrayType.baseType()->storageSize() <= 1)
		{
			unsigned baseBytes = _arrayType.baseType()->storageBytes();
			if (baseBytes == 0)
				m_context << Instruction::POP << u256(1);
			else if (baseBytes <= 16)
			{
				unsigned itemsPerSlot = 32 / baseBytes;
				m_context
					<< u256(itemsPerSlot - 1) << Instruction::ADD
					<< u256(itemsPerSlot) << Instruction::SWAP1 << Instruction::DIV;
			}
		}
		else
			m_context << _arrayType.baseType()->storageSize() << Instruction::MUL;
	}
	else
	{
		if (!_arrayType.isByteArrayOrString())
		{
			if (_arrayType.location() == DataLocation::Memory)
				m_context << _arrayType.memoryStride();
			else
				m_context << _arrayType.calldataStride();
			m_context << Instruction::MUL;
		}
		else if (_pad)
			m_context << u256(31) << Instruction::ADD
				<< u256(32) << Instruction::DUP1
				<< Instruction::SWAP2 << Instruction::DIV << Instruction::MUL;
	}
}

void ArrayUtils::retrieveLength(ArrayType const& _arrayType, unsigned _stackDepth) const
{
	if (!_arrayType.isDynamicallySized())
		m_context << _arrayType.length();
	else
	{
		m_context << dupInstruction(1 + _stackDepth);
		switch (_arrayType.location())
		{
		case DataLocation::CallData:
			// length is stored on the stack
			break;
		case DataLocation::Memory:
			m_context << Instruction::MLOAD;
			break;
		case DataLocation::Storage:
			m_context << Instruction::SLOAD;
			if (_arrayType.isByteArrayOrString())
				m_context.callYulFunction(m_context.utilFunctions().extractByteArrayLengthFunction(), 1, 1);
			break;
		case DataLocation::Transient:
			solUnimplemented("Transient data location is only supported for value types.");
			break;
		}
	}
}

void ArrayUtils::accessIndex(ArrayType const& _arrayType, bool _doBoundsCheck, bool _keepReference) const
{
	/// Stack: reference [length] index
	DataLocation location = _arrayType.location();

	if (_doBoundsCheck)
	{
		// retrieve length
		ArrayUtils::retrieveLength(_arrayType, 1);
		// Stack: ref [length] index length
		// check out-of-bounds access
		m_context << Instruction::DUP2 << Instruction::LT << Instruction::ISZERO;
		// out-of-bounds access throws exception
		m_context.appendConditionalPanic(util::PanicCode::ArrayOutOfBounds);
	}
	if (location == DataLocation::CallData && _arrayType.isDynamicallySized())
		// remove length if present
		m_context << Instruction::SWAP1 << Instruction::POP;

	// stack: <base_ref> <index>
	switch (location)
	{
	case DataLocation::Memory:
		// stack: <base_ref> <index>
		if (!_arrayType.isByteArrayOrString())
			m_context << u256(_arrayType.memoryHeadSize()) << Instruction::MUL;
		if (_arrayType.isDynamicallySized())
			m_context << u256(32) << Instruction::ADD;
		if (_keepReference)
			m_context << Instruction::DUP2;
		m_context << Instruction::ADD;
		break;
	case DataLocation::CallData:
		if (!_arrayType.isByteArrayOrString())
		{
			m_context << _arrayType.calldataStride();
			m_context << Instruction::MUL;
		}
		// stack: <base_ref> <index * size>
		if (_keepReference)
			m_context << Instruction::DUP2;
		m_context << Instruction::ADD;
		break;
	case DataLocation::Storage:
	{
		if (_keepReference)
			m_context << Instruction::DUP2;
		else
			m_context << Instruction::SWAP1;
		// stack: [<base_ref>] <index> <base_ref>

		evmasm::AssemblyItem endTag = m_context.newTag();
		if (_arrayType.isByteArrayOrString())
		{
			// Special case of short byte arrays.
			m_context << Instruction::SWAP1;
			m_context << Instruction::DUP2 << Instruction::SLOAD;
			m_context << u256(1) << Instruction::AND << Instruction::ISZERO;
			// No action needed for short byte arrays.
			m_context.appendConditionalJumpTo(endTag);
			m_context << Instruction::SWAP1;
		}
		if (_arrayType.isDynamicallySized())
			CompilerUtils(m_context).computeHashStatic();
		m_context << Instruction::SWAP1;
		if (_arrayType.baseType()->storageBytes() <= 16)
		{
			// stack: <data_ref> <index>
			// goal:
			// <ref> <byte_number> = <base_ref + index / itemsPerSlot> <(index % itemsPerSlot) * byteSize>
			unsigned byteSize = _arrayType.baseType()->storageBytes();
			solAssert(byteSize != 0, "");
			unsigned itemsPerSlot = 32 / byteSize;
			m_context << u256(itemsPerSlot) << Instruction::SWAP2;
			// stack: itemsPerSlot index data_ref
			m_context
				<< Instruction::DUP3 << Instruction::DUP3
				<< Instruction::DIV << Instruction::ADD
			// stack: itemsPerSlot index (data_ref + index / itemsPerSlot)
				<< Instruction::SWAP2 << Instruction::SWAP1
				<< Instruction::MOD;
			if (byteSize != 1)
				m_context << u256(byteSize) << Instruction::MUL;
		}
		else
		{
			if (_arrayType.baseType()->storageSize() != 1)
				m_context << _arrayType.baseType()->storageSize() << Instruction::MUL;
			m_context << Instruction::ADD << u256(0);
		}
		m_context << endTag;
		break;
	}
	case DataLocation::Transient:
		solUnimplemented("Transient data location is only supported for value types.");
		break;
	}
}

void ArrayUtils::accessCallDataArrayElement(ArrayType const& _arrayType, bool _doBoundsCheck) const
{
	solAssert(_arrayType.location() == DataLocation::CallData, "");
	if (_arrayType.baseType()->isDynamicallyEncoded())
	{
		// stack layout: <base_ref> <length> <index>
		ArrayUtils(m_context).accessIndex(_arrayType, _doBoundsCheck, true);
		// stack layout: <base_ref> <ptr_to_tail>

		CompilerUtils(m_context).accessCalldataTail(*_arrayType.baseType());
		// stack layout: <tail_ref> [length]
	}
	else
	{
		ArrayUtils(m_context).accessIndex(_arrayType, _doBoundsCheck);
		if (_arrayType.baseType()->isValueType())
		{
			solAssert(_arrayType.baseType()->storageBytes() <= 32, "");
			if (
				!_arrayType.isByteArrayOrString() &&
				_arrayType.baseType()->storageBytes() < 32 &&
				m_context.useABICoderV2()
			)
			{
				m_context << u256(32);
				CompilerUtils(m_context).abiDecodeV2({_arrayType.baseType()}, false);
			}
			else
				CompilerUtils(m_context).loadFromMemoryDynamic(
					*_arrayType.baseType(),
					true,
					!_arrayType.isByteArrayOrString(),
					false
				);
		}
		else
			solAssert(
				_arrayType.baseType()->category() == Type::Category::Struct ||
				_arrayType.baseType()->category() == Type::Category::Array,
				"Invalid statically sized non-value base type on array access."
			);
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
		m_context << swapInstruction(_byteOffsetPosition - 1);
	m_context << u256(_byteSize) << Instruction::ADD;
	if (_byteOffsetPosition > 1)
		m_context << swapInstruction(_byteOffsetPosition - 1);
	// compute, X := (byteOffset + byteSize - 1) / 32, should be 1 iff byteOffset + bytesize > 32
	m_context
		<< u256(32) << dupInstruction(1 + _byteOffsetPosition) << u256(_byteSize - 1)
		<< Instruction::ADD << Instruction::DIV;
	// increment storage offset if X == 1 (just add X to it)
	// stack: X
	m_context
		<< swapInstruction(_storageOffsetPosition) << dupInstruction(_storageOffsetPosition + 1)
		<< Instruction::ADD << swapInstruction(_storageOffsetPosition);
	// stack: X
	// set source_byte_offset to zero if X == 1 (using source_byte_offset *= 1 - X)
	m_context << u256(1) << Instruction::SUB;
	// stack: 1 - X
	if (_byteOffsetPosition == 1)
		m_context << Instruction::MUL;
	else
		m_context
			<< dupInstruction(_byteOffsetPosition + 1) << Instruction::MUL
			<< swapInstruction(_byteOffsetPosition) << Instruction::POP;
}
