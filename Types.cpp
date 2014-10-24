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

#include <cassert>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>
#include <libsolidity/Types.h>
#include <libsolidity/AST.h>

namespace dev
{
namespace solidity
{

std::shared_ptr<Type> Type::fromElementaryTypeName(Token::Value _typeToken)
{
	if (Token::INT <= _typeToken && _typeToken <= Token::HASH256)
	{
		int offset = _typeToken - Token::INT;
		int bits = offset % 5;
		if (bits == 0)
			bits = 256;
		else
			bits = (1 << (bits - 1)) * 32;
		int modifier = offset / 5;
		return std::make_shared<IntegerType>(bits,
											 modifier == 0 ? IntegerType::Modifier::SIGNED :
											 modifier == 1 ? IntegerType::Modifier::UNSIGNED :
											 IntegerType::Modifier::HASH);
	}
	else if (_typeToken == Token::ADDRESS)
		return std::make_shared<IntegerType>(0, IntegerType::Modifier::ADDRESS);
	else if (_typeToken == Token::BOOL)
		return std::make_shared<BoolType>();
	else
		assert(false); // @todo add other tyes
}

std::shared_ptr<Type> Type::fromUserDefinedTypeName(UserDefinedTypeName const& _typeName)
{
	return std::make_shared<StructType>(*_typeName.getReferencedStruct());
}

std::shared_ptr<Type> Type::fromMapping(Mapping const&)
{
	assert(false); //@todo not yet implemented
	return std::shared_ptr<Type>();
}

std::shared_ptr<Type> Type::forLiteral(Literal const& _literal)
{
	switch (_literal.getToken())
	{
	case Token::TRUE_LITERAL:
	case Token::FALSE_LITERAL:
		return std::make_shared<BoolType>();
	case Token::NUMBER:
		return IntegerType::smallestTypeForLiteral(_literal.getValue());
	case Token::STRING_LITERAL:
		return std::shared_ptr<Type>(); // @todo
	default:
		return std::shared_ptr<Type>();
	}
}

std::shared_ptr<IntegerType> IntegerType::smallestTypeForLiteral(std::string const&)
{
	//@todo
	return std::make_shared<IntegerType>(256, Modifier::UNSIGNED);
}

IntegerType::IntegerType(int _bits, IntegerType::Modifier _modifier):
	m_bits(_bits), m_modifier(_modifier)
{
	if (isAddress())
		_bits = 160;
	assert(_bits > 0 && _bits <= 256 && _bits % 8 == 0);
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
	else if (isHash())
		return convertTo.isHash();
	else if (isSigned())
		return convertTo.isSigned();
	else
		return !convertTo.isSigned() || convertTo.m_bits > m_bits;
}

bool IntegerType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	return _convertTo.getCategory() == getCategory();
}

bool IntegerType::acceptsBinaryOperator(Token::Value _operator) const
{
	if (isAddress())
		return Token::isCompareOp(_operator);
	else if (isHash())
		return Token::isCompareOp(_operator) || Token::isBitOp(_operator);
	else
		return true;
}

bool IntegerType::acceptsUnaryOperator(Token::Value _operator) const
{
	if (_operator == Token::DELETE)
		return true;
	if (isAddress())
		return false;
	if (_operator == Token::BIT_NOT)
		return true;
	if (isHash())
		return false;
	return _operator == Token::ADD || _operator == Token::SUB ||
		   _operator == Token::INC || _operator == Token::DEC;
}

bool IntegerType::operator==(const Type& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	IntegerType const& other = dynamic_cast<IntegerType const&>(_other);
	return other.m_bits == m_bits && other.m_modifier == m_modifier;
}

std::string IntegerType::toString() const
{
	if (isAddress())
		return "address";
	std::string prefix = isHash() ? "hash" : (isSigned() ? "int" : "uint");
	return prefix + dev::toString(m_bits);
}

bytes IntegerType::literalToBigEndian(const Literal& _literal) const
{
	bigint value(_literal.getValue());
	if (!isSigned() && value < 0)
		return bytes(); // @todo this should already be caught by "smallestTypeforLiteral"
	//@todo check that the number of bits is correct
	//@todo does "toCompactBigEndian" work for signed numbers?
	return toCompactBigEndian(value);
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

bytes BoolType::literalToBigEndian(const Literal& _literal) const
{
	if (_literal.getToken() == Token::TRUE_LITERAL)
		return bytes(1, 1);
	else if (_literal.getToken() == Token::FALSE_LITERAL)
		return bytes(1, 0);
	else
		return NullBytes;
}

bool ContractType::operator==(const Type& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	ContractType const& other = dynamic_cast<ContractType const&>(_other);
	return other.m_contract == m_contract;
}

bool StructType::operator==(const Type& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	StructType const& other = dynamic_cast<StructType const&>(_other);
	return other.m_struct == m_struct;
}

bool FunctionType::operator==(const Type& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	FunctionType const& other = dynamic_cast<FunctionType const&>(_other);
	return other.m_function == m_function;
}

bool MappingType::operator==(const Type& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	MappingType const& other = dynamic_cast<MappingType const&>(_other);
	return *other.m_keyType == *m_keyType && *other.m_valueType == *m_valueType;
}

bool TypeType::operator==(const Type& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	TypeType const& other = dynamic_cast<TypeType const&>(_other);
	return *getActualType() == *other.getActualType();
}

}
}
