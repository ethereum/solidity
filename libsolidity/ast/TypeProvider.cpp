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

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::util;

BoolType const TypeProvider::m_boolean{};
InaccessibleDynamicType const TypeProvider::m_inaccessibleDynamic{};

/// The string and bytes unique_ptrs are initialized when they are first used because
/// they rely on `byte` being available which we cannot guarantee in the static init context.
unique_ptr<ArrayType> TypeProvider::m_bytesStorage;
unique_ptr<ArrayType> TypeProvider::m_bytesMemory;
unique_ptr<ArrayType> TypeProvider::m_bytesCalldata;
unique_ptr<ArrayType> TypeProvider::m_stringStorage;
unique_ptr<ArrayType> TypeProvider::m_stringMemory;

TupleType const TypeProvider::m_emptyTuple{};
AddressType const TypeProvider::m_payableAddress{StateMutability::Payable};
AddressType const TypeProvider::m_address{StateMutability::NonPayable};

array<unique_ptr<IntegerType>, 32> const TypeProvider::m_intM{{
	{make_unique<IntegerType>(8 * 1, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 2, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 3, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 4, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 5, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 6, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 7, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 8, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 9, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 10, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 11, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 12, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 13, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 14, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 15, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 16, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 17, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 18, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 19, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 20, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 21, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 22, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 23, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 24, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 25, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 26, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 27, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 28, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 29, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 30, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 31, IntegerType::Modifier::Signed)},
	{make_unique<IntegerType>(8 * 32, IntegerType::Modifier::Signed)}
}};

array<unique_ptr<IntegerType>, 32> const TypeProvider::m_uintM{{
	{make_unique<IntegerType>(8 * 1, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 2, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 3, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 4, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 5, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 6, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 7, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 8, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 9, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 10, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 11, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 12, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 13, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 14, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 15, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 16, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 17, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 18, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 19, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 20, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 21, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 22, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 23, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 24, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 25, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 26, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 27, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 28, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 29, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 30, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 31, IntegerType::Modifier::Unsigned)},
	{make_unique<IntegerType>(8 * 32, IntegerType::Modifier::Unsigned)}
}};

array<unique_ptr<FixedBytesType>, 32> const TypeProvider::m_bytesM{{
	{make_unique<FixedBytesType>(1)},
	{make_unique<FixedBytesType>(2)},
	{make_unique<FixedBytesType>(3)},
	{make_unique<FixedBytesType>(4)},
	{make_unique<FixedBytesType>(5)},
	{make_unique<FixedBytesType>(6)},
	{make_unique<FixedBytesType>(7)},
	{make_unique<FixedBytesType>(8)},
	{make_unique<FixedBytesType>(9)},
	{make_unique<FixedBytesType>(10)},
	{make_unique<FixedBytesType>(11)},
	{make_unique<FixedBytesType>(12)},
	{make_unique<FixedBytesType>(13)},
	{make_unique<FixedBytesType>(14)},
	{make_unique<FixedBytesType>(15)},
	{make_unique<FixedBytesType>(16)},
	{make_unique<FixedBytesType>(17)},
	{make_unique<FixedBytesType>(18)},
	{make_unique<FixedBytesType>(19)},
	{make_unique<FixedBytesType>(20)},
	{make_unique<FixedBytesType>(21)},
	{make_unique<FixedBytesType>(22)},
	{make_unique<FixedBytesType>(23)},
	{make_unique<FixedBytesType>(24)},
	{make_unique<FixedBytesType>(25)},
	{make_unique<FixedBytesType>(26)},
	{make_unique<FixedBytesType>(27)},
	{make_unique<FixedBytesType>(28)},
	{make_unique<FixedBytesType>(29)},
	{make_unique<FixedBytesType>(30)},
	{make_unique<FixedBytesType>(31)},
	{make_unique<FixedBytesType>(32)}
}};

array<unique_ptr<MagicType>, 4> const TypeProvider::m_magics{{
	{make_unique<MagicType>(MagicType::Kind::Block)},
	{make_unique<MagicType>(MagicType::Kind::Message)},
	{make_unique<MagicType>(MagicType::Kind::Transaction)},
	{make_unique<MagicType>(MagicType::Kind::ABI)}
	// MetaType is stored separately
}};

inline void clearCache(Type const& type)
{
	type.clearCache();
}

template <typename T>
inline void clearCache(unique_ptr<T> const& type)
{
	// Some lazy-initialized types might not exist yet.
	if (type)
		type->clearCache();
}

template <typename Container>
inline void clearCaches(Container& container)
{
	for (auto const& e: container)
		clearCache(e);
}

void TypeProvider::reset()
{
	clearCache(m_boolean);
	clearCache(m_inaccessibleDynamic);
	clearCache(m_bytesStorage);
	clearCache(m_bytesMemory);
	clearCache(m_bytesCalldata);
	clearCache(m_stringStorage);
	clearCache(m_stringMemory);
	clearCache(m_emptyTuple);
	clearCache(m_payableAddress);
	clearCache(m_address);
	clearCaches(instance().m_intM);
	clearCaches(instance().m_uintM);
	clearCaches(instance().m_bytesM);
	clearCaches(instance().m_magics);

	instance().m_generalTypes.clear();
	instance().m_stringLiteralTypes.clear();
	instance().m_ufixedMxN.clear();
	instance().m_fixedMxN.clear();
}

template <typename T, typename... Args>
inline T const* TypeProvider::createAndGet(Args&& ... _args)
{
	instance().m_generalTypes.emplace_back(make_unique<T>(std::forward<Args>(_args)...));
	return static_cast<T const*>(instance().m_generalTypes.back().get());
}

Type const* TypeProvider::fromElementaryTypeName(ElementaryTypeNameToken const& _type, std::optional<StateMutability> _stateMutability)
{
	solAssert(
		TokenTraits::isElementaryTypeName(_type.token()),
		"Expected an elementary type name but got " + _type.toString()
	);

	unsigned const m = _type.firstNumber();
	unsigned const n = _type.secondNumber();

	switch (_type.token())
	{
	case Token::IntM:
		return integer(m, IntegerType::Modifier::Signed);
	case Token::UIntM:
		return integer(m, IntegerType::Modifier::Unsigned);
	case Token::Byte:
		return byte();
	case Token::BytesM:
		return fixedBytes(m);
	case Token::FixedMxN:
		return fixedPoint(m, n, FixedPointType::Modifier::Signed);
	case Token::UFixedMxN:
		return fixedPoint(m, n, FixedPointType::Modifier::Unsigned);
	case Token::Int:
		return integer(256, IntegerType::Modifier::Signed);
	case Token::UInt:
		return integer(256, IntegerType::Modifier::Unsigned);
	case Token::Fixed:
		return fixedPoint(128, 18, FixedPointType::Modifier::Signed);
	case Token::UFixed:
		return fixedPoint(128, 18, FixedPointType::Modifier::Unsigned);
	case Token::Address:
	{
		if (_stateMutability)
		{
			solAssert(*_stateMutability == StateMutability::Payable, "");
			return payableAddress();
		}
		return address();
	}
	case Token::Bool:
		return boolean();
	case Token::Bytes:
		return bytesStorage();
	case Token::String:
		return stringStorage();
	default:
		solAssert(
			false,
			"Unable to convert elementary typename " + _type.toString() + " to type."
		);
	}
}

Type const* TypeProvider::fromElementaryTypeName(string const& _name)
{
	vector<string> nameParts;
	boost::split(nameParts, _name, boost::is_any_of(" "));
	solAssert(nameParts.size() == 1 || nameParts.size() == 2, "Cannot parse elementary type: " + _name);

	Token token;
	unsigned short firstNum, secondNum;
	tie(token, firstNum, secondNum) = TokenTraits::fromIdentifierOrKeyword(nameParts[0]);

	auto t = fromElementaryTypeName(ElementaryTypeNameToken(token, firstNum, secondNum));
	if (auto* ref = dynamic_cast<ReferenceType const*>(t))
	{
		DataLocation location = DataLocation::Storage;
		if (nameParts.size() == 2)
		{
			if (nameParts[1] == "storage")
				location = DataLocation::Storage;
			else if (nameParts[1] == "calldata")
				location = DataLocation::CallData;
			else if (nameParts[1] == "memory")
				location = DataLocation::Memory;
			else
				solAssert(false, "Unknown data location: " + nameParts[1]);
		}
		return withLocation(ref, location, true);
	}
	else if (t->category() == Type::Category::Address)
	{
		if (nameParts.size() == 2)
		{
			if (nameParts[1] == "payable")
				return payableAddress();
			else
				solAssert(false, "Invalid state mutability for address type: " + nameParts[1]);
		}
		return address();
	}
	else
	{
		solAssert(nameParts.size() == 1, "Storage location suffix only allowed for reference types");
		return t;
	}
}

ArrayType const* TypeProvider::bytesStorage()
{
	if (!m_bytesStorage)
		m_bytesStorage = make_unique<ArrayType>(DataLocation::Storage, false);
	return m_bytesStorage.get();
}

ArrayType const* TypeProvider::bytesMemory()
{
	if (!m_bytesMemory)
		m_bytesMemory = make_unique<ArrayType>(DataLocation::Memory, false);
	return m_bytesMemory.get();
}

ArrayType const* TypeProvider::bytesCalldata()
{
	if (!m_bytesCalldata)
		m_bytesCalldata = make_unique<ArrayType>(DataLocation::CallData, false);
	return m_bytesCalldata.get();
}

ArrayType const* TypeProvider::stringStorage()
{
	if (!m_stringStorage)
		m_stringStorage = make_unique<ArrayType>(DataLocation::Storage, true);
	return m_stringStorage.get();
}

ArrayType const* TypeProvider::stringMemory()
{
	if (!m_stringMemory)
		m_stringMemory = make_unique<ArrayType>(DataLocation::Memory, true);
	return m_stringMemory.get();
}

Type const* TypeProvider::forLiteral(Literal const& _literal)
{
	switch (_literal.token())
	{
	case Token::TrueLiteral:
	case Token::FalseLiteral:
		return boolean();
	case Token::Number:
		return rationalNumber(_literal);
	case Token::StringLiteral:
	case Token::UnicodeStringLiteral:
	case Token::HexStringLiteral:
		return stringLiteral(_literal.value());
	default:
		return nullptr;
	}
}

RationalNumberType const* TypeProvider::rationalNumber(Literal const& _literal)
{
	solAssert(_literal.token() == Token::Number, "");
	std::tuple<bool, rational> validLiteral = RationalNumberType::isValidLiteral(_literal);
	if (std::get<0>(validLiteral))
	{
		Type const* compatibleBytesType = nullptr;
		if (_literal.isHexNumber())
		{
			size_t const digitCount = _literal.valueWithoutUnderscores().length() - 2;
			if (digitCount % 2 == 0 && (digitCount / 2) <= 32)
				compatibleBytesType = fixedBytes(static_cast<unsigned>(digitCount / 2));
		}

		return rationalNumber(std::get<1>(validLiteral), compatibleBytesType);
	}
	return nullptr;
}

StringLiteralType const* TypeProvider::stringLiteral(string const& literal)
{
	auto i = instance().m_stringLiteralTypes.find(literal);
	if (i != instance().m_stringLiteralTypes.end())
		return i->second.get();
	else
		return instance().m_stringLiteralTypes.emplace(literal, make_unique<StringLiteralType>(literal)).first->second.get();
}

FixedPointType const* TypeProvider::fixedPoint(unsigned m, unsigned n, FixedPointType::Modifier _modifier)
{
	auto& map = _modifier == FixedPointType::Modifier::Unsigned ? instance().m_ufixedMxN : instance().m_fixedMxN;

	auto i = map.find(make_pair(m, n));
	if (i != map.end())
		return i->second.get();

	return map.emplace(
		make_pair(m, n),
		make_unique<FixedPointType>(m, n, _modifier)
	).first->second.get();
}

TupleType const* TypeProvider::tuple(vector<Type const*> members)
{
	if (members.empty())
		return &m_emptyTuple;

	return createAndGet<TupleType>(std::move(members));
}

ReferenceType const* TypeProvider::withLocation(ReferenceType const* _type, DataLocation _location, bool _isPointer)
{
	if (_type->location() == _location && _type->isPointer() == _isPointer)
		return _type;

	instance().m_generalTypes.emplace_back(_type->copyForLocation(_location, _isPointer));
	return static_cast<ReferenceType const*>(instance().m_generalTypes.back().get());
}

FunctionType const* TypeProvider::function(FunctionDefinition const& _function, FunctionType::Kind _kind)
{
	return createAndGet<FunctionType>(_function, _kind);
}

FunctionType const* TypeProvider::function(VariableDeclaration const& _varDecl)
{
	return createAndGet<FunctionType>(_varDecl);
}

FunctionType const* TypeProvider::function(EventDefinition const& _def)
{
	return createAndGet<FunctionType>(_def);
}

FunctionType const* TypeProvider::function(ErrorDefinition const& _def)
{
	return createAndGet<FunctionType>(_def);
}

FunctionType const* TypeProvider::function(FunctionTypeName const& _typeName)
{
	return createAndGet<FunctionType>(_typeName);
}

FunctionType const* TypeProvider::function(
	strings const& _parameterTypes,
	strings const& _returnParameterTypes,
	FunctionType::Kind _kind,
	StateMutability _stateMutability,
	FunctionType::Options _options
)
{
	// Can only use this constructor for "arbitraryParameters".
	solAssert(!_options.valueSet && !_options.gasSet && !_options.saltSet && !_options.boundToType);
	return createAndGet<FunctionType>(
		_parameterTypes,
		_returnParameterTypes,
		_kind,
		_stateMutability,
		std::move(_options)
	);
}

FunctionType const* TypeProvider::function(
	TypePointers const& _parameterTypes,
	TypePointers const& _returnParameterTypes,
	strings _parameterNames,
	strings _returnParameterNames,
	FunctionType::Kind _kind,
	StateMutability _stateMutability,
	Declaration const* _declaration,
	FunctionType::Options _options
)
{
	return createAndGet<FunctionType>(
		_parameterTypes,
		_returnParameterTypes,
		_parameterNames,
		_returnParameterNames,
		_kind,
		_stateMutability,
		_declaration,
		std::move(_options)
	);
}

RationalNumberType const* TypeProvider::rationalNumber(rational const& _value, Type const* _compatibleBytesType)
{
	return createAndGet<RationalNumberType>(_value, _compatibleBytesType);
}

ArrayType const* TypeProvider::array(DataLocation _location, bool _isString)
{
	if (_isString)
	{
		if (_location == DataLocation::Storage)
			return stringStorage();
		if (_location == DataLocation::Memory)
			return stringMemory();
	}
	else
	{
		if (_location == DataLocation::Storage)
			return bytesStorage();
		if (_location == DataLocation::Memory)
			return bytesMemory();
	}
	return createAndGet<ArrayType>(_location, _isString);
}

ArrayType const* TypeProvider::array(DataLocation _location, Type const* _baseType)
{
	return createAndGet<ArrayType>(_location, _baseType);
}

ArrayType const* TypeProvider::array(DataLocation _location, Type const* _baseType, u256 const& _length)
{
	return createAndGet<ArrayType>(_location, _baseType, _length);
}

ArraySliceType const* TypeProvider::arraySlice(ArrayType const& _arrayType)
{
	return createAndGet<ArraySliceType>(_arrayType);
}

ContractType const* TypeProvider::contract(ContractDefinition const& _contractDef, bool _isSuper)
{
	return createAndGet<ContractType>(_contractDef, _isSuper);
}

EnumType const* TypeProvider::enumType(EnumDefinition const& _enumDef)
{
	return createAndGet<EnumType>(_enumDef);
}

ModuleType const* TypeProvider::module(SourceUnit const& _source)
{
	return createAndGet<ModuleType>(_source);
}

TypeType const* TypeProvider::typeType(Type const* _actualType)
{
	return createAndGet<TypeType>(_actualType);
}

StructType const* TypeProvider::structType(StructDefinition const& _struct, DataLocation _location)
{
	return createAndGet<StructType>(_struct, _location);
}

ModifierType const* TypeProvider::modifier(ModifierDefinition const& _def)
{
	return createAndGet<ModifierType>(_def);
}

MagicType const* TypeProvider::magic(MagicType::Kind _kind)
{
	solAssert(_kind != MagicType::Kind::MetaType, "MetaType is handled separately");
	return m_magics.at(static_cast<size_t>(_kind)).get();
}

MagicType const* TypeProvider::meta(Type const* _type)
{
	solAssert(
		_type && (
			_type->category() == Type::Category::Contract ||
			_type->category() == Type::Category::Integer ||
			_type->category() == Type::Category::Enum
		),
		"Only enum, contracts or integer types supported for now."
	);
	return createAndGet<MagicType>(_type);
}

MappingType const* TypeProvider::mapping(Type const* _keyType, Type const* _valueType)
{
	return createAndGet<MappingType>(_keyType, _valueType);
}

UserDefinedValueType const* TypeProvider::userDefinedValueType(UserDefinedValueTypeDefinition const& _definition)
{
	return createAndGet<UserDefinedValueType>(_definition);
}
