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

#include <set>
#include <string>
#include <variant>
#include <vector>

namespace solidity::frontend
{
class Declaration;
class TypeClassDefinition;
}

namespace solidity::frontend::experimental
{

class TypeSystem;

struct TypeConstant;
struct TypeVariable;

using Type = std::variant<TypeConstant, TypeVariable>;

enum class BuiltinType
{
	Type,
	Sort,
	Void,
	Function,
	Unit,
	Pair,
	Word,
	Integer
};

using TypeConstructor = std::variant<BuiltinType, Declaration const*>;

struct TypeConstant
{
	TypeConstructor constructor;
	std::vector<Type> arguments;
	bool operator<(TypeConstant const& _rhs) const;
	bool operator==(TypeConstant const& _rhs) const
	{
		// TODO
		return !(*this < _rhs) && !(_rhs < *this);
	}
	bool operator!=(TypeConstant const& _rhs) const
	{
		return !operator==(_rhs);
	}
};

enum class BuiltinClass
{
	Type,
	Kind,
	Constraint
};

struct TypeClass
{
	std::variant<BuiltinClass, TypeClassDefinition const*> declaration;
	std::string toString() const;
	bool operator<(TypeClass const& _rhs) const;
	bool operator==(TypeClass const& _rhs) const;
	bool operator!=(TypeClass const& _rhs) const { return !operator==(_rhs); }
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
	size_t index() const { return m_index; }
	bool generic() const { return m_generic; }
	Sort const& sort() const { return m_sort; }
	bool operator<(TypeVariable const& _rhs) const
	{
		// TODO: more robust comparison?
		return m_index < _rhs.m_index;
	}
	bool operator==(TypeVariable const& _rhs) const
	{
		// TODO
		return !(*this < _rhs) && !(_rhs < *this);
	}
	bool operator!=(TypeVariable const& _rhs) const
	{
		return !operator==(_rhs);
	}
private:
	friend class TypeSystem;
	size_t m_index = 0;
	Sort m_sort;
	bool m_generic = false;
	TypeVariable(size_t _index, Sort _sort, bool _generic):
	m_index(_index), m_sort(std::move(_sort)), m_generic(_generic) {}
};

}
