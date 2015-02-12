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

#pragma once

#include <libsolidity/CompilerContext.h>
#include <libsolidity/ASTForward.h>

namespace dev {
namespace solidity {

class Type; // forward

class CompilerUtils
{
public:
	CompilerUtils(CompilerContext& _context): m_context(_context) {}

	/// Loads data from memory to the stack.
	/// @param _offset offset in memory (or calldata)
	/// @param _bytes number of bytes to load
	/// @param _leftAligned if true, store left aligned on stack (otherwise right aligned)
	/// @param _fromCalldata if true, load from calldata, not from memory
	/// @param _padToWordBoundaries if true, assume the data is padded to word (32 byte) boundaries
	/// @returns the number of bytes consumed in memory (can be different from _bytes if
	///          _padToWordBoundaries is true)
	unsigned loadFromMemory(unsigned _offset, unsigned _bytes = 32, bool _leftAligned = false,
							bool _fromCalldata = false, bool _padToWordBoundaries = false);
	/// Stores data from stack in memory.
	/// @param _offset offset in memory
	/// @param _type type of the data on the stack
	/// @param _padToWordBoundaries if true, pad the data to word (32 byte) boundaries
	/// @returns the number of bytes written to memory (can be different from _bytes if
	///          _padToWordBoundaries is true)
	unsigned storeInMemory(unsigned _offset, Type const& _type = IntegerType(256), bool _padToWordBoundaries = false);
	/// Dynamic version of @see storeInMemory, expects the memory offset below the value on the stack
	/// and also updates that.
	/// Stack pre: memory_offset value...
	/// Stack post: (memory_offset+length)
	void storeInMemoryDynamic(Type const& _type, bool _padToWordBoundaries = true);
	/// @returns _size rounded up to the next multiple of 32 (the number of bytes occupied in the
	///          padded calldata)
	static unsigned getPaddedSize(unsigned _size) { return ((_size + 31) / 32) * 32; }

	/// Moves the value that is at the top of the stack to a stack variable.
	void moveToStackVariable(VariableDeclaration const& _variable);
	/// Copies a variable of type @a _type from a stack depth of @a _stackDepth to the top of the stack.
	void copyToStackTop(unsigned _stackDepth, Type const& _type);
	/// Removes the current value from the top of the stack.
	void popStackElement(Type const& _type);

	template <class T>
	static unsigned getSizeOnStack(std::vector<T> const& _variables);
	static unsigned getSizeOnStack(std::vector<std::shared_ptr<Type const>> const& _variableTypes);

	/// Appends code that computes tha SHA3 hash of the topmost stack element of type @a _type.
	/// If @a _pad is set, padds the type to muliples of 32 bytes.
	/// @note Only works for types of fixed size.
	void computeHashStatic(Type const& _type = IntegerType(256), bool _padToWordBoundaries = false);

	/// Copies a byte array to a byte array in storage.
	/// Stack pre: [source_reference] target_reference
	/// Stack post: target_reference
	void copyByteArrayToStorage(ByteArrayType const& _targetType, ByteArrayType const& _sourceType) const;
	/// Clears the length and data elements of the byte array referenced on the stack.
	/// Stack pre: reference
	/// Stack post:
	void clearByteArray(ByteArrayType const& _type) const;

	/// Bytes we need to the start of call data.
	///  - The size in bytes of the function (hash) identifier.
	static const unsigned int dataStartOffset;

private:
	/// Prepares the given type for storing in memory by shifting it if necessary.
	unsigned prepareMemoryStore(Type const& _type, bool _padToWordBoundaries) const;
	/// Appends a loop that clears a sequence of storage slots (excluding end).
	/// Stack pre: end_ref start_ref
	/// Stack post: end_ref
	void clearStorageLoop() const;

	CompilerContext& m_context;
};


template <class T>
unsigned CompilerUtils::getSizeOnStack(std::vector<T> const& _variables)
{
	unsigned size = 0;
	for (T const& variable: _variables)
		size += variable->getType()->getSizeOnStack();
	return size;
}

}
}
