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
 * @author Christian <chris@ethereum.org>
 * @date 2017
 * Routines that generate Yul code related to ABI encoding, decoding and type conversions.
 */

#pragma once

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>
#include <libsolidity/codegen/YulUtilFunctions.h>

#include <liblangutil/EVMVersion.h>

#include <functional>
#include <map>
#include <set>
#include <vector>

namespace dev
{
namespace solidity
{

class Type;
class ArrayType;
class StructType;
class FunctionType;
using TypePointer = Type const*;
using TypePointers = std::vector<TypePointer>;

/**
 * Class to generate encoding and decoding functions. Also maintains a collection
 * of "functions to be generated" in order to avoid generating the same function
 * multiple times.
 *
 * Make sure to include the result of ``requestedFunctions()`` to a block that
 * is visible from the code that was generated here, or use named labels.
 */
class ABIFunctions
{
public:
	explicit ABIFunctions(
		langutil::EVMVersion _evmVersion,
		std::shared_ptr<MultiUseYulFunctionCollector> _functionCollector = std::make_shared<MultiUseYulFunctionCollector>()
	):
		m_evmVersion(_evmVersion),
		m_functionCollector(std::move(_functionCollector)),
		m_utils(_evmVersion, m_functionCollector)
	{}

	/// @returns name of an assembly function to ABI-encode values of @a _givenTypes
	/// into memory, converting the types to @a _targetTypes on the fly.
	/// Parameters are: <headStart> <value_n> ... <value_1>, i.e.
	/// the layout on the stack is <value_1> ... <value_n> <headStart> with
	/// the top of the stack on the right.
	/// The values represent stack slots. If a type occupies more or less than one
	/// stack slot, it takes exactly that number of values.
	/// Returns a pointer to the end of the area written in memory.
	/// Does not allocate memory (does not change the free memory pointer), but writes
	/// to memory starting at $headStart and an unrestricted amount after that.
	std::string tupleEncoder(
		TypePointers const& _givenTypes,
		TypePointers const& _targetTypes,
		bool _encodeAsLibraryTypes = false
	);

	/// @returns name of an assembly function to encode values of @a _givenTypes
	/// with packed encoding into memory, converting the types to @a _targetTypes on the fly.
	/// Parameters are: <memPos> <value_n> ... <value_1>, i.e.
	/// the layout on the stack is <value_1> ... <value_n> <memPos> with
	/// the top of the stack on the right.
	/// The values represent stack slots. If a type occupies more or less than one
	/// stack slot, it takes exactly that number of values.
	/// Returns a pointer to the end of the area written in memory.
	/// Does not allocate memory (does not change the free memory pointer), but writes
	/// to memory starting at memPos and an unrestricted amount after that.
	std::string tupleEncoderPacked(TypePointers const& _givenTypes, TypePointers const& _targetTypes);

	/// @returns name of an assembly function to ABI-decode values of @a _types
	/// into memory. If @a _fromMemory is true, decodes from memory instead of
	/// from calldata.
	/// Can allocate memory.
	/// Inputs: <source_offset> <source_end> (layout reversed on stack)
	/// Outputs: <value0> <value1> ... <valuen>
	/// The values represent stack slots. If a type occupies more or less than one
	/// stack slot, it takes exactly that number of values.
	std::string tupleDecoder(TypePointers const& _types, bool _fromMemory = false);

	/// @returns concatenation of all generated functions and a set of the
	/// externally used functions.
	/// Clears the internal list, i.e. calling it again will result in an
	/// empty return value.
	std::pair<std::string, std::set<std::string>> requestedFunctions();

private:
	struct EncodingOptions
	{
		/// Pad/signextend value types and bytes/string to multiples of 32 bytes.
		/// If false, data is always left-aligned.
		/// Note that this is always re-set to true for the elements of arrays and structs.
		bool padded = true;
		/// Store arrays and structs in place without "data pointer" and do not store the length.
		bool dynamicInplace = false;
		/// Only for external function types: The value is a pair of address / function id instead
		/// of a memory pointer to the compression representation.
		bool encodeFunctionFromStack = false;
		/// Encode storage pointers as storage pointers (we are targeting a library call).
		bool encodeAsLibraryTypes = false;

		/// @returns a string to uniquely identify the encoding options for the encoding
		/// function name. Skips everything that has its default value.
		std::string toFunctionNameSuffix() const;
	};

	/// Performs cleanup after reading from a potentially compressed storage slot.
	/// The function does not perform any validation, it just masks or sign-extends
	/// higher order bytes or left-aligns (in case of bytesNN).
	/// The storage cleanup expects the value to be right-aligned with potentially
	/// dirty higher order bytes.
	/// @param _splitFunctionTypes if false, returns the address and function signature in a
	/// single variable.
	std::string cleanupFromStorageFunction(Type const& _type, bool _splitFunctionTypes);

	/// @returns the name of the ABI encoding function with the given type
	/// and queues the generation of the function to the requested functions.
	/// @param _fromStack if false, the input value was just loaded from storage
	/// or memory and thus might be compacted into a single slot (depending on the type).
	std::string abiEncodingFunction(
		Type const& _givenType,
		Type const& _targetType,
		EncodingOptions const& _options
	);
	/// @returns the name of a function that internally calls `abiEncodingFunction`
	/// but always returns the updated encoding position, even if the type is
	/// statically encoded.
	std::string abiEncodeAndReturnUpdatedPosFunction(
		Type const& _givenType,
		Type const& _targetType,
		EncodingOptions const& _options
	);
	/// Part of @a abiEncodingFunction for array target type and given calldata array.
	/// Uses calldatacopy and does not perform cleanup or validation and can therefore only
	/// be used for byte arrays and arrays with the base type uint256 or bytes32.
	std::string abiEncodingFunctionCalldataArrayWithoutCleanup(
		Type const& _givenType,
		Type const& _targetType,
		EncodingOptions const& _options
	);
	/// Part of @a abiEncodingFunction for array target type and given memory array or
	/// a given storage array with one item per slot.
	std::string abiEncodingFunctionSimpleArray(
		ArrayType const& _givenType,
		ArrayType const& _targetType,
		EncodingOptions const& _options
	);
	std::string abiEncodingFunctionMemoryByteArray(
		ArrayType const& _givenType,
		ArrayType const& _targetType,
		EncodingOptions const& _options
	);
	/// Part of @a abiEncodingFunction for array target type and given storage array
	/// where multiple items are packed into the same storage slot.
	std::string abiEncodingFunctionCompactStorageArray(
		ArrayType const& _givenType,
		ArrayType const& _targetType,
		EncodingOptions const& _options
	);

	/// Part of @a abiEncodingFunction for struct types.
	std::string abiEncodingFunctionStruct(
		StructType const& _givenType,
		StructType const& _targetType,
		EncodingOptions const& _options
	);

	// @returns the name of the ABI encoding function with the given type
	// and queues the generation of the function to the requested functions.
	// Case for _givenType being a string literal
	std::string abiEncodingFunctionStringLiteral(
		Type const& _givenType,
		Type const& _targetType,
		EncodingOptions const& _options
	);

	std::string abiEncodingFunctionFunctionType(
		FunctionType const& _from,
		Type const& _to,
		EncodingOptions const& _options
	);

	/// @returns the name of the ABI decoding function for the given type
	/// and queues the generation of the function to the requested functions.
	/// The caller has to ensure that no out of bounds access (at least to the static
	/// part) can happen inside this function.
	/// @param _fromMemory if decoding from memory instead of from calldata
	/// @param _forUseOnStack if the decoded value is stored on stack or in memory.
	std::string abiDecodingFunction(
		Type const& _Type,
		bool _fromMemory,
		bool _forUseOnStack
	);

	/// Part of @a abiDecodingFunction for value types.
	std::string abiDecodingFunctionValueType(Type const& _type, bool _fromMemory);
	/// Part of @a abiDecodingFunction for "regular" array types.
	std::string abiDecodingFunctionArray(ArrayType const& _type, bool _fromMemory);
	/// Part of @a abiDecodingFunction for calldata array types.
	std::string abiDecodingFunctionCalldataArray(ArrayType const& _type);
	/// Part of @a abiDecodingFunction for byte array types.
	std::string abiDecodingFunctionByteArray(ArrayType const& _type, bool _fromMemory);
	/// Part of @a abiDecodingFunction for calldata struct types.
	std::string abiDecodingFunctionCalldataStruct(StructType const& _type);
	/// Part of @a abiDecodingFunction for struct types.
	std::string abiDecodingFunctionStruct(StructType const& _type, bool _fromMemory);
	/// Part of @a abiDecodingFunction for array types.
	std::string abiDecodingFunctionFunctionType(FunctionType const& _type, bool _fromMemory, bool _forUseOnStack);

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

	/// @returns the name of a function that retrieves an element from calldata.
	std::string calldataAccessFunction(Type const& _type);

	/// @returns the name of a function used during encoding that stores the length
	/// if the array is dynamically sized (and the options do not request in-place encoding).
	/// It returns the new encoding position.
	/// If the array is not dynamically sized (or in-place encoding was requested),
	/// does nothing and just returns the position again.
	std::string arrayStoreLengthForEncodingFunction(ArrayType const& _type, EncodingOptions const& _options);

	/// Helper function that uses @a _creator to create a function and add it to
	/// @a m_requestedFunctions if it has not been created yet and returns @a _name in both
	/// cases.
	std::string createFunction(std::string const& _name, std::function<std::string()> const& _creator);

	/// Helper function that uses @a _creator to create a function and add it to
	/// @a m_requestedFunctions if it has not been created yet and returns @a _name in both
	/// cases. Also adds it to the list of externally used functions.
	std::string createExternallyUsedFunction(std::string const& _name, std::function<std::string()> const& _creator);

	/// @returns the size of the static part of the encoding of the given types.
	static size_t headSize(TypePointers const& _targetTypes);

	/// @returns the number of variables needed to store a type.
	/// This is one for almost all types. The exception being dynamically sized calldata arrays or
	/// external function types (if we are encoding from stack, i.e. _options.encodeFunctionFromStack
	/// is true), for which it is two.
	static size_t numVariablesForType(Type const& _type, EncodingOptions const& _options);

	langutil::EVMVersion m_evmVersion;
	std::shared_ptr<MultiUseYulFunctionCollector> m_functionCollector;
	std::set<std::string> m_externallyUsedFunctions;
	YulUtilFunctions m_utils;
};

}
}
