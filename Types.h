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
 * @date 2014
 * Solidity data types
 */

#pragma once

#include <memory>
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <libdevcore/Common.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/ASTForward.h>
#include <libsolidity/Token.h>

namespace dev
{
namespace solidity
{

// @todo realMxN, dynamic strings, text, arrays

class Type; // forward
class FunctionType; // forward
using TypePointer = std::shared_ptr<Type const>;
using TypePointers = std::vector<TypePointer>;

/**
 * List of members of a type.
 */
class MemberList
{
public:
	using MemberMap = std::map<std::string, TypePointer>;

	MemberList() {}
	explicit MemberList(MemberMap const& _members): m_memberTypes(_members) {}
	TypePointer getMemberType(std::string const& _name) const
	{
		auto it = m_memberTypes.find(_name);
		return it != m_memberTypes.end() ? it->second : TypePointer();
	}

	MemberMap::const_iterator begin() const { return m_memberTypes.begin(); }
	MemberMap::const_iterator end() const { return m_memberTypes.end(); }

private:
	MemberMap m_memberTypes;
};


/**
 * Abstract base class that forms the root of the type hierarchy.
 */
class Type: private boost::noncopyable, public std::enable_shared_from_this<Type>
{
public:
	enum class Category
	{
		INTEGER, INTEGER_CONSTANT, BOOL, REAL, STRING, CONTRACT, STRUCT, FUNCTION, MAPPING, VOID, TYPE, MAGIC
	};

	///@{
	///@name Factory functions
	/// Factory functions that convert an AST @ref TypeName to a Type.
	static TypePointer fromElementaryTypeName(Token::Value _typeToken);
	static TypePointer fromUserDefinedTypeName(UserDefinedTypeName const& _typeName);
	static TypePointer fromMapping(Mapping const& _typeName);
	static TypePointer fromFunction(FunctionDefinition const& _function);
	/// @}

	/// Auto-detect the proper type for a literal. @returns an empty pointer if the literal does
	/// not fit any type.
	static TypePointer forLiteral(Literal const& _literal);
	/// @returns a pointer to _a or _b if the other is implicitly convertible to it or nullptr otherwise
	static TypePointer commonType(TypePointer const& _a, TypePointer const& _b);

	virtual Category getCategory() const = 0;
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

	virtual bool operator==(Type const& _other) const { return getCategory() == _other.getCategory(); }
	virtual bool operator!=(Type const& _other) const { return !this->operator ==(_other); }

	/// @returns number of bytes used by this type when encoded for CALL, or 0 if the encoding
	/// is not a simple big-endian encoding or the type cannot be stored in calldata.
	/// Note that irrespective of this size, each calldata element is padded to a multiple of 32 bytes.
	virtual unsigned getCalldataEncodedSize() const { return 0; }
	/// @returns number of bytes required to hold this value in storage.
	/// For dynamically "allocated" types, it returns the size of the statically allocated head,
	virtual u256 getStorageSize() const { return 1; }
	/// Returns true if the type can be stored in storage.
	virtual bool canBeStored() const { return true; }
	/// Returns false if the type cannot live outside the storage, i.e. if it includes some mapping.
	virtual bool canLiveOutsideStorage() const { return true; }
	/// Returns true if the type can be stored as a value (as opposed to a reference) on the stack,
	/// i.e. it behaves differently in lvalue context and in value context.
	virtual bool isValueType() const { return false; }
	virtual unsigned getSizeOnStack() const { return 1; }

	/// Returns the list of all members of this type. Default implementation: no members.
	virtual MemberList const& getMembers() const { return EmptyMemberList; }
	/// Convenience method, returns the type of the given named member or an empty pointer if no such member exists.
	TypePointer getMemberType(std::string const& _name) const { return getMembers().getMemberType(_name); }

	virtual std::string toString() const = 0;
	virtual u256 literalValue(Literal const*) const
	{
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Literal value requested "
																		  "for type without literals."));
	}

protected:
	/// Convenience object used when returning an empty member list.
	static const MemberList EmptyMemberList;
};

/**
 * Any kind of integer type including hash and address.
 */
class IntegerType: public Type
{
public:
	enum class Modifier
	{
		UNSIGNED, SIGNED, HASH, ADDRESS
	};
	virtual Category getCategory() const override { return Category::INTEGER; }

	explicit IntegerType(int _bits, Modifier _modifier = Modifier::UNSIGNED);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual bool operator==(Type const& _other) const override;

	virtual unsigned getCalldataEncodedSize() const override { return m_bits / 8; }
	virtual bool isValueType() const override { return true; }

	virtual MemberList const& getMembers() const { return isAddress() ? AddressMemberList : EmptyMemberList; }

	virtual std::string toString() const override;

	int getNumBits() const { return m_bits; }
	bool isHash() const { return m_modifier == Modifier::HASH || m_modifier == Modifier::ADDRESS; }
	bool isAddress() const { return m_modifier == Modifier::ADDRESS; }
	bool isSigned() const { return m_modifier == Modifier::SIGNED; }

	static const MemberList AddressMemberList;

private:
	int m_bits;
	Modifier m_modifier;
};

/**
 * Integer constants either literals or computed. Example expressions: 2, 2+10, ~10.
 * There is one distinct type per value.
 */
class IntegerConstantType: public Type
{
public:
	virtual Category getCategory() const override { return Category::INTEGER_CONSTANT; }

	static std::shared_ptr<IntegerConstantType const> fromLiteral(std::string const& _literal);

	explicit IntegerConstantType(bigint _value): m_value(_value) {}

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual bool operator==(Type const& _other) const override;

	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned getSizeOnStack() const override { return 1; }

	virtual std::string toString() const override;
	virtual u256 literalValue(Literal const* _literal) const override;

	/// @returns the smallest integer type that can hold the value or an empty pointer if not possible.
	std::shared_ptr<IntegerType const> getIntegerType() const;

private:
	bigint m_value;
};

/**
 * String type with fixed length, up to 32 bytes.
 */
class StaticStringType: public Type
{
public:
	virtual Category getCategory() const override { return Category::STRING; }

	/// @returns the smallest string type for the given literal or an empty pointer
	/// if no type fits.
	static std::shared_ptr<StaticStringType> smallestTypeForLiteral(std::string const& _literal);

	StaticStringType(int _bytes);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool operator==(Type const& _other) const override;

	virtual unsigned getCalldataEncodedSize() const override { return m_bytes; }
	virtual bool isValueType() const override { return true; }

	virtual std::string toString() const override { return "string" + dev::toString(m_bytes); }
	virtual u256 literalValue(Literal const* _literal) const override;

	int getNumBytes() const { return m_bytes; }

private:
	int m_bytes;
};

/**
 * The boolean type.
 */
class BoolType: public Type
{
public:
	BoolType() {}
	virtual Category getCategory() const { return Category::BOOL; }
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override
	{
		return (_operator == Token::NOT || _operator == Token::DELETE) ? shared_from_this() : TypePointer();
	}
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual unsigned getCalldataEncodedSize() const { return 1; }
	virtual bool isValueType() const override { return true; }

	virtual std::string toString() const override { return "bool"; }
	virtual u256 literalValue(Literal const* _literal) const override;
};

/**
 * The type of a contract instance, there is one distinct type for each contract definition.
 */
class ContractType: public Type
{
public:
	virtual Category getCategory() const override { return Category::CONTRACT; }
	ContractType(ContractDefinition const& _contract): m_contract(_contract) {}
	/// Contracts can be implicitly converted to super classes and to addresses.
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	/// Contracts can be converted to themselves and to integers.
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool operator==(Type const& _other) const override;
	virtual u256 getStorageSize() const override;
	virtual bool isValueType() const override { return true; }
	virtual std::string toString() const override;

	virtual MemberList const& getMembers() const override;

	/// Returns the function type of the constructor. Note that the location part of the function type
	/// is not used, as this type cannot be the type of a variable or expression.
	std::shared_ptr<FunctionType const> const& getConstructorType() const;

	/// @returns the identifier of the function with the given name or Invalid256 if such a name does
	/// not exist.
	u256 getFunctionIdentifier(std::string const& _functionName) const;

private:
	ContractDefinition const& m_contract;
	/// Type of the constructor, @see getConstructorType. Lazily initialized.
	mutable std::shared_ptr<FunctionType const> m_constructorType;
	/// List of member types, will be lazy-initialized because of recursive references.
	mutable std::unique_ptr<MemberList> m_members;
};

/**
 * The type of a struct instance, there is one distinct type per struct definition.
 */
class StructType: public Type
{
public:
	virtual Category getCategory() const override { return Category::STRUCT; }
	StructType(StructDefinition const& _struct): m_struct(_struct) {}
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override
	{
		return _operator == Token::DELETE ? shared_from_this() : TypePointer();
	}

	virtual bool operator==(Type const& _other) const override;
	virtual u256 getStorageSize() const override;
	virtual bool canLiveOutsideStorage() const override;
	virtual unsigned getSizeOnStack() const override { return 1; /*@todo*/ }
	virtual std::string toString() const override;

	virtual MemberList const& getMembers() const override;

	u256 getStorageOffsetOfMember(std::string const& _name) const;

private:
	StructDefinition const& m_struct;
	/// List of member types, will be lazy-initialized because of recursive references.
	mutable std::unique_ptr<MemberList> m_members;
};

/**
 * The type of a function, identified by its (return) parameter types.
 * @todo the return parameters should also have names, i.e. return parameters should be a struct
 * type.
 */
class FunctionType: public Type
{
public:
	/// The meaning of the value(s) on the stack referencing the function:
	/// INTERNAL: jump tag, EXTERNAL: contract address + function identifier,
	/// BARE: contract address (non-abi contract call)
	/// OTHERS: special virtual function, nothing on the stack
	enum class Location { INTERNAL, EXTERNAL, SEND,
						  SHA3, SUICIDE,
						  ECRECOVER, SHA256, RIPEMD160,
						  LOG0, LOG1, LOG2, LOG3, LOG4,
						  SET_GAS, SET_VALUE,
						  BARE };

	virtual Category getCategory() const override { return Category::FUNCTION; }
	explicit FunctionType(FunctionDefinition const& _function, bool _isInternal = true);
	FunctionType(strings const& _parameterTypes, strings const& _returnParameterTypes,
				 Location _location = Location::INTERNAL):
		FunctionType(parseElementaryTypeVector(_parameterTypes), parseElementaryTypeVector(_returnParameterTypes),
					 _location) {}
	FunctionType(TypePointers const& _parameterTypes, TypePointers const& _returnParameterTypes,
				 Location _location = Location::INTERNAL,
				 bool _gasSet = false, bool _valueSet = false);

	TypePointers const& getParameterTypes() const { return m_parameterTypes; }
	TypePointers const& getReturnParameterTypes() const { return m_returnParameterTypes; }

	virtual bool operator==(Type const& _other) const override;
	virtual std::string toString() const override;
	virtual bool canBeStored() const override { return false; }
	virtual u256 getStorageSize() const override { BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage size of non-storable function type requested.")); }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned getSizeOnStack() const override;
	virtual MemberList const& getMembers() const override;

	Location const& getLocation() const { return m_location; }
	std::string getCanonicalSignature() const;

	bool gasSet() const { return m_gasSet; }
	bool valueSet() const { return m_valueSet; }

	/// @returns a copy of this type, where gas or value are set manually. This will never set one
	/// of the parameters to fals.
	TypePointer copyAndSetGasOrValue(bool _setGas, bool _setValue) const;

private:
	static TypePointers parseElementaryTypeVector(strings const& _types);

	TypePointers m_parameterTypes;
	TypePointers m_returnParameterTypes;
	Location m_location;
	unsigned m_sizeOnStack = 0;
	bool m_gasSet = false; ///< true iff the gas value to be used is on the stack
	bool m_valueSet = false; ///< true iff the value to be sent is on the stack
	mutable std::unique_ptr<MemberList> m_members;
};

/**
 * The type of a mapping, there is one distinct type per key/value type pair.
 */
class MappingType: public Type
{
public:
	virtual Category getCategory() const override { return Category::MAPPING; }
	MappingType(TypePointer const& _keyType, TypePointer const& _valueType):
		m_keyType(_keyType), m_valueType(_valueType) {}

	virtual bool operator==(Type const& _other) const override;
	virtual std::string toString() const override;
	virtual bool canLiveOutsideStorage() const override { return false; }

	TypePointer const& getKeyType() const { return m_keyType; }
	TypePointer const& getValueType() const { return m_valueType; }

private:
	TypePointer m_keyType;
	TypePointer m_valueType;
};

/**
 * The void type, can only be implicitly used as the type that is returned by functions without
 * return parameters.
 */
class VoidType: public Type
{
public:
	virtual Category getCategory() const override { return Category::VOID; }
	VoidType() {}

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual std::string toString() const override { return "void"; }
	virtual bool canBeStored() const override { return false; }
	virtual u256 getStorageSize() const override { BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage size of non-storable void type requested.")); }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned getSizeOnStack() const override { return 0; }
};

/**
 * The type of a type reference. The type of "uint32" when used in "a = uint32(2)" is an example
 * of a TypeType.
 */
class TypeType: public Type
{
public:
	virtual Category getCategory() const override { return Category::TYPE; }
	TypeType(TypePointer const& _actualType): m_actualType(_actualType) {}
	TypePointer const& getActualType() const { return m_actualType; }

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual bool operator==(Type const& _other) const override;
	virtual bool canBeStored() const override { return false; }
	virtual u256 getStorageSize() const override { BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage size of non-storable type type requested.")); }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual std::string toString() const override { return "type(" + m_actualType->toString() + ")"; }

private:
	TypePointer m_actualType;
};


/**
 * Special type for magic variables (block, msg, tx), similar to a struct but without any reference
 * (it always references a global singleton by name).
 */
class MagicType: public Type
{
public:
	enum class Kind { BLOCK, MSG, TX };
	virtual Category getCategory() const override { return Category::MAGIC; }

	MagicType(Kind _kind);

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override
	{
		return TypePointer();
	}

	virtual bool operator==(Type const& _other) const;
	virtual bool canBeStored() const override { return false; }
	virtual bool canLiveOutsideStorage() const override { return true; }
	virtual unsigned getSizeOnStack() const override { return 0; }
	virtual MemberList const& getMembers() const override { return m_members; }

	virtual std::string toString() const override;

private:
	Kind m_kind;

	MemberList m_members;
};

}
}
