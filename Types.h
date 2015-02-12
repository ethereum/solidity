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
using FunctionTypePointer = std::shared_ptr<FunctionType const>;
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
		Integer, IntegerConstant, Bool, Real, String,
		ByteArray, Mapping,
		Contract, Struct, Function,
		Void, TypeType, Modifier, Magic
	};

	///@{
	///@name Factory functions
	/// Factory functions that convert an AST @ref TypeName to a Type.
	static TypePointer fromElementaryTypeName(Token::Value _typeToken);
	static TypePointer fromElementaryTypeName(std::string const& _name);
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
	/// @returns the real type of some types, like e.g: IntegerConstant
	virtual TypePointer getRealType() const { return shared_from_this(); }

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
		Unsigned, Signed, Hash, Address
	};
	virtual Category getCategory() const override { return Category::Integer; }

	explicit IntegerType(int _bits, Modifier _modifier = Modifier::Unsigned);

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
	bool isHash() const { return m_modifier == Modifier::Hash || m_modifier == Modifier::Address; }
	bool isAddress() const { return m_modifier == Modifier::Address; }
	bool isSigned() const { return m_modifier == Modifier::Signed; }

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
	virtual Category getCategory() const override { return Category::IntegerConstant; }

	explicit IntegerConstantType(Literal const& _literal);
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
	virtual TypePointer getRealType() const override;

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
	virtual Category getCategory() const override { return Category::String; }

	/// @returns the smallest string type for the given literal or an empty pointer
	/// if no type fits.
	static std::shared_ptr<StaticStringType> smallestTypeForLiteral(std::string const& _literal);

	explicit StaticStringType(int _bytes);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
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
	virtual Category getCategory() const override { return Category::Bool; }
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual TypePointer binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const override;

	virtual unsigned getCalldataEncodedSize() const { return 1; }
	virtual bool isValueType() const override { return true; }

	virtual std::string toString() const override { return "bool"; }
	virtual u256 literalValue(Literal const* _literal) const override;
};

/**
 * The type of a byte array, prototype for a general array.
 */
class ByteArrayType: public Type
{
public:
	enum class Location { Storage, CallData, Memory };

	virtual Category getCategory() const override { return Category::ByteArray; }
	explicit ByteArrayType(Location _location): m_location(_location) {}
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual bool operator==(const Type& _other) const override;
	virtual unsigned getSizeOnStack() const override;
	virtual std::string toString() const override { return "bytes"; }
	virtual MemberList const& getMembers() const override { return s_byteArrayMemberList; }

	Location getLocation() const { return m_location; }

private:
	Location m_location;
	static const MemberList s_byteArrayMemberList;
};

/**
 * The type of a contract instance, there is one distinct type for each contract definition.
 */
class ContractType: public Type
{
public:
	virtual Category getCategory() const override { return Category::Contract; }
	explicit ContractType(ContractDefinition const& _contract, bool _super = false):
		m_contract(_contract), m_super(_super) {}
	/// Contracts can be implicitly converted to super classes and to addresses.
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	/// Contracts can be converted to themselves and to integers.
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
	virtual bool operator==(Type const& _other) const override;
	virtual bool isValueType() const override { return true; }
	virtual std::string toString() const override;

	virtual MemberList const& getMembers() const override;

	bool isSuper() const { return m_super; }
	ContractDefinition const& getContractDefinition() const { return m_contract; }

	/// Returns the function type of the constructor. Note that the location part of the function type
	/// is not used, as this type cannot be the type of a variable or expression.
	FunctionTypePointer const& getConstructorType() const;

	/// @returns the identifier of the function with the given name or Invalid256 if such a name does
	/// not exist.
	u256 getFunctionIdentifier(std::string const& _functionName) const;

private:
	ContractDefinition const& m_contract;
	/// If true, it is the "super" type of the current contract, i.e. it contains only inherited
	/// members.
	bool m_super;
	/// Type of the constructor, @see getConstructorType. Lazily initialized.
	mutable FunctionTypePointer m_constructorType;
	/// List of member types, will be lazy-initialized because of recursive references.
	mutable std::unique_ptr<MemberList> m_members;
};

/**
 * The type of a struct instance, there is one distinct type per struct definition.
 */
class StructType: public Type
{
public:
	virtual Category getCategory() const override { return Category::Struct; }
	explicit StructType(StructDefinition const& _struct): m_struct(_struct) {}
	virtual TypePointer unaryOperatorResult(Token::Value _operator) const override;
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
	/// @todo This documentation is outdated, and Location should rather be named "Type"
	enum class Location { Internal, External, Creation, Send,
						  SHA3, Suicide,
						  ECRecover, SHA256, RIPEMD160,
						  Log0, Log1, Log2, Log3, Log4, Event,
						  SetGas, SetValue, BlockHash,
						  Bare };

	virtual Category getCategory() const override { return Category::Function; }
	explicit FunctionType(FunctionDefinition const& _function, bool _isInternal = true);
	explicit FunctionType(VariableDeclaration const& _varDecl);
	explicit FunctionType(EventDefinition const& _event);
	FunctionType(strings const& _parameterTypes, strings const& _returnParameterTypes,
				 Location _location = Location::Internal, bool _arbitraryParameters = false):
		FunctionType(parseElementaryTypeVector(_parameterTypes), parseElementaryTypeVector(_returnParameterTypes),
					 _location, _arbitraryParameters) {}
	FunctionType(TypePointers const& _parameterTypes, TypePointers const& _returnParameterTypes,
				 Location _location = Location::Internal,
				 bool _arbitraryParameters = false, bool _gasSet = false, bool _valueSet = false):
		m_parameterTypes(_parameterTypes), m_returnParameterTypes(_returnParameterTypes),
		m_location(_location),
		m_arbitraryParameters(_arbitraryParameters), m_gasSet(_gasSet), m_valueSet(_valueSet) {}

	TypePointers const& getParameterTypes() const { return m_parameterTypes; }
	std::vector<std::string> const& getParameterNames() const { return m_parameterNames; }
	std::vector<std::string> const getParameterTypeNames() const;
	TypePointers const& getReturnParameterTypes() const { return m_returnParameterTypes; }
	std::vector<std::string> const& getReturnParameterNames() const { return m_returnParameterNames; }
	std::vector<std::string> const getReturnParameterTypeNames() const;

	virtual bool operator==(Type const& _other) const override;
	virtual std::string toString() const override;
	virtual bool canBeStored() const override { return false; }
	virtual u256 getStorageSize() const override { BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage size of non-storable function type requested.")); }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned getSizeOnStack() const override;
	virtual MemberList const& getMembers() const override;

	Location const& getLocation() const { return m_location; }
	/// @returns the canonical signature of this function type given the function name
	/// If @a _name is not provided (empty string) then the @c m_declaration member of the
	/// function type is used
	std::string getCanonicalSignature(std::string const& _name = "") const;
	Declaration const& getDeclaration() const
	{
		solAssert(m_declaration, "Requested declaration from a FunctionType that has none");
		return *m_declaration;
	}
	bool hasDeclaration() const { return !!m_declaration; }
	bool isConstant() const { return m_isConstant; }
	/// @return A shared pointer of an ASTString.
	/// Can contain a nullptr in which case indicates absence of documentation
	ASTPointer<ASTString> getDocumentation() const;

	/// true iff arguments are to be padded to multiples of 32 bytes for external calls
	bool padArguments() const { return !(m_location == Location::SHA3 || m_location == Location::SHA256 || m_location == Location::RIPEMD160); }
	bool takesArbitraryParameters() const { return m_arbitraryParameters; }
	bool gasSet() const { return m_gasSet; }
	bool valueSet() const { return m_valueSet; }

	/// @returns a copy of this type, where gas or value are set manually. This will never set one
	/// of the parameters to fals.
	TypePointer copyAndSetGasOrValue(bool _setGas, bool _setValue) const;

private:
	static TypePointers parseElementaryTypeVector(strings const& _types);

	TypePointers m_parameterTypes;
	TypePointers m_returnParameterTypes;
	std::vector<std::string> m_parameterNames;
	std::vector<std::string> m_returnParameterNames;
	Location const m_location;
	/// true iff the function takes an arbitrary number of arguments of arbitrary types
	bool const m_arbitraryParameters = false;
	bool const m_gasSet = false; ///< true iff the gas value to be used is on the stack
	bool const m_valueSet = false; ///< true iff the value to be sent is on the stack
	bool m_isConstant;
	mutable std::unique_ptr<MemberList> m_members;
	Declaration const* m_declaration = nullptr;
};

/**
 * The type of a mapping, there is one distinct type per key/value type pair.
 */
class MappingType: public Type
{
public:
	virtual Category getCategory() const override { return Category::Mapping; }
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
	virtual Category getCategory() const override { return Category::Void; }
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
	virtual Category getCategory() const override { return Category::TypeType; }
	explicit TypeType(TypePointer const& _actualType, ContractDefinition const* _currentContract = nullptr):
		m_actualType(_actualType), m_currentContract(_currentContract) {}
	TypePointer const& getActualType() const { return m_actualType; }

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual bool operator==(Type const& _other) const override;
	virtual bool canBeStored() const override { return false; }
	virtual u256 getStorageSize() const override { BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage size of non-storable type type requested.")); }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned getSizeOnStack() const override { return 0; }
	virtual std::string toString() const override { return "type(" + m_actualType->toString() + ")"; }
	virtual MemberList const& getMembers() const override;

private:
	TypePointer m_actualType;
	/// Context in which this type is used (influences visibility etc.), can be nullptr.
	ContractDefinition const* m_currentContract;
	/// List of member types, will be lazy-initialized because of recursive references.
	mutable std::unique_ptr<MemberList> m_members;
};


/**
 * The type of a function modifier. Not used for anything for now.
 */
class ModifierType: public Type
{
public:
	virtual Category getCategory() const override { return Category::Modifier; }
	explicit ModifierType(ModifierDefinition const& _modifier);

	virtual TypePointer binaryOperatorResult(Token::Value, TypePointer const&) const override { return TypePointer(); }
	virtual bool canBeStored() const override { return false; }
	virtual u256 getStorageSize() const override { BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage size of non-storable type type requested.")); }
	virtual bool canLiveOutsideStorage() const override { return false; }
	virtual unsigned getSizeOnStack() const override { return 0; }
	virtual bool operator==(Type const& _other) const override;
	virtual std::string toString() const override;

private:
	TypePointers m_parameterTypes;
};


/**
 * Special type for magic variables (block, msg, tx), similar to a struct but without any reference
 * (it always references a global singleton by name).
 */
class MagicType: public Type
{
public:
	enum class Kind { Block, Message, Transaction };
	virtual Category getCategory() const override { return Category::Magic; }

	explicit MagicType(Kind _kind);

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
