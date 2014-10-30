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
#include <boost/noncopyable.hpp>
#include <libdevcore/Common.h>
#include <libsolidity/ASTForward.h>
#include <libsolidity/Token.h>

namespace dev
{
namespace solidity
{

// @todo realMxN, string<N>, mapping

/// Abstract base class that forms the root of the type hierarchy.
class Type: private boost::noncopyable
{
public:
	enum class Category
	{
		INTEGER, BOOL, REAL, STRING, CONTRACT, STRUCT, FUNCTION, MAPPING, VOID, TYPE
	};

	///@{
	///@name Factory functions
	/// Factory functions that convert an AST @ref TypeName to a Type.
	static std::shared_ptr<Type> fromElementaryTypeName(Token::Value _typeToken);
	static std::shared_ptr<Type> fromUserDefinedTypeName(UserDefinedTypeName const& _typeName);
	static std::shared_ptr<Type> fromMapping(Mapping const& _typeName);
	/// @}

	/// Auto-detect the proper type for a literal
	static std::shared_ptr<Type> forLiteral(Literal const& _literal);

	virtual Category getCategory() const = 0;
	virtual bool isImplicitlyConvertibleTo(Type const& _other) const { return *this == _other; }
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const
	{
		return isImplicitlyConvertibleTo(_convertTo);
	}
	virtual bool acceptsBinaryOperator(Token::Value) const { return false; }
	virtual bool acceptsUnaryOperator(Token::Value) const { return false; }

	virtual bool operator==(Type const& _other) const { return getCategory() == _other.getCategory(); }
	virtual bool operator!=(Type const& _other) const { return !this->operator ==(_other); }

	/// @returns number of bytes used by this type when encoded for CALL, or 0 if the encoding
	/// is not a simple big-endian encoding or the type cannot be stored on the stack.
	virtual unsigned getCalldataEncodedSize() const { return 0; }

	virtual std::string toString() const = 0;
	virtual u256 literalValue(Literal const&) const { assert(false); }
};

/// Any kind of integer type including hash and address.
class IntegerType: public Type
{
public:
	enum class Modifier
	{
		UNSIGNED, SIGNED, HASH, ADDRESS
	};
	virtual Category getCategory() const override { return Category::INTEGER; }

	static std::shared_ptr<IntegerType> smallestTypeForLiteral(std::string const& _literal);

	explicit IntegerType(int _bits, Modifier _modifier = Modifier::UNSIGNED);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool acceptsBinaryOperator(Token::Value _operator) const override;
	virtual bool acceptsUnaryOperator(Token::Value _operator) const override;

	virtual bool operator==(Type const& _other) const override;

	virtual unsigned getCalldataEncodedSize() const { return m_bits / 8; }

	virtual std::string toString() const override;
	virtual u256 literalValue(Literal const& _literal) const override;

	int getNumBits() const { return m_bits; }
	bool isHash() const { return m_modifier == Modifier::HASH || m_modifier == Modifier::ADDRESS; }
	bool isAddress() const { return m_modifier == Modifier::ADDRESS; }
	int isSigned() const { return m_modifier == Modifier::SIGNED; }

private:
	int m_bits;
	Modifier m_modifier;
};

/// The boolean type.
class BoolType: public Type
{
public:
	virtual Category getCategory() const { return Category::BOOL; }
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool acceptsBinaryOperator(Token::Value _operator) const override
	{
		return _operator == Token::AND || _operator == Token::OR;
	}
	virtual bool acceptsUnaryOperator(Token::Value _operator) const override
	{
		return _operator == Token::NOT || _operator == Token::DELETE;
	}

	virtual unsigned getCalldataEncodedSize() const { return 1; }

	virtual std::string toString() const override { return "bool"; }
	virtual u256 literalValue(Literal const& _literal) const override;
};

/// The type of a contract instance, there is one distinct type for each contract definition.
class ContractType: public Type
{
public:
	virtual Category getCategory() const override { return Category::CONTRACT; }
	ContractType(ContractDefinition const& _contract): m_contract(_contract) {}

	virtual bool operator==(Type const& _other) const override;

	virtual std::string toString() const override { return "contract{...}"; }

private:
	ContractDefinition const& m_contract;
};

/// The type of a struct instance, there is one distinct type per struct definition.
class StructType: public Type
{
public:
	virtual Category getCategory() const override { return Category::STRUCT; }
	StructType(StructDefinition const& _struct): m_struct(_struct) {}
	virtual bool acceptsUnaryOperator(Token::Value _operator) const override
	{
		return _operator == Token::DELETE;
	}

	virtual bool operator==(Type const& _other) const override;

	virtual std::string toString() const override { return "struct{...}"; }

private:
	StructDefinition const& m_struct;
};

/// The type of a function, there is one distinct type per function definition.
class FunctionType: public Type
{
public:
	virtual Category getCategory() const override { return Category::FUNCTION; }
	FunctionType(FunctionDefinition const& _function): m_function(_function) {}

	FunctionDefinition const& getFunction() const { return m_function; }

	virtual std::string toString() const override { return "function(...)returns(...)"; }

	virtual bool operator==(Type const& _other) const override;

private:
	FunctionDefinition const& m_function;
};

/// The type of a mapping, there is one distinct type per key/value type pair.
class MappingType: public Type
{
public:
	virtual Category getCategory() const override { return Category::MAPPING; }
	MappingType() {}
	virtual std::string toString() const override { return "mapping(...=>...)"; }

	virtual bool operator==(Type const& _other) const override;

private:
	std::shared_ptr<Type const> m_keyType;
	std::shared_ptr<Type const> m_valueType;
};

/// The void type, can only be implicitly used as the type that is returned by functions without
/// return parameters.
class VoidType: public Type
{
public:
	virtual Category getCategory() const override { return Category::VOID; }
	VoidType() {}

	virtual std::string toString() const override { return "void"; }
};

/// The type of a type reference. The type of "uint32" when used in "a = uint32(2)" is an example
/// of a TypeType.
class TypeType: public Type
{
public:
	virtual Category getCategory() const override { return Category::TYPE; }
	TypeType(std::shared_ptr<Type const> const& _actualType): m_actualType(_actualType) {}

	std::shared_ptr<Type const> const& getActualType() const { return m_actualType; }

	virtual bool operator==(Type const& _other) const override;

	virtual std::string toString() const override { return "type(" + m_actualType->toString() + ")"; }

private:
	std::shared_ptr<Type const> m_actualType;
};


}
}
