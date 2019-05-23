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
 * Component that can generate various useful Yul functions.
 */

#pragma once

#include <liblangutil/EVMVersion.h>

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <memory>
#include <string>
#include <vector>

namespace dev
{
namespace solidity
{

class Type;
class ArrayType;
class MappingType;

/**
 * Component that can generate various useful Yul functions.
 */
class YulUtilFunctions
{
public:
	explicit YulUtilFunctions(
		langutil::EVMVersion _evmVersion,
		std::shared_ptr<MultiUseYulFunctionCollector> _functionCollector
	):
		m_evmVersion(_evmVersion),
		m_functionCollector(std::move(_functionCollector))
	{}

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

	// @returns the name of a function that has the equivalent logic of an
	// `assert` or `require` call.
	std::string requireOrAssertFunction(bool _assert, Type const* _messageType = nullptr);

	/// @returns the name of a function that takes a (cleaned) value of the given value type and
	/// left-aligns it, usually for use in non-padded encoding.
	std::string leftAlignFunction(Type const& _type);

	std::string shiftLeftFunction(size_t _numBits);
	std::string shiftRightFunction(size_t _numBits);

	/// @returns the name of a function f(value, toInsert) -> newValue which replaces the
	/// _numBytes bytes starting at byte position _shiftBytes (counted from the least significant
	/// byte) by the _numBytes least significant bytes of `toInsert`.
	std::string updateByteSliceFunction(size_t _numBytes, size_t _shiftBytes);

	/// @returns the name of a function that rounds its input to the next multiple
	/// of 32 or the input if it is a multiple of 32.
	std::string roundUpFunction();

	std::string overflowCheckedUIntAddFunction(size_t _bits);

	std::string overflowCheckedUIntMulFunction(size_t _bits);

	/// @returns computes the difference between two values.
	/// Assumes the input to be in range for the type.
	std::string overflowCheckedUIntSubFunction();

	std::string arrayLengthFunction(ArrayType const& _type);
	/// @returns the name of a function that computes the number of bytes required
	/// to store an array in memory given its length (internally encoded, not ABI encoded).
	/// The function reverts for too large lengths.
	std::string arrayAllocationSizeFunction(ArrayType const& _type);
	/// @returns the name of a function that converts a storage slot number
	/// a memory pointer or a calldata pointer to the slot number / memory pointer / calldata pointer
	/// for the data position of an array which is stored in that slot / memory area / calldata area.
	std::string arrayDataAreaFunction(ArrayType const& _type);
	/// @returns the name of a function that advances an array data pointer to the next element.
	/// Only works for memory arrays, calldata arrays and storage arrays that store one item per slot.
	std::string nextArrayElementFunction(ArrayType const& _type);

	/// @returns the name of a function that performs index access for mappings.
	/// @param _mappingType the type of the mapping
	/// @param _keyType the type of the value provided
	std::string mappingIndexAccessFunction(MappingType const& _mappingType, Type const& _keyType);

	/// @returns a function that reads a value type from storage.
	/// Performs bit mask/sign extend cleanup and appropriate left / right shift, but not validation.
	/// @param _splitFunctionTypes if false, returns the address and function signature in a
	/// single variable.
	std::string readFromStorage(Type const& _type, size_t _offset, bool _splitFunctionTypes);

	/// @returns a function that extracts a value type from storage slot that has been
	/// retrieved already.
	/// Performs bit mask/sign extend cleanup and appropriate left / right shift, but not validation.
	/// @param _splitFunctionTypes if false, returns the address and function signature in a
	/// single variable.
	std::string extractFromStorageValue(Type const& _type, size_t _offset, bool _splitFunctionTypes);

	/// Performs cleanup after reading from a potentially compressed storage slot.
	/// The function does not perform any validation, it just masks or sign-extends
	/// higher order bytes or left-aligns (in case of bytesNN).
	/// The storage cleanup expects the value to be right-aligned with potentially
	/// dirty higher order bytes.
	/// @param _splitFunctionTypes if false, returns the address and function signature in a
	/// single variable.
	std::string cleanupFromStorageFunction(Type const& _type, bool _splitFunctionTypes);

	/// @returns the name of a function that prepares a value of the given type
	/// for being stored in storage. This usually includes cleanup and right-alignment
	/// to fit the number of bytes in storage.
	/// The resulting value might still have dirty higher order bits.
	std::string prepareStoreFunction(Type const& _type);

	/// @returns the name of a function that allocates memory.
	/// Modifies the "free memory pointer"
	/// Arguments: size
	/// Return value: pointer
	std::string allocationFunction();

	/// @returns the name of the function that converts a value of type @a _from
	/// to a value of type @a _to. The resulting vale is guaranteed to be in range
	/// (i.e. "clean"). Asserts on failure.
	///
	/// This is used for data being encoded or general type conversions in the code.
	std::string conversionFunction(Type const& _from, Type const& _to);

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
	std::string validatorFunction(Type const& _type, bool _revertOnFailure = false);

	std::string packedHashFunction(std::vector<Type const*> const& _givenTypes, std::vector<Type const*> const& _targetTypes);

	/// @returns the name of a function that reverts and uses returndata (if available)
	/// as reason string.
	std::string forwardingRevertFunction();

	/// @returns a string containing a comma-separated list of variable names consisting of @a _baseName suffixed
	/// with increasing integers in the range [@a _startSuffix, @a _endSuffix), if @a _startSuffix < @a _endSuffix,
	/// and with decreasing integers in the range [@a _endSuffix, @a _startSuffix), if @a _endSuffix < @a _startSuffix.
	/// If @a _startSuffix == @a _endSuffix, the empty string is returned.
	static std::string suffixedVariableNameList(std::string const& _baseName, size_t _startSuffix, size_t _endSuffix);


	std::string incrementCheckedFunction(Type const& _type);
	std::string decrementCheckedFunction(Type const& _type);

	std::string negateNumberCheckedFunction(Type const& _type);

	/// @returns the name of a function that returns the zero value for the
	/// provided type
	std::string zeroValueFunction(Type const& _type);
private:
	/// Special case of conversionFunction - handles everything that does not
	/// use exactly one variable to hold the value.
	std::string conversionFunctionSpecial(Type const& _from, Type const& _to);

	langutil::EVMVersion m_evmVersion;
	std::shared_ptr<MultiUseYulFunctionCollector> m_functionCollector;
};

}
}
