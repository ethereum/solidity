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

shared_ptr<Type const> Type::fromElementaryTypeName(Token::Value _typeToken)
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
		return make_shared<IntegerType const>(bytes * 8,
										modifier == 0 ? IntegerType::Modifier::SIGNED :
										modifier == 1 ? IntegerType::Modifier::UNSIGNED :
										IntegerType::Modifier::HASH);
	}
	else if (_typeToken == Token::ADDRESS)
		return make_shared<IntegerType const>(0, IntegerType::Modifier::ADDRESS);
	else if (_typeToken == Token::BOOL)
		return make_shared<BoolType const>();
	else if (Token::STRING1 <= _typeToken && _typeToken <= Token::STRING32)
		return make_shared<StaticStringType const>(int(_typeToken) - int(Token::STRING1) + 1);
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unable to convert elementary typename " +
																		 std::string(Token::toString(_typeToken)) + " to type."));
}

shared_ptr<Type const> Type::fromUserDefinedTypeName(UserDefinedTypeName const& _typeName)
{
	Declaration const* declaration = _typeName.getReferencedDeclaration();
	if (StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(declaration))
		return make_shared<StructType const>(*structDef);
	else if (FunctionDefinition const* function = dynamic_cast<FunctionDefinition const*>(declaration))
		return make_shared<FunctionType const>(*function);
	else if (ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(declaration))
		return make_shared<ContractType const>(*contract);
	return shared_ptr<Type const>();
}

shared_ptr<Type const> Type::fromMapping(Mapping const& _typeName)
{
	shared_ptr<Type const> keyType = _typeName.getKeyType().toType();
	if (!keyType)
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Error resolving type name."));
	shared_ptr<Type const> valueType = _typeName.getValueType().toType();
	if (!valueType)
		BOOST_THROW_EXCEPTION(_typeName.getValueType().createTypeError("Invalid type name"));
	return make_shared<MappingType const>(keyType, valueType);
}

shared_ptr<Type const> Type::forLiteral(Literal const& _literal)
{
	switch (_literal.getToken())
	{
	case Token::TRUE_LITERAL:
	case Token::FALSE_LITERAL:
		return make_shared<BoolType const>();
	case Token::NUMBER:
		return IntegerType::smallestTypeForLiteral(_literal.getValue());
	case Token::STRING_LITERAL:
		//@todo put larger strings into dynamic strings
		return StaticStringType::smallestTypeForLiteral(_literal.getValue());
	default:
		return shared_ptr<Type const>();
	}
}

const MemberList Type::EmptyMemberList = MemberList();

shared_ptr<IntegerType const> IntegerType::smallestTypeForLiteral(string const& _literal)
{
	bigint value(_literal);
	bool isSigned = value < 0 || (!_literal.empty() && _literal.front() == '-');
	if (isSigned)
		// convert to positive number of same bit requirements
		value = ((-value) - 1) << 1;
	unsigned bytes = max(bytesRequired(value), 1u);
	if (bytes > 32)
		return shared_ptr<IntegerType const>();
	return make_shared<IntegerType const>(bytes * 8, isSigned ? Modifier::SIGNED : Modifier::UNSIGNED);
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
	return _convertTo.getCategory() == getCategory() || _convertTo.getCategory() == Category::CONTRACT;
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

const MemberList IntegerType::AddressMemberList =
		MemberList({{"balance", make_shared<IntegerType const>(256)},
					{"send", make_shared<FunctionType const>(TypePointers({make_shared<IntegerType const>(256)}),
															 TypePointers(), FunctionType::Location::SEND)}});

shared_ptr<StaticStringType> StaticStringType::smallestTypeForLiteral(string const& _literal)
{
	if (0 < _literal.length() && _literal.length() <= 32)
		return make_shared<StaticStringType>(_literal.length());
	return shared_ptr<StaticStringType>();
}

StaticStringType::StaticStringType(int _bytes): m_bytes(_bytes)
{
	if (asserts(m_bytes > 0 && m_bytes <= 32))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid byte number for static string type: " +
																		 dev::toString(m_bytes)));
}

bool StaticStringType::isImplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (_convertTo.getCategory() != getCategory())
		return false;
	StaticStringType const& convertTo = dynamic_cast<StaticStringType const&>(_convertTo);
	return convertTo.m_bytes >= m_bytes;
}

bool StaticStringType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	StaticStringType const& other = dynamic_cast<StaticStringType const&>(_other);
	return other.m_bytes == m_bytes;
}

u256 StaticStringType::literalValue(const Literal& _literal) const
{
	u256 value = 0;
	for (char c: _literal.getValue())
		value = (value << 8) | byte(c);
	return value << ((32 - _literal.getValue().length()) * 8);
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

bool ContractType::isExplicitlyConvertibleTo(Type const& _convertTo) const
{
	if (isImplicitlyConvertibleTo(_convertTo))
		return true;
	if (_convertTo.getCategory() == Category::INTEGER)
		return dynamic_cast<IntegerType const&>(_convertTo).isAddress();
	return false;
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

string ContractType::toString() const
{
	return "contract " + m_contract.getName();
}

MemberList const& ContractType::getMembers() const
{
	// We need to lazy-initialize it because of recursive references.
	if (!m_members)
	{
		map<string, shared_ptr<Type const>> members;
		for (FunctionDefinition const* function: m_contract.getInterfaceFunctions())
			members[function->getName()] = make_shared<FunctionType>(*function, false);
		m_members.reset(new MemberList(members));
	}
	return *m_members;
}

unsigned ContractType::getFunctionIndex(string const& _functionName) const
{
	unsigned index = 0;
	for (FunctionDefinition const* function: m_contract.getInterfaceFunctions())
	{
		if (function->getName() == _functionName)
			return index;
		++index;
	}
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Index of non-existing contract function requested."));
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
	for (pair<string, shared_ptr<Type const>> const& member: getMembers())
		size += member.second->getStorageSize();
	return max<u256>(1, size);
}

bool StructType::canLiveOutsideStorage() const
{
	for (pair<string, shared_ptr<Type const>> const& member: getMembers())
		if (!member.second->canLiveOutsideStorage())
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
		map<string, shared_ptr<Type const>> members;
		for (ASTPointer<VariableDeclaration> const& variable: m_struct.getMembers())
			members[variable->getName()] = variable->getType();
		m_members.reset(new MemberList(members));
	}
	return *m_members;
}

u256 StructType::getStorageOffsetOfMember(string const& _name) const
{
	//@todo cache member offset?
	u256 offset;
	for (ASTPointer<VariableDeclaration> variable: m_struct.getMembers())
	{
		if (variable->getName() == _name)
			return offset;
		offset += variable->getType()->getStorageSize();
	}
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Storage offset of non-existing member requested."));
}

FunctionType::FunctionType(FunctionDefinition const& _function, bool _isInternal)
{
	TypePointers params;
	TypePointers retParams;
	params.reserve(_function.getParameters().size());
	for (ASTPointer<VariableDeclaration> const& var: _function.getParameters())
		params.push_back(var->getType());
	retParams.reserve(_function.getReturnParameters().size());
	for (ASTPointer<VariableDeclaration> const& var: _function.getReturnParameters())
		retParams.push_back(var->getType());
	swap(params, m_parameterTypes);
	swap(retParams, m_returnParameterTypes);
	m_location = _isInternal ? Location::INTERNAL : Location::EXTERNAL;
}

bool FunctionType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	FunctionType const& other = dynamic_cast<FunctionType const&>(_other);

	if (m_location != other.m_location)
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

unsigned FunctionType::getSizeOnStack() const
{
	switch (m_location)
	{
	case Location::INTERNAL:
		return 1;
	case Location::EXTERNAL:
		return 2;
	default:
		return 0;
	}
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

bool TypeType::operator==(Type const& _other) const
{
	if (_other.getCategory() != getCategory())
		return false;
	TypeType const& other = dynamic_cast<TypeType const&>(_other);
	return *getActualType() == *other.getActualType();
}

MagicType::MagicType(MagicType::Kind _kind):
	m_kind(_kind)
{
	switch (m_kind)
	{
	case Kind::BLOCK:
		m_members = MemberList({{"coinbase", make_shared<IntegerType const>(0, IntegerType::Modifier::ADDRESS)},
								{"timestamp", make_shared<IntegerType const>(256)},
								{"prevhash", make_shared<IntegerType const>(256, IntegerType::Modifier::HASH)},
								{"difficulty", make_shared<IntegerType const>(256)},
								{"number", make_shared<IntegerType const>(256)},
								{"gaslimit", make_shared<IntegerType const>(256)}});
		break;
	case Kind::MSG:
		m_members = MemberList({{"sender", make_shared<IntegerType const>(0, IntegerType::Modifier::ADDRESS)},
								{"gas", make_shared<IntegerType const>(256)},
								{"value", make_shared<IntegerType const>(256)}});
		break;
	case Kind::TX:
		m_members = MemberList({{"origin", make_shared<IntegerType const>(0, IntegerType::Modifier::ADDRESS)},
								{"gasprice", make_shared<IntegerType const>(256)}});
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
	case Kind::BLOCK:
		return "block";
	case Kind::MSG:
		return "msg";
	case Kind::TX:
		return "tx";
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown kind of magic."));
	}
}

}
}
