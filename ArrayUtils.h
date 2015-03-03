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
	/// Stack pre: [source_reference] target_reference
	/// Stack post: target_reference
	void copyArrayToStorage(ArrayType const& _targetType, ArrayType const& _sourceType) const;
	/// Clears the given dynamic or static array.
	/// Stack pre: reference
	/// Stack post:
	void clearArray(ArrayType const& _type) const;
	/// Clears the length and data elements of the array referenced on the stack.
	/// Stack pre: reference
	/// Stack post:
	void clearDynamicArray(ArrayType const& _type) const;
	/// Changes the size of a dynamic array and clears the tail if it is shortened.
	/// Stack pre: reference new_length
	/// Stack post:
	void resizeDynamicArray(ArrayType const& _type) const;
	/// Appends a loop that clears a sequence of storage slots of the given type (excluding end).
	/// Stack pre: end_ref start_ref
	/// Stack post: end_ref
	void clearStorageLoop(Type const& _type) const;
	/// Converts length to size (multiplies by size on stack), rounds up for byte arrays.
	/// Stack pre: length
	/// Stack post: size
	void convertLengthToSize(ArrayType const& _arrayType) const;
	/// Retrieves the length (number of elements) of the array ref on the stack. This also
	/// works for statically-sized arrays.
	/// Stack pre: reference
	/// Stack post: reference length
	void retrieveLength(ArrayType const& _arrayType) const;

private:
	CompilerContext& m_context;
};

}
}
