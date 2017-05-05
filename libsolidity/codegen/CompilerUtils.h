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

#pragma once

#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/ast/ASTForward.h>

namespace dev {
namespace solidity {

class Type; // forward

class CompilerUtils
{
public:
	CompilerUtils(CompilerContext& _context): m_context(_context) {}

	/// Stores the initial value of the free-memory-pointer at its position;
	void initialiseFreeMemoryPointer();
	/// Copies the free memory pointer to the stack.
	void fetchFreeMemoryPointer();
	/// Stores the free memory pointer from the stack.
	void storeFreeMemoryPointer();
	/// Allocates a number of bytes in memory as given on the stack.
	/// Stack pre: <size>
	/// Stack post: <mem_start>
	void allocateMemory();
	/// Appends code that transforms memptr to (memptr - free_memptr) memptr
	void toSizeAfterFreeMemoryPointer();

	/// Loads data from memory to the stack.
	/// @param _offset offset in memory (or calldata)
	/// @param _type data type to load
	/// @param _fromCalldata if true, load from calldata, not from memory
	/// @param _padToWords if true, assume the data is padded to full words (32 bytes)
	/// @returns the number of bytes consumed in memory.
	unsigned loadFromMemory(
		unsigned _offset,
		Type const& _type = IntegerType(256),
		bool _fromCalldata = false,
		bool _padToWords = false
	);
	/// Dynamic version of @see loadFromMemory, expects the memory offset on the stack.
	/// Stack pre: memory_offset
	/// Stack post: value... (memory_offset+length)
	void loadFromMemoryDynamic(
		Type const& _type,
		bool _fromCalldata = false,
		bool _padToWords = true,
		bool _keepUpdatedMemoryOffset = true
	);
	/// Stores a 256 bit integer from stack in memory.
	/// @param _offset offset in memory
	/// @param _type type of the data on the stack
	void storeInMemory(unsigned _offset);
	/// Dynamic version of @see storeInMemory, expects the memory offset below the value on the stack
	/// and also updates that. For reference types, only copies the data pointer. Fails for
	/// non-memory-references.
	/// @param _padToWords if true, adds zeros to pad to multiple of 32 bytes. Array elements
	/// are always padded (except for byte arrays), regardless of this parameter.
	/// Stack pre: memory_offset value...
	/// Stack post: (memory_offset+length)
	void storeInMemoryDynamic(Type const& _type, bool _padToWords = true);

	/// Copies values (of types @a _givenTypes) given on the stack to a location in memory given
	/// at the stack top, encoding them according to the ABI as the given types @a _targetTypes.
	/// Removes the values from the stack and leaves the updated memory pointer.
	/// Stack pre: <v1> <v2> ... <vn> <memptr>
	/// Stack post: <memptr_updated>
	/// Does not touch the memory-free pointer.
	/// @param _padToWords if false, all values are concatenated without padding.
	/// @param _copyDynamicDataInPlace if true, dynamic types is stored (without length)
	/// together with fixed-length data.
	/// @param _encodeAsLibraryTypes if true, encodes for a library function, e.g. does not
	/// convert storage pointer types to memory types.
	/// @note the locations of target reference types are ignored, because it will always be
	/// memory.
	void encodeToMemory(
		TypePointers const& _givenTypes = {},
		TypePointers const& _targetTypes = {},
		bool _padToWords = true,
		bool _copyDynamicDataInPlace = false,
		bool _encodeAsLibraryTypes = false
	);

	/// Zero-initialises (the data part of) an already allocated memory array.
	/// Length has to be nonzero!
	/// Stack pre: <length> <memptr>
	/// Stack post: <updated_memptr>
	void zeroInitialiseMemoryArray(ArrayType const& _type);

	/// Copies full 32 byte words in memory (regions cannot overlap), i.e. may copy more than length.
	/// Stack pre: <size> <target> <source>
	/// Stack post:
	void memoryCopy32();
	/// Copies data in memory (regions cannot overlap).
	/// Stack pre: <size> <target> <source>
	/// Stack post:
	void memoryCopy();

	/// Converts the combined and left-aligned (right-aligned if @a _rightAligned is true)
	/// external function type <address><function identifier> into two stack slots:
	/// address (right aligned), function identifier (right aligned)
	void splitExternalFunctionType(bool _rightAligned);
	/// Performs the opposite operation of splitExternalFunctionType(_rightAligned)
	void combineExternalFunctionType(bool _rightAligned);
	/// Appends code that combines the construction-time (if available) and runtime function
	/// entry label of the given function into a single stack slot.
	/// Note: This might cause the compilation queue of the runtime context to be extended.
	void pushCombinedFunctionEntryLabel(Declaration const& _function);

	/// Appends code for an implicit or explicit type conversion. This includes erasing higher
	/// order bits (@see appendHighBitCleanup) when widening integer but also copy to memory
	/// if a reference type is converted from calldata or storage to memory.
	/// If @a _cleanupNeeded, high order bits cleanup is also done if no type conversion would be
	/// necessary.
	/// If @a _chopSignBits, the function resets the signed bits out of the width of the signed integer.
	void convertType(Type const& _typeOnStack, Type const& _targetType, bool _cleanupNeeded = false, bool _chopSignBits = false);

	/// Creates a zero-value for the given type and puts it onto the stack. This might allocate
	/// memory for memory references.
	void pushZeroValue(Type const& _type);

	/// Moves the value that is at the top of the stack to a stack variable.
	void moveToStackVariable(VariableDeclaration const& _variable);
	/// Copies an item that occupies @a _itemSize stack slots from a stack depth of @a _stackDepth
	/// to the top of the stack.
	void copyToStackTop(unsigned _stackDepth, unsigned _itemSize);
	/// Moves an item that occupies @a _itemSize stack slots and has items occupying @a _stackDepth
	/// slots above it to the top of the stack.
	void moveToStackTop(unsigned _stackDepth, unsigned _itemSize = 1);
	/// Moves @a _itemSize elements past @a _stackDepth other stack elements
	void moveIntoStack(unsigned _stackDepth, unsigned _itemSize = 1);
	/// Rotates the topmost @a _items items on the stack, such that the previously topmost element
	/// is bottom-most.
	void rotateStackUp(unsigned _items);
	/// Rotates the topmost @a _items items on the stack, such that the previously bottom-most element
	/// is now topmost.
	void rotateStackDown(unsigned _items);
	/// Removes the current value from the top of the stack.
	void popStackElement(Type const& _type);
	/// Removes element from the top of the stack _amount times.
	void popStackSlots(size_t _amount);

	template <class T>
	static unsigned sizeOnStack(std::vector<T> const& _variables);
	static unsigned sizeOnStack(std::vector<std::shared_ptr<Type const>> const& _variableTypes);

	/// Appends code that computes tha SHA3 hash of the topmost stack element of 32 byte type.
	void computeHashStatic();

	/// Bytes we need to the start of call data.
	///  - The size in bytes of the function (hash) identifier.
	static const unsigned dataStartOffset;

	/// Position of the free-memory-pointer in memory;
	static const size_t freeMemoryPointer;

private:
	/// Address of the precompiled identity contract.
	static const unsigned identityContractAddress;

	/// Stores the given string in memory.
	/// Stack pre: mempos
	/// Stack post:
	void storeStringData(bytesConstRef _data);

	/// Appends code that cleans higher-order bits for integer types.
	void cleanHigherOrderBits(IntegerType const& _typeOnStack);

	/// Prepares the given type for storing in memory by shifting it if necessary.
	unsigned prepareMemoryStore(Type const& _type, bool _padToWords);
	/// Loads type from memory assuming memory offset is on stack top.
	unsigned loadFromMemoryHelper(Type const& _type, bool _fromCalldata, bool _padToWords);

	CompilerContext& m_context;
};


template <class T>
unsigned CompilerUtils::sizeOnStack(std::vector<T> const& _variables)
{
	unsigned size = 0;
	for (T const& variable: _variables)
		size += variable->annotation().type->sizeOnStack();
	return size;
}

}
}
