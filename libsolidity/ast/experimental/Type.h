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
	Bool,
	Integer
};

using TypeConstructor = std::variant<BuiltinType, Declaration const*>;
}
namespace std
{
template<>
struct less<solidity::frontend::experimental::TypeConstructor>
{
	bool operator()(solidity::frontend::experimental::TypeConstructor const& _lhs, solidity::frontend::experimental::TypeConstructor const& _rhs) const;
};
}
namespace solidity::frontend::experimental
{
struct TypeConstant
{
	TypeConstructor constructor;
	std::vector<Type> arguments;
};

enum class BuiltinClass
{
	Type,
	Kind,
	Constraint,
	Integer,
	Mul,
	Add,
	Equal,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual
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
private:
	friend class TypeSystem;
	size_t m_index = 0;
	Sort m_sort;
	bool m_generic = false;
	TypeVariable(size_t _index, Sort _sort, bool _generic):
	m_index(_index), m_sort(std::move(_sort)), m_generic(_generic) {}
};

}
