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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity data types
 */

#pragma once

#include <libsolidity/ast/ASTEnums.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/parsing/Token.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/LazyInit.h>
#include <libsolutil/Result.h>

#include <boost/rational.hpp>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>

namespace solidity::frontend
{

class TypeProvider;
class Type; // forward
class FunctionType; // forward
using FunctionTypePointer = FunctionType const*;
using TypePointers = std::vector<Type const*>;
using rational = boost::rational<bigint>;
using TypeResult = util::Result<Type const*>;
using BoolResult = util::Result<bool>;

}

namespace solidity::frontend
{

inline rational makeRational(bigint const& _numerator, bigint const& _denominator)
{
	solAssert(_denominator != 0, "division by zero");
	// due to a bug in certain versions of boost the denominator has to be positive
	if (_denominator < 0)
		return rational(-_numerator, -_denominator);
	else
		return rational(_numerator, _denominator);
}

enum class DataLocation { Storage, CallData, Memory };


/**
 * Helper class to compute storage offsets of members of structs and contracts.
 */
class StorageOffsets
{
public:
	/// Resets the StorageOffsets objects and determines the position in storage for each
	/// of the elements of @a _types.
	void computeOffsets(TypePointers const& _types);
	/// @returns the offset of the given member, might be null if the member is not part of storage.
	std::pair<u256, unsigned> const* offset(size_t _index) const;
	/// @returns the total number of slots occupied by all members.
	u256 const& storageSize() const { return m_storageSize; }

private:
	u256 m_storageSize;
	std::map<size_t, std::pair<u256, unsigned>> m_offsets;
};

/**
 * List of members of a type.
 */
class MemberList
{
public:
	struct Member
	{
		/// Manual constructor for members that are not taken from a declaration.
		Member(char const* _name, Type const* _type):
			name(_name),
			type(_type),
			declaration(nullptr)
		{
		}

		/// Constructs a Member with the name extracted from @p _declaration's name.
		Member(Declaration const* _declaration, Type const* _type);
		Member(Declaration const* _declaration, Type const* _type, std::string _name);

		std::string name;
		Type const* type = nullptr;
		Declaration const* declaration = nullptr;
	};

	using MemberMap = std::vector<Member>;

	explicit MemberList(MemberMap _members): m_memberTypes(std::move(_members)) {}

	void combine(MemberList const& _other);
	Type const* memberType(std::string const& _name) const
	{
		Type const* type = nullptr;
		for (auto const& it: m_memberTypes)
			if (it.name == _name)
			{
				solAssert(!type, "Requested member type by non-unique name.");
				type = it.type;
			}
		return type;
	}
	MemberMap membersByName(std::string const& _name) const
	{
		MemberMap members;
		for (auto const& it: m_memberTypes)
			if (it.name == _name)
				members.push_back(it);
		return members;
	}
	/// @returns the offset of the given member in storage slots and bytes inside a slot or
	/// a nullptr if the member is not part of storage.
	std::pair<u256, unsigned> const* memberStorageOffset(std::string const& _name) const;
	/// @returns the number of storage slots occupied by the members.
	u256 const& storageSize() const;

	MemberMap::const_iterator begin() const { return m_memberTypes.begin(); }
	MemberMap::const_iterator end() const { return m_memberTypes.end(); }

private:
	StorageOffsets const& storageOffsets() const;

	MemberMap m_memberTypes;
	util::LazyInit<StorageOffsets> m_storageOffsets;
};

static_assert(std::is_nothrow_move_constructible<MemberList>::value, "MemberList should be noexcept move constructible");

/**
 * Abstract base class that forms the root of the type hierarchy.
 */
class Type
{
public:
	Type() = default;
	Type(Type const&) = delete;
	Type(Type&&) = delete;
	Type& operator=(Type const&) = delete;
	Type& operator=(Type&&) = delete;
	virtual ~Type() = default;

	enum class Category
	{
		Address, Integer, RationalNumber, StringLiteral, Bool, FixedPoint, Array, ArraySlice,
		FixedBytes, Contract, Struct, Function, Enum, UserDefinedValueType, Tuple,
		Mapping, TypeType, Modifier, Magic, Module,
		InaccessibleDynamic
	};

	/// @returns a pointer to _a or _b if the other is implicitly convertible to it or nullptr otherwise
	static Type const* commonType(Type const* _a, Type const* _b);

	virtual Category category() const = 0;
	/// @returns a valid solidity identifier such that two types should compare equal if and
	/// only if they have the same identifier.
	/// The identifier should start with "t_".
	/// Can contain characters which are invalid in identifiers.
	virtual std::string richIdentifier() const = 0;
	/// @returns a valid solidity identifier such that two types should compare equal if and
	/// only if they have the same identifier.
	/// The identifier should start with "t_".
	/// Will not contain any character which would be invalid as an identifier.
	std::string identifier() const;

	/// More complex identifier strings use "parentheses", where $_ is interpreted as
	/// "opening parenthesis", _$ as "closing parenthesis", _$_ as "comma" and any $ that
	/// appears as part of a user-supplied identifier is escaped as _$$$_.
	/// @returns an escaped identifier (will not contain any parenthesis or commas)
	static std::string escapeIdentifier(std::string const& _identifier);

	virtual BoolResult isImplicitlyConvertibleTo(Type const& _other) const { return *this == _other; }
	virtual BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const
	{
		return isImplicitlyConvertibleTo(_convertTo);
	}
	/// @returns the resulting type of applying the given unary operator or an empty pointer if
	/// this is not possible.
	/// The default implementation does not allow any unary operator.
	virtual TypeResult unaryOperatorResult(Token) const { return nullptr; }
	/// @returns the resulting type of applying the given binary operator or an empty pointer if
	/// this is not possible.
	/// The default implementation allows comparison operators if a common type exists
	virtual TypeResult binaryOperatorResult(Token _operator, Type const* _other) const
	{
		return TokenTraits::isCompareOp(_operator) ? commonType(this, _other) : nullptr;
	}

	virtual bool operator==(Type const& _other) const { return category() == _other.category(); }
	virtual bool operator!=(Type const& _other) const { return !this->operator ==(_other); }

	/// @returns number of bytes used by this type when encoded for CALL. Cannot be used for
	/// dynamically encoded types.
	/// Always returns a value greater than zero and throws if the type cannot be encoded in calldata
	/// (or is dynamically encoded).
	/// If @a _padded then it is assumed that each element is padded to a multiple of 32 bytes.
	virtual unsigned calldataEncodedSize([[maybe_unused]] bool _padded) const { solAssert(false, ""); }
	/// Convenience version of @see calldataEncodedSize(bool)
	unsigned calldataEncodedSize() const { return calldataEncodedSize(true); }
	/// @returns the distance between two elements of this type in a calldata array, tuple or struct.
	/// For statically encoded types this is the same as calldataEncodedSize(true).
	/// For dynamically encoded types this is the distance between two tail pointers, i.e. 32.
	/// Always returns a value greater than zero and throws if the type cannot be encoded in calldata.
	unsigned calldataHeadSize() const { return isDynamicallyEncoded() ? 32 : calldataEncodedSize(true); }
	/// @returns the (minimal) size of the calldata tail for this type. Can only be used for
	/// dynamically encoded types. For dynamically-sized arrays this is 32 (the size of the length),
	/// for statically-sized, but dynamically encoded arrays this is 32*length(), for structs
	/// this is the sum of the calldataHeadSize's of its members.
	/// Always returns a value greater than zero and throws if the type cannot be encoded in calldata
	/// (or is not dynamically encoded).
	virtual unsigned calldataEncodedTailSize() const { solAssert(false, ""); }
	/// @returns the size of this data type in bytes when stored in memory. For memory-reference
	/// types, this is the size of the memory pointer.
	virtual unsigned memoryHeadSize() const { return calldataEncodedSize(); }
	/// @returns the size of this data type in bytes when stored in memory. For memory-reference
	/// types, this is the size of the actual data area, if it is statically-sized.
	virtual u256 memoryDataSize() const { return calldataEncodedSize(); }
	/// @returns true if the type is a dynamic array
	virtual bool isDynamicallySized() const { return false; }
	/// @returns true if the type is dynamically encoded in the ABI
	virtual bool isDynamicallyEncoded() const { return false; }
	/// @returns the number of storage slots required to hold this value in storage.
	/// For dynamically "allocated" types, it returns the size of the statically allocated head,
	virtual u256 storageSize() const { return 1; }
	/// @returns an upper bound on the total storage size required by this type, descending
	/// into structs and statically-sized arrays. This is mainly to ensure that the storage
	/// slot allocation algorithm does not overflow, it is not a protection against collisions.
	virtual bigint storageSizeUpperBound() const { return 1; }
	/// Multiple small types can be packed into a single storage slot. If such a packing is possible
	/// this function @returns the size in bytes smaller than 32. Data is moved to the next slot if
	/// it does not fit.
	/// In order to avoid computation at runtime of whether such moving is necessary, structs and
	/// array data (not each element) always start a new slot.
	virtual unsigned storageBytes() const { return 32; }
	/// Returns true if the type is a value type that is left-aligned on the stack with a size of
	/// storageBytes() bytes. Returns false if the type is a value type that is right-aligned on
	/// the stack with a size of storageBytes() bytes. Asserts if it is not a value type or the
	/// encoding is more complicated.
	/// Signed integers are not considered "more complicated" even though they need to be
	/// sign-extended.
	virtual bool leftAligned() const { solAssert(false, "Alignment property of non-value type requested."); }
	/// Returns true if the type can be stored in storage.
	virtual bool canBeStored() const { return true; }
	/// Returns false if the type cannot live outside the storage, i.e. if it includes some mapping.
	virtual bool containsNestedMapping() const
	{
		solAssert(nameable(), "Called for a non nameable type.");
		return false;
	}
	/// Returns true if the type can be stored as a value (as opposed to a reference) on the stack,
	/// i.e. it behaves differently in lvalue context and in value context.
	virtual bool isValueType() const { return false; }
	/// @returns true if this type can be used for variables. It returns false for
	/// types like magic types, literals and function types with a kind that is not
	/// internal or external.
	virtual bool nameable() const { return false; }
	/// @returns a list of named and typed stack items that determine the layout of this type on the stack.
	/// A stack item either has an empty name and type ``nullptr`` referring to a single stack slot, or
	/// has a non-empty name and a valid type referring to the stack layout of that type.
	/// The complete layout of a type on the stack can be obtained from its stack items recursively as follows:
	/// - Each unnamed stack item is untyped (its type is ``nullptr``) and contributes exactly one stack slot.
	/// - Each named stack item is typed and contributes the stack slots given by the stack items of its type.
	std::vector<std::tuple<std::string, Type const*>> const& stackItems() const
	{
		if (!m_stackItems)
			m_stackItems = makeStackItems();
		return *m_stackItems;
	}
	/// Total number of stack slots occupied by this type. This is the sum of ``sizeOnStack`` of all ``stackItems()``.
	// TODO: consider changing the return type to be size_t
	unsigned sizeOnStack() const
	{
		if (!m_stackSize)
		{
			size_t sizeOnStack = 0;
			for (auto const& slot: stackItems())
				if (std::get<1>(slot))
					sizeOnStack += std::get<1>(slot)->sizeOnStack();
				else
					++sizeOnStack;
			m_stackSize = sizeOnStack;
		}
		return static_cast<unsigned>(*m_stackSize);
	}
	/// If it is possible to initialize such a value in memory by just writing zeros
	/// of the size memoryHeadSize().
	virtual bool hasSimpleZeroValueInMemory() const { return true; }
	/// @returns the mobile (in contrast to static) type corresponding to the given type.
	/// This returns the corresponding IntegerType or FixedPointType for RationalNumberType
	/// and the pointer type for storage reference types.
	/// Might return a null pointer if there is no fitting type.
	virtual Type const* mobileType() const { return this; }
	/// @returns true if this is a non-value type and the data of this type is stored at the
	/// given location.
	virtual bool dataStoredIn(DataLocation) const { return false; }

	/// Returns the list of all members of this type. Default implementation: no members apart from bound.
	/// @param _currentScope scope in which the members are accessed.
	MemberList const& members(ASTNode const* _currentScope) const;
	/// Convenience method, returns the type of the given named member or an empty pointer if no such member exists.
	Type const* memberType(std::string const& _name, ASTNode const* _currentScope = nullptr) const
	{
		return members(_currentScope).memberType(_name);
	}

	virtual std::string toString(bool _withoutDataLocation) const = 0;
	std::string toString() const { return toString(false); }
	/// @returns the canonical name of this type for use in library function signatures.
	virtual std::string canonicalName() const { return toString(true); }
	virtual std::string humanReadableName() const { return toString(); }
	/// @returns the signature of this type in external functions, i.e. `uint256` for integers
	/// or `(uint256,bytes8)[2]` for an array of structs. If @a _structsByName,
	/// structs are given by canonical name like `ContractName.StructName[2]`.
	virtual std::string signatureInExternalFunction(bool /*_structsByName*/) const
	{
		return canonicalName();
	}
	virtual u256 literalValue(Literal const*) const
	{
		solAssert(false, "Literal value requested for type without literals: " + toString(false));
	}

	/// @returns a (simpler) type that is encoded in the same way for external function calls.
	/// This for example returns address for contract types.
	/// If there is no such type, returns an empty shared pointer.
	virtual Type const* encodingType() const { return nullptr; }
	/// @returns the encoding type used under the given circumstances for the type of an expression
	/// when used for e.g. abi.encode(...) or the empty pointer if the object
	/// cannot be encoded.
	/// This is different from encodingType since it takes implicit conversions into account.
	Type const* fullEncodingType(bool _inLibraryCall, bool _encoderV2, bool _packed) const;
	/// @returns a (simpler) type that is used when decoding this type in calldata.
	virtual Type const* decodingType() const { return encodingType(); }
	/// @returns a type that will be used outside of Solidity for e.g. function signatures.
	/// This for example returns address for contract types.
	/// If there is no such type, returns an empty shared pointer.
	/// @param _inLibrary if set, returns types as used in a library, e.g. struct and contract types
	/// are returned without modification.
	virtual TypeResult interfaceType(bool /*_inLibrary*/) const { return nullptr; }

	/// @returns the declaration of a user defined type (enum, struct, user defined value type).
	/// Returns nullptr otherwise.
	virtual Declaration const* typeDefinition() const { return nullptr; }

	/// Clears all internally cached values (if any).
	virtual void clearCache() const;

private:
	/// @returns a member list containing all members added to this type by `using for` directives.
	static MemberList::MemberMap boundFunctions(Type const& _type, ASTNode const& _scope);

protected:
	/// @returns the members native to this type depending on the given context. This function
	/// is used (in conjunction with boundFunctions to fill m_members below.
	virtual MemberList::MemberMap nativeMembers(ASTNode const* /*_currentScope*/) const
	{
		return MemberList::MemberMap();
	}
	/// Generates the stack items to be returned by ``stackItems()``. Defaults
	/// to exactly one unnamed and untyped stack item referring to a single stack slot.
	virtual std::vector<std::tuple<std::string, Type const*>> makeStackItems() const
	{
		return {std::make_tuple(std::string(), nullptr)};
	}


	/// List of member types (parameterised by scape), will be lazy-initialized.
	mutable std::map<ASTNode const*, std::unique_ptr<MemberList>> m_members;
	mutable std::optional<std::vector<std::tuple<std::string, Type const*>>> m_stackItems;
	mutable std::optional<size_t> m_stackSize;
};

/**
 * Type for addresses.
 */
class AddressType: public Type
{
public:
	explicit AddressType(StateMutability _stateMutability);

	Category category() const override { return Category::Address; }

	std::string richIdentifier() const override;
	BoolResult isImplicitlyConvertibleTo(Type const& _other) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token _operator, Type const* _other) const override;

	bool operator==(Type const& _other) const override;

	unsigned calldataEncodedSize(bool _padded = true) const override { return _padded ? 32 : 160 / 8; }
	unsigned storageBytes() const override { return 160 / 8; }
	bool leftAligned() const override { return false; }
	bool isValueType() const override { return true; }
	bool nameable() const override { return true; }

	MemberList::MemberMap nativeMembers(ASTNode const*) const override;

	std::string toString(bool _withoutDataLocation) const override;
	std::string canonicalName() const override;

	u256 literalValue(Literal const* _literal) const override;

	Type const* encodingType() const override { return this; }
	TypeResult interfaceType(bool) const override { return this; }

	StateMutability stateMutability(void) const { return m_stateMutability; }

private:
	StateMutability m_stateMutability;
};

/**
 * Any kind of integer type (signed, unsigned).
 */
class IntegerType: public Type
{
public:
	enum class Modifier
	{
		Unsigned, Signed
	};

	explicit IntegerType(unsigned _bits, Modifier _modifier = Modifier::Unsigned);

	Category category() const override { return Category::Integer; }

	std::string richIdentifier() const override;
	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token _operator, Type const* _other) const override;

	bool operator==(Type const& _other) const override;

	unsigned calldataEncodedSize(bool _padded = true) const override { return _padded ? 32 : m_bits / 8; }
	unsigned storageBytes() const override { return m_bits / 8; }
	bool leftAligned() const override { return false; }
	bool isValueType() const override { return true; }
	bool nameable() const override { return true; }

	std::string toString(bool _withoutDataLocation) const override;

	Type const* encodingType() const override { return this; }
	TypeResult interfaceType(bool) const override { return this; }

	unsigned numBits() const { return m_bits; }
	bool isSigned() const { return m_modifier == Modifier::Signed; }

	u256 min() const;
	u256 max() const;

	bigint minValue() const;
	bigint maxValue() const;

private:
	unsigned const m_bits;
	Modifier const m_modifier;
};

/**
 * A fixed point type number (signed, unsigned).
 */
class FixedPointType: public Type
{
public:
	enum class Modifier
	{
		Unsigned, Signed
	};

	explicit FixedPointType(unsigned _totalBits, unsigned _fractionalDigits, Modifier _modifier = Modifier::Unsigned);
	Category category() const override { return Category::FixedPoint; }

	std::string richIdentifier() const override;
	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token _operator, Type const* _other) const override;

	bool operator==(Type const& _other) const override;

	unsigned calldataEncodedSize(bool _padded = true) const override { return _padded ? 32 : m_totalBits / 8; }
	unsigned storageBytes() const override { return m_totalBits / 8; }
	bool leftAligned() const override { return false; }
	bool isValueType() const override { return true; }
	bool nameable() const override { return true; }

	std::string toString(bool _withoutDataLocation) const override;

	Type const* encodingType() const override { return this; }
	TypeResult interfaceType(bool) const override { return this; }

	/// Number of bits used for this type in total.
	unsigned numBits() const { return m_totalBits; }
	/// Number of decimal digits after the radix point.
	unsigned fractionalDigits() const { return m_fractionalDigits; }
	bool isSigned() const { return m_modifier == Modifier::Signed; }
	/// @returns the largest integer value this type con hold. Note that this is not the
	/// largest value in general.
	bigint maxIntegerValue() const;
	/// @returns the smallest integer value this type can hold. Note hat this is not the
	/// smallest value in general.
	bigint minIntegerValue() const;

	/// @returns the smallest integer type that can hold this type with fractional parts shifted to integers.
	IntegerType const* asIntegerType() const;

private:
	unsigned m_totalBits;
	unsigned m_fractionalDigits;
	Modifier m_modifier;
};

/**
 * Integer and fixed point constants either literals or computed.
 * Example expressions: 2, 3.14, 2+10.2, ~10.
 * There is one distinct type per value.
 */
class RationalNumberType: public Type
{
public:
	explicit RationalNumberType(rational _value, Type const* _compatibleBytesType = nullptr):
		m_value(std::move(_value)), m_compatibleBytesType(_compatibleBytesType)
	{}

	Category category() const override { return Category::RationalNumber; }

	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token _operator, Type const* _other) const override;

	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;

	bool canBeStored() const override { return false; }

	std::string toString(bool _withoutDataLocation) const override;
	u256 literalValue(Literal const* _literal) const override;
	Type const* mobileType() const override;

	/// @returns the underlying raw literal value.
	///
	/// @see literalValue(Literal const*))
	rational const& value() const noexcept { return m_value; }

	/// @returns the smallest integer type that can hold the value or an empty pointer if not possible.
	IntegerType const* integerType() const;
	/// @returns the smallest fixed type that can hold the value or incurs the least precision loss,
	/// unless the value was truncated, then a suitable type will be chosen to indicate such event.
	/// If the integer part does not fit, returns an empty pointer.
	FixedPointType const* fixedPointType() const;

	/// @returns true if the value is not an integer.
	bool isFractional() const { return m_value.denominator() != 1; }

	/// @returns true if the value is negative.
	bool isNegative() const { return m_value < 0; }

	/// @returns true if the value is zero.
	bool isZero() const { return m_value == 0; }

	/// @returns true if the literal is a valid integer.
	static std::tuple<bool, rational> isValidLiteral(Literal const& _literal);

private:
	rational m_value;

	/// Bytes type to which the rational can be implicitly converted.
	/// Empty for all rationals that are not directly parsed from hex literals.
	Type const* m_compatibleBytesType;

	/// @returns true if the literal is a valid rational number.
	static std::tuple<bool, rational> parseRational(std::string const& _value);

	/// @returns a truncated readable representation of the bigint keeping only
	/// up to 4 leading and 4 trailing digits.
	static std::string bigintToReadableString(bigint const& num);
};

/**
 * Literal string, can be converted to bytes, bytesX or string.
 */
class StringLiteralType: public Type
{
public:
	explicit StringLiteralType(Literal const& _literal);
	explicit StringLiteralType(std::string _value);

	Category category() const override { return Category::StringLiteral; }

	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult binaryOperatorResult(Token, Type const*) const override
	{
		return nullptr;
	}

	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;

	bool canBeStored() const override { return false; }

	std::string toString(bool) const override;
	Type const* mobileType() const override;

	std::string const& value() const { return m_value; }

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override { return {}; }
private:
	std::string m_value;
};

/**
 * Bytes type with fixed length of up to 32 bytes.
 */
class FixedBytesType: public Type
{
public:
	explicit FixedBytesType(unsigned _bytes);

	Category category() const override { return Category::FixedBytes; }

	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token _operator, Type const* _other) const override;

	unsigned calldataEncodedSize(bool _padded) const override { return _padded && m_bytes > 0 ? 32 : m_bytes; }
	unsigned storageBytes() const override { return m_bytes; }
	bool leftAligned() const override { return true; }
	bool isValueType() const override { return true; }
	bool nameable() const override { return true; }

	std::string toString(bool) const override { return "bytes" + util::toString(m_bytes); }
	MemberList::MemberMap nativeMembers(ASTNode const*) const override;
	Type const* encodingType() const override { return this; }
	TypeResult interfaceType(bool) const override { return this; }

	unsigned numBytes() const { return m_bytes; }

private:
	unsigned m_bytes;
};

/**
 * The boolean type.
 */
class BoolType: public Type
{
public:
	Category category() const override { return Category::Bool; }
	std::string richIdentifier() const override { return "t_bool"; }
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token _operator, Type const* _other) const override;

	unsigned calldataEncodedSize(bool _padded) const override{ return _padded ? 32 : 1; }
	unsigned storageBytes() const override { return 1; }
	bool leftAligned() const override { return false; }
	bool isValueType() const override { return true; }
	bool nameable() const override { return true; }

	std::string toString(bool) const override { return "bool"; }
	u256 literalValue(Literal const* _literal) const override;
	Type const* encodingType() const override { return this; }
	TypeResult interfaceType(bool) const override { return this; }
};

/**
 * Base class for types which can be thought of as several elements of other types put together.
 * For example a struct is composed of its members, an array is composed of multiple copies of its
 * base element and a mapping is composed of its value type elements (note that keys are not
 * stored anywhere).
 */
class CompositeType: public Type
{
protected:
	CompositeType() = default;

public:
	/// @returns a list containing the type itself, elements of its decomposition,
	/// elements of decomposition of these elements and so on, up to non-composite types.
	/// Each type is included only once.
	std::vector<Type const*> fullDecomposition() const;

protected:
	/// @returns a list of types that together make up the data part of this type.
	/// Contains all types that will have to be implicitly stored, whenever an object of this type is stored.
	/// In particular, it returns the base type for arrays and array slices, the member types for structs,
	/// the component types for tuples and the value type for mappings
	/// (note that the key type of a mapping is *not* part of the list).
	virtual std::vector<Type const*> decomposition() const = 0;
};

/**
 * Base class used by types which are not value types and can be stored either in storage, memory
 * or calldata. This is currently used by arrays and structs.
 */
class ReferenceType: public CompositeType
{
protected:
	explicit ReferenceType(DataLocation _location): m_location(_location) {}

public:
	DataLocation location() const { return m_location; }

	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token, Type const*) const override
	{
		return nullptr;
	}
	unsigned memoryHeadSize() const override { return 32; }
	u256 memoryDataSize() const override = 0;

	unsigned calldataEncodedSize(bool) const override = 0;
	unsigned calldataEncodedTailSize() const override = 0;

	/// @returns a copy of this type with location (recursively) changed to @a _location,
	/// whereas isPointer is only shallowly changed - the deep copy is always a bound reference.
	virtual std::unique_ptr<ReferenceType> copyForLocation(DataLocation _location, bool _isPointer) const = 0;

	Type const* mobileType() const override { return withLocation(m_location, true); }
	bool dataStoredIn(DataLocation _location) const override { return m_location == _location; }
	bool hasSimpleZeroValueInMemory() const override { return false; }

	/// Storage references can be pointers or bound references. In general, local variables are of
	/// pointer type, state variables are bound references. Assignments to pointers or deleting
	/// them will not modify storage (that will only change the pointer). Assignment from
	/// non-storage objects to a variable of storage pointer type is not possible.
	/// For anything other than storage, this always returns true because assignments
	/// never change the contents of the original value.
	bool isPointer() const;

	/// @returns true if this is valid to be stored in data location _loc
	/// The function mostly checks sizes. For calldata, this should only be called
	/// if the type has an interfaceType.
	virtual BoolResult validForLocation(DataLocation _loc) const = 0;

	bool operator==(ReferenceType const& _other) const
	{
		return location() == _other.location() && isPointer() == _other.isPointer();
	}

	Type const* withLocation(DataLocation _location, bool _isPointer) const;

protected:
	Type const* copyForLocationIfReference(Type const* _type) const;
	/// @returns a human-readable description of the reference part of the type.
	std::string stringForReferencePart() const;
	/// @returns the suffix computed from the reference part to be used by identifier();
	std::string identifierLocationSuffix() const;

	DataLocation m_location = DataLocation::Storage;
	bool m_isPointer = true;
};

/**
 * The type of an array. The flavours are byte array (bytes), statically- (<type>[<length>])
 * and dynamically-sized array (<type>[]).
 * In storage, all arrays are packed tightly (as long as more than one elementary type fits in
 * one slot). Dynamically sized arrays (including byte arrays) start with their size as a uint and
 * thus start on their own slot.
 */
class ArrayType: public ReferenceType
{
public:
	/// Constructor for a byte array ("bytes") and string.
	explicit ArrayType(DataLocation _location, bool _isString = false);

	/// Constructor for a dynamically sized array type ("type[]")
	ArrayType(DataLocation _location, Type const* _baseType):
		ReferenceType(_location),
		m_baseType(copyForLocationIfReference(_baseType))
	{
	}

	/// Constructor for a fixed-size array type ("type[20]")
	ArrayType(DataLocation _location, Type const* _baseType, u256 _length):
		ReferenceType(_location),
		m_baseType(copyForLocationIfReference(_baseType)),
		m_hasDynamicLength(false),
		m_length(std::move(_length))
	{}

	Category category() const override { return Category::Array; }

	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	unsigned calldataEncodedSize(bool) const override;
	unsigned calldataEncodedTailSize() const override;
	bool isDynamicallySized() const override { return m_hasDynamicLength; }
	bool isDynamicallyEncoded() const override;
	bigint storageSizeUpperBound() const override;
	u256 storageSize() const override;
	bool containsNestedMapping() const override { return m_baseType->containsNestedMapping(); }
	bool nameable() const override { return true; }

	std::string toString(bool _withoutDataLocation) const override;
	std::string humanReadableName() const override;
	std::string canonicalName() const override;
	std::string signatureInExternalFunction(bool _structsByName) const override;
	MemberList::MemberMap nativeMembers(ASTNode const* _currentScope) const override;
	Type const* encodingType() const override;
	Type const* decodingType() const override;
	TypeResult interfaceType(bool _inLibrary) const override;

	BoolResult validForLocation(DataLocation _loc) const override;

	/// @returns true if this is a byte array.
	bool isByteArray() const { return m_arrayKind == ArrayKind::Bytes; }
	/// @returns true if this is a byte array or a string
	bool isByteArrayOrString() const { return m_arrayKind != ArrayKind::Ordinary; }
	/// @returns true if this is a string
	bool isString() const { return m_arrayKind == ArrayKind::String; }
	Type const* baseType() const { solAssert(!!m_baseType, ""); return m_baseType; }
	Type const* finalBaseType(bool breakIfDynamicArrayType) const;
	u256 const& length() const { return m_length; }
	u256 memoryDataSize() const override;

	std::unique_ptr<ReferenceType> copyForLocation(DataLocation _location, bool _isPointer) const override;

	/// The offset to advance in calldata to move from one array element to the next.
	unsigned calldataStride() const { return isByteArrayOrString() ? 1 : m_baseType->calldataHeadSize(); }
	/// The offset to advance in memory to move from one array element to the next.
	unsigned memoryStride() const { return isByteArrayOrString() ? 1 : m_baseType->memoryHeadSize(); }
	/// The offset to advance in storage to move from one array element to the next.
	unsigned storageStride() const { return isByteArrayOrString() ? 1 : m_baseType->storageBytes(); }

	void clearCache() const override;

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
	std::vector<Type const*> decomposition() const override { return {m_baseType}; }

private:
	enum class ArrayKind { Ordinary, Bytes, String };

	bigint unlimitedStaticCalldataSize(bool _padded) const;

	///< Byte arrays ("bytes") and strings have different semantics from ordinary arrays.
	ArrayKind m_arrayKind = ArrayKind::Ordinary;
	Type const* m_baseType;
	bool m_hasDynamicLength = true;
	u256 m_length;
	mutable std::optional<TypeResult> m_interfaceType;
	mutable std::optional<TypeResult> m_interfaceType_library;
};

class ArraySliceType: public ReferenceType
{
public:
	explicit ArraySliceType(ArrayType const& _arrayType): ReferenceType(_arrayType.location()), m_arrayType(_arrayType) {}
	Category category() const override { return Category::ArraySlice; }

	BoolResult isImplicitlyConvertibleTo(Type const& _other) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	unsigned calldataEncodedSize(bool) const override { solAssert(false, ""); }
	unsigned calldataEncodedTailSize() const override { return 32; }
	bool isDynamicallySized() const override { return true; }
	bool isDynamicallyEncoded() const override { return true; }
	std::string toString(bool _withoutDataLocation) const override;
	std::string humanReadableName() const override;
	Type const* mobileType() const override;

	BoolResult validForLocation(DataLocation _loc) const override { return m_arrayType.validForLocation(_loc); }

	ArrayType const& arrayType() const { return m_arrayType; }
	u256 memoryDataSize() const override { solAssert(false, ""); }

	std::unique_ptr<ReferenceType> copyForLocation(DataLocation, bool) const override { solAssert(false, ""); }

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
	std::vector<Type const*> decomposition() const override { return {m_arrayType.baseType()}; }

private:
	ArrayType const& m_arrayType;
};

/**
 * The type of a contract instance or library, there is one distinct type for each contract definition.
 */
class ContractType: public Type
{
public:
	explicit ContractType(ContractDefinition const& _contract, bool _super = false):
		m_contract(_contract), m_super(_super) {}

	Category category() const override { return Category::Contract; }
	/// Contracts can be implicitly converted only to base contracts.
	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	/// Contracts can only be explicitly converted to address types and base contracts.
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	unsigned calldataEncodedSize(bool _padded ) const override
	{
		solAssert(!isSuper(), "");
		return encodingType()->calldataEncodedSize(_padded);
	}
	unsigned storageBytes() const override { solAssert(!isSuper(), ""); return 20; }
	bool leftAligned() const override { solAssert(!isSuper(), ""); return false; }
	bool isValueType() const override { return !isSuper(); }
	bool nameable() const override { return !isSuper(); }
	std::string toString(bool _withoutDataLocation) const override;
	std::string canonicalName() const override;

	MemberList::MemberMap nativeMembers(ASTNode const* _currentScope) const override;

	Type const* encodingType() const override;

	TypeResult interfaceType(bool _inLibrary) const override
	{
		if (isSuper())
			return nullptr;
		return _inLibrary ? this : encodingType();
	}

	/// See documentation of m_super
	bool isSuper() const { return m_super; }

	// @returns true if and only if the contract has a receive ether function or a payable fallback function, i.e.
	// if it has code that will be executed on plain ether transfers
	bool isPayable() const;

	ContractDefinition const& contractDefinition() const { return m_contract; }

	/// Returns the function type of the constructor modified to return an object of the contract's type.
	FunctionType const* newExpressionType() const;

	/// @returns a list of all state variables (including inherited) of the contract and their
	/// offsets in storage.
	std::vector<std::tuple<VariableDeclaration const*, u256, unsigned>> stateVariables() const;
	/// @returns a list of all immutable variables (including inherited) of the contract.
	std::vector<VariableDeclaration const*> immutableVariables() const;
protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
private:
	ContractDefinition const& m_contract;
	/// If true, this is a special "super" type of m_contract containing only members that m_contract inherited
	bool m_super = false;
	/// Type of the constructor, @see constructorType. Lazily initialized.
	mutable FunctionType const* m_constructorType = nullptr;
};

/**
 * The type of a struct instance, there is one distinct type per struct definition.
 */
class StructType: public ReferenceType
{
public:
	explicit StructType(StructDefinition const& _struct, DataLocation _location = DataLocation::Storage):
		ReferenceType(_location), m_struct(_struct) {}

	Category category() const override { return Category::Struct; }
	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	unsigned calldataEncodedSize(bool) const override;
	unsigned calldataEncodedTailSize() const override;
	bool isDynamicallyEncoded() const override;
	u256 memoryDataSize() const override;
	bigint storageSizeUpperBound() const override;
	u256 storageSize() const override;
	bool containsNestedMapping() const override;
	bool nameable() const override { return true; }
	std::string toString(bool _withoutDataLocation) const override;

	MemberList::MemberMap nativeMembers(ASTNode const* _currentScope) const override;

	Type const* encodingType() const override;
	TypeResult interfaceType(bool _inLibrary) const override;

	Declaration const* typeDefinition() const override;

	BoolResult validForLocation(DataLocation _loc) const override;

	bool recursive() const;

	std::unique_ptr<ReferenceType> copyForLocation(DataLocation _location, bool _isPointer) const override;

	std::string canonicalName() const override;
	std::string signatureInExternalFunction(bool _structsByName) const override;

	/// @returns a function that performs the type conversion between a list of struct members
	/// and a memory struct of this type.
	FunctionType const* constructorType() const;

	std::pair<u256, unsigned> const& storageOffsetsOfMember(std::string const& _name) const;
	u256 memoryOffsetOfMember(std::string const& _name) const;
	unsigned calldataOffsetOfMember(std::string const& _name) const;

	StructDefinition const& structDefinition() const { return m_struct; }

	/// @returns the vector of types of members available in memory.
	TypePointers memoryMemberTypes() const;

	void clearCache() const override;

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
	std::vector<Type const*> decomposition() const override;

private:
	StructDefinition const& m_struct;
	// Caches for interfaceType(bool)
	mutable std::optional<TypeResult> m_interfaceType;
	mutable std::optional<TypeResult> m_interfaceType_library;
};

/**
 * The type of an enum instance, there is one distinct type per enum definition.
 */
class EnumType: public Type
{
public:
	explicit EnumType(EnumDefinition const& _enum): m_enum(_enum) {}

	Category category() const override { return Category::Enum; }
	TypeResult unaryOperatorResult(Token _operator) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	unsigned calldataEncodedSize(bool _padded) const override
	{
		return encodingType()->calldataEncodedSize(_padded);
	}
	unsigned storageBytes() const override;
	bool leftAligned() const override { return false; }
	std::string toString(bool _withoutDataLocation) const override;
	std::string canonicalName() const override;
	bool isValueType() const override { return true; }
	bool nameable() const override { return true; }

	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	Type const* encodingType() const override;
	TypeResult interfaceType(bool _inLibrary) const override
	{
		return _inLibrary ? this : encodingType();
	}

	Declaration const* typeDefinition() const override;

	EnumDefinition const& enumDefinition() const { return m_enum; }
	/// @returns the value that the string has in the Enum
	unsigned int memberValue(ASTString const& _member) const;
	size_t numberOfMembers() const;
	unsigned int minValue() const { return 0; }
	unsigned int maxValue() const
	{
		solAssert(numberOfMembers() <= 256, "");
		return static_cast<unsigned int>(numberOfMembers()) - 1;
	}

private:
	EnumDefinition const& m_enum;
};

/**
 * The type of a UserDefinedValueType.
 */
class UserDefinedValueType: public Type
{
public:
	explicit UserDefinedValueType(UserDefinedValueTypeDefinition const& _definition):
		m_definition(_definition)
	{}

	Category category() const override { return Category::UserDefinedValueType; }
	Type const& underlyingType() const;
	UserDefinedValueTypeDefinition const& definition() const { return m_definition; }

	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	Type const* encodingType() const override { return &underlyingType(); }
	TypeResult interfaceType(bool /* _inLibrary */) const override {return &underlyingType(); }

	Declaration const* typeDefinition() const override;

	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;

	unsigned calldataEncodedSize(bool _padded) const override { return underlyingType().calldataEncodedSize(_padded); }

	bool leftAligned() const override { return underlyingType().leftAligned(); }
	bool canBeStored() const override { return underlyingType().canBeStored(); }
	u256 storageSize() const override { return underlyingType().storageSize(); }
	unsigned storageBytes() const override { return underlyingType().storageBytes(); }

	bool isValueType() const override { return true; }
	bool nameable() const override
	{
		solAssert(underlyingType().nameable(), "");
		return true;
	}

	bool containsNestedMapping() const override
	{
		solAssert(nameable(), "Called for a non nameable type.");
		// DeclarationTypeChecker::endVisit(VariableDeclaration const&)
		// assumes that this will never be true.
		solAssert(!underlyingType().containsNestedMapping(), "");
		return false;
	}

	bool hasSimpleZeroValueInMemory() const override
	{
		solAssert(underlyingType().hasSimpleZeroValueInMemory(), "");
		return true;
	}

	bool dataStoredIn(DataLocation _loc) const override
	{
		solAssert(!underlyingType().dataStoredIn(_loc), "");
		return false;
	}

	std::string toString(bool _withoutDataLocation) const override;
	std::string canonicalName() const override;
	std::string signatureInExternalFunction(bool) const override { solAssert(false, ""); }

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;

private:
	UserDefinedValueTypeDefinition const& m_definition;
};

/**
 * Type that can hold a finite sequence of values of different types.
 * In some cases, the components are empty pointers (when used as placeholders).
 */
class TupleType: public CompositeType
{
public:
	explicit TupleType(std::vector<Type const*> _types = {}): m_components(std::move(_types)) {}

	Category category() const override { return Category::Tuple; }

	BoolResult isImplicitlyConvertibleTo(Type const& _other) const override;
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	std::string toString(bool _withoutDataLocation) const override;
	std::string humanReadableName() const override;
	bool canBeStored() const override { return false; }
	u256 storageSize() const override;
	bool hasSimpleZeroValueInMemory() const override { return false; }
	Type const* mobileType() const override;

	std::vector<Type const*> const& components() const { return m_components; }

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
	std::vector<Type const*> decomposition() const override
	{
		// Currently calling TupleType::decomposition() is not expected, because we cannot declare a variable of a tuple type.
		// If that changes, before removing the solAssert, make sure the function does the right thing and is used properly.
		// Note that different tuple members can have different data locations, so using decomposition() to check
		// the tuple validity for a data location might require special care.
		solUnimplemented("Tuple decomposition is not expected.");
		return m_components;
	}

private:
	std::vector<Type const*> const m_components;
};

/**
 * The type of a function, identified by its (return) parameter types.
 * @todo the return parameters should also have names, i.e. return parameters should be a struct
 * type.
 */
class FunctionType: public Type
{
public:
	/// How this function is invoked on the EVM.
	enum class Kind
	{
		Internal, ///< stack-call using plain JUMP
		External, ///< external call using CALL
		DelegateCall, ///< external call using DELEGATECALL, i.e. not exchanging the storage
		BareCall, ///< CALL without function hash
		BareCallCode, ///< CALLCODE without function hash
		BareDelegateCall, ///< DELEGATECALL without function hash
		BareStaticCall, ///< STATICCALL without function hash
		Creation, ///< external call using CREATE
		Send, ///< CALL, but without data and gas
		Transfer, ///< CALL, but without data and throws on error
		KECCAK256, ///< KECCAK256
		Selfdestruct, ///< SELFDESTRUCT
		Revert, ///< REVERT
		ECRecover, ///< CALL to special contract for ecrecover
		SHA256, ///< CALL to special contract for sha256
		RIPEMD160, ///< CALL to special contract for ripemd160
		Event, ///< syntactic sugar for LOG*
		Error, ///< creating an error instance in revert or require
		Wrap, ///< customType.wrap(...) for user defined value types
		Unwrap, ///< customType.unwrap(...) for user defined value types
		SetGas, ///< modify the default gas value for the function call
		SetValue, ///< modify the default value transfer for the function call
		BlockHash, ///< BLOCKHASH
		AddMod, ///< ADDMOD
		MulMod, ///< MULMOD
		ArrayPush, ///< .push() to a dynamically sized array in storage
		ArrayPop, ///< .pop() from a dynamically sized array in storage
		BytesConcat, ///< .concat() on bytes (type type)
		StringConcat, ///< .concat() on string (type type)
		ObjectCreation, ///< array creation using new
		Assert, ///< assert()
		Require, ///< require()
		ABIEncode,
		ABIEncodePacked,
		ABIEncodeWithSelector,
		ABIEncodeCall,
		ABIEncodeWithSignature,
		ABIDecode,
		GasLeft, ///< gasleft()
		MetaType, ///< type(...)
		/// Refers to a function declaration without calling context
		/// (i.e. when accessed directly via the name of the containing contract).
		/// Cannot be called.
		Declaration,
	};
	struct Options
	{
		/// true iff the function takes an arbitrary number of arguments of arbitrary types
		bool arbitraryParameters = false;
		/// true iff the gas value to be used is on the stack
		bool gasSet = false;
		/// true iff the value to be sent is on the stack
		bool valueSet = false;
		/// iff the salt value (for create2) to be used is on the stack
		bool saltSet = false;
		/// true iff the function is called as arg1.fun(arg2, ..., argn).
		/// This is achieved through the "using for" directive.
		bool bound = false;

		static Options withArbitraryParameters()
		{
			Options result;
			result.arbitraryParameters = true;
			return result;
		}
		static Options fromFunctionType(FunctionType const& _type)
		{
			Options result;
			result.arbitraryParameters = _type.takesArbitraryParameters();
			result.gasSet = _type.gasSet();
			result.valueSet = _type.valueSet();
			result.saltSet = _type.saltSet();
			result.bound = _type.bound();
			return result;
		}
	};


	/// Creates the type of a function.
	/// @arg _kind must be Kind::Internal, Kind::External or Kind::Declaration.
	explicit FunctionType(FunctionDefinition const& _function, Kind _kind = Kind::Declaration);
	/// Creates the accessor function type of a state variable.
	explicit FunctionType(VariableDeclaration const& _varDecl);
	/// Creates the function type of an event.
	explicit FunctionType(EventDefinition const& _event);
	explicit FunctionType(ErrorDefinition const& _error);
	/// Creates the type of a function type name.
	explicit FunctionType(FunctionTypeName const& _typeName);
	/// Function type constructor to be used for a plain type (not derived from a declaration).
	FunctionType(
		strings const& _parameterTypes,
		strings const& _returnParameterTypes,
		Kind _kind,
		StateMutability _stateMutability = StateMutability::NonPayable,
		Options _options = Options{false, false, false, false, false}
	): FunctionType(
		parseElementaryTypeVector(_parameterTypes),
		parseElementaryTypeVector(_returnParameterTypes),
		strings(_parameterTypes.size(), ""),
		strings(_returnParameterTypes.size(), ""),
		_kind,
		_stateMutability,
		nullptr,
		std::move(_options)
	)
	{
		// In this constructor, only the "arbitrary Parameters" option should be used.
		solAssert(!bound() && !gasSet() && !valueSet() && !saltSet());
	}

	/// Detailed constructor, use with care.
	FunctionType(
		TypePointers _parameterTypes,
		TypePointers _returnParameterTypes,
		strings _parameterNames = strings(),
		strings _returnParameterNames = strings(),
		Kind _kind = Kind::Internal,
		StateMutability _stateMutability = StateMutability::NonPayable,
		Declaration const* _declaration = nullptr,
		Options _options = Options{false, false, false, false, false}
	):
		m_parameterTypes(std::move(_parameterTypes)),
		m_returnParameterTypes(std::move(_returnParameterTypes)),
		m_parameterNames(std::move(_parameterNames)),
		m_returnParameterNames(std::move(_returnParameterNames)),
		m_kind(_kind),
		m_stateMutability(_stateMutability),
		m_declaration(_declaration),
		m_options(std::move(_options))
	{
		solAssert(
			m_parameterNames.size() == m_parameterTypes.size(),
			"Parameter names list must match parameter types list!"
		);
		solAssert(
			m_returnParameterNames.size() == m_returnParameterTypes.size(),
			"Return parameter names list must match return parameter types list!"
		);
		solAssert(
			!bound() || !m_parameterTypes.empty(),
			"Attempted construction of bound function without self type"
		);
	}

	Category category() const override { return Category::Function; }

	/// @returns the type of the "new Contract" function, i.e. basically the constructor.
	static FunctionTypePointer newExpressionType(ContractDefinition const& _contract);

	TypePointers parameterTypes() const;
	TypePointers const& parameterTypesIncludingSelf() const;
	std::vector<std::string> parameterNames() const;
	TypePointers const& returnParameterTypes() const { return m_returnParameterTypes; }
	/// @returns the list of return parameter types. All dynamically-sized types (this excludes
	/// storage pointers) are replaced by InaccessibleDynamicType instances.
	TypePointers returnParameterTypesWithoutDynamicTypes() const;
	std::vector<std::string> const& returnParameterNames() const { return m_returnParameterNames; }
	/// @returns the "self" parameter type for a bound function
	Type const* selfType() const;

	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	BoolResult isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	TypeResult unaryOperatorResult(Token _operator) const override;
	TypeResult binaryOperatorResult(Token, Type const*) const override;
	std::string canonicalName() const override;
	std::string humanReadableName() const override;
	std::string toString(bool _withoutDataLocation) const override;
	unsigned calldataEncodedSize(bool _padded) const override;
	bool canBeStored() const override { return m_kind == Kind::Internal || m_kind == Kind::External; }
	u256 storageSize() const override;
	bool leftAligned() const override;
	unsigned storageBytes() const override;
	bool isValueType() const override { return true; }
	bool nameable() const override;
	bool hasSimpleZeroValueInMemory() const override { return false; }
	MemberList::MemberMap nativeMembers(ASTNode const* _currentScope) const override;
	Type const* encodingType() const override;
	TypeResult interfaceType(bool _inLibrary) const override;
	Type const* mobileType() const override;

	/// @returns Type const* of a new FunctionType object. All input/return parameters are an
	/// appropriate external types (i.e. the interfaceType()s) of input/return parameters of
	/// current function.
	/// @returns an empty shared pointer if one of the input/return parameters does not have an
	/// external type.
	FunctionTypePointer interfaceFunctionType() const;

	/// @returns true if this function can take the given arguments (possibly
	/// after implicit conversion).
	/// @param _selfType if the function is bound, this has to be supplied and is the type of the
	/// expression the function is called on.
	bool canTakeArguments(
		FuncCallArguments const& _arguments,
		Type const* _selfType = nullptr
	) const;

	/// @returns true if the types of parameters are equal (does not check return parameter types)
	bool hasEqualParameterTypes(FunctionType const& _other) const;
	/// @returns true iff the return types are equal (does not check parameter types)
	bool hasEqualReturnTypes(FunctionType const& _other) const;
	/// @returns true iff the function type is equal to the given type, ignoring state mutability differences.
	bool equalExcludingStateMutability(FunctionType const& _other) const;

	/// @returns true if the ABI is NOT used for this call (only meaningful for external calls)
	bool isBareCall() const;
	Kind const& kind() const { return m_kind; }
	StateMutability stateMutability() const { return m_stateMutability; }
	/// @returns the external signature of this function type given the function name
	std::string externalSignature() const;
	/// @returns the external identifier of this function (the hash of the signature).
	u256 externalIdentifier() const;
	/// @returns the external identifier of this function (the hash of the signature) as a hex string.
	std::string externalIdentifierHex() const;
	Declaration const& declaration() const
	{
		solAssert(m_declaration, "Requested declaration from a FunctionType that has none");
		return *m_declaration;
	}
	bool hasDeclaration() const { return !!m_declaration; }
	/// @returns true if the result of this function only depends on its arguments,
	/// does not modify the state and is a compile-time constant.
	/// Currently, this will only return true for internal functions like keccak and ecrecover.
	bool isPure() const;
	bool isPayable() const { return m_stateMutability == StateMutability::Payable; }
	/// @return A shared pointer of StructuredDocumentation.
	/// Can contain a nullptr in which case indicates absence of documentation.
	ASTPointer<StructuredDocumentation> documentation() const;

	/// true iff arguments are to be padded to multiples of 32 bytes for external calls
	/// The only functions that do not pad are hash functions, the low-level call functions
	/// and abi.encodePacked.
	bool padArguments() const;
	bool takesArbitraryParameters() const { return m_options.arbitraryParameters; }
	/// true iff the function takes a single bytes parameter and it is passed on without padding.
	bool takesSinglePackedBytesParameter() const
	{
		switch (m_kind)
		{
		case FunctionType::Kind::KECCAK256:
		case FunctionType::Kind::SHA256:
		case FunctionType::Kind::RIPEMD160:
		case FunctionType::Kind::BareCall:
		case FunctionType::Kind::BareCallCode:
		case FunctionType::Kind::BareDelegateCall:
		case FunctionType::Kind::BareStaticCall:
			return true;
		default:
			return false;
		}
	}

	bool gasSet() const { return m_options.gasSet; }
	bool valueSet() const { return m_options.valueSet; }
	bool saltSet() const { return m_options.saltSet; }
	bool bound() const { return m_options.bound; }

	/// @returns a copy of this type, where gas or value are set manually. This will never set one
	/// of the parameters to false.
	Type const* copyAndSetCallOptions(bool _setGas, bool _setValue, bool _setSalt) const;

	/// @returns a copy of this function type with the `bound` flag set to true.
	/// Should only be called on library functions.
	FunctionTypePointer asBoundFunction() const;

	/// @returns a copy of this function type where the location of reference types is changed
	/// from CallData to Memory. This is the type that would be used when the function is
	/// called externally, as opposed to the parameter types that are available inside the function body.
	/// Also supports variants to be used for library or bound calls.
	/// @param _inLibrary if true, uses DelegateCall as location.
	FunctionTypePointer asExternallyCallableFunction(bool _inLibrary) const;

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
private:
	static TypePointers parseElementaryTypeVector(strings const& _types);

	TypePointers m_parameterTypes;
	TypePointers m_returnParameterTypes;
	std::vector<std::string> m_parameterNames;
	std::vector<std::string> m_returnParameterNames;
	Kind const m_kind;
	StateMutability m_stateMutability = StateMutability::NonPayable;
	Declaration const* m_declaration = nullptr;
	Options const m_options;
};

/**
 * The type of a mapping, there is one distinct type per key/value type pair.
 * Mappings always occupy their own storage slot, but do not actually use it.
 */
class MappingType: public CompositeType
{
public:
	MappingType(Type const* _keyType, Type const* _valueType):
		m_keyType(_keyType), m_valueType(_valueType) {}

	Category category() const override { return Category::Mapping; }

	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	std::string toString(bool _withoutDataLocation) const override;
	std::string canonicalName() const override;
	bool containsNestedMapping() const override { return true; }
	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	Type const* encodingType() const override;
	TypeResult interfaceType(bool _inLibrary) const override;
	bool dataStoredIn(DataLocation _location) const override { return _location == DataLocation::Storage; }
	/// Cannot be stored in memory, but just in case.
	bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	bool nameable() const override { return true; }

	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;

	Type const* keyType() const { return m_keyType; }
	Type const* valueType() const { return m_valueType; }

protected:
	std::vector<Type const*> decomposition() const override { return {m_valueType}; }

private:
	Type const* m_keyType;
	Type const* m_valueType;
};

/**
 * The type of a type reference. The type of "uint32" when used in "a = uint32(2)" is an example
 * of a TypeType.
 * For super contracts or libraries, this has members directly.
 */
class TypeType: public Type
{
public:
	explicit TypeType(Type const* _actualType): m_actualType(_actualType) {}

	Category category() const override { return Category::TypeType; }
	Type const* actualType() const { return m_actualType; }

	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	bool canBeStored() const override { return false; }
	u256 storageSize() const override;
	bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	std::string toString(bool _withoutDataLocation) const override { return "type(" + m_actualType->toString(_withoutDataLocation) + ")"; }
	MemberList::MemberMap nativeMembers(ASTNode const* _currentScope) const override;

	BoolResult isExplicitlyConvertibleTo(Type const& _convertTo) const override;
protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override;
private:
	Type const* m_actualType;
};


/**
 * The type of a function modifier. Not used for anything for now.
 */
class ModifierType: public Type
{
public:
	explicit ModifierType(ModifierDefinition const& _modifier);

	Category category() const override { return Category::Modifier; }

	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	bool canBeStored() const override { return false; }
	u256 storageSize() const override;
	bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	std::string toString(bool _withoutDataLocation) const override;
protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override { return {}; }
private:
	TypePointers m_parameterTypes;
};



/**
 * Special type for imported modules. These mainly give access to their scope via members.
 */
class ModuleType: public Type
{
public:
	explicit ModuleType(SourceUnit const& _source): m_sourceUnit(_source) {}

	Category category() const override { return Category::Module; }

	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	bool canBeStored() const override { return false; }
	bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	MemberList::MemberMap nativeMembers(ASTNode const*) const override;

	std::string toString(bool _withoutDataLocation) const override;

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override { return {}; }
private:
	SourceUnit const& m_sourceUnit;
};

/**
 * Special type for magic variables (block, msg, tx, type(...)), similar to a struct but without any reference.
 */
class MagicType: public Type
{
public:
	enum class Kind {
		Block, ///< "block"
		Message, ///< "msg"
		Transaction, ///< "tx"
		ABI, ///< "abi"
		MetaType ///< "type(...)"
	};

public:
	explicit MagicType(Kind _kind): m_kind(_kind) {}
	explicit MagicType(Type const* _metaTypeArg): m_kind{Kind::MetaType}, m_typeArgument{_metaTypeArg} {}

	Category category() const override { return Category::Magic; }

	TypeResult binaryOperatorResult(Token, Type const*) const override
	{
		return nullptr;
	}

	std::string richIdentifier() const override;
	bool operator==(Type const& _other) const override;
	bool canBeStored() const override { return false; }
	bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	MemberList::MemberMap nativeMembers(ASTNode const*) const override;

	std::string toString(bool _withoutDataLocation) const override;

	Kind kind() const { return m_kind; }

	Type const* typeArgument() const;

protected:
	std::vector<std::tuple<std::string, Type const*>> makeStackItems() const override { return {}; }
private:
	Kind m_kind;
	/// Contract type used for contract metadata magic.
	Type const* m_typeArgument;
};

/**
 * Special type that is used for dynamic types in returns from external function calls
 * (The EVM currently cannot access dynamically-sized return values).
 */
class InaccessibleDynamicType: public Type
{
public:
	Category category() const override { return Category::InaccessibleDynamic; }

	std::string richIdentifier() const override { return "t_inaccessible"; }
	BoolResult isImplicitlyConvertibleTo(Type const&) const override { return false; }
	BoolResult isExplicitlyConvertibleTo(Type const&) const override { return false; }
	TypeResult binaryOperatorResult(Token, Type const*) const override { return nullptr; }
	unsigned calldataEncodedSize(bool) const override { return 32; }
	bool canBeStored() const override { return false; }
	bool isValueType() const override { return true; }
	bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	std::string toString(bool) const override { return "inaccessible dynamic type"; }
	Type const* decodingType() const override;
};

}
