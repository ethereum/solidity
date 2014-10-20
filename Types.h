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
#include <boost/assert.hpp>
#include <libsolidity/ASTForward.h>
#include <libsolidity/Token.h>

namespace dev
{
namespace solidity
{

// @todo realMxN, string<N>, mapping

class Type: private boost::noncopyable
{
public:
	enum class Category
	{
		INTEGER, BOOL, REAL, STRING, CONTRACT, STRUCT, FUNCTION, MAPPING, VOID, TYPE
	};

	//! factory functions that convert an AST TypeName to a Type.
	static std::shared_ptr<Type> fromElementaryTypeName(Token::Value _typeToken);
	static std::shared_ptr<Type> fromUserDefinedTypeName(UserDefinedTypeName const& _typeName);
	static std::shared_ptr<Type> fromMapping(Mapping const& _typeName);

	static std::shared_ptr<Type> forLiteral(Literal const& _literal);

	virtual Category getCategory() const = 0;
	virtual bool isImplicitlyConvertibleTo(Type const&) const { return false; }
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const
	{
		return isImplicitlyConvertibleTo(_convertTo);
	}
	virtual bool acceptsBinaryOperator(Token::Value) const { return false; }
	virtual bool acceptsUnaryOperator(Token::Value) const { return false; }
};

class IntegerType: public Type
{
public:
	enum class Modifier
	{
		UNSIGNED, SIGNED, HASH, ADDRESS
	};
	virtual Category getCategory() const { return Category::INTEGER; }

	static std::shared_ptr<IntegerType> smallestTypeForLiteral(std::string const& _literal);

	explicit IntegerType(int _bits, Modifier _modifier = Modifier::UNSIGNED);

	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool acceptsBinaryOperator(Token::Value _operator) const override;
	virtual bool acceptsUnaryOperator(Token::Value _operator) const override;

	int getNumBits() const { return m_bits; }
	bool isHash() const { return m_modifier == Modifier::HASH || m_modifier == Modifier::ADDRESS; }
	bool isAddress() const { return m_modifier == Modifier::ADDRESS; }
	int isSigned() const { return m_modifier == Modifier::SIGNED; }

private:
	int m_bits;
	Modifier m_modifier;
};

class BoolType: public Type
{
public:
	virtual Category getCategory() const { return Category::BOOL; }
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const override
	{
		return _convertTo.getCategory() == Category::BOOL;
	}
	virtual bool isExplicitlyConvertibleTo(Type const& _convertTo) const override;
	virtual bool acceptsBinaryOperator(Token::Value _operator) const override
	{
		return _operator == Token::AND || _operator == Token::OR;
	}
	virtual bool acceptsUnaryOperator(Token::Value _operator) const override
	{
		return _operator == Token::NOT || _operator == Token::DELETE;
	}
};

class ContractType: public Type
{
public:
	virtual Category getCategory() const { return Category::CONTRACT; }
	ContractType(ContractDefinition const& _contract): m_contract(_contract) {}
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const;

private:
	ContractDefinition const& m_contract;
};

class StructType: public Type
{
public:
	virtual Category getCategory() const { return Category::STRUCT; }
	StructType(StructDefinition const& _struct): m_struct(_struct) {}
	virtual bool isImplicitlyConvertibleTo(Type const& _convertTo) const;
	virtual bool acceptsUnaryOperator(Token::Value _operator) const override
	{
		return _operator == Token::DELETE;
	}

private:
	StructDefinition const& m_struct;
};

class FunctionType: public Type
{
public:
	virtual Category getCategory() const { return Category::FUNCTION; }
	FunctionType(FunctionDefinition const& _function): m_function(_function) {}

	FunctionDefinition const& getFunction() const { return m_function; }

private:
	FunctionDefinition const& m_function;
};

class MappingType: public Type
{
public:
	virtual Category getCategory() const { return Category::MAPPING; }
	MappingType() {}
private:
	//@todo
};

//@todo should be changed into "empty anonymous struct"
class VoidType: public Type
{
public:
	virtual Category getCategory() const { return Category::VOID; }
	VoidType() {}
};

class TypeType: public Type
{
public:
	virtual Category getCategory() const { return Category::TYPE; }
	TypeType(std::shared_ptr<Type const> const& _actualType): m_actualType(_actualType) {}

	std::shared_ptr<Type const> const& getActualType() const { return m_actualType; }

private:
	std::shared_ptr<Type const> m_actualType;
};


}
}
