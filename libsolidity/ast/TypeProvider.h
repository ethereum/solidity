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

#pragma once

#include <libsolidity/ast/Types.h>

#include <array>
#include <map>
#include <memory>
#include <utility>

namespace dev
{
namespace solidity
{

/**
 * API for accessing the Solidity Type System.
 *
 * This is the Solidity Compiler's type provider. Use it to request for types. The caller does
 * <b>not</b> own the types.
 *
 * It is not recommended to explicitly instantiate types unless you really know what and why
 * you are doing it.
 */
class TypeProvider
{
public:
	TypeProvider() = default;
	TypeProvider(TypeProvider&&) = default;
	TypeProvider(TypeProvider const&) = delete;
	TypeProvider& operator=(TypeProvider&&) = default;
	TypeProvider& operator=(TypeProvider const&) = delete;
	~TypeProvider() = default;

	/// Resets state of this TypeProvider to initial state, wiping all mutable types.
	/// This invalidates all dangling pointers to types provided by this TypeProvider.
	static void reset();

	/// @name Factory functions
	/// Factory functions that convert an AST @ref TypeName to a Type.
	static Type const* fromElementaryTypeName(ElementaryTypeNameToken const& _type);

	/// Converts a given elementary type name with optional data location
	/// suffix " storage", " calldata" or " memory" to a type pointer. If suffix not given, defaults to " storage".
	static TypePointer fromElementaryTypeName(std::string const& _name);

	/// @returns boolean type.
	static BoolType const* boolType() noexcept { return &m_boolType; }

	static FixedBytesType const* byteType() { return fixedBytesType(1); }
	static FixedBytesType const* fixedBytesType(unsigned m) { return m_bytesM.at(m - 1).get(); }

	static ArrayType const* bytesStorageType();
	static ArrayType const* bytesMemoryType();
	static ArrayType const* stringStorageType();
	static ArrayType const* stringMemoryType();

	/// Constructor for a byte array ("bytes") and string.
	static ArrayType const* arrayType(DataLocation _location, bool _isString = false);

	/// Constructor for a dynamically sized array type ("type[]")
	static ArrayType const* arrayType(DataLocation _location, Type const* _baseType);

	/// Constructor for a fixed-size array type ("type[20]")
	static ArrayType const* arrayType(DataLocation _location, Type const* _baseType, u256 const& _length);

	static AddressType const* payableAddressType() noexcept { return &m_payableAddressType; }
	static AddressType const* addressType() noexcept { return &m_addressType; }

	static IntegerType const* integerType(unsigned _bits, IntegerType::Modifier _modifier)
	{
		solAssert((_bits % 8) == 0, "");
		if (_modifier == IntegerType::Modifier::Unsigned)
			return m_uintM.at(_bits / 8 - 1).get();
		else
			return m_intM.at(_bits / 8 - 1).get();
	}
	static IntegerType const* uint(unsigned _bits) { return integerType(_bits, IntegerType::Modifier::Unsigned); }

	static IntegerType const* uint256() { return uint(256); }

	static FixedPointType const* fixedPointType(unsigned m, unsigned n, FixedPointType::Modifier _modifier);

	static StringLiteralType const* stringLiteralType(std::string const& literal);

	/// @param members the member types the tuple type must contain. This is passed by value on purspose.
	/// @returns a tuple type with the given members.
	static TupleType const* tupleType(std::vector<Type const*> members);

	static TupleType const* emptyTupleType() noexcept { return &m_emptyTupleType; }

	static ReferenceType const* withLocation(ReferenceType const* _type, DataLocation _location, bool _isPointer);

	/// @returns a copy of @a _type having the same location as this (and is not a pointer type)
	///          if _type is a reference type and an unmodified copy of _type otherwise.
	///          This function is mostly useful to modify inner types appropriately.
	static Type const* withLocationIfReference(DataLocation _location, Type const* _type)
	{
		if (auto refType = dynamic_cast<ReferenceType const*>(_type))
			return withLocation(refType, _location, false);

		return _type;
	}

	/// @returns the internally-facing or externally-facing type of a function.
	static FunctionType const* functionType(FunctionDefinition const& _function, bool _isInternal = true);

	/// @returns the accessor function type of a state variable.
	static FunctionType const* functionType(VariableDeclaration const& _varDecl);

	/// @returns the function type of an event.
	static FunctionType const* functionType(EventDefinition const& _event);

	/// @returns the type of a function type name.
	static FunctionType const* functionType(FunctionTypeName const& _typeName);

	/// @returns the function type to be used for a plain type (not derived from a declaration).
	static FunctionType const* functionType(
		strings const& _parameterTypes,
		strings const& _returnParameterTypes,
		FunctionType::Kind _kind = FunctionType::Kind::Internal,
		bool _arbitraryParameters = false,
		StateMutability _stateMutability = StateMutability::NonPayable
	);

	/// @returns a highly customized FunctionType, use with care.
	static FunctionType const* functionType(
		TypePointers const& _parameterTypes,
		TypePointers const& _returnParameterTypes,
		strings _parameterNames = strings{},
		strings _returnParameterNames = strings{},
		FunctionType::Kind _kind = FunctionType::Kind::Internal,
		bool _arbitraryParameters = false,
		StateMutability _stateMutability = StateMutability::NonPayable,
		Declaration const* _declaration = nullptr,
		bool _gasSet = false,
		bool _valueSet = false,
		bool _bound = false
	);

	/// Auto-detect the proper type for a literal. @returns an empty pointer if the literal does
	/// not fit any type.
	static TypePointer forLiteral(Literal const& _literal);
	static RationalNumberType const* rationalNumberType(Literal const& _literal);

	static RationalNumberType const* rationalNumberType(
		rational const& _value,
		Type const* _compatibleBytesType = nullptr
	);

	static ContractType const* contractType(ContractDefinition const& _contract, bool _isSuper = false);

	static InaccessibleDynamicType const* inaccessibleDynamicType() noexcept { return &m_inaccessibleDynamicType; }

	/// @returns the type of an enum instance for given definition, there is one distinct type per enum definition.
	static EnumType const* enumType(EnumDefinition const& _enum);

	/// @returns special type for imported modules. These mainly give access to their scope via members.
	static ModuleType const* moduleType(SourceUnit const& _source);

	static TypeType const* typeType(Type const* _actualType);

	static StructType const* structType(StructDefinition const& _struct, DataLocation _location);

	static ModifierType const* modifierType(ModifierDefinition const& _modifierDef);

	static MagicType const* magicType(MagicType::Kind _kind);

	static MagicType const* metaType(Type const* _type);

	static MappingType const* mappingType(Type const* _keyType, Type const* _valueType);

private:
	/// Global TypeProvider instance.
	static TypeProvider& instance()
	{
		static TypeProvider _provider;
		return _provider;
	}

	template <typename T, typename... Args>
	static inline T const* createAndGet(Args&& ... _args);

	static BoolType const m_boolType;
	static InaccessibleDynamicType const m_inaccessibleDynamicType;

	/// These are lazy-initialized because they depend on `byte` being available.
	static std::unique_ptr<ArrayType> m_bytesStorageType;
	static std::unique_ptr<ArrayType> m_bytesMemoryType;
	static std::unique_ptr<ArrayType> m_stringStorageType;
	static std::unique_ptr<ArrayType> m_stringMemoryType;

	static TupleType const m_emptyTupleType;
	static AddressType const m_payableAddressType;
	static AddressType const m_addressType;
	static std::array<std::unique_ptr<IntegerType>, 32> const m_intM;
	static std::array<std::unique_ptr<IntegerType>, 32> const m_uintM;
	static std::array<std::unique_ptr<FixedBytesType>, 32> const m_bytesM;
	static std::array<std::unique_ptr<MagicType>, 4> const m_magicTypes;     ///< MagicType's except MetaType

	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_ufixedMxN{};
	std::map<std::pair<unsigned, unsigned>, std::unique_ptr<FixedPointType>> m_fixedMxN{};
	std::map<std::string, std::unique_ptr<StringLiteralType>> m_stringLiteralTypes{};
	std::vector<std::unique_ptr<Type>> m_generalTypes{};
};

} // namespace solidity
} // namespace dev
