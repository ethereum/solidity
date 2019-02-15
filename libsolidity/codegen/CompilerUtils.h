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

#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/codegen/CompilerContext.h>

namespace dev {
namespace solidity {

class Type; // forward

class CompilerUtils
{
public:
	explicit CompilerUtils(CompilerContext& _context): m_context(_context) {}

	/// Stores the initial value of the free-memory-pointer at its position;
	void initialiseFreeMemoryPointer();
	/// Copies the free memory pointer to the stack.
	/// Stack pre:
	/// Stack post: <mem_start>
	void fetchFreeMemoryPointer();
	/// Stores the free memory pointer from the stack.
	/// Stack pre: <mem_end>
	/// Stack post:
	void storeFreeMemoryPointer();
	/// Allocates a number of bytes in memory as given on the stack.
	/// Stack pre: <size>
	/// Stack post: <mem_start>
	void allocateMemory();
	/// Appends code that transforms memptr to (memptr - free_memptr) memptr
	/// Stack pre: <mem_end>
	/// Stack post: <size> <mem_start>
	void toSizeAfterFreeMemoryPointer();

	/// Appends code that performs a revert, providing the given string data.
	/// Will also append an error signature corresponding to Error(string).
	/// @param _argumentType the type of the string argument, will be converted to memory string.
	/// Stack pre: string data
	/// Stack post:
	void revertWithStringData(Type const& _argumentType);

	/// Loads data from memory to the stack.
	/// @param _offset offset in memory (or calldata)
	/// @param _type data type to load
	/// @param _fromCalldata if true, load from calldata, not from memory
	/// @param _padToWords if true, assume the data is padded to full words (32 bytes)
	/// @returns the number of bytes consumed in memory.
	unsigned loadFromMemory(
		unsigned _offset,
		Type const& _type = IntegerType::uint256(),
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

	/// Creates code that unpacks the arguments according to their types specified by a vector of TypePointers.
	/// From memory if @a _fromMemory is true, otherwise from call data.
	/// Calls revert if the supplied size is shorter than the static data requirements
	/// or if dynamic data pointers reach outside of the area.
	/// Also has a hard cap of 0x100000000 for any given length/offset field.
	/// Stack pre: <source_offset> <length>
	/// Stack post: <value0> <value1> ... <valuen>
	void abiDecode(TypePointers const& _typeParameters, bool _fromMemory = false);

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
		TypePointers const& _givenTypes,
		TypePointers const& _targetTypes,
		bool _padToWords,
		bool _copyDynamicDataInPlace,
		bool _encodeAsLibraryTypes = false
	);

	/// Special case of @a encodeToMemory which assumes tight packing, e.g. no zero padding
	/// and dynamic data is encoded in-place.
	/// Stack pre: <value0> <value1> ... <valueN-1> <head_start>
	/// Stack post: <mem_ptr>
	void packedEncode(
		TypePointers const& _givenTypes,
		TypePointers const& _targetTypes,
		bool _encodeAsLibraryTypes = false
	)
	{
		encodeToMemory(_givenTypes, _targetTypes, false, true, _encodeAsLibraryTypes);
	}

	/// Special case of @a encodeToMemory which assumes that everything is padded to words
	/// and dynamic data is not copied in place (i.e. a proper ABI encoding).
	/// Stack pre: <value0> <value1> ... <valueN-1> <head_start>
	/// Stack post: <mem_ptr>
	void abiEncode(
		TypePointers const& _givenTypes,
		TypePointers const& _targetTypes,
		bool _encodeAsLibraryTypes = false
	)
	{
		encodeToMemory(_givenTypes, _targetTypes, true, false, _encodeAsLibraryTypes);
	}

	/// Special case of @a encodeToMemory which assumes that everything is padded to words
	/// and dynamic data is not copied in place (i.e. a proper ABI encoding).
	/// Uses a new, less tested encoder implementation.
	/// Stack pre: <value0> <value1> ... <valueN-1> <head_start>
	/// Stack post: <mem_ptr>
	void abiEncodeV2(
		TypePointers const& _givenTypes,
		TypePointers const& _targetTypes,
		bool _encodeAsLibraryTypes = false,
		bool _padToWordBoundaries = true
	);

	/// Decodes data from ABI encoding into internal encoding. If @a _fromMemory is set to true,
	/// the data is taken from memory instead of from calldata.
	/// Can allocate memory.
	/// Stack pre: <source_offset> <length>
	/// Stack post: <value0> <value1> ... <valuen>
	void abiDecodeV2(TypePointers const& _parameterTypes, bool _fromMemory = false);

	/// Zero-initialises (the data part of) an already allocated memory array.
	/// Length has to be nonzero!
	/// Stack pre: <length> <memptr>
	/// Stack post: <updated_memptr>
	void zeroInitialiseMemoryArray(ArrayType const& _type);

	/// Copies full 32 byte words in memory (regions cannot overlap), i.e. may copy more than length.
	/// Length can be zero, in this case, it copies nothing.
	/// Stack pre: <size> <target> <source>
	/// Stack post:
	void memoryCopy32();
	/// Copies data in memory (regions cannot overlap).
	/// Length can be zero, in this case, it copies nothing.
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
	/// If @a _runtimeOnly, the entry label will include the runtime assembly tag.
	void pushCombinedFunctionEntryLabel(Declaration const& _function, bool _runtimeOnly = true);

	/// Appends code for an implicit or explicit type conversion. This includes erasing higher
	/// order bits (@see appendHighBitCleanup) when widening integer but also copy to memory
	/// if a reference type is converted from calldata or storage to memory.
	/// If @a _cleanupNeeded, high order bits cleanup is also done if no type conversion would be
	/// necessary.
	/// If @a _chopSignBits, the function resets the signed bits out of the width of the signed integer.
	/// If @a _asPartOfArgumentDecoding is true, failed conversions are flagged via REVERT,
	/// otherwise they are flagged with INVALID.
	void convertType(
		Type const& _typeOnStack,
		Type const& _targetType,
		bool _cleanupNeeded = false,
		bool _chopSignBits = false,
		bool _asPartOfArgumentDecoding = false
	);

	/// Creates a zero-value for the given type and puts it onto the stack. This might allocate
	/// memory for memory references.
	void pushZeroValue(Type const& _type);
	/// Pushes a pointer to the stack that points to a (potentially shared) location in memory
	/// that always contains a zero. It is not allowed to write there.
	void pushZeroPointer();

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
	/// Pops slots from the stack such that its height is _toHeight.
	/// Adds jump to _jumpTo.
	/// Readjusts the stack offset to the original value.
	void popAndJump(unsigned _toHeight, eth::AssemblyItem const& _jumpTo);

	template <class T>
	static unsigned sizeOnStack(std::vector<T> const& _variables);
	static unsigned sizeOnStack(std::vector<std::shared_ptr<Type const>> const& _variableTypes);

	/// Helper function to shift top value on the stack to the left.
	/// Stack pre: <value> <shift_by_bits>
	/// Stack post: <shifted_value>
	void leftShiftNumberOnStack(unsigned _bits);

	/// Helper function to shift top value on the stack to the right.
	/// Stack pre: <value> <shift_by_bits>
	/// Stack post: <shifted_value>
	void rightShiftNumberOnStack(unsigned _bits);

	/// Appends code that computes the Keccak-256 hash of the topmost stack element of 32 byte type.
	void computeHashStatic();

	/// Apppends code that copies the code of the given contract to memory.
	/// Stack pre: Memory position
	/// Stack post: Updated memory position
	/// @param creation if true, copies creation code, if false copies runtime code.
	/// @note the contract has to be compiled already, so beware of cyclic dependencies!
	void copyContractCodeToMemory(ContractDefinition const& contract, bool _creationCode);

	/// Bytes we need to the start of call data.
	///  - The size in bytes of the function (hash) identifier.
	static unsigned const dataStartOffset;

	/// Position of the free-memory-pointer in memory;
	static size_t const freeMemoryPointer;
	/// Position of the memory slot that is always zero.
	static size_t const zeroPointer;
	/// Starting offset for memory available to the user (aka the contract).
	static size_t const generalPurposeMemoryStart;

private:
	/// Address of the precompiled identity contract.
	static unsigned const identityContractAddress;

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
