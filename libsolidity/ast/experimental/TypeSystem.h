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

#include <liblangutil/Exceptions.h>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace solidity::frontend
{
class Declaration;
}

namespace solidity::frontend::experimental
{

class TypeSystem;
class TypeEnvironment;

struct TypeExpression;
struct TypeVariable;

using Type = std::variant<TypeExpression, TypeVariable>;

std::string canonicalTypeName(Type _type);

enum class BuiltinType
{
	Void,
	Function,
	Unit,
	Pair,
	Word,
	Integer
};

struct TypeExpression
{
	using Constructor = std::variant<BuiltinType, Declaration const*>;
	Constructor constructor;
	std::vector<Type> arguments;
	bool operator<(TypeExpression const& _rhs) const;
	bool operator==(TypeExpression const& _rhs) const
	{
		// TODO
		return !(*this < _rhs) && !(_rhs < *this);
	}
	bool operator!=(TypeExpression const& _rhs) const
	{
		return !operator==(_rhs);
	}
};

struct TypeClass
{
	Declaration const* declaration = nullptr;
	bool operator<(TypeClass const& _rhs) const;
	bool operator==(TypeClass const& _rhs) const { return declaration == _rhs.declaration; }
	bool operator!=(TypeClass const& _rhs) const { return declaration != _rhs.declaration; }
};

struct Sort
{
	std::set<TypeClass> classes;
	bool operator==(Sort const& _rhs) const;
	bool operator!=(Sort const& _rhs) const { return !operator==(_rhs); }
	bool operator<(Sort const& _rhs) const;
	Sort operator+(Sort const& _rhs) const;
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

class TypeEnvironment
{
public:
	TypeEnvironment(TypeSystem& _typeSystem): m_typeSystem(_typeSystem) {}
	TypeEnvironment(TypeEnvironment const&) = delete;
	TypeEnvironment& operator=(TypeEnvironment const&) = delete;
	TypeEnvironment clone() const;
	Type resolve(Type _type) const;
	Type resolveRecursive(Type _type) const;
	Type fresh(Type _type, bool _generalize);
	struct TypeMismatch { Type a; Type b; };
	struct SortMismatch { Type type; Sort sort; };
	using UnificationFailure = std::variant<TypeMismatch, SortMismatch>;
	[[nodiscard]] std::vector<UnificationFailure> unify(Type _a, Type _b);
	std::string typeToString(Type const& _type) const;
private:
	TypeEnvironment(TypeEnvironment&& _env): m_typeSystem(_env.m_typeSystem), m_typeVariables(std::move(_env.m_typeVariables)) {}
	[[nodiscard]] std::vector<TypeEnvironment::UnificationFailure> instantiate(TypeVariable _variable, Type _type);
	TypeSystem& m_typeSystem;
	std::map<size_t, Type> m_typeVariables;
};

class TypeSystem
{
public:
	TypeSystem() {}
	TypeSystem(TypeSystem const&) = delete;
	TypeSystem const& operator=(TypeSystem const&) = delete;
	Type type(TypeExpression::Constructor _typeConstructor, std::vector<Type> _arguments) const;
	std::string typeName(TypeExpression::Constructor _typeConstructor) const
	{
		// TODO: proper error handling
		return m_typeConstructors.at(_typeConstructor).name;
	}
	void declareTypeConstructor(TypeExpression::Constructor _typeConstructor, std::string _name, size_t _arguments);
	size_t constructorArguments(TypeExpression::Constructor _typeConstructor) const
	{
		// TODO: error handling
		return m_typeConstructors.at(_typeConstructor).arguments;
	}
	void instantiateClass(TypeExpression::Constructor _typeConstructor, Arity _arity);

	Type freshTypeVariable(bool _generic, Sort const& _sort);

	TypeEnvironment const& env() const { return m_globalTypeEnvironment; }
	TypeEnvironment& env() { return m_globalTypeEnvironment; }
	Sort sort(Type _type) const;
private:
	size_t m_numTypeVariables = 0;
	struct TypeConstructorInfo
	{
		std::string name;
		size_t arguments;
		std::vector<Arity> arities;
	};
	std::map<TypeExpression::Constructor, TypeConstructorInfo> m_typeConstructors;
	TypeEnvironment m_globalTypeEnvironment{*this};
};

struct TypeSystemHelpers
{
	TypeSystem const& typeSystem;
	std::tuple<TypeExpression::Constructor, std::vector<Type>> destTypeExpression(Type _functionType) const;
	Type tupleType(std::vector<Type> _elements) const;
	std::vector<Type> destTupleType(Type _tupleType) const;
	Type functionType(Type _argType, Type _resultType) const;
	std::tuple<Type, Type> destFunctionType(Type _functionType) const;
	std::vector<Type> typeVars(Type _type) const;
	std::string sortToString(Sort _sort) const;
};

}
