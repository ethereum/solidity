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

#include <libdevcore/CommonIO.h>
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
		BOOST_ASSERT(false); // @todo add other tyes
}

std::shared_ptr<Type> Type::fromUserDefinedTypeName(UserDefinedTypeName const& _typeName)
{
	return std::make_shared<StructType>(*_typeName.getReferencedStruct());
}

std::shared_ptr<Type> Type::fromMapping(Mapping const&)
{
	BOOST_ASSERT(false); //@todo not yet implemented
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
	BOOST_ASSERT(_bits > 0 && _bits <= 256 && _bits % 8 == 0);
	if (isAddress())
		_bits = 160;
}

bool IntegerType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (_convertTo.getCategory() != Category::INTEGER)
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
	return _convertTo.getCategory() == Category::INTEGER;
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
	return _operator == Token::DELETE || (!isAddress() && _operator == Token::BIT_NOT);
}

std::string IntegerType::toString() const
{
	if (isAddress())
		return "address";
	std::string prefix = isHash() ? "hash" : (isSigned() ? "int" : "uint");
	return prefix + dev::toString(m_bits);
}

bool BoolType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	// conversion to integer is fine, but not to address
	// this is an example of explicit conversions being not transitive (though implicit should be)
	if (_convertTo.getCategory() == Category::INTEGER)
	{
		IntegerType const& convertTo = dynamic_cast<IntegerType const&>(_convertTo);
		if (!convertTo.isAddress())
			return true;
	}
	return isImplicitlyConvertibleTo(_convertTo);
}

bool ContractType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (_convertTo.getCategory() != Category::CONTRACT)
		return false;
	ContractType const& convertTo = dynamic_cast<ContractType const&>(_convertTo);
	return &m_contract == &convertTo.m_contract;
}

bool StructType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (_convertTo.getCategory() != Category::STRUCT)
		return false;
	StructType const& convertTo = dynamic_cast<StructType const&>(_convertTo);
	return &m_struct == &convertTo.m_struct;
}


}
}
