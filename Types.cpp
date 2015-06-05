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

#include <libsolidity/Types.h>
#include <limits>
#include <boost/range/adaptor/reversed.hpp>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/SHA3.h>
#include <libsolidity/Utils.h>
#include <libsolidity/AST.h>

using namespace std;

namespace dev
{
namespace solidity
{

void StorageOffsets::computeOffsets(TypePointers const& _types)
{
	bigint slotOffset = 0;
	unsigned byteOffset = 0;
	map<size_t, pair<u256, unsigned>> offsets;
	for (size_t i = 0; i < _types.size(); ++i)
	{
		TypePointer const& type = _types[i];
		if (!type->canBeStored())
			continue;
		if (byteOffset + type->getStorageBytes() > 32)
		{
			// would overflow, go to next slot
			++slotOffset;
			byteOffset = 0;
		}
		if (slotOffset >= bigint(1) << 256)
			BOOST_THROW_EXCEPTION(TypeError() << errinfo_comment("Object too large for storage."));
		offsets[i] = make_pair(u256(slotOffset), byteOffset);
		solAssert(type->getStorageSize() >= 1, "Invalid storage size.");
		if (type->getStorageSize() == 1 && byteOffset + type->getStorageBytes() <= 32)
			byteOffset += type->getStorageBytes();
		else
		{
			slotOffset += type->getStorageSize();
			byteOffset = 0;
		}
	}
	if (byteOffset > 0)
		++slotOffset;
	if (slotOffset >= bigint(1) << 256)
		BOOST_THROW_EXCEPTION(TypeError() << errinfo_comment("Object too large for storage."));
	m_storageSize = u256(slotOffset);
	swap(m_offsets, offsets);
}

pair<u256, unsigned> const* StorageOffsets::getOffset(size_t _index) const
{
	if (m_offsets.count(_index))
		return &m_offsets.at(_index);
	else
		return nullptr;
}

MemberList& MemberList::operator=(MemberList&& _other)
{
	m_memberTypes = std::move(_other.m_memberTypes);
	m_storageOffsets = std::move(_other.m_storageOffsets);
	return *this;
}

std::pair<u256, unsigned> const* MemberList::getMemberStorageOffset(string const& _name) const
{
	if (!m_storageOffsets)
	{
		TypePointers memberTypes;
		memberTypes.reserve(m_memberTypes.size());
		for (auto const& member: m_memberTypes)
			memberTypes.push_back(member.type);
		m_storageOffsets.reset(new StorageOffsets());
		m_storageOffsets->computeOffsets(memberTypes);
	}
	for (size_t index = 0; index < m_memberTypes.size(); ++index)
		if (m_memberTypes[index].name == _name)
			return m_storageOffsets->getOffset(index);
	return nullptr;
}

u256 const& MemberList::getStorageSize() const
{
	// trigger lazy computation
	getMemberStorageOffset("");
	return m_storageOffsets->getStorageSize();
}

TypePointer Type::fromElementaryTypeName(Token::Value _typeToken)
{
	char const* tokenCstr = Token::toString(_typeToken);
	solAssert(Token::isElementaryTypeName(_typeToken),
		"Expected an elementary type name but got " + ((tokenCstr) ? std::string(Token::toString(_typeToken)) : ""));

	if (Token::Int <= _typeToken && _typeToken <= Token::Bytes32)
	{
		int offset = _typeToken - Token::Int;
		int bytes = offset % 33;
		if (bytes == 0 && _typeToken != Token::Bytes0)
			bytes = 32;
		int modifier = offset / 33;
		switch(modifier)
		{
		case 0:
			return make_shared<IntegerType>(bytes * 8, IntegerType::Modifier::Signed);
		case 1:
			return make_shared<IntegerType>(bytes * 8, IntegerType::Modifier::Unsigned);
		case 2:
			return make_shared<FixedBytesType>(bytes);
		default:
			solAssert(false, "Unexpected modifier value. Should never happen");
			return TypePointer();
		}
	}
	else if (_typeToken == Token::Byte)
		return make_shared<FixedBytesType>(1);
	else if (_typeToken == Token::Address)
		return make_shared<IntegerType>(0, IntegerType::Modifier::Address);
	else if (_typeToken == Token::Bool)
		return make_shared<BoolType>();
	else if (_typeToken == Token::Bytes)
		return make_shared<ArrayType>(ReferenceType::Location::Storage);
	else if (_typeToken == Token::String)
		return make_shared<ArrayType>(ReferenceType::Location::Storage, true);
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unable to convert elementary typename " +
																		 std::string(Token::toString(_typeToken)) + " to type."));
}

TypePointer Type::fromElementaryTypeName(string const& _name)
{
	return fromElementaryTypeName(Token::fromIdentifierOrKeyword(_name));
}

TypePointer Type::fromUserDefinedTypeName(UserDefinedTypeName const& _typeName)
{
	Declaration const* declaration = _typeName.getReferencedDeclaration();
	if (StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(declaration))
		return make_shared<StructType>(*structDef);
	else if (EnumDefinition const* enumDef = dynamic_cast<EnumDefinition const*>(declaration))
		return make_shared<EnumType>(*enumDef);
	else if (FunctionDefinition const* function = dynamic_cast<FunctionDefinition const*>(declaration))
		return make_shared<FunctionType>(*function);
	else if (ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(declaration))
		return make_shared<ContractType>(*contract);
	return TypePointer();
}

TypePointer Type::fromMapping(ElementaryTypeName& _keyType, TypeName& _valueType)
{
	TypePointer keyType = _keyType.toType();
	if (!keyType)
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Error resolving type name."));
	TypePointer valueType = _valueType.toType();
	if (!valueType)
		BOOST_THROW_EXCEPTION(_valueType.createTypeError("Invalid type name."));
	return make_shared<MappingType>(keyType, valueType);
}

TypePointer Type::fromArrayTypeName(TypeName& _baseTypeName, Expression* _length)
{
	TypePointer baseType = _baseTypeName.toType();
	if (!baseType)
		BOOST_THROW_EXCEPTION(_baseTypeName.createTypeError("Invalid type name."));
	if (baseType->getStorageBytes() == 0)
		BOOST_THROW_EXCEPTION(_baseTypeName.createTypeError("Illegal base type of storage size zero for array."));
	if (_length)
	{
		if (!_length->getType())
			_length->checkTypeRequirements(nullptr);
		auto const* length = dynamic_cast<IntegerConstantType const*>(_length->getType().get());
		if (!length)
			BOOST_THROW_EXCEPTION(_length->createTypeError("Invalid array length."));
		return make_shared<ArrayType>(ReferenceType::Location::Storage, baseType, length->literalValue(nullptr));
	}
	else
		return make_shared<ArrayType>(ReferenceType::Location::Storage, baseType);
}

TypePointer Type::forLiteral(Literal const& _literal)
{
	switch (_literal.getToken())
	{
	case Token::TrueLiteral:
	case Token::FalseLiteral:
		return make_shared<BoolType>();
	case Token::Number:
		return make_shared<IntegerConstantType>(_literal);
	case Token::StringLiteral:
		//@todo put larger strings into dynamic strings
		return FixedBytesType::smallestTypeForLiteral(_literal.getValue());
	default:
		return shared_ptr<Type>();
	}
}

TypePointer Type::commonType(TypePointer const& _a, TypePointer const& _b)
{
	if (_b->isImplicitlyConvertibleTo(*_a))
		return _a;
	else if (_a->isImplicitlyConvertibleTo(*_b))
		return _b;
	else
		return TypePointer();
}

const MemberList Type::EmptyMemberList;

IntegerType::IntegerType(int _bits, IntegerType::Modifier _modifier):
	m_bits(_bits), m_modifier(_modifier)
{
	if (isAddress())
		m_bits = 160;
	solAssert(m_bits > 0 && m_bits <= 256 && m_bits % 8 == 0,
			  "Invalid bit number for integer type: " + dev::toString(_bits));
}

bool IntegerType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (_convertTo.getCategory() != getCategory())
		return false;
	IntegerType const& convertTo = dynamic_cast<IntegerType const&>(_convertTo);
	if (convertTo.m_bits < m_bits)
		return false;
	if (isAddress())
		return convertTo.isAddress();
	else if (isSigned())
		return convertTo.isSigned();
	else
		return !convertTo.isSigned() || convertTo.m_bits > m_bits;
}

bool IntegerType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	return _convertTo.getCategory() == getCategory() ||
		_convertTo.getCategory() == Category::Contract ||
		_convertTo.getCategory() == Category::Enum ||
		_convertTo.getCategory() == Category::FixedBytes;
}

TypePointer IntegerType::unaryOperatorResult(Token::Value _operator) const
{
	// "delete" is ok for all integer types
	if (_operator == Token::Delete)
		return make_shared<VoidType>();
	// no further unary operators for addresses
	else if (isAddress())
		return TypePointer();
	// for non-address integers, we allow +, -, ++ and --
	else if (_operator == Token::Add || _operator == Token::Sub ||
			_operator == Token::Inc || _operator == Token::Dec ||
			_operator == Token::After || _operator == Token::BitNot)
		return shared_from_this();
	else
		return TypePointer();
}

bool IntegerType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	IntegerType const& other = dynamic_cast<IntegerType const&>(_other);
	return other.m_bits == m_bits && other.m_modifier == m_modifier;
}

string IntegerType::toString() const
{
	if (isAddress())
		return "address";
	string prefix = isSigned() ? "int" : "uint";
	return prefix + dev::toString(m_bits);
}

TypePointer IntegerType::binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const
{
	if (_other->getCategory() != Category::IntegerConstant && _other->getCategory() != getCategory())
		return TypePointer();
	auto commonType = dynamic_pointer_cast<IntegerType const>(Type::commonType(shared_from_this(), _other));

	if (!commonType)
		return TypePointer();

	// All integer types can be compared
	if (Token::isCompareOp(_operator))
		return commonType;
	// Nothing else can be done with addresses
	if (commonType->isAddress())
		return TypePointer();

	return commonType;
}

const MemberList IntegerType::AddressMemberList({
	{"balance", make_shared<IntegerType >(256)},
	{"call", make_shared<FunctionType>(strings(), strings{"bool"}, FunctionType::Location::Bare, true)},
	{"callcode", make_shared<FunctionType>(strings(), strings{"bool"}, FunctionType::Location::BareCallCode, true)},
	{"send", make_shared<FunctionType>(strings{"uint"}, strings{"bool"}, FunctionType::Location::Send)}
});

IntegerConstantType::IntegerConstantType(Literal const& _literal)
{
	m_value = bigint(_literal.getValue());

	switch (_literal.getSubDenomination())
	{
	case Literal::SubDenomination::Wei:
	case Literal::SubDenomination::Second:
	case Literal::SubDenomination::None:
		break;
	case Literal::SubDenomination::Szabo:
		m_value *= bigint("1000000000000");
		break;
	case Literal::SubDenomination::Finney:
		m_value *= bigint("1000000000000000");
		break;
	case Literal::SubDenomination::Ether:
		m_value *= bigint("1000000000000000000");
		break;
	case Literal::SubDenomination::Minute:
		m_value *= bigint("60");
		break;
	case Literal::SubDenomination::Hour:
		m_value *= bigint("3600");
		break;
	case Literal::SubDenomination::Day:
		m_value *= bigint("86400");
		break;
	case Literal::SubDenomination::Week:
		m_value *= bigint("604800");
		break;
	case Literal::SubDenomination::Year:
		m_value *= bigint("31536000");
		break;
	}
}

bool IntegerConstantType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (auto targetType = dynamic_cast<IntegerType const*>(&_convertTo))
	{
		if (m_value == 0)
			return true;
		int forSignBit = (targetType->isSigned() ? 1 : 0);
		if (m_value > 0)
		{
			if (m_value <= (u256(-1) >> (256 - targetType->getNumBits() + forSignBit)))
				return true;
		}
		else if (targetType->isSigned() && -m_value <= (u256(1) << (targetType->getNumBits() - forSignBit)))
			return true;
		return false;
	}
	else if (_convertTo.getCategory() == Category::FixedBytes)
	{
		FixedBytesType const& fixedBytes = dynamic_cast<FixedBytesType const&>(_convertTo);
		return fixedBytes.getNumBytes() * 8 >= getIntegerType()->getNumBits();
	}
	else
		return false;
}

bool IntegerConstantType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	TypePointer integerType = getIntegerType();
	return integerType && integerType->isExplicitlyConvertibleTo(_convertTo);
}

TypePointer IntegerConstantType::unaryOperatorResult(Token::Value _operator) const
{
	bigint value;
	switch (_operator)
	{
	case Token::BitNot:
		value = ~m_value;
		break;
	case Token::Add:
		value = m_value;
		break;
	case Token::Sub:
		value = -m_value;
		break;
	case Token::After:
		return shared_from_this();
	default:
		return TypePointer();
	}
	return make_shared<IntegerConstantType>(value);
}

TypePointer IntegerConstantType::binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const
{
	if (_other->getCategory() == Category::Integer)
	{
		shared_ptr<IntegerType const> integerType = getIntegerType();
		if (!integerType)
			return TypePointer();
		return integerType->binaryOperatorResult(_operator, _other);
	}
	else if (_other->getCategory() != getCategory())
		return TypePointer();

	IntegerConstantType const& other = dynamic_cast<IntegerConstantType const&>(*_other);
	if (Token::isCompareOp(_operator))
	{
		shared_ptr<IntegerType const> thisIntegerType = getIntegerType();
		shared_ptr<IntegerType const> otherIntegerType = other.getIntegerType();
		if (!thisIntegerType || !otherIntegerType)
			return TypePointer();
		return thisIntegerType->binaryOperatorResult(_operator, otherIntegerType);
	}
	else
	{
		bigint value;
		switch (_operator)
		{
		case Token::BitOr:
			value = m_value | other.m_value;
			break;
		case Token::BitXor:
			value = m_value ^ other.m_value;
			break;
		case Token::BitAnd:
			value = m_value & other.m_value;
			break;
		case Token::Add:
			value = m_value + other.m_value;
			break;
		case Token::Sub:
			value = m_value - other.m_value;
			break;
		case Token::Mul:
			value = m_value * other.m_value;
			break;
		case Token::Div:
			if (other.m_value == 0)
				return TypePointer();
			value = m_value / other.m_value;
			break;
		case Token::Mod:
			if (other.m_value == 0)
				return TypePointer();
			value = m_value % other.m_value;
			break;
		case Token::Exp:
			if (other.m_value < 0)
				return TypePointer();
			else if (other.m_value > std::numeric_limits<unsigned int>::max())
				return TypePointer();
			else
				value = boost::multiprecision::pow(m_value, other.m_value.convert_to<unsigned int>());
			break;
		default:
			return TypePointer();
		}
		return make_shared<IntegerConstantType>(value);
	}
}

bool IntegerConstantType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	return m_value == dynamic_cast<IntegerConstantType const&>(_other).m_value;
}

string IntegerConstantType::toString() const
{
	return "int_const " + m_value.str();
}

u256 IntegerConstantType::literalValue(Literal const*) const
{
	u256 value;
	// we ignore the literal and hope that the type was correctly determined
	solAssert(m_value <= u256(-1), "Integer constant too large.");
	solAssert(m_value >= -(bigint(1) << 255), "Integer constant too small.");

	if (m_value >= 0)
		value = u256(m_value);
	else
		value = s2u(s256(m_value));

	return value;
}

TypePointer IntegerConstantType::getRealType() const
{
	auto intType = getIntegerType();
	solAssert(!!intType, "getRealType called with invalid integer constant " + toString());
	return intType;
}

shared_ptr<IntegerType const> IntegerConstantType::getIntegerType() const
{
	bigint value = m_value;
	bool negative = (value < 0);
	if (negative) // convert to positive number of same bit requirements
		value = ((-value) - 1) << 1;
	if (value > u256(-1))
		return shared_ptr<IntegerType const>();
	else
		return make_shared<IntegerType>(
			max(bytesRequired(value), 1u) * 8,
			negative ? IntegerType::Modifier::Signed : IntegerType::Modifier::Unsigned
		);
}

shared_ptr<FixedBytesType> FixedBytesType::smallestTypeForLiteral(string const& _literal)
{
	if (_literal.length() <= 32)
		return make_shared<FixedBytesType>(_literal.length());
	return shared_ptr<FixedBytesType>();
}

FixedBytesType::FixedBytesType(int _bytes): m_bytes(_bytes)
{
	solAssert(m_bytes >= 0 && m_bytes <= 32,
			  "Invalid byte number for fixed bytes type: " + dev::toString(m_bytes));
}

bool FixedBytesType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (_convertTo.getCategory() != getCategory())
		return false;
	FixedBytesType const& convertTo = dynamic_cast<FixedBytesType const&>(_convertTo);
	return convertTo.m_bytes >= m_bytes;
}

bool FixedBytesType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	return _convertTo.getCategory() == Category::Integer ||
		_convertTo.getCategory() == Category::Contract ||
		_convertTo.getCategory() == getCategory();
}

TypePointer FixedBytesType::unaryOperatorResult(Token::Value _operator) const
{
	// "delete" and "~" is okay for FixedBytesType
	if (_operator == Token::Delete)
		return make_shared<VoidType>();
	else if (_operator == Token::BitNot)
		return shared_from_this();

	return TypePointer();
}

TypePointer FixedBytesType::binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const
{
	auto commonType = dynamic_pointer_cast<FixedBytesType const>(Type::commonType(shared_from_this(), _other));
	if (!commonType)
		return TypePointer();

	// FixedBytes can be compared and have bitwise operators applied to them
	if (Token::isCompareOp(_operator) || Token::isBitOp(_operator))
		return commonType;

	return TypePointer();
}

bool FixedBytesType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	FixedBytesType const& other = dynamic_cast<FixedBytesType const&>(_other);
	return other.m_bytes == m_bytes;
}

u256 FixedBytesType::literalValue(const Literal* _literal) const
{
	solAssert(_literal, "");
	u256 value = 0;
	for (char c: _literal->getValue())
		value = (value << 8) | byte(c);
	return value << ((32 - _literal->getValue().length()) * 8);
}

bool BoolType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	// conversion to integer is fine, but not to address
	// this is an example of explicit conversions being not transitive (though implicit should be)
	if (_convertTo.getCategory() == getCategory())
	{
		IntegerType const& convertTo = dynamic_cast<IntegerType const&>(_convertTo);
		if (!convertTo.isAddress())
			return true;
	}
	return isImplicitlyConvertibleTo(_convertTo);
}

u256 BoolType::literalValue(Literal const* _literal) const
{
	solAssert(_literal, "");
	if (_literal->getToken() == Token::TrueLiteral)
		return u256(1);
	else if (_literal->getToken() == Token::FalseLiteral)
		return u256(0);
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Bool type constructed from non-boolean literal."));
}

TypePointer BoolType::unaryOperatorResult(Token::Value _operator) const
{
	if (_operator == Token::Delete)
		return make_shared<VoidType>();
	return (_operator == Token::Not) ? shared_from_this() : TypePointer();
}

TypePointer BoolType::binaryOperatorResult(Token::Value _operator, TypePointer const& _other) const
{
	if (getCategory() != _other->getCategory())
		return TypePointer();
	if (Token::isCompareOp(_operator) || _operator == Token::And || _operator == Token::Or)
		return _other;
	else
		return TypePointer();
}

bool ContractType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (*this == _convertTo)
		return true;
	if (_convertTo.getCategory() == Category::Integer)
		return dynamic_cast<IntegerType const&>(_convertTo).isAddress();
	if (_convertTo.getCategory() == Category::Contract)
	{
		auto const& bases = getContractDefinition().getLinearizedBaseContracts();
		if (m_super && bases.size() <= 1)
			return false;
		return find(m_super ? ++bases.begin() : bases.begin(), bases.end(),
					&dynamic_cast<ContractType const&>(_convertTo).getContractDefinition()) != bases.end();
	}
	return false;
}

bool ContractType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	return isImplicitlyConvertibleTo(_convertTo) || _convertTo.getCategory() == Category::Integer ||
			_convertTo.getCategory() == Category::Contract;
}

TypePointer ContractType::unaryOperatorResult(Token::Value _operator) const
{
	return _operator == Token::Delete ? make_shared<VoidType>() : TypePointer();
}

bool ArrayType::isImplicitlyConvertibleTo(const Type& _convertTo) const
{
	if (_convertTo.getCategory() != getCategory())
		return false;
	auto& convertTo = dynamic_cast<ArrayType const&>(_convertTo);
	// let us not allow assignment to memory arrays for now
	if (convertTo.location() != Location::Storage)
		return false;
	if (convertTo.isByteArray() != isByteArray() || convertTo.isString() != isString())
		return false;
	if (!getBaseType()->isImplicitlyConvertibleTo(*convertTo.getBaseType()))
		return false;
	if (convertTo.isDynamicallySized())
		return true;
	return !isDynamicallySized() && convertTo.getLength() >= getLength();
}

TypePointer ArrayType::unaryOperatorResult(Token::Value _operator) const
{
	if (_operator == Token::Delete)
		return make_shared<VoidType>();
	return TypePointer();
}

bool ArrayType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	ArrayType const& other = dynamic_cast<ArrayType const&>(_other);
	if (
		other.m_location != m_location ||
		other.isByteArray() != isByteArray() ||
		other.isString() != isString() ||
		other.isDynamicallySized() != isDynamicallySized()
	)
		return false;
	return isDynamicallySized() || getLength()  == other.getLength();
}

unsigned ArrayType::getCalldataEncodedSize(bool _padded) const
{
	if (isDynamicallySized())
		return 0;
	bigint size = bigint(getLength()) * (isByteArray() ? 1 : getBaseType()->getCalldataEncodedSize(_padded));
	size = ((size + 31) / 32) * 32;
	solAssert(size <= numeric_limits<unsigned>::max(), "Array size does not fit unsigned.");
	return unsigned(size);
}

u256 ArrayType::getStorageSize() const
{
	if (isDynamicallySized())
		return 1;

	bigint size;
	unsigned baseBytes = getBaseType()->getStorageBytes();
	if (baseBytes == 0)
		size = 1;
	else if (baseBytes < 32)
	{
		unsigned itemsPerSlot = 32 / baseBytes;
		size = (bigint(getLength()) + (itemsPerSlot - 1)) / itemsPerSlot;
	}
	else
		size = bigint(getLength()) * getBaseType()->getStorageSize();
	if (size >= bigint(1) << 256)
		BOOST_THROW_EXCEPTION(TypeError() << errinfo_comment("Array too large for storage."));
	return max<u256>(1, u256(size));
}

unsigned ArrayType::getSizeOnStack() const
{
	if (m_location == Location::CallData)
		// offset [length] (stack top)
		return 1 + (isDynamicallySized() ? 1 : 0);
	else if (m_location == Location::Storage)
		// storage_key storage_offset
		return 2;
	else
		// offset
		return 1;
}

string ArrayType::toString() const
{
	if (isString())
		return "string";
	else if (isByteArray())
		return "bytes";
	string ret = getBaseType()->toString() + "[";
	if (!isDynamicallySized())
		ret += getLength().str();
	return ret + "]";
}

TypePointer ArrayType::externalType() const
{
	if (m_arrayKind != ArrayKind::Ordinary)
		return shared_from_this();
	if (!m_baseType->externalType())
		return TypePointer();
	if (m_baseType->getCategory() == Category::Array && m_baseType->isDynamicallySized())
		return TypePointer();

	if (isDynamicallySized())
		return std::make_shared<ArrayType>(Location::CallData, m_baseType->externalType());
	else
		return std::make_shared<ArrayType>(Location::CallData, m_baseType->externalType(), m_length);
}

TypePointer ArrayType::copyForLocation(ReferenceType::Location _location) const
{
	auto copy = make_shared<ArrayType>(_location);
	copy->m_arrayKind = m_arrayKind;
	if (auto ref = dynamic_cast<ReferenceType const*>(m_baseType.get()))
		copy->m_baseType = ref->copyForLocation(_location);
	else
		copy->m_baseType = m_baseType;
	copy->m_hasDynamicLength = m_hasDynamicLength;
	copy->m_length = m_length;
	return copy;
}

const MemberList ArrayType::s_arrayTypeMemberList({{"length", make_shared<IntegerType>(256)}});

bool ContractType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	ContractType const& other = dynamic_cast<ContractType const&>(_other);
	return other.m_contract == m_contract && other.m_super == m_super;
}

string ContractType::toString() const
{
	return "contract " + string(m_super ? "super " : "") + m_contract.getName();
}

MemberList const& ContractType::getMembers() const
{
	// We need to lazy-initialize it because of recursive references.
	if (!m_members)
	{
		// All address members and all interface functions
		MemberList::MemberMap members(
			IntegerType::AddressMemberList.begin(),
			IntegerType::AddressMemberList.end()
		);
		if (m_super)
		{
			// add the most derived of all functions which are visible in derived contracts
			for (ContractDefinition const* base: m_contract.getLinearizedBaseContracts())
				for (ASTPointer<FunctionDefinition> const& function: base->getDefinedFunctions())
				{
					if (!function->isVisibleInDerivedContracts())
						continue;
					auto functionType = make_shared<FunctionType>(*function, true);
					bool functionWithEqualArgumentsFound = false;
					for (auto const& member: members)
					{
						if (member.name != function->getName())
							continue;
						auto memberType = dynamic_cast<FunctionType const*>(member.type.get());
						solAssert(!!memberType, "Override changes type.");
						if (!memberType->hasEqualArgumentTypes(*functionType))
							continue;
						functionWithEqualArgumentsFound = true;
						break;
					}
					if (!functionWithEqualArgumentsFound)
						members.push_back(MemberList::Member(
							function->getName(),
							functionType,
							function.get()
						));
				}
		}
		else
			for (auto const& it: m_contract.getInterfaceFunctions())
				members.push_back(MemberList::Member(
					it.second->getDeclaration().getName(),
					it.second,
					&it.second->getDeclaration()
				));
		m_members.reset(new MemberList(members));
	}
	return *m_members;
}

shared_ptr<FunctionType const> const& ContractType::getConstructorType() const
{
	if (!m_constructorType)
	{
		FunctionDefinition const* constructor = m_contract.getConstructor();
		if (constructor)
			m_constructorType = make_shared<FunctionType>(*constructor);
		else
			m_constructorType = make_shared<FunctionType>(TypePointers(), TypePointers());
	}
	return m_constructorType;
}

vector<tuple<VariableDeclaration const*, u256, unsigned>> ContractType::getStateVariables() const
{
	vector<VariableDeclaration const*> variables;
	for (ContractDefinition const* contract: boost::adaptors::reverse(m_contract.getLinearizedBaseContracts()))
		for (ASTPointer<VariableDeclaration> const& variable: contract->getStateVariables())
			if (!variable->isConstant())
				variables.push_back(variable.get());
	TypePointers types;
	for (auto variable: variables)
		types.push_back(variable->getType());
	StorageOffsets offsets;
	offsets.computeOffsets(types);

	vector<tuple<VariableDeclaration const*, u256, unsigned>> variablesAndOffsets;
	for (size_t index = 0; index < variables.size(); ++index)
		if (auto const* offset = offsets.getOffset(index))
			variablesAndOffsets.push_back(make_tuple(variables[index], offset->first, offset->second));
	return variablesAndOffsets;
}

TypePointer StructType::unaryOperatorResult(Token::Value _operator) const
{
	return _operator == Token::Delete ? make_shared<VoidType>() : TypePointer();
}

bool StructType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	StructType const& other = dynamic_cast<StructType const&>(_other);
	return other.m_struct == m_struct;
}

u256 StructType::getStorageSize() const
{
	return max<u256>(1, getMembers().getStorageSize());
}

bool StructType::canLiveOutsideStorage() const
{
	for (auto const& member: getMembers())
		if (!member.type->canLiveOutsideStorage())
			return false;
	return true;
}

string StructType::toString() const
{
	return string("struct ") + m_struct.getName();
}

MemberList const& StructType::getMembers() const
{
	// We need to lazy-initialize it because of recursive references.
	if (!m_members)
	{
		MemberList::MemberMap members;
		for (ASTPointer<VariableDeclaration> const& variable: m_struct.getMembers())
			members.push_back(MemberList::Member(variable->getName(), variable->getType(), variable.get()));
		m_members.reset(new MemberList(members));
	}
	return *m_members;
}

TypePointer StructType::copyForLocation(ReferenceType::Location _location) const
{
	auto copy = make_shared<StructType>(m_struct);
	copy->m_location = _location;
	return copy;
}

pair<u256, unsigned> const& StructType::getStorageOffsetsOfMember(string const& _name) const
{
	auto const* offsets = getMembers().getMemberStorageOffset(_name);
	solAssert(offsets, "Storage offset of non-existing member requested.");
	return *offsets;
}

TypePointer EnumType::unaryOperatorResult(Token::Value _operator) const
{
	return _operator == Token::Delete ? make_shared<VoidType>() : TypePointer();
}

bool EnumType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	EnumType const& other = dynamic_cast<EnumType const&>(_other);
	return other.m_enum == m_enum;
}

unsigned EnumType::getStorageBytes() const
{
	size_t elements = m_enum.getMembers().size();
	if (elements <= 1)
		return 1;
	else
		return dev::bytesRequired(elements - 1);
}

string EnumType::toString() const
{
	return string("enum ") + m_enum.getName();
}

bool EnumType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	return _convertTo.getCategory() == getCategory() || _convertTo.getCategory() == Category::Integer;
}

unsigned int EnumType::getMemberValue(ASTString const& _member) const
{
	unsigned int index = 0;
	for (ASTPointer<EnumValue> const& decl: m_enum.getMembers())
	{
		if (decl->getName() == _member)
			return index;
		++index;
	}
	BOOST_THROW_EXCEPTION(m_enum.createTypeError("Requested unknown enum value ." + _member));
}

FunctionType::FunctionType(FunctionDefinition const& _function, bool _isInternal):
	m_location(_isInternal ? Location::Internal : Location::External),
	m_isConstant(_function.isDeclaredConst()),
	m_declaration(&_function)
{
	TypePointers params;
	vector<string> paramNames;
	TypePointers retParams;
	vector<string> retParamNames;

	params.reserve(_function.getParameters().size());
	paramNames.reserve(_function.getParameters().size());
	for (ASTPointer<VariableDeclaration> const& var: _function.getParameters())
	{
		paramNames.push_back(var->getName());
		params.push_back(var->getType());
	}
	retParams.reserve(_function.getReturnParameters().size());
	retParamNames.reserve(_function.getReturnParameters().size());
	for (ASTPointer<VariableDeclaration> const& var: _function.getReturnParameters())
	{
		retParamNames.push_back(var->getName());
		retParams.push_back(var->getType());
	}
	swap(params, m_parameterTypes);
	swap(paramNames, m_parameterNames);
	swap(retParams, m_returnParameterTypes);
	swap(retParamNames, m_returnParameterNames);
}

FunctionType::FunctionType(VariableDeclaration const& _varDecl):
	m_location(Location::External), m_isConstant(true), m_declaration(&_varDecl)
{
	TypePointers paramTypes;
	vector<string> paramNames;
	auto returnType = _varDecl.getType();

	while (true)
	{
		if (auto mappingType = dynamic_cast<MappingType const*>(returnType.get()))
		{
			paramTypes.push_back(mappingType->getKeyType());
			paramNames.push_back("");
			returnType = mappingType->getValueType();
		}
		else if (auto arrayType = dynamic_cast<ArrayType const*>(returnType.get()))
		{
			returnType = arrayType->getBaseType();
			paramNames.push_back("");
			paramTypes.push_back(make_shared<IntegerType>(256));
		}
		else
			break;
	}

	TypePointers retParams;
	vector<string> retParamNames;
	if (auto structType = dynamic_cast<StructType const*>(returnType.get()))
	{
		for (auto const& member: structType->getMembers())
			if (member.type->getCategory() != Category::Mapping && member.type->getCategory() != Category::Array)
			{
				retParamNames.push_back(member.name);
				retParams.push_back(member.type);
			}
	}
	else
	{
		retParams.push_back(returnType);
		retParamNames.push_back("");
	}

	swap(paramTypes, m_parameterTypes);
	swap(paramNames, m_parameterNames);
	swap(retParams, m_returnParameterTypes);
	swap(retParamNames, m_returnParameterNames);
}

FunctionType::FunctionType(const EventDefinition& _event):
	m_location(Location::Event), m_isConstant(true), m_declaration(&_event)
{
	TypePointers params;
	vector<string> paramNames;
	params.reserve(_event.getParameters().size());
	paramNames.reserve(_event.getParameters().size());
	for (ASTPointer<VariableDeclaration> const& var: _event.getParameters())
	{
		paramNames.push_back(var->getName());
		params.push_back(var->getType());
	}
	swap(params, m_parameterTypes);
	swap(paramNames, m_parameterNames);
}

bool FunctionType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	FunctionType const& other = dynamic_cast<FunctionType const&>(_other);

	if (m_location != other.m_location)
		return false;
	if (m_isConstant != other.isConstant())
		return false;

	if (m_parameterTypes.size() != other.m_parameterTypes.size() ||
			m_returnParameterTypes.size() != other.m_returnParameterTypes.size())
		return false;
	auto typeCompare = [](TypePointer const& _a, TypePointer const& _b) -> bool { return *_a == *_b; };

	if (!equal(m_parameterTypes.cbegin(), m_parameterTypes.cend(),
			   other.m_parameterTypes.cbegin(), typeCompare))
		return false;
	if (!equal(m_returnParameterTypes.cbegin(), m_returnParameterTypes.cend(),
			   other.m_returnParameterTypes.cbegin(), typeCompare))
		return false;
	//@todo this is ugly, but cannot be prevented right now
	if (m_gasSet != other.m_gasSet || m_valueSet != other.m_valueSet)
		return false;
	return true;
}

string FunctionType::toString() const
{
	string name = "function (";
	for (auto it = m_parameterTypes.begin(); it != m_parameterTypes.end(); ++it)
		name += (*it)->toString() + (it + 1 == m_parameterTypes.end() ? "" : ",");
	name += ") returns (";
	for (auto it = m_returnParameterTypes.begin(); it != m_returnParameterTypes.end(); ++it)
		name += (*it)->toString() + (it + 1 == m_returnParameterTypes.end() ? "" : ",");
	return name + ")";
}

u256 FunctionType::getStorageSize() const
{
	BOOST_THROW_EXCEPTION(
		InternalCompilerError()
			<< errinfo_comment("Storage size of non-storable function type requested."));
}

unsigned FunctionType::getSizeOnStack() const
{
	Location location = m_location;
	if (m_location == Location::SetGas || m_location == Location::SetValue)
	{
		solAssert(m_returnParameterTypes.size() == 1, "");
		location = dynamic_cast<FunctionType const&>(*m_returnParameterTypes.front()).m_location;
	}

	unsigned size = 0;
	if (location == Location::External || location == Location::CallCode)
		size = 2;
	else if (location == Location::Bare || location == Location::BareCallCode)
		size = 1;
	else if (location == Location::Internal)
		size = 1;
	if (m_gasSet)
		size++;
	if (m_valueSet)
		size++;
	return size;
}

FunctionTypePointer FunctionType::externalFunctionType() const
{
	TypePointers paramTypes;
	TypePointers retParamTypes;

	for (auto type: m_parameterTypes)
	{
		if (!type->externalType())
			return FunctionTypePointer();
		paramTypes.push_back(type->externalType());
	}
	for (auto type: m_returnParameterTypes)
	{
		if (!type->externalType())
			return FunctionTypePointer();
		retParamTypes.push_back(type->externalType());
	}
	return make_shared<FunctionType>(paramTypes, retParamTypes, m_parameterNames, m_returnParameterNames, m_location, m_arbitraryParameters);
}

MemberList const& FunctionType::getMembers() const
{
	switch (m_location)
	{
	case Location::External:
	case Location::Creation:
	case Location::ECRecover:
	case Location::SHA256:
	case Location::RIPEMD160:
	case Location::Bare:
	case Location::BareCallCode:
		if (!m_members)
		{
			MemberList::MemberMap members{
				{
					"value",
					make_shared<FunctionType>(
						parseElementaryTypeVector({"uint"}),
						TypePointers{copyAndSetGasOrValue(false, true)},
						strings(),
						strings(),
						Location::SetValue,
						false,
						m_gasSet,
						m_valueSet
					)
				}
			};
			if (m_location != Location::Creation)
				members.push_back(
					MemberList::Member(
						"gas",
						make_shared<FunctionType>(
							parseElementaryTypeVector({"uint"}),
							TypePointers{copyAndSetGasOrValue(true, false)},
							strings(),
							strings(),
							Location::SetGas,
							false,
							m_gasSet,
							m_valueSet
						)
					)
				);
			m_members.reset(new MemberList(members));
		}
		return *m_members;
	default:
		return EmptyMemberList;
	}
}

bool FunctionType::canTakeArguments(TypePointers const& _argumentTypes) const
{
	TypePointers const& parameterTypes = getParameterTypes();
	if (takesArbitraryParameters())
		return true;
	else if (_argumentTypes.size() != parameterTypes.size())
		return false;
	else
		return std::equal(
			_argumentTypes.cbegin(),
			_argumentTypes.cend(),
			parameterTypes.cbegin(),
			[](TypePointer const& argumentType, TypePointer const& parameterType)
			{
				return argumentType->isImplicitlyConvertibleTo(*parameterType);
			}
		);
}

bool FunctionType::hasEqualArgumentTypes(FunctionType const& _other) const
{
	if (m_parameterTypes.size() != _other.m_parameterTypes.size())
		return false;
	return equal(
		m_parameterTypes.cbegin(),
		m_parameterTypes.cend(),
		_other.m_parameterTypes.cbegin(),
		[](TypePointer const& _a, TypePointer const& _b) -> bool { return *_a == *_b; }
	);
}

bool FunctionType::isBareCall() const
{
	switch (m_location)
	{
	case Location::Bare:
	case Location::BareCallCode:
	case Location::ECRecover:
	case Location::SHA256:
	case Location::RIPEMD160:
		return true;
	default:
		return false;
	}
}

string FunctionType::externalSignature(std::string const& _name) const
{
	std::string funcName = _name;
	if (_name == "")
	{
		solAssert(m_declaration != nullptr, "Function type without name needs a declaration");
		funcName = m_declaration->getName();
	}
	string ret = funcName + "(";

	FunctionTypePointer external = externalFunctionType();
	solAssert(!!external, "External function type requested.");
	TypePointers externalParameterTypes = external->getParameterTypes();
	for (auto it = externalParameterTypes.cbegin(); it != externalParameterTypes.cend(); ++it)
	{
		solAssert(!!(*it), "Parameter should have external type");
		ret += (*it)->toString() + (it + 1 == externalParameterTypes.cend() ? "" : ",");
	}

	return ret + ")";
}

u256 FunctionType::externalIdentifier() const
{
	return FixedHash<4>::Arith(FixedHash<4>(dev::sha3(externalSignature())));
}

TypePointers FunctionType::parseElementaryTypeVector(strings const& _types)
{
	TypePointers pointers;
	pointers.reserve(_types.size());
	for (string const& type: _types)
		pointers.push_back(Type::fromElementaryTypeName(type));
	return pointers;
}

TypePointer FunctionType::copyAndSetGasOrValue(bool _setGas, bool _setValue) const
{
	return make_shared<FunctionType>(
		m_parameterTypes,
		m_returnParameterTypes,
		m_parameterNames,
		m_returnParameterNames,
		m_location,
		m_arbitraryParameters,
		m_gasSet || _setGas,
		m_valueSet || _setValue
	);
}

vector<string> const FunctionType::getParameterTypeNames() const
{
	vector<string> names;
	for (TypePointer const& t: m_parameterTypes)
		names.push_back(t->toString());

	return names;
}

vector<string> const FunctionType::getReturnParameterTypeNames() const
{
	vector<string> names;
	for (TypePointer const& t: m_returnParameterTypes)
		names.push_back(t->toString());

	return names;
}

ASTPointer<ASTString> FunctionType::getDocumentation() const
{
	auto function = dynamic_cast<Documented const*>(m_declaration);
	if (function)
		return function->getDocumentation();

	return ASTPointer<ASTString>();
}

bool MappingType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	MappingType const& other = dynamic_cast<MappingType const&>(_other);
	return *other.m_keyType == *m_keyType && *other.m_valueType == *m_valueType;
}

string MappingType::toString() const
{
	return "mapping(" + getKeyType()->toString() + " => " + getValueType()->toString() + ")";
}

u256 VoidType::getStorageSize() const
{
	BOOST_THROW_EXCEPTION(
		InternalCompilerError()
			<< errinfo_comment("Storage size of non-storable void type requested."));
}

bool TypeType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	TypeType const& other = dynamic_cast<TypeType const&>(_other);
	return *getActualType() == *other.getActualType();
}

u256 TypeType::getStorageSize() const
{
	BOOST_THROW_EXCEPTION(
		InternalCompilerError()
			<< errinfo_comment("Storage size of non-storable type type requested."));
}

MemberList const& TypeType::getMembers() const
{
	// We need to lazy-initialize it because of recursive references.
	if (!m_members)
	{
		MemberList::MemberMap members;
		if (m_actualType->getCategory() == Category::Contract && m_currentContract != nullptr)
		{
			ContractDefinition const& contract = dynamic_cast<ContractType const&>(*m_actualType).getContractDefinition();
			vector<ContractDefinition const*> currentBases = m_currentContract->getLinearizedBaseContracts();
			if (find(currentBases.begin(), currentBases.end(), &contract) != currentBases.end())
				// We are accessing the type of a base contract, so add all public and protected
				// members. Note that this does not add inherited functions on purpose.
				for (Declaration const* decl: contract.getInheritableMembers())
					members.push_back(MemberList::Member(decl->getName(), decl->getType(), decl));
		}
		else if (m_actualType->getCategory() == Category::Enum)
		{
			EnumDefinition const& enumDef = dynamic_cast<EnumType const&>(*m_actualType).getEnumDefinition();
			auto enumType = make_shared<EnumType>(enumDef);
			for (ASTPointer<EnumValue> const& enumValue: enumDef.getMembers())
				members.push_back(MemberList::Member(enumValue->getName(), enumType));
		}
		m_members.reset(new MemberList(members));
	}
	return *m_members;
}

ModifierType::ModifierType(const ModifierDefinition& _modifier)
{
	TypePointers params;
	params.reserve(_modifier.getParameters().size());
	for (ASTPointer<VariableDeclaration> const& var: _modifier.getParameters())
		params.push_back(var->getType());
	swap(params, m_parameterTypes);
}

u256 ModifierType::getStorageSize() const
{
	BOOST_THROW_EXCEPTION(
		InternalCompilerError()
			<< errinfo_comment("Storage size of non-storable type type requested."));
}

bool ModifierType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	ModifierType const& other = dynamic_cast<ModifierType const&>(_other);

	if (m_parameterTypes.size() != other.m_parameterTypes.size())
		return false;
	auto typeCompare = [](TypePointer const& _a, TypePointer const& _b) -> bool { return *_a == *_b; };

	if (!equal(m_parameterTypes.cbegin(), m_parameterTypes.cend(),
			   other.m_parameterTypes.cbegin(), typeCompare))
		return false;
	return true;
}

string ModifierType::toString() const
{
	string name = "modifier (";
	for (auto it = m_parameterTypes.begin(); it != m_parameterTypes.end(); ++it)
		name += (*it)->toString() + (it + 1 == m_parameterTypes.end() ? "" : ",");
	return name + ")";
}

MagicType::MagicType(MagicType::Kind _kind):
	m_kind(_kind)
{
	switch (m_kind)
	{
	case Kind::Block:
		m_members = move(MemberList({
			{"coinbase", make_shared<IntegerType>(0, IntegerType::Modifier::Address)},
			{"timestamp", make_shared<IntegerType>(256)},
			{"blockhash", make_shared<FunctionType>(strings{"uint"}, strings{"bytes32"}, FunctionType::Location::BlockHash)},
			{"difficulty", make_shared<IntegerType>(256)},
			{"number", make_shared<IntegerType>(256)},
			{"gaslimit", make_shared<IntegerType>(256)}
		}));
		break;
	case Kind::Message:
		m_members = move(MemberList({
			{"sender", make_shared<IntegerType>(0, IntegerType::Modifier::Address)},
			{"gas", make_shared<IntegerType>(256)},
			{"value", make_shared<IntegerType>(256)},
			{"data", make_shared<ArrayType>(ReferenceType::Location::CallData)},
			{"sig", make_shared<FixedBytesType>(4)}
		}));
		break;
	case Kind::Transaction:
		m_members = move(MemberList({
			{"origin", make_shared<IntegerType>(0, IntegerType::Modifier::Address)},
			{"gasprice", make_shared<IntegerType>(256)}
		}));
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown kind of magic."));
	}
}

bool MagicType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	MagicType const& other = dynamic_cast<MagicType const&>(_other);
	return other.m_kind == m_kind;
}

string MagicType::toString() const
{
	switch (m_kind)
	{
	case Kind::Block:
		return "block";
	case Kind::Message:
		return "msg";
	case Kind::Transaction:
		return "tx";
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown kind of magic."));
	}
}

}
}
