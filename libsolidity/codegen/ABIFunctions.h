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

#include <libsolidity/interface/EVMVersion.h>

#include <libsolidity/ast/ASTForward.h>

#include <vector>
#include <functional>
#include <map>

namespace dev {
namespace solidity {

class Type;
class ArrayType;
class StructType;
class FunctionType;
using TypePointer = std::shared_ptr<Type const>;
using TypePointers = std::vector<TypePointer>;

///
/// Class to generate encoding and decoding functions. Also maintains a collection
/// of "functions to be generated" in order to avoid generating the same function
/// multiple times.
///
/// Make sure to include the result of ``requestedFunctions()`` to a block that
/// is visible from the code that was generated here, or use named labels.
class ABIFunctions
{
public:
	explicit ABIFunctions(EVMVersion _evmVersion = EVMVersion{}) : m_evmVersion(_evmVersion) {}

	/// @returns name of an assembly function to ABI-encode values of @a _givenTypes
	/// into memory, converting the types to @a _targetTypes on the fly.
	/// Parameters are: <headStart> <value_n> ... <value_1>, i.e.
	/// the layout on the stack is <value_1> ... <value_n> <headStart> with
	/// the top of the stack on the right.
	/// The values represent stack slots. If a type occupies more or less than one
	/// stack slot, it takes exactly that number of values.
	/// Returns a pointer to the end of the area written in memory.
	/// Does not allocate memory (does not change the memory head pointer), but writes
	/// to memory starting at $headStart and an unrestricted amount after that.
	/// Assigns the end of encoded memory either to $value0 or (if that is not present)
	/// to $headStart.
	std::string tupleEncoder(
		TypePointers const& _givenTypes,
		TypePointers const& _targetTypes,
		bool _encodeAsLibraryTypes = false
	);

	/// @returns name of an assembly function to ABI-decode values of @a _types
	/// into memory. If @a _fromMemory is true, decodes from memory instead of
	/// from calldata.
	/// Can allocate memory.
	/// Inputs: <source_offset> <source_end> (layout reversed on stack)
	/// Outputs: <value0> <value1> ... <valuen>
	/// The values represent stack slots. If a type occupies more or less than one
	/// stack slot, it takes exactly that number of values.
	std::string tupleDecoder(TypePointers const& _types, bool _fromMemory = false);

	/// @returns concatenation of all generated functions.
	std::string requestedFunctions();

private:
	/// @returns the name of the cleanup function for the given type and
	/// adds its implementation to the requested functions.
	/// @param _revertOnFailure if true, causes revert on invalid data,
	/// otherwise an assertion failure.
	std::string cleanupFunction(Type const& _type, bool _revertOnFailure = false);

	/// @returns the name of the function that converts a value of type @a _from
	/// to a value of type @a _to. The resulting vale is guaranteed to be in range
	/// (i.e. "clean"). Asserts on failure.
	std::string conversionFunction(Type const& _from, Type const& _to);

	std::string cleanupCombinedExternalFunctionIdFunction();

	/// @returns a function that combines the address and selector to a single value
	/// for use in the ABI.
	std::string combineExternalFunctionIdFunction();

	/// @returns a function that splits the address and selector from a single value
	/// for use in the ABI.
	std::string splitExternalFunctionIdFunction();

	/// @returns the name of the ABI encoding function with the given type
	/// and queues the generation of the function to the requested functions.
	/// @param _fromStack if false, the input value was just loaded from storage
	/// or memory and thus might be compacted into a single slot (depending on the type).
	std::string abiEncodingFunction(
		Type const& _givenType,
		Type const& _targetType,
		bool _encodeAsLibraryTypes,
		bool _fromStack
	);
	/// Part of @a abiEncodingFunction for array target type and given calldata array.
	std::string abiEncodingFunctionCalldataArray(
		Type const& _givenType,
		Type const& _targetType,
		bool _encodeAsLibraryTypes
	);
	/// Part of @a abiEncodingFunction for array target type and given memory array or
	/// a given storage array with one item per slot.
	std::string abiEncodingFunctionSimpleArray(
		ArrayType const& _givenType,
		ArrayType const& _targetType,
		bool _encodeAsLibraryTypes
	);
	std::string abiEncodingFunctionMemoryByteArray(
		ArrayType const& _givenType,
		ArrayType const& _targetType,
		bool _encodeAsLibraryTypes
	);
	/// Part of @a abiEncodingFunction for array target type and given storage array
	/// where multiple items are packed into the same storage slot.
	std::string abiEncodingFunctionCompactStorageArray(
		ArrayType const& _givenType,
		ArrayType const& _targetType,
		bool _encodeAsLibraryTypes
	);

	/// Part of @a abiEncodingFunction for struct types.
	std::string abiEncodingFunctionStruct(
		StructType const& _givenType,
		StructType const& _targetType,
		bool _encodeAsLibraryTypes
	);

	// @returns the name of the ABI encoding function with the given type
	// and queues the generation of the function to the requested functions.
	// Case for _givenType being a string literal
	std::string abiEncodingFunctionStringLiteral(
		Type const& _givenType,
		Type const& _targetType,
		bool _encodeAsLibraryTypes
	);

	std::string abiEncodingFunctionFunctionType(
		FunctionType const& _from,
		Type const& _to,
		bool _encodeAsLibraryTypes,
		bool _fromStack
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
	/// Part of @a abiDecodingFunction for struct types.
	std::string abiDecodingFunctionStruct(StructType const& _type, bool _fromMemory);
	/// Part of @a abiDecodingFunction for array types.
	std::string abiDecodingFunctionFunctionType(FunctionType const& _type, bool _fromMemory, bool _forUseOnStack);

	/// @returns a function that copies raw bytes of dynamic length from calldata
	/// or memory to memory.
	/// Pads with zeros and might write more than exactly length.
	std::string copyToMemoryFunction(bool _fromCalldata);

	std::string shiftLeftFunction(size_t _numBits);
	std::string shiftRightFunction(size_t _numBits);
	/// @returns the name of a function that rounds its input to the next multiple
	/// of 32 or the input if it is a multiple of 32.
	std::string roundUpFunction();

	std::string arrayLengthFunction(ArrayType const& _type);
	/// @returns the name of a function that computes the number of bytes required
	/// to store an array in memory given its length (internally encoded, not ABI encoded).
	/// The function reverts for too large lengthes.
	std::string arrayAllocationSizeFunction(ArrayType const& _type);
	/// @returns the name of a function that converts a storage slot number
	/// or a memory pointer to the slot number / memory pointer for the data position of an array
	/// which is stored in that slot / memory area.
	std::string arrayDataAreaFunction(ArrayType const& _type);
	/// @returns the name of a function that advances an array data pointer to the next element.
	/// Only works for memory arrays and storage arrays that store one item per slot.
	std::string nextArrayElementFunction(ArrayType const& _type);

	/// @returns the name of a function that allocates memory.
	/// Modifies the "free memory pointer"
	/// Arguments: size
	/// Return value: pointer
	std::string allocationFunction();

	/// Helper function that uses @a _creator to create a function and add it to
	/// @a m_requestedFunctions if it has not been created yet and returns @a _name in both
	/// cases.
	std::string createFunction(std::string const& _name, std::function<std::string()> const& _creator);

	/// @returns the size of the static part of the encoding of the given types.
	static size_t headSize(TypePointers const& _targetTypes);

	/// Map from function name to code for a multi-use function.
	std::map<std::string, std::string> m_requestedFunctions;

	EVMVersion m_evmVersion;
};

}
}
