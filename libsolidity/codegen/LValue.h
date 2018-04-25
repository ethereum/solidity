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
 * @date 2015
 * LValues for use in the expression compiler.
 */

#pragma once

#include <memory>
#include <vector>
#include <libevmasm/SourceLocation.h>
#include <libsolidity/codegen/ArrayUtils.h>

namespace dev
{
namespace solidity
{

class Declaration;
class Type;
class TupleType;
class ArrayType;
class CompilerContext;
class VariableDeclaration;

/**
 * Abstract class used to retrieve, delete and store data in lvalues/variables.
 */
class LValue
{
protected:
	explicit LValue(CompilerContext& _compilerContext, Type const* _dataType = nullptr):
		m_context(_compilerContext), m_dataType(_dataType) {}

public:
	virtual ~LValue() {}
	/// @returns the number of stack slots occupied by the lvalue reference
	virtual unsigned sizeOnStack() const { return 1; }
	/// Copies the value of the current lvalue to the top of the stack and, if @a _remove is true,
	/// also removes the reference from the stack.
	/// @a _location source location of the current expression, used for error reporting.
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const = 0;
	/// Moves a value from the stack to the lvalue. Removes the value if @a _move is true.
	/// @a _location is the source location of the expression that caused this operation.
	/// Stack pre: value [lvalue_ref]
	/// Stack post: if !_move: value_of(lvalue_ref)
	virtual void storeValue(Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(), bool _move = false) const = 0;
	/// Stores zero in the lvalue. Removes the reference from the stack if @a _removeReference is true.
	/// @a _location is the source location of the requested operation
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const = 0;

protected:
	CompilerContext& m_context;
	Type const* m_dataType;
};

/**
 * Local variable that is completely stored on the stack.
 */
class StackVariable: public LValue
{
public:
	StackVariable(CompilerContext& _compilerContext, VariableDeclaration const& _declaration);

	virtual unsigned sizeOnStack() const override { return 0; }
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const override;
	virtual void storeValue(
		Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(),
		bool _move = false
	) const override;
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const override;

private:
	/// Base stack offset (@see CompilerContext::baseStackOffsetOfVariable) of the local variable.
	unsigned m_baseStackOffset;
	/// Number of stack elements occupied by the value (not the reference).
	unsigned m_size;
};

/**
 * Reference to some item in memory.
 */
class MemoryItem: public LValue
{
public:
	MemoryItem(CompilerContext& _compilerContext, Type const& _type, bool _padded = true);
	virtual unsigned sizeOnStack() const override { return 1; }
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const override;
	virtual void storeValue(
		Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(),
		bool _move = false
	) const override;
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const override;
private:
	/// Special flag to deal with byte array elements.
	bool m_padded = false;
};

/**
 * Reference to some item in storage. On the stack this is <storage key> <offset_inside_value>,
 * where 0 <= offset_inside_value < 32 and an offset of i means that the value is multiplied
 * by 2**i before storing it.
 */
class StorageItem: public LValue
{
public:
	/// Constructs the LValue and pushes the location of @a _declaration onto the stack.
	StorageItem(CompilerContext& _compilerContext, VariableDeclaration const& _declaration);
	/// Constructs the LValue and assumes that the storage reference is already on the stack.
	StorageItem(CompilerContext& _compilerContext, Type const& _type);
	virtual unsigned sizeOnStack() const override { return 2; }
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const override;
	virtual void storeValue(
		Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(),
		bool _move = false
	) const override;
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const override;
};

/**
 * Reference to a single byte inside a storage byte array.
 * Stack: <storage_ref> <byte_number>
 */
class StorageByteArrayElement: public LValue
{
public:
	/// Constructs the LValue and assumes that the storage reference is already on the stack.
	StorageByteArrayElement(CompilerContext& _compilerContext);
	virtual unsigned sizeOnStack() const override { return 2; }
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const override;
	virtual void storeValue(
		Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(),
		bool _move = false
	) const override;
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const override;
};

/**
 * Reference to the "length" member of a dynamically-sized array. This is an LValue with special
 * semantics since assignments to it might reduce its length and thus arrays members have to be
 * deleted.
 */
class StorageArrayLength: public LValue
{
public:
	/// Constructs the LValue, assumes that the reference to the array head is already on the stack.
	StorageArrayLength(CompilerContext& _compilerContext, ArrayType const& _arrayType);
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const override;
	virtual void storeValue(
		Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(),
		bool _move = false
	) const override;
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const override;

private:
	ArrayType const& m_arrayType;
};

/**
 * Tuple object that can itself hold several LValues.
 */
class TupleObject: public LValue
{
public:
	/// Constructs the LValue assuming that the other LValues are present on the stack.
	/// Empty unique_ptrs are possible if e.g. some values should be ignored during assignment.
	TupleObject(CompilerContext& _compilerContext, std::vector<std::unique_ptr<LValue>>&& _lvalues);
	virtual unsigned sizeOnStack() const override;
	virtual void retrieveValue(SourceLocation const& _location, bool _remove = false) const override;
	virtual void storeValue(
		Type const& _sourceType,
		SourceLocation const& _location = SourceLocation(),
		bool _move = false
	) const override;
	virtual void setToZero(
		SourceLocation const& _location = SourceLocation(),
		bool _removeReference = true
	) const override;

private:
	std::vector<std::unique_ptr<LValue>> m_lvalues;
};

}
}
