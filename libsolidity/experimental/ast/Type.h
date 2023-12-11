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
#pragma once

#include <cstddef>
#include <set>
#include <variant>
#include <vector>

namespace solidity::frontend::experimental
{

class TypeSystem;

struct TypeConstant;
struct TypeVariable;

using Type = std::variant<std::monostate, TypeConstant, TypeVariable>;

enum class PrimitiveType
{
	Void,
	Function,
	TypeFunction,
	Itself,
	Unit,
	Pair,
	Sum,
	Word,
	Bool,
	Integer
};

enum class PrimitiveClass
{
	Type
};

struct TypeConstructor
{
public:
	TypeConstructor(TypeConstructor const& _typeConstructor): m_index(_typeConstructor.m_index) {}
	TypeConstructor& operator=(TypeConstructor const& _typeConstructor)
	{
		m_index = _typeConstructor.m_index;
		return *this;
	}
	bool operator<(TypeConstructor const& _rhs) const
	{
		return m_index < _rhs.m_index;
	}
	bool operator==(TypeConstructor const& _rhs) const
	{
		return m_index == _rhs.m_index;
	}
	bool operator!=(TypeConstructor const& _rhs) const
	{
		return m_index != _rhs.m_index;
	}
private:
	friend class TypeSystem;
	TypeConstructor(std::size_t _index): m_index(_index) {}
	std::size_t m_index = 0;
};

struct TypeConstant
{
	TypeConstructor constructor;
	std::vector<Type> arguments;
};

struct TypeClass
{
public:
	TypeClass(TypeClass const& _typeClass): m_index(_typeClass.m_index) {}
	TypeClass& operator=(TypeClass const& _typeConstructor)
	{
		m_index = _typeConstructor.m_index;
		return *this;
	}
	bool operator<(TypeClass const& _rhs) const
	{
		return m_index < _rhs.m_index;
	}
	bool operator==(TypeClass const& _rhs) const
	{
		return m_index == _rhs.m_index;
	}
	bool operator!=(TypeClass const& _rhs) const
	{
		return m_index != _rhs.m_index;
	}
private:
	friend class TypeSystem;
	TypeClass(std::size_t _index): m_index(_index) {}
	std::size_t m_index = 0;
};

struct Sort
{
	std::set<TypeClass> classes;
	bool operator==(Sort const& _rhs) const;
	bool operator!=(Sort const& _rhs) const { return !operator==(_rhs); }
	bool operator<=(Sort const& _rhs) const;
	Sort operator+(Sort const& _rhs) const;
	Sort operator-(Sort const& _rhs) const;
};

struct Arity
{
	std::vector<Sort> argumentSorts;
	TypeClass typeClass;
};

struct TypeVariable
{
	std::size_t index() const { return m_index; }
	Sort const& sort() const { return m_sort; }
private:
	friend class TypeSystem;
	std::size_t m_index = 0;
	Sort m_sort;
	TypeVariable(std::size_t _index, Sort _sort): m_index(_index), m_sort(std::move(_sort)) {}
};

}
