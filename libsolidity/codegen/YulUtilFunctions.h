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
 * Component that can generate various useful Yul functions.
 */

#pragma once

#include <liblangutil/EVMVersion.h>

#include <libsolidity/ast/Types.h>
#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <libsolidity/interface/DebugSettings.h>

#include <libsolutil/ErrorCodes.h>

#include <memory>
#include <string>
#include <vector>

namespace solidity::frontend
{

class Type;
class ArrayType;
class MappingType;
class IntegerType;
class StructType;

/**
 * Component that can generate various useful Yul functions.
 */
class YulUtilFunctions
{
public:
	explicit YulUtilFunctions(
		langutil::EVMVersion _evmVersion,
		RevertStrings _revertStrings,
		MultiUseYulFunctionCollector& _functionCollector
	):
		m_evmVersion(_evmVersion),
		m_revertStrings(_revertStrings),
		m_functionCollector(_functionCollector)
	{}

	/// @returns the name of a function that returns its argument.
	/// Sometimes needed to satisfy templates.
	std::string identityFunction();

	/// @returns a function that combines the address and selector to a single value
	/// for use in the ABI.
	std::string combineExternalFunctionIdFunction();

	/// @returns a function that splits the address and selector from a single value
	/// for use in the ABI.
	std::string splitExternalFunctionIdFunction();

	/// @returns a function that copies raw bytes of dynamic length from calldata
	/// or memory to memory.
	/// Pads with zeros and might write more than exactly length.
	std::string copyToMemoryFunction(bool _fromCalldata);

	/// @returns the name of a function that copies a string literal to memory
	/// and returns a pointer to the memory area containing the string literal.
	/// signature: () -> memPtr
	std::string copyLiteralToMemoryFunction(std::string const& _literal);

	/// @returns the name of a function that stores a string literal at a specific location in memory
	/// signature: (memPtr) ->
	std::string storeLiteralInMemoryFunction(std::string const& _literal);

	/// @returns the name of a function that stores a string literal at a specific location in storage
	/// signature: (slot) ->
	std::string copyLiteralToStorageFunction(std::string const& _literal);

	// @returns the name of a function that has the equivalent logic of an
	// `assert` or `require` call.
	std::string requireOrAssertFunction(bool _assert, Type const* _messageType = nullptr);

	/// @returns the name of a function that takes a (cleaned) value of the given value type and
	/// left-aligns it, usually for use in non-padded encoding.
	std::string leftAlignFunction(Type const& _type);

	std::string shiftLeftFunction(size_t _numBits);
	std::string shiftLeftFunctionDynamic();
	std::string shiftRightFunction(size_t _numBits);
	std::string shiftRightFunctionDynamic();
	std::string shiftRightSignedFunctionDynamic();

	/// @returns the name of a function that performs a left shift and subsequent cleanup
	/// and, if needed, prior cleanup.
	/// signature: (value, amountToShift) -> result
	std::string typedShiftLeftFunction(Type const& _type, Type const& _amountType);
	std::string typedShiftRightFunction(Type const& _type, Type const& _amountType);

	/// @returns the name of a function which replaces the
	/// _numBytes bytes starting at byte position _shiftBytes (counted from the least significant
	/// byte) by the _numBytes least significant bytes of `toInsert`.
	/// signature: (value, toInsert) -> result
	std::string updateByteSliceFunction(size_t _numBytes, size_t _shiftBytes);

	/// signature: (value, shiftBytes, toInsert) -> result
	std::string updateByteSliceFunctionDynamic(size_t _numBytes);

	/// Function that sets all but the first ``bytes`` bytes of ``value`` to zero.
	/// @note ``bytes`` has to be small enough not to overflow ``8 * bytes``.
	/// signature: (value, bytes) -> result
	std::string maskBytesFunctionDynamic();

	/// Zeroes out all bytes above the first ``_bytes`` lower order bytes.
	/// signature: (value) -> result
	std::string maskLowerOrderBytesFunction(size_t _bytes);

	/// Zeroes out all bytes above the first ``bytes`` lower order bytes.
	/// @note ``bytes`` has to be small enough not to overflow ``8 * bytes``.
	/// signature: (value, bytes) -> result
	std::string maskLowerOrderBytesFunctionDynamic();

	/// @returns the name of a function that rounds its input to the next multiple
	/// of 32 or the input if it is a multiple of 32.
	/// Ignores overflow.
	/// signature: (value) -> result
	std::string roundUpFunction();

	/// @returns the name of a function that divides by 32 and rounds up during the division.
	/// In other words, on input x it returns the smallest y such that y * 32 >= x.
	/// Ignores overflow.
	/// signature: (x) -> y
	std::string divide32CeilFunction();

	/// signature: (x, y) -> sum
	std::string overflowCheckedIntAddFunction(IntegerType const& _type);
	/// signature: (x, y) -> sum
	std::string wrappingIntAddFunction(IntegerType const& _type);

	/// signature: (x, y) -> product
	std::string overflowCheckedIntMulFunction(IntegerType const& _type);
	/// signature: (x, y) -> product
	std::string wrappingIntMulFunction(IntegerType const& _type);

	/// @returns name of function to perform division on integers.
	/// Checks for division by zero and the special case of
	/// signed division of the smallest number by -1.
	std::string overflowCheckedIntDivFunction(IntegerType const& _type);
	/// @returns name of function to perform division on integers.
	/// Checks for division by zero.
	std::string wrappingIntDivFunction(IntegerType const& _type);

	/// @returns name of function to perform modulo on integers.
	/// Reverts for modulo by zero.
	std::string intModFunction(IntegerType const& _type);

	/// @returns computes the difference between two values.
	/// Assumes the input to be in range for the type.
	/// signature: (x, y) -> diff
	std::string overflowCheckedIntSubFunction(IntegerType const& _type);

	/// @returns computes the difference between two values.
	/// signature: (x, y) -> diff
	std::string wrappingIntSubFunction(IntegerType const& _type);

	/// @returns the name of the exponentiation function.
	/// signature: (base, exponent) -> power
	std::string overflowCheckedIntExpFunction(IntegerType const& _type, IntegerType const& _exponentType);

	/// @returns the name of the exponentiation function, specialized for literal base.
	/// signature: exponent -> power
	std::string overflowCheckedIntLiteralExpFunction(
		RationalNumberType const& _baseType,
		IntegerType const& _exponentType,
		IntegerType const& _commonType
	);

	/// Generic unsigned checked exponentiation function.
	/// Reverts if the result is larger than max.
	/// signature: (base, exponent, max) -> power
	std::string overflowCheckedUnsignedExpFunction();

	/// Generic signed checked exponentiation function.
	/// Reverts if the result is smaller than min or larger than max.
	/// The code relies on max <= |min| and min < 0.
	/// signature: (base, exponent, min, max) -> power
	std::string overflowCheckedSignedExpFunction();

	/// Helper function for the two checked exponentiation functions.
	/// signature: (power, base, exponent, max) -> power
	std::string overflowCheckedExpLoopFunction();

	/// @returns the name of the exponentiation function.
	/// signature: (base, exponent) -> power
	std::string wrappingIntExpFunction(IntegerType const& _type, IntegerType const& _exponentType);

	/// @returns the name of a function that fetches the length of the given
	/// array
	/// signature: (array) -> length
	std::string arrayLengthFunction(ArrayType const& _type);

	/// @returns function name that extracts and returns byte array length from the value
	/// stored at the slot.
	/// Causes a Panic if the length encoding is wrong.
	/// signature: (data) -> length
	std::string extractByteArrayLengthFunction();

	/// @returns the name of a function that resizes a storage array
	/// for statically sized arrays, it will just clean-up elements of array starting from newLen until the end
	/// signature: (array, newLen)
	std::string resizeArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that zeroes all storage array elements from `startIndex` to `len` (excluding).
	/// Assumes that `len` is the array length. Does nothing if `startIndex >= len`. Does not modify the stored length.
	/// signature: (array, len, startIndex)
	std::string cleanUpStorageArrayEndFunction(ArrayType const& _type);

	/// @returns the name of a function that reduces the size of a storage array by one element
	/// signature: (array)
	std::string storageArrayPopFunction(ArrayType const& _type);

	/// @returns the name of a function that pushes an element to a storage array
/// @param _fromType represents the type of the element being pushed.
/// If _fromType is ReferenceType the function will perform deep copy.
	/// signature: (array, value)
	std::string storageArrayPushFunction(ArrayType const& _type, Type const* _fromType = nullptr);

	/// @returns the name of a function that pushes the base type's zero element to a storage array and returns storage slot and offset of the added element.
	/// signature: (array) -> slot, offset
	std::string storageArrayPushZeroFunction(ArrayType const& _type);

	/// @returns the name of a function that will clear the storage area given
	/// by the start and end (exclusive) parameters (slots).
	/// signature: (start, end)
	std::string clearStorageRangeFunction(Type const& _type);

	/// @returns the name of a function that will clear the given storage array
	/// signature: (slot) ->
	std::string clearStorageArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that will copy an array to storage
	/// signature (to_slot, from_ptr) ->
	std::string copyArrayToStorageFunction(ArrayType const& _fromType, ArrayType const& _toType);

	/// @returns the name of a function that will copy a byte array to storage
	/// signature (to_slot, from_ptr) ->
	std::string copyByteArrayToStorageFunction(ArrayType const& _fromType, ArrayType const& _toType);

	/// @returns the name of a function that will copy an array of value types from storage to storage.
	/// signature (to_slot, from_slot) ->
	std::string copyValueArrayStorageToStorageFunction(ArrayType const& _fromType, ArrayType const& _toType);

	/// Returns the name of a function that will convert a given length to the
	/// size in memory (number of storage slots or calldata/memory bytes) it
	/// will require.
	/// signature: (length) -> size
	std::string arrayConvertLengthToSize(ArrayType const& _type);

	/// @returns the name of a function that computes the number of bytes required
	/// to store an array in memory given its length (internally encoded, not ABI encoded).
	/// The function reverts for too large lengths.
	std::string arrayAllocationSizeFunction(ArrayType const& _type);

	/// @returns the name of a function that converts a storage slot number
	/// a memory pointer or a calldata pointer to the slot number / memory pointer / calldata pointer
	/// for the data position of an array which is stored in that slot / memory area / calldata area.
	std::string arrayDataAreaFunction(ArrayType const& _type);

	/// @returns the name of a function that returns the slot and offset for the
	/// given array and index
	/// signature: (array, index) -> slot, offset
	std::string storageArrayIndexAccessFunction(ArrayType const& _type);

	/// @returns the name of a function that returns the memory address for the
	/// given array base ref and index.
	/// Causes invalid opcode on out of range access.
	/// signature: (baseRef, index) -> address
	std::string memoryArrayIndexAccessFunction(ArrayType const& _type);

	/// @returns the name of a function that returns the calldata address for the
	/// given array base ref and index.
	/// signature: (baseRef, index) -> offset[, length]
	std::string calldataArrayIndexAccessFunction(ArrayType const& _type);

	/// @returns the name of a function that returns offset and length for array slice
	/// for the given array offset, length and start and end indices for slice
	/// signature: (arrayOffset, arrayLength, sliceStart, sliceEnd) -> offset, length
	std::string calldataArrayIndexRangeAccess(ArrayType const& _type);

	/// @returns the name of a function that follows a calldata tail while performing
	/// bounds checks.
	/// signature: (baseRef, tailPointer) -> offset[, length]
	std::string accessCalldataTailFunction(Type const& _type);

	/// @returns the name of a function that advances an array data pointer to the next element.
	/// Only works for memory arrays, calldata arrays and storage arrays that every item occupies one or multiple full slots.
	std::string nextArrayElementFunction(ArrayType const& _type);

	/// @returns the name of a function that allocates a memory array and copies the contents
	/// of the storage array into it.
	std::string copyArrayFromStorageToMemoryFunction(ArrayType const& _from, ArrayType const& _to);

	/// @returns the name of a function that does concatenation of variadic number of
	/// bytes if @a functionTypeKind is FunctionType::Kind::BytesConcat,
	/// or of strings, if @a functionTypeKind is FunctionType::Kind::StringConcat.
	std::string bytesOrStringConcatFunction(
		std::vector<Type const*> const& _argumentTypes,
		FunctionType::Kind _functionTypeKind
	);

	/// @returns the name of a function that performs index access for mappings.
	/// @param _mappingType the type of the mapping
	/// @param _keyType the type of the value provided
	std::string mappingIndexAccessFunction(MappingType const& _mappingType, Type const& _keyType);

	/// @returns a function that reads a type from storage.
	/// @param _splitFunctionTypes if false, returns the address and function signature in a
	/// single variable.
	std::string readFromStorage(Type const& _type, size_t _offset, bool _splitFunctionTypes);
	std::string readFromStorageDynamic(Type const& _type, bool _splitFunctionTypes);

	/// @returns a function that reads a value type from memory. Performs cleanup.
	/// signature: (addr) -> value
	std::string readFromMemory(Type const& _type);
	/// @returns a function that reads a value type from calldata.
	/// Reverts on invalid input.
	/// signature: (addr) -> value
	std::string readFromCalldata(Type const& _type);

	/// @returns a function that extracts a value type from storage slot that has been
	/// retrieved already.
	/// Performs bit mask/sign extend cleanup and appropriate left / right shift, but not validation.
	///
	/// For external function types, input and output is in "compressed"/"unsplit" form.
	std::string extractFromStorageValue(Type const& _type, size_t _offset);
	std::string extractFromStorageValueDynamic(Type const& _type);

	/// Returns the name of a function will write the given value to
	/// the specified slot and offset. If offset is not given, it is expected as
	/// runtime parameter.
	/// For reference types, offset is checked to be zero at runtime.
	/// signature: (slot, [offset,] value)
	std::string updateStorageValueFunction(
		Type const& _fromType,
		Type const& _toType,
		std::optional<unsigned> const& _offset = std::optional<unsigned>()
	);

	/// Returns the name of a function that will write the given value to
	/// the specified address.
	/// Performs a cleanup before writing for value types.
	/// signature: (memPtr, value) ->
	std::string writeToMemoryFunction(Type const& _type);

	/// Performs cleanup after reading from a potentially compressed storage slot.
	/// The function does not perform any validation, it just masks or sign-extends
	/// higher order bytes or left-aligns (in case of bytesNN).
	/// The storage cleanup expects the value to be right-aligned with potentially
	/// dirty higher order bytes.
	/// For external functions, input and output is in "compressed"/"unsplit" form.
	std::string cleanupFromStorageFunction(Type const& _type);

	/// @returns the name of a function that prepares a value of the given type
	/// for being stored in storage. This usually includes cleanup and right-alignment
	/// to fit the number of bytes in storage.
	/// The resulting value might still have dirty higher order bits.
	std::string prepareStoreFunction(Type const& _type);

	/// @returns the name of a function that allocates memory.
	/// Modifies the "free memory pointer"
	/// signature: (size) -> memPtr
	std::string allocationFunction();

	/// @returns the name of the function that allocates memory whose size might be defined later.
	/// The allocation can be finalized using finalizeAllocationFunction.
	/// Any other allocation will invalidate the memory pointer unless finalizeAllocationFunction
	/// is called.
	/// signature: () -> memPtr
	std::string allocateUnboundedFunction();

	/// @returns the name of the function that finalizes an unbounded memory allocation,
	/// i.e. sets its size and makes the allocation permanent.
	/// signature: (memPtr, size) ->
	std::string finalizeAllocationFunction();

	/// @returns the name of a function that zeroes an array.
	/// signature: (dataStart, dataSizeInBytes) ->
	std::string zeroMemoryArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that zeroes a chunk of memory.
	/// signature: (dataStart, dataSizeInBytes) ->
	std::string zeroMemoryFunction(Type const& _type);

	/// @returns the name of a function that zeroes an array
	/// where the base does not have simple zero value in memory.
	/// signature: (dataStart, dataSizeInBytes) ->
	std::string zeroComplexMemoryArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that allocates a memory array.
	/// For dynamic arrays it adds space for length and stores it.
	/// The contents of the data area are unspecified.
	/// signature: (length) -> memPtr
	std::string allocateMemoryArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that allocates and zeroes a memory array.
	/// For dynamic arrays it adds space for length and stores it.
	/// signature: (length) -> memPtr
	std::string allocateAndInitializeMemoryArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that allocates a memory struct (no
	/// initialization takes place).
	/// signature: () -> memPtr
	std::string allocateMemoryStructFunction(StructType const& _type);

	/// @returns the name of a function that allocates and zeroes a memory struct.
	/// signature: () -> memPtr
	std::string allocateAndInitializeMemoryStructFunction(StructType const& _type);

	/// @returns the name of the function that converts a value of type @a _from
	/// to a value of type @a _to. The resulting vale is guaranteed to be in range
	/// (i.e. "clean"). Asserts on failure.
	///
	/// This is used for data being encoded or general type conversions in the code.
	std::string conversionFunction(Type const& _from, Type const& _to);

	/// @returns the name of a function that converts bytes array to fixed bytes type
	/// signature: (array) -> value
	std::string bytesToFixedBytesConversionFunction(ArrayType const& _from, FixedBytesType const& _to);

	/// @returns the name of the cleanup function for the given type and
	/// adds its implementation to the requested functions.
	/// The cleanup function defers to the validator function with "assert"
	/// if there is no reasonable way to clean a value.
	std::string cleanupFunction(Type const& _type);

	/// @returns the name of the validator function for the given type and
	/// adds its implementation to the requested functions.
	/// @param _revertOnFailure if true, causes revert on invalid data,
	/// otherwise an assertion failure.
	///
	/// This is used for data decoded from external sources.
	std::string validatorFunction(Type const& _type, bool _revertOnFailure);

	std::string packedHashFunction(std::vector<Type const*> const& _givenTypes, std::vector<Type const*> const& _targetTypes);

	/// @returns the name of a function that reverts and uses returndata (if available)
	/// as reason string.
	std::string forwardingRevertFunction();

	std::string incrementCheckedFunction(Type const& _type);
	std::string incrementWrappingFunction(Type const& _type);
	std::string decrementCheckedFunction(Type const& _type);
	std::string decrementWrappingFunction(Type const& _type);

	std::string negateNumberCheckedFunction(Type const& _type);
	std::string negateNumberWrappingFunction(Type const& _type);

	/// @returns the name of a function that returns the zero value for the
	/// provided type.
	/// @param _splitFunctionTypes if false, returns two zeroes
	std::string zeroValueFunction(Type const& _type, bool _splitFunctionTypes = true);

	/// @returns the name of a function that will set the given storage item to
	/// zero
	/// signature: (slot, offset) ->
	std::string storageSetToZeroFunction(Type const& _type);

	/// If revertStrings is debug, @returns the name of a function that
	/// stores @param _message in memory position 0 and reverts.
	/// Otherwise returns the name of a function that uses "revert(0, 0)".
	std::string revertReasonIfDebugFunction(std::string const& _message = "");

	/// @returns the function body of ``revertReasonIfDebug``.
	/// Should only be used internally and by the old code generator.
	static std::string revertReasonIfDebugBody(
		RevertStrings _revertStrings,
		std::string const& _allocation,
		std::string const& _message
	);

	/// Reverts with ``Panic(uint256)`` and the given code.
	std::string panicFunction(util::PanicCode _code);

	/// @returns the name of a function that returns the return data selector.
	/// Returns zero if return data is too short.
	std::string returnDataSelectorFunction();

	/// @returns the name of a function that tries to abi-decode a string from offset 4 in the
	/// return data. On failure, returns 0, otherwise a pointer to the newly allocated string.
	/// Does not check the return data signature.
	/// signature: () -> ptr
	std::string tryDecodeErrorMessageFunction();

	/// @returns the name of a function that tries to abi-decode a uint256 value from offset 4 in the
	/// return data.
	/// Does not check the return data signature.
	/// signature: () -> success, value
	std::string tryDecodePanicDataFunction();

	/// Returns a function name that returns a newly allocated `bytes` array that contains the return data.
	///
	/// If returndatacopy() is not supported by the underlying target, a empty array will be returned instead.
	std::string extractReturndataFunction();

	/// @returns function name that returns constructor arguments copied to memory
	/// signature: () -> arguments
	std::string copyConstructorArgumentsToMemoryFunction(
		ContractDefinition const& _contract,
		std::string const& _creationObjectName
	);

	/// @returns the name of a function that copies code from a given address to a newly
	/// allocated byte array in memory.
	/// Signature: (address) -> mpos
	std::string externalCodeFunction();

	/// @return the name of a function that that checks if two external functions pointers are equal or not
	std::string externalFunctionPointersEqualFunction();

private:
/// @returns the name of a function that copies a struct from calldata or memory to storage
	/// signature: (slot, value) ->
	std::string copyStructToStorageFunction(StructType const& _from, StructType const& _to);

	/// Special case of conversion functions - handles all array conversions.
	std::string arrayConversionFunction(ArrayType const& _from, ArrayType const& _to);

	/// Special case of conversionFunction - handles everything that does not
	/// use exactly one variable to hold the value.
	std::string conversionFunctionSpecial(Type const& _from, Type const& _to);

	/// @returns the name of a function that reduces the size of a storage byte array by one element
	/// signature: (byteArray)
	std::string storageByteArrayPopFunction(ArrayType const& _type);

	std::string readFromMemoryOrCalldata(Type const& _type, bool _fromCalldata);

	/// @returns a function that reads a value type from storage.
	/// Performs bit mask/sign extend cleanup and appropriate left / right shift, but not validation.
	/// @param _splitFunctionTypes if false, returns the address and function signature in a
	/// single variable.
	/// @param _offset if provided, read from static offset, otherwise offset is a parameter of the Yul function.
	std::string readFromStorageValueType(Type const& _type, std::optional<size_t> _offset, bool _splitFunctionTypes);

	/// @returns a function that reads a reference type from storage to memory (performing a deep copy).
	std::string readFromStorageReferenceType(Type const& _type);

	/// @returns the name of a function that will clear given storage slot
	/// starting with given offset until the end of the slot
	/// signature: (slot, offset)
	std::string partialClearStorageSlotFunction();

	/// @returns the name of a function that will clear the given storage struct
	/// signature: (slot) ->
	std::string clearStorageStructFunction(StructType const& _type);

	/// @returns the name of a function that resizes a storage byte array
	/// signature: (array, newLen)
	std::string resizeDynamicByteArrayFunction(ArrayType const& _type);

	/// @returns the name of a function that cleans up elements of a storage byte array starting from startIndex.
	/// It will not copy elements in case of transformation to short byte array, and will not change array length.
	/// In case of startIndex is greater than len, doesn't do anything.
	/// In case of short byte array (< 32 bytes) doesn't do anything.
	/// If the first slot to be cleaned up is partially occupied, does not touch it. Cleans up only completely unused slots.
	/// signature: (array, len, startIndex)
	std::string cleanUpDynamicByteArrayEndSlotsFunction(ArrayType const& _type);

	/// @returns the name of a function that increases size of byte array
	/// when we resize byte array frextractUsedSetLenom < 32 elements to >= 32 elements or we push to byte array of size 31 copying of data will  occur
	/// signature: (array, data, oldLen, newLen)
	std::string increaseByteArraySizeFunction(ArrayType const& _type);

	/// @returns the name of a function that decreases size of byte array
	/// when we resize byte array from >= 32 elements to < 32 elements or we pop from byte array of size 32 copying of data will  occur
	/// signature: (array, data, oldLen, newLen)
	std::string decreaseByteArraySizeFunction(ArrayType const& _type);

	/// @returns the name of a function that sets size of short byte array while copying data
	/// should be called when we resize from long byte array (more than 32 elements) to short byte array
	/// signature: (array, data, len)
	std::string byteArrayTransitLongToShortFunction(ArrayType const& _type);

	/// @returns the name of a function that extracts only used part of slot that represents short byte array
	/// signature: (data, len) -> data
	std::string shortByteArrayEncodeUsedAreaSetLengthFunction();

	/// @returns the name of a function that calculates slot and offset for index
	/// Doesn't perform length checks, assumes that index is in bounds
	/// signature: (array, index)
	std::string longByteArrayStorageIndexAccessNoCheckFunction();

	langutil::EVMVersion m_evmVersion;
	RevertStrings m_revertStrings;
	MultiUseYulFunctionCollector& m_functionCollector;
};

}
