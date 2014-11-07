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
#include <libdevcore/CommonData.h>
#include <libsolidity/Types.h>
#include <libsolidity/AST.h>

using namespace std;

namespace dev
{
namespace solidity
{

shared_ptr<Type> Type::fromElementaryTypeName(Token::Value _typeToken)
{
	if (asserts(Token::isElementaryTypeName(_typeToken)))
		BOOST_THROW_EXCEPTION(InternalCompilerError());

	if (Token::INT <= _typeToken && _typeToken <= Token::HASH256)
	{
		int offset = _typeToken - Token::INT;
		int bytes = offset % 33;
		if (bytes == 0)
			bytes = 32;
		int modifier = offset / 33;
		return make_shared<IntegerType>(bytes * 8,
										modifier == 0 ? IntegerType::Modifier::SIGNED :
										modifier == 1 ? IntegerType::Modifier::UNSIGNED :
										IntegerType::Modifier::HASH);
	}
	else if (_typeToken == Token::ADDRESS)
		return make_shared<IntegerType>(0, IntegerType::Modifier::ADDRESS);
	else if (_typeToken == Token::BOOL)
		return make_shared<BoolType>();
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unable to convert elementary typename " +
																		 std::string(Token::toString(_typeToken)) + " to type."));
	return shared_ptr<Type>();
}

shared_ptr<Type> Type::fromUserDefinedTypeName(UserDefinedTypeName const& _typeName)
{
	return make_shared<StructType>(*_typeName.getReferencedStruct());
}

shared_ptr<Type> Type::fromMapping(Mapping const&)
{
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Mapping types not yet implemented."));
	return shared_ptr<Type>();
}

shared_ptr<Type> Type::forLiteral(Literal const& _literal)
{
	switch (_literal.getToken())
	{
	case Token::TRUE_LITERAL:
	case Token::FALSE_LITERAL:
		return make_shared<BoolType>();
	case Token::NUMBER:
		return IntegerType::smallestTypeForLiteral(_literal.getValue());
	case Token::STRING_LITERAL:
		return shared_ptr<Type>(); // @todo
	default:
		return shared_ptr<Type>();
	}
}

shared_ptr<IntegerType> IntegerType::smallestTypeForLiteral(string const& _literal)
{
	bigint value(_literal);
	bool isSigned = value < 0 || (!_literal.empty() && _literal.front() == '-');
	if (isSigned)
		// convert to positive number of same bit requirements
		value = ((-value) - 1) << 1;
	unsigned bytes = max(bytesRequired(value), 1u);
	if (bytes > 32)
		return shared_ptr<IntegerType>();
	return make_shared<IntegerType>(bytes * 8, isSigned ? Modifier::SIGNED : Modifier::UNSIGNED);
}

IntegerType::IntegerType(int _bits, IntegerType::Modifier _modifier):
	m_bits(_bits), m_modifier(_modifier)
{
	if (isAddress())
		m_bits = 160;
	if (asserts(m_bits > 0 && m_bits <= 256 && m_bits % 8 == 0))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid bit number for integer type: " + dev::toString(_bits)));
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
	string prefix = isHash() ? "hash" : (isSigned() ? "int" : "uint");
	return prefix + dev::toString(m_bits);
}

u256 IntegerType::literalValue(Literal const& _literal) const
{
	bigint value(_literal.getValue());
	return u256(value);
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

u256 BoolType::literalValue(Literal const& _literal) const
{
	if (_literal.getToken() == Token::TRUE_LITERAL)
		return u256(1);
	else if (_literal.getToken() == Token::FALSE_LITERAL)
		return u256(0);
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Bool type constructed from non-boolean literal."));
}

bool ContractType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	ContractType const& other = dynamic_cast<ContractType const&>(_other);
	return other.m_contract == m_contract;
}

u256 ContractType::getStorageSize() const
{
	u256 size = 0;
	for (ASTPointer<VariableDeclaration> const& variable: m_contract.getStateVariables())
		size += variable->getType()->getStorageSize();
	return max<u256>(1, size);
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
	u256 size = 0;
	for (ASTPointer<VariableDeclaration> const& variable: m_struct.getMembers())
		size += variable->getType()->getStorageSize();
	return max<u256>(1, size);
}

bool FunctionType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	FunctionType const& other = dynamic_cast<FunctionType const&>(_other);
	return other.m_function == m_function;
}

bool MappingType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	MappingType const& other = dynamic_cast<MappingType const&>(_other);
	return *other.m_keyType == *m_keyType && *other.m_valueType == *m_valueType;
}

bool TypeType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	TypeType const& other = dynamic_cast<TypeType const&>(_other);
	return *getActualType() == *other.getActualType();
}

}
}
