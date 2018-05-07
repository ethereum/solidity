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
 * Solidity data types
 */

#pragma once

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTEnums.h>
#include <libsolidity/parsing/Token.h>

#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>

#include <boost/noncopyable.hpp>
#include <boost/rational.hpp>
#include <boost/optional.hpp>

#include <memory>
#include <string>
#include <map>
#include <set>

namespace dev
{
namespace solidity
{

class Type; // forward
class FunctionType; // forward
using TypePointer = std::shared_ptr<Type const>;
using FunctionTypePointer = std::shared_ptr<FunctionType const>;
using TypePointers = std::vector<TypePointer>;
using rational = boost::rational<dev::bigint>;


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
		Member(std::string const& _name, TypePointer const& _type, Declaration const* _declaration = nullptr):
			name(_name),
			type(_type),
			declaration(_declaration)
		{
		}

		std::string name;
		TypePointer type;
		Declaration const* declaration = nullptr;
	};

	using MemberMap = std::vector<Member>;

	MemberList() {}
	explicit MemberList(MemberMap const& _members): m_memberTypes(_members) {}
	MemberList& operator=(MemberList&& _other);
	void combine(MemberList const& _other);
	TypePointer memberType(std::string const& _name) const
	{
		TypePointer type;
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
	MemberMap m_memberTypes;
	mutable std::unique_ptr<StorageOffsets> m_storageOffsets;
};

/**
 * Abstract base class that forms the root of the type hierarchy.
 */
class Type: private boost::noncopyable, public std::enable_shared_from_this<Type>
{
public:
	virtual ~Type() = default;
	enum class Category
	{
		Integer, RationalNumber, StringLiteral, Bool, FixedPoint, Array,
		FixedBytes, Contract, Struct, Function, Enum, Tuple,
		Mapping, TypeType, Modifier, Magic, Module,
		InaccessibleDynamic
	};

	/// @{
	/// @name Factory functions
	/// Factory functions that convert an AST @ref TypeName to a Type.
	static TypePointer fromElementaryTypeName(ElementaryTypeNameToken const& _type);
	/// Converts a given elementary type name with optional data location
	/// suffix " storage", " calldata" or " memory" to a type pointer. If suffix not given, defaults to " storage".
	static TypePointer fromElementaryTypeName(std::string const& _name);
	/// @}

	/// Auto-detect the proper type for a literal. @returns an empty pointer if the literal does
	/// not fit any type.
	static TypePointer forLiteral(Literal const& _literal);
	/// @returns a pointer to _a or _b if the other is implicitly convertible to it or nullptr otherwise
	static TypePointer commonType(TypePointer const& _a, TypePointer const& _b);

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
	std::string identifier() const { return escapeIdentifier(richIdentifier()); }

	/// More complex identifier strings use "parentheses", where $_ is interpreted as as
	/// "opening parenthesis", _$ as "closing parenthesis", _$_ as "comma" and any $ that
	/// appears as part of a user-supplied identifier is escaped as _$$$_.
	/// @returns an escaped identifier (will not contain any parenthesis or commas)
	static std::string escapeIdentifier(std::string const& _identifier);

	virtual bool isImplicitlyConvertibleTo(Type const& _other) const { return *this == _other; }
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const
	{
		return isImplicitlyConvertibleTo(_convertTo);
	}
	/// @returns the resulting type of applying the given unary operator or an empty pointer if
	/// this is not possible.
	/// The default implementation does not allow any unary operator.
	virtual TypePointer unaryOperatorResult(Token::Value) const { return TypePointer(); }
	/// @returns the resulting type of applying the given binary operator or an empty pointer if
	/// this is not possible.
	/// The default implementation allows comparison operators if a common type exists
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const
	{
		return Token::isCompareOp(_operator) ? commonType(shared_from_this(), _other) : TypePointer();
	}

	virtual bool operator==(Type const& _other) const { return category() == _other.category(); }
	virtual bool operator!=(Type const& _other) const { return !this->operator ==(_other); }

	/// @returns number of bytes used by this type when encoded for CALL. If it is a dynamic type,
	/// returns the size of the pointer (usually 32). Returns 0 if the type cannot be encoded
	/// in calldata.
	/// @note: This should actually not be called on types, where isDynamicallyEncoded returns true.
	/// If @a _padded then it is assumed that each element is padded to a multiple of 32 bytes.
	virtual unsigned calldataEncodedSize(bool _padded) const { (void)_padded; return 0; }
	/// @returns the size of this data type in bytes when stored in memory. For memory-reference
	/// types, this is the size of the memory pointer.
	virtual unsigned memoryHeadSize() const { return calldataEncodedSize(); }
	/// Convenience version of @see calldataEncodedSize(bool)
	unsigned calldataEncodedSize() const { return calldataEncodedSize(true); }
	/// @returns true if the type is a dynamic array
	virtual bool isDynamicallySized() const { return false; }
	/// @returns true if the type is dynamically encoded in the ABI
	virtual bool isDynamicallyEncoded() const { return false; }
	/// @returns the number of storage slots required to hold this value in storage.
	/// For dynamically "allocated" types, it returns the size of the statically allocated head,
	virtual u256 storageSize() const { return 1; }
	/// Multiple small types can be packed into a single storage slot. If such a packing is possible
	/// this function @returns the size in bytes smaller than 32. Data is moved to the next slot if
	/// it does not fit.
	/// In order to avoid computation at runtime of whether such moving is necessary, structs and
	/// array data (not each element) always start a new slot.
	virtual unsigned storageBytes() const { return 32; }
	/// Returns true if the type can be stored in storage.
	virtual bool canBeStored() const { return true; }
	/// Returns false if the type cannot live outside the storage, i.e. if it includes some mapping.
	virtual bool canLiveOutsideStorage() const { return true; }
	/// Returns true if the type can be stored as a value (as opposed to a reference) on the stack,
	/// i.e. it behaves differently in lvalue context and in value context.
	virtual bool isValueType() const { return false; }
	virtual unsigned sizeOnStack() const { return 1; }
	/// If it is possible to initialize such a value in memory by just writing zeros
	/// of the size memoryHeadSize().
	virtual bool hasSimpleZeroValueInMemory() const { return true; }
	/// @returns the mobile (in contrast to static) type corresponding to the given type.
	/// This returns the corresponding IntegerType or FixedPointType for RationalNumberType
	/// and the pointer type for storage reference types.
	/// Might return a null pointer if there is no fitting type.
	virtual TypePointer mobileType() const { return shared_from_this(); }
	/// @returns true if this is a non-value type and the data of this type is stored at the
	/// given location.
	virtual bool dataStoredIn(DataLocation) const { return false; }
	/// @returns the type of a temporary during assignment to a variable of the given type.
	/// Specifically, returns the requested itself if it can be dynamically allocated (or is a value type)
	/// and the mobile type otherwise.
	virtual TypePointer closestTemporaryType(TypePointer const& _targetType) const
	{
		return _targetType->dataStoredIn(DataLocation::Storage) ? mobileType() : _targetType;
	}

	/// Returns the list of all members of this type. Default implementation: no members apart from bound.
	/// @param _currentScope scope in which the members are accessed.
	MemberList const& members(ContractDefinition const* _currentScope) const;
	/// Convenience method, returns the type of the given named member or an empty pointer if no such member exists.
	TypePointer memberType(std::string const& _name, ContractDefinition const* _currentScope = nullptr) const
	{
		return members(_currentScope).memberType(_name);
	}

	virtual std::string toString(bool _short) const = 0;
	std::string toString() const { return toString(false); }
	/// @returns the canonical name of this type for use in library function signatures.
	virtual std::string canonicalName() const { return toString(true); }
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
	virtual TypePointer encodingType() const { return TypePointer(); }
	/// @returns a (simpler) type that is used when decoding this type in calldata.
	virtual TypePointer decodingType() const { return encodingType(); }
	/// @returns a type that will be used outside of Solidity for e.g. function signatures.
	/// This for example returns address for contract types.
	/// If there is no such type, returns an empty shared pointer.
	/// @param _inLibrary if set, returns types as used in a library, e.g. struct and contract types
	/// are returned without modification.
	virtual TypePointer interfaceType(bool /*_inLibrary*/) const { return TypePointer(); }
	/// @returns true iff this type can be passed on via calls (to libraries if _inLibrary is true),
	/// should be have identical to !!interfaceType(_inLibrary) but might do optimizations.
	virtual bool canBeUsedExternally(bool _inLibrary) const { return !!interfaceType(_inLibrary); }

private:
	/// @returns a member list containing all members added to this type by `using for` directives.
	static MemberList::MemberMap boundFunctions(Type const& _type, ContractDefinition const& _scope);

protected:
	/// @returns the members native to this type depending on the given context. This function
	/// is used (in conjunction with boundFunctions to fill m_members below.
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const* /*_currentScope*/) const
	{
		return MemberList::MemberMap();
	}

	/// List of member types (parameterised by scape), will be lazy-initialized.
	mutable std::map<ContractDefinition const*, std::unique_ptr<MemberList>> m_members;
};

/**
 * Any kind of integer type (signed, unsigned, address).
 */
class IntegerType: public Type
{
public:
	enum class Modifier
	{
		Unsigned, Signed, Address
	};
	virtual Category category() const override { return Category::Integer; }

	explicit IntegerType(unsigned _bits, Modifier _modifier = Modifier::Unsigned);

	virtual std::string richIdentifier() const override;
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual bool operator==(Type const& _other) const override;

	virtual unsigned calldataEncodedSize(bool _padded = true) const override { return _padded ? 32 : m_bits / 8; }
	virtual unsigned storageBytes() const override { return m_bits / 8; }
	virtual bool isValueType() const override { return true; }

	virtual MemberList::MemberMap nativeMembers(ContractDefinition const*) const override;

	virtual std::string toString(bool _short) const override;

	virtual u256 literalValue(Literal const* _literal) const override;

	virtual TypePointer encodingType() const override { return shared_from_this(); }
	virtual TypePointer interfaceType(bool) const override { return shared_from_this(); }

	unsigned numBits() const { return m_bits; }
	bool isAddress() const { return m_modifier == Modifier::Address; }
	bool isSigned() const { return m_modifier == Modifier::Signed; }

	bigint minValue() const;
	bigint maxValue() const;

private:
	unsigned m_bits;
	Modifier m_modifier;
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
	virtual Category category() const override { return Category::FixedPoint; }

	explicit FixedPointType(unsigned _totalBits, unsigned _fractionalDigits, Modifier _modifier = Modifier::Unsigned);

	virtual std::string richIdentifier() const override;
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual bool operator==(Type const& _other) const override;

	virtual unsigned calldataEncodedSize(bool _padded = true) const override { return _padded ? 32 : m_totalBits / 8; }
	virtual unsigned storageBytes() const override { return m_totalBits / 8; }
	virtual bool isValueType() const override { return true; }

	virtual std::string toString(bool _short) const override;

	virtual TypePointer encodingType() const override { return shared_from_this(); }
	virtual TypePointer interfaceType(bool) const override { return shared_from_this(); }

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
	std::shared_ptr<IntegerType> asIntegerType() const;

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

	virtual Category category() const override { return Category::RationalNumber; }

	/// @returns true if the literal is a valid integer.
	static std::tuple<bool, rational> isValidLiteral(Literal const& _literal);

	explicit RationalNumberType(rational const& _value):
		m_value(_value)
	{}
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;

	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return false; }

	virtual std::string toString(bool _short) const override;
	virtual u256 literalValue(Literal const* _literal) const override;
	virtual TypePointer mobileType() const override;

	/// @returns the smallest integer type that can hold the value or an empty pointer if not possible.
	std::shared_ptr<IntegerType const> integerType() const;
	/// @returns the smallest fixed type that can  hold the value or incurs the least precision loss.
	/// If the integer part does not fit, returns an empty pointer.
	std::shared_ptr<FixedPointType const> fixedPointType() const;

	/// @returns true if the value is not an integer.
	bool isFractional() const { return m_value.denominator() != 1; }

	/// @returns true if the value is negative.
	bool isNegative() const { return m_value < 0; }

	/// @returns true if the value is zero.
	bool isZero() const { return m_value == 0; }

private:
	rational m_value;

	/// @returns true if the literal is a valid rational number.
	static std::tuple<bool, rational> parseRational(std::string const& _value);

	/// @returns a truncated readable representation of the bigint keeping only
	/// up to 4 leading and 4 trailing digits.
	static std::string bigintToReadableString(dev::bigint const& num);
};

/**
 * Literal string, can be converted to bytes, bytesX or string.
 */
class StringLiteralType: public Type
{
public:
	virtual Category category() const override { return Category::StringLiteral; }

	explicit StringLiteralType(Literal const& _literal);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override
	{
		return TypePointer();
	}

	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;

	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned sizeOnStack() const override { return 0; }

	virtual std::string toString(bool) const override;
	virtual TypePointer mobileType() const override;

	bool isValidUTF8() const;

	std::string const& value() const { return m_value; }

private:
	std::string m_value;
};

/**
 * Bytes type with fixed length of up to 32 bytes.
 */
class FixedBytesType: public Type
{
public:
	virtual Category category() const override { return Category::FixedBytes; }

	explicit FixedBytesType(unsigned _bytes);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual unsigned calldataEncodedSize(bool _padded) const override { return _padded && m_bytes > 0 ? 32 : m_bytes; }
	virtual unsigned storageBytes() const override { return m_bytes; }
	virtual bool isValueType() const override { return true; }

	virtual std::string toString(bool) const override { return "bytes" + dev::toString(m_bytes); }
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const*) const override;
	virtual TypePointer encodingType() const override { return shared_from_this(); }
	virtual TypePointer interfaceType(bool) const override { return shared_from_this(); }

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
	BoolType() {}
	virtual Category category() const override { return Category::Bool; }
	virtual std::string richIdentifier() const override { return "t_bool"; }
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual unsigned calldataEncodedSize(bool _padded) const override{ return _padded ? 32 : 1; }
	virtual unsigned storageBytes() const override { return 1; }
	virtual bool isValueType() const override { return true; }

	virtual std::string toString(bool) const override { return "bool"; }
	virtual u256 literalValue(Literal const* _literal) const override;
	virtual TypePointer encodingType() const override { return shared_from_this(); }
	virtual TypePointer interfaceType(bool) const override { return shared_from_this(); }
};

/**
 * Base class used by types which are not value types and can be stored either in storage, memory
 * or calldata. This is currently used by arrays and structs.
 */
class ReferenceType: public Type
{
public:
	explicit ReferenceType(DataLocation _location): m_location(_location) {}
	DataLocation location() const { return m_location; }

	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override
	{
		return TypePointer();
	}
	virtual unsigned memoryHeadSize() const override { return 32; }

	/// @returns a copy of this type with location (recursively) changed to @a _location,
	/// whereas isPointer is only shallowly changed - the deep copy is always a bound reference.
	virtual TypePointer copyForLocation(DataLocation _location, bool _isPointer) const = 0;

	virtual TypePointer mobileType() const override { return copyForLocation(m_location, true); }
	virtual bool dataStoredIn(DataLocation _location) const override { return m_location == _location; }
	virtual bool hasSimpleZeroValueInMemory() const override { return false; }

	/// Storage references can be pointers or bound references. In general, local variables are of
	/// pointer type, state variables are bound references. Assignments to pointers or deleting
	/// them will not modify storage (that will only change the pointer). Assignment from
	/// non-storage objects to a variable of storage pointer type is not possible.
	bool isPointer() const { return m_isPointer; }

	bool operator==(ReferenceType const& _other) const
	{
		return location() == _other.location() && isPointer() == _other.isPointer();
	}

	/// @returns a copy of @a _type having the same location as this (and is not a pointer type)
	/// if _type is a reference type and an unmodified copy of _type otherwise.
	/// This function is mostly useful to modify inner types appropriately.
	static TypePointer copyForLocationIfReference(DataLocation _location, TypePointer const& _type);

protected:
	TypePointer copyForLocationIfReference(TypePointer const& _type) const;
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
	virtual Category category() const override { return Category::Array; }

	/// Constructor for a byte array ("bytes") and string.
	explicit ArrayType(DataLocation _location, bool _isString = false):
		ReferenceType(_location),
		m_arrayKind(_isString ? ArrayKind::String : ArrayKind::Bytes),
		m_baseType(std::make_shared<FixedBytesType>(1))
	{
	}
	/// Constructor for a dynamically sized array type ("type[]")
	ArrayType(DataLocation _location, TypePointer const& _baseType):
		ReferenceType(_location),
		m_baseType(copyForLocationIfReference(_baseType))
	{
	}
	/// Constructor for a fixed-size array type ("type[20]")
	ArrayType(DataLocation _location, TypePointer const& _baseType, u256 const& _length):
		ReferenceType(_location),
		m_baseType(copyForLocationIfReference(_baseType)),
		m_hasDynamicLength(false),
		m_length(_length)
	{}

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual std::string richIdentifier() const override;
	virtual bool operator==(const Type& _other) const override;
	virtual unsigned calldataEncodedSize(bool _padded) const override;
	virtual bool isDynamicallySized() const override { return m_hasDynamicLength; }
	virtual bool isDynamicallyEncoded() const override;
	virtual u256 storageSize() const override;
	virtual bool canLiveOutsideStorage() const override { return m_baseType->canLiveOutsideStorage(); }
	virtual unsigned sizeOnStack() const override;
	virtual std::string toString(bool _short) const override;
	virtual std::string canonicalName() const override;
	virtual std::string signatureInExternalFunction(bool _structsByName) const override;
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const* _currentScope) const override;
	virtual TypePointer encodingType() const override;
	virtual TypePointer decodingType() const override;
	virtual TypePointer interfaceType(bool _inLibrary) const override;
	virtual bool canBeUsedExternally(bool _inLibrary) const override;

	/// @returns true if this is valid to be stored in calldata
	bool validForCalldata() const;

	/// @returns true if this is a byte array or a string
	bool isByteArray() const { return m_arrayKind != ArrayKind::Ordinary; }
	/// @returns true if this is a string
	bool isString() const { return m_arrayKind == ArrayKind::String; }
	TypePointer const& baseType() const { solAssert(!!m_baseType, ""); return m_baseType;}
	u256 const& length() const { return m_length; }
	u256 memorySize() const;

	TypePointer copyForLocation(DataLocation _location, bool _isPointer) const override;

private:
	/// String is interpreted as a subtype of Bytes.
	enum class ArrayKind { Ordinary, Bytes, String };

	bigint unlimitedCalldataEncodedSize(bool _padded) const;

	///< Byte arrays ("bytes") and strings have different semantics from ordinary arrays.
	ArrayKind m_arrayKind = ArrayKind::Ordinary;
	TypePointer m_baseType;
	bool m_hasDynamicLength = true;
	u256 m_length;
};

/**
 * The type of a contract instance or library, there is one distinct type for each contract definition.
 */
class ContractType: public Type
{
public:
	virtual Category category() const override { return Category::Contract; }
	explicit ContractType(ContractDefinition const& _contract, bool _super = false):
		m_contract(_contract), m_super(_super) {}
	/// Contracts can be implicitly converted to super classes and to addresses.
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	/// Contracts can be converted to themselves and to integers.
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual unsigned calldataEncodedSize(bool _padded ) const override
	{
		solAssert(!isSuper(), "");
		return encodingType()->calldataEncodedSize(_padded);
	}
	virtual unsigned storageBytes() const override { solAssert(!isSuper(), ""); return 20; }
	virtual bool canLiveOutsideStorage() const override { return !isSuper(); }
	virtual unsigned sizeOnStack() const override { return m_super ? 0 : 1; }
	virtual bool isValueType() const override { return !isSuper(); }
	virtual std::string toString(bool _short) const override;
	virtual std::string canonicalName() const override;

	virtual MemberList::MemberMap nativeMembers(ContractDefinition const* _currentScope) const override;
	virtual TypePointer encodingType() const override
	{
		if (isSuper())
			return TypePointer{};
		return std::make_shared<IntegerType>(160, IntegerType::Modifier::Address);
	}
	virtual TypePointer interfaceType(bool _inLibrary) const override
	{
		if (isSuper())
			return TypePointer{};
		return _inLibrary ? shared_from_this() : encodingType();
	}

	bool isSuper() const { return m_super; }

	// @returns true if and only if the contract has a payable fallback function
	bool isPayable() const;

	ContractDefinition const& contractDefinition() const { return m_contract; }

	/// Returns the function type of the constructor modified to return an object of the contract's type.
	FunctionTypePointer const& newExpressionType() const;

	/// @returns a list of all state variables (including inherited) of the contract and their
	/// offsets in storage.
	std::vector<std::tuple<VariableDeclaration const*, u256, unsigned>> stateVariables() const;

private:
	static void addNonConflictingAddressMembers(MemberList::MemberMap& _members);

	ContractDefinition const& m_contract;
	/// If true, it is the "super" type of the current contract, i.e. it contains only inherited
	/// members.
	bool m_super = false;
	/// Type of the constructor, @see constructorType. Lazily initialized.
	mutable FunctionTypePointer m_constructorType;
};

/**
 * The type of a struct instance, there is one distinct type per struct definition.
 */
class StructType: public ReferenceType
{
public:
	virtual Category category() const override { return Category::Struct; }
	explicit StructType(StructDefinition const& _struct, DataLocation _location = DataLocation::Storage):
		ReferenceType(_location), m_struct(_struct) {}
	virtual bool isImplicitlyConvertibleTo(const Type& _convertTo) const override;
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual unsigned calldataEncodedSize(bool _padded) const override;
	virtual bool isDynamicallyEncoded() const override;
	u256 memorySize() const;
	virtual u256 storageSize() const override;
	virtual bool canLiveOutsideStorage() const override { return true; }
	virtual std::string toString(bool _short) const override;

	virtual MemberList::MemberMap nativeMembers(ContractDefinition const* _currentScope) const override;
	virtual TypePointer encodingType() const override
	{
		return location() == DataLocation::Storage ? std::make_shared<IntegerType>(256) : shared_from_this();
	}
	virtual TypePointer interfaceType(bool _inLibrary) const override;
	virtual bool canBeUsedExternally(bool _inLibrary) const override;

	TypePointer copyForLocation(DataLocation _location, bool _isPointer) const override;

	virtual std::string canonicalName() const override;
	virtual std::string signatureInExternalFunction(bool _structsByName) const override;

	/// @returns a function that performs the type conversion between a list of struct members
	/// and a memory struct of this type.
	FunctionTypePointer constructorType() const;

	std::pair<u256, unsigned> const& storageOffsetsOfMember(std::string const& _name) const;
	u256 memoryOffsetOfMember(std::string const& _name) const;

	StructDefinition const& structDefinition() const { return m_struct; }

	/// @returns the vector of types of members available in memory.
	TypePointers memoryMemberTypes() const;
	/// @returns the set of all members that are removed in the memory version (typically mappings).
	std::set<std::string> membersMissingInMemory() const;

	/// @returns true if the same struct is used recursively in one of its members. Only
	/// analyses the "memory" representation, i.e. mappings are ignored in all structs.
	bool recursive() const;

private:
	StructDefinition const& m_struct;
	/// Cache for the recursive() function.
	mutable boost::optional<bool> m_recursive;
};

/**
 * The type of an enum instance, there is one distinct type per enum definition.
 */
class EnumType: public Type
{
public:
	virtual Category category() const override { return Category::Enum; }
	explicit EnumType(EnumDefinition const& _enum): m_enum(_enum) {}
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual unsigned calldataEncodedSize(bool _padded) const override
	{
		return encodingType()->calldataEncodedSize(_padded);
	}
	virtual unsigned storageBytes() const override;
	virtual bool canLiveOutsideStorage() const override { return true; }
	virtual std::string toString(bool _short) const override;
	virtual std::string canonicalName() const override;
	virtual bool isValueType() const override { return true; }

	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer encodingType() const override
	{
		return std::make_shared<IntegerType>(8 * int(storageBytes()));
	}
	virtual TypePointer interfaceType(bool _inLibrary) const override
	{
		return _inLibrary ? shared_from_this() : encodingType();
	}

	EnumDefinition const& enumDefinition() const { return m_enum; }
	/// @returns the value that the string has in the Enum
	unsigned int memberValue(ASTString const& _member) const;
	size_t numberOfMembers() const;

private:
	EnumDefinition const& m_enum;
};

/**
 * Type that can hold a finite sequence of values of different types.
 * In some cases, the components are empty pointers (when used as placeholders).
 */
class TupleType: public Type
{
public:
	virtual Category category() const override { return Category::Tuple; }
	explicit TupleType(std::vector<TypePointer> const& _types = std::vector<TypePointer>()): m_components(_types) {}
	virtual bool isImplicitlyConvertibleTo(Type const& _other) const override;
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual std::string toString(bool) const override;
	virtual bool canBeStored() const override { return false; }
	virtual u256 storageSize() const override;
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned sizeOnStack() const override;
	virtual bool hasSimpleZeroValueInMemory() const override { return false; }
	virtual TypePointer mobileType() const override;
	/// Converts components to their temporary types and performs some wildcard matching.
	virtual TypePointer closestTemporaryType(TypePointer const& _targetType) const override;

	std::vector<TypePointer> const& components() const { return m_components; }

private:
	std::vector<TypePointer> const m_components;
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
		CallCode, ///< external call using CALLCODE, i.e. not exchanging the storage
		DelegateCall, ///< external call using DELEGATECALL, i.e. not exchanging the storage
		BareCall, ///< CALL without function hash
		BareCallCode, ///< CALLCODE without function hash
		BareDelegateCall, ///< DELEGATECALL without function hash
		Creation, ///< external call using CREATE
		Send, ///< CALL, but without data and gas
		Transfer, ///< CALL, but without data and throws on error
		SHA3, ///< SHA3
		Selfdestruct, ///< SELFDESTRUCT
		Revert, ///< REVERT
		ECRecover, ///< CALL to special contract for ecrecover
		SHA256, ///< CALL to special contract for sha256
		RIPEMD160, ///< CALL to special contract for ripemd160
		Log0,
		Log1,
		Log2,
		Log3,
		Log4,
		Event, ///< syntactic sugar for LOG*
		SetGas, ///< modify the default gas value for the function call
		SetValue, ///< modify the default value transfer for the function call
		BlockHash, ///< BLOCKHASH
		AddMod, ///< ADDMOD
		MulMod, ///< MULMOD
		ArrayPush, ///< .push() to a dynamically sized array in storage
		ArrayPop, ///< .pop() from a dynamically sized array in storage
		ByteArrayPush, ///< .push() to a dynamically sized byte array in storage
		ObjectCreation, ///< array creation using new
		Assert, ///< assert()
		Require, ///< require()
		ABIEncode,
		ABIEncodePacked,
		ABIEncodeWithSelector,
		ABIEncodeWithSignature,
		GasLeft ///< gasleft()
	};

	virtual Category category() const override { return Category::Function; }

	/// Creates the type of a function.
	explicit FunctionType(FunctionDefinition const& _function, bool _isInternal = true);
	/// Creates the accessor function type of a state variable.
	explicit FunctionType(VariableDeclaration const& _varDecl);
	/// Creates the function type of an event.
	explicit FunctionType(EventDefinition const& _event);
	/// Creates the type of a function type name.
	explicit FunctionType(FunctionTypeName const& _typeName);
	/// Function type constructor to be used for a plain type (not derived from a declaration).
	FunctionType(
		strings const& _parameterTypes,
		strings const& _returnParameterTypes,
		Kind _kind = Kind::Internal,
		bool _arbitraryParameters = false,
		StateMutability _stateMutability = StateMutability::NonPayable
	): FunctionType(
		parseElementaryTypeVector(_parameterTypes),
		parseElementaryTypeVector(_returnParameterTypes),
		strings(),
		strings(),
		_kind,
		_arbitraryParameters,
		_stateMutability
	)
	{
	}

	/// @returns the type of the "new Contract" function, i.e. basically the constructor.
	static FunctionTypePointer newExpressionType(ContractDefinition const& _contract);

	/// Detailed constructor, use with care.
	FunctionType(
		TypePointers const& _parameterTypes,
		TypePointers const& _returnParameterTypes,
		strings _parameterNames = strings(),
		strings _returnParameterNames = strings(),
		Kind _kind = Kind::Internal,
		bool _arbitraryParameters = false,
		StateMutability _stateMutability = StateMutability::NonPayable,
		Declaration const* _declaration = nullptr,
		bool _gasSet = false,
		bool _valueSet = false,
		bool _bound = false
	):
		m_parameterTypes(_parameterTypes),
		m_returnParameterTypes(_returnParameterTypes),
		m_parameterNames(_parameterNames),
		m_returnParameterNames(_returnParameterNames),
		m_kind(_kind),
		m_stateMutability(_stateMutability),
		m_arbitraryParameters(_arbitraryParameters),
		m_gasSet(_gasSet),
		m_valueSet(_valueSet),
		m_bound(_bound),
		m_declaration(_declaration)
	{
		solAssert(
			!m_bound || !m_parameterTypes.empty(),
			"Attempted construction of bound function without self type"
		);
	}

	TypePointers parameterTypes() const;
	std::vector<std::string> parameterNames() const;
	TypePointers const& returnParameterTypes() const { return m_returnParameterTypes; }
	/// @returns the list of return parameter types. All dynamically-sized types (this excludes
	/// storage pointers) are replaced by InaccessibleDynamicType instances.
	TypePointers returnParameterTypesWithoutDynamicTypes() const;
	std::vector<std::string> const& returnParameterNames() const { return m_returnParameterNames; }
	/// @returns the "self" parameter type for a bound function
	TypePointer const& selfType() const;

	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override;
	virtual std::string canonicalName() const override;
	virtual std::string toString(bool _short) const override;
	virtual unsigned calldataEncodedSize(bool _padded) const override;
	virtual bool canBeStored() const override { return m_kind == Kind::Internal || m_kind == Kind::External; }
	virtual u256 storageSize() const override;
	virtual unsigned storageBytes() const override;
	virtual bool isValueType() const override { return true; }
	virtual bool canLiveOutsideStorage() const override { return m_kind == Kind::Internal || m_kind == Kind::External; }
	virtual unsigned sizeOnStack() const override;
	virtual bool hasSimpleZeroValueInMemory() const override { return false; }
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const* _currentScope) const override;
	virtual TypePointer encodingType() const override;
	virtual TypePointer interfaceType(bool _inLibrary) const override;

	/// @returns TypePointer of a new FunctionType object. All input/return parameters are an
	/// appropriate external types (i.e. the interfaceType()s) of input/return parameters of
	/// current function.
	/// @returns an empty shared pointer if one of the input/return parameters does not have an
	/// external type.
	FunctionTypePointer interfaceFunctionType() const;

	/// @returns true if this function can take the given argument types (possibly
	/// after implicit conversion).
	/// @param _selfType if the function is bound, this has to be supplied and is the type of the
	/// expression the function is called on.
	bool canTakeArguments(TypePointers const& _arguments, TypePointer const& _selfType = TypePointer()) const;
	/// @returns true if the types of parameters are equal (does't check return parameter types)
	bool hasEqualArgumentTypes(FunctionType const& _other) const;

	/// @returns true if the ABI is used for this call (only meaningful for external calls)
	bool isBareCall() const;
	Kind const& kind() const { return m_kind; }
	StateMutability stateMutability() const { return m_stateMutability; }
	/// @returns the external signature of this function type given the function name
	std::string externalSignature() const;
	/// @returns the external identifier of this function (the hash of the signature).
	u256 externalIdentifier() const;
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
	/// @return A shared pointer of an ASTString.
	/// Can contain a nullptr in which case indicates absence of documentation
	ASTPointer<ASTString> documentation() const;

	/// true iff arguments are to be padded to multiples of 32 bytes for external calls
	/// TODO should this be true in general for bareCall*?
	bool padArguments() const { return !(m_kind == Kind::SHA3 || m_kind == Kind::SHA256 || m_kind == Kind::RIPEMD160 || m_kind == Kind::ABIEncodePacked); }
	bool takesArbitraryParameters() const { return m_arbitraryParameters; }
	/// true iff the function takes a single bytes parameter and it is passed on without padding.
	bool takesSinglePackedBytesParameter() const
	{
		switch (m_kind)
		{
		case FunctionType::Kind::SHA3:
		case FunctionType::Kind::SHA256:
		case FunctionType::Kind::RIPEMD160:
		case FunctionType::Kind::BareCall:
		case FunctionType::Kind::BareCallCode:
		case FunctionType::Kind::BareDelegateCall:
			return true;
		default:
			return false;
		}
	}

	bool gasSet() const { return m_gasSet; }
	bool valueSet() const { return m_valueSet; }
	bool bound() const { return m_bound; }

	/// @returns a copy of this type, where gas or value are set manually. This will never set one
	/// of the parameters to false.
	TypePointer copyAndSetGasOrValue(bool _setGas, bool _setValue) const;

	/// @returns a copy of this function type where all return parameters of dynamic size are
	/// removed and the location of reference types is changed from CallData to Memory.
	/// This is needed if external functions are called on other contracts, as they cannot return
	/// dynamic values.
	/// Returns empty shared pointer on a failure. Namely, if a bound function has no parameters.
	/// @param _inLibrary if true, uses DelegateCall as location.
	/// @param _bound if true, the arguments are placed as `arg1.functionName(arg2, ..., argn)`.
	FunctionTypePointer asMemberFunction(bool _inLibrary, bool _bound = false) const;

private:
	static TypePointers parseElementaryTypeVector(strings const& _types);

	TypePointers m_parameterTypes;
	TypePointers m_returnParameterTypes;
	std::vector<std::string> m_parameterNames;
	std::vector<std::string> m_returnParameterNames;
	Kind const m_kind;
	StateMutability m_stateMutability = StateMutability::NonPayable;
	/// true if the function takes an arbitrary number of arguments of arbitrary types
	bool const m_arbitraryParameters = false;
	bool const m_gasSet = false; ///< true iff the gas value to be used is on the stack
	bool const m_valueSet = false; ///< true iff the value to be sent is on the stack
	bool const m_bound = false; ///< true iff the function is called as arg1.fun(arg2, ..., argn)
	Declaration const* m_declaration = nullptr;
};

/**
 * The type of a mapping, there is one distinct type per key/value type pair.
 * Mappings always occupy their own storage slot, but do not actually use it.
 */
class MappingType: public Type
{
public:
	virtual Category category() const override { return Category::Mapping; }
	MappingType(TypePointer const& _keyType, TypePointer const& _valueType):
		m_keyType(_keyType), m_valueType(_valueType) {}

	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual std::string toString(bool _short) const override;
	virtual std::string canonicalName() const override;
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual TypePointer encodingType() const override
	{
		return std::make_shared<IntegerType>(256);
	}
	virtual TypePointer interfaceType(bool _inLibrary) const override
	{
		return _inLibrary ? shared_from_this() : TypePointer();
	}
	virtual bool dataStoredIn(DataLocation _location) const override { return _location == DataLocation::Storage; }
	/// Cannot be stored in memory, but just in case.
	virtual bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }

	TypePointer const& keyType() const { return m_keyType; }
	TypePointer const& valueType() const { return m_valueType; }

private:
	TypePointer m_keyType;
	TypePointer m_valueType;
};

/**
 * The type of a type reference. The type of "uint32" when used in "a = uint32(2)" is an example
 * of a TypeType.
 * For super contracts or libraries, this has members directly.
 */
class TypeType: public Type
{
public:
	virtual Category category() const override { return Category::TypeType; }
	explicit TypeType(TypePointer const& _actualType): m_actualType(_actualType) {}
	TypePointer const& actualType() const { return m_actualType; }

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual bool canBeStored() const override { return false; }
	virtual u256 storageSize() const override;
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned sizeOnStack() const override;
	virtual bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	virtual std::string toString(bool _short) const override { return "type(" + m_actualType->toString(_short) + ")"; }
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const* _currentScope) const override;

private:
	TypePointer m_actualType;
};


/**
 * The type of a function modifier. Not used for anything for now.
 */
class ModifierType: public Type
{
public:
	virtual Category category() const override { return Category::Modifier; }
	explicit ModifierType(ModifierDefinition const& _modifier);

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual bool canBeStored() const override { return false; }
	virtual u256 storageSize() const override;
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned sizeOnStack() const override { return 0; }
	virtual bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual std::string toString(bool _short) const override;

private:
	TypePointers m_parameterTypes;
};



/**
 * Special type for imported modules. These mainly give access to their scope via members.
 */
class ModuleType: public Type
{
public:
	virtual Category category() const override { return Category::Module; }

	explicit ModuleType(SourceUnit const& _source): m_sourceUnit(_source) {}

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return true; }
	virtual bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	virtual unsigned sizeOnStack() const override { return 0; }
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const*) const override;

	virtual std::string toString(bool _short) const override;

private:
	SourceUnit const& m_sourceUnit;
};

/**
 * Special type for magic variables (block, msg, tx), similar to a struct but without any reference
 * (it always references a global singleton by name).
 */
class MagicType: public Type
{
public:
	enum class Kind { Block, Message, Transaction, ABI };
	virtual Category category() const override { return Category::Magic; }

	explicit MagicType(Kind _kind): m_kind(_kind) {}

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override
	{
		return TypePointer();
	}

	virtual std::string richIdentifier() const override;
	virtual bool operator==(Type const& _other) const override;
	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return true; }
	virtual bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	virtual unsigned sizeOnStack() const override { return 0; }
	virtual MemberList::MemberMap nativeMembers(ContractDefinition const*) const override;

	virtual std::string toString(bool _short) const override;

	Kind kind() const { return m_kind; }

private:
	Kind m_kind;
};

/**
 * Special type that is used for dynamic types in returns from external function calls
 * (The EVM currently cannot access dynamically-sized return values).
 */
class InaccessibleDynamicType: public Type
{
public:
	virtual Category category() const override { return Category::InaccessibleDynamic; }

	virtual std::string richIdentifier() const override { return "t_inaccessible"; }
	virtual bool isImplicitlyConvertibleTo(Type const&) const override { return false; }
	virtual bool isExplicitlyConvertibleTo(Type const&) const override { return false; }
	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual unsigned calldataEncodedSize(bool _padded) const override { (void)_padded; return 32; }
	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual bool isValueType() const override { return true; }
	virtual unsigned sizeOnStack() const override { return 1; }
	virtual bool hasSimpleZeroValueInMemory() const override { solAssert(false, ""); }
	virtual std::string toString(bool) const override { return "inaccessible dynamic type"; }
	virtual TypePointer decodingType() const override { return std::make_shared<IntegerType>(256); }
};

}
}
