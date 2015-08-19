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

#pragma once

namespace dev
{
namespace solidity
{

class CompilerContext;
class Type;
class ArrayType;

/**
 * Class that provides code generation for handling arrays.
 */
class ArrayUtils
{
public:
	ArrayUtils(CompilerContext& _context): m_context(_context) {}

	/// Copies an array to an array in storage. The arrays can be of different types only if
	/// their storage representation is the same.
	/// Stack pre: source_reference [source_length] target_reference
	/// Stack post: target_reference
	void copyArrayToStorage(ArrayType const& _targetType, ArrayType const& _sourceType) const;
	/// Copies the data part of an array (which cannot be dynamically nested) from anywhere
	/// to a given position in memory.
	/// This always copies contained data as is (i.e. structs and fixed-size arrays are copied in
	/// place as required by the ABI encoding). Use CompilerUtils::convertType if you want real
	/// memory copies of nested arrays.
	/// Stack pre: memory_offset source_item
	/// Stack post: memory_offest + length(padded)
	void copyArrayToMemory(ArrayType const& _sourceType, bool _padToWordBoundaries = true) const;
	/// Clears the given dynamic or static array.
	/// Stack pre: storage_ref storage_byte_offset
	/// Stack post:
	void clearArray(ArrayType const& _type) const;
	/// Clears the length and data elements of the array referenced on the stack.
	/// Stack pre: reference (excludes byte offset)
	/// Stack post:
	void clearDynamicArray(ArrayType const& _type) const;
	/// Changes the size of a dynamic array and clears the tail if it is shortened.
	/// Stack pre: reference (excludes byte offset) new_length
	/// Stack post:
	void resizeDynamicArray(ArrayType const& _type) const;
	/// Appends a loop that clears a sequence of storage slots of the given type (excluding end).
	/// Stack pre: end_ref start_ref
	/// Stack post: end_ref
	void clearStorageLoop(Type const& _type) const;
	/// Converts length to size (number of storage slots or calldata/memory bytes).
	/// if @a _pad then add padding to multiples of 32 bytes for calldata/memory.
	/// Stack pre: length
	/// Stack post: size
	void convertLengthToSize(ArrayType const& _arrayType, bool _pad = false) const;
	/// Retrieves the length (number of elements) of the array ref on the stack. This also
	/// works for statically-sized arrays.
	/// Stack pre: reference (excludes byte offset for dynamic storage arrays)
	/// Stack post: reference length
	void retrieveLength(ArrayType const& _arrayType) const;
	/// Performs bounds checking and returns a reference on the stack.
	/// Stack pre: reference [length] index
	/// Stack post (storage): storage_slot byte_offset
	/// Stack post: memory/calldata_offset
	void accessIndex(ArrayType const& _arrayType, bool _doBoundsCheck = true) const;

private:
	/// Adds the given number of bytes to a storage byte offset counter and also increments
	/// the storage offset if adding this number again would increase the counter over 32.
	/// @param byteOffsetPosition the stack offset of the storage byte offset
	/// @param storageOffsetPosition the stack offset of the storage slot offset
	void incrementByteOffset(unsigned _byteSize, unsigned _byteOffsetPosition, unsigned _storageOffsetPosition) const;

	CompilerContext& m_context;
};

}
}
