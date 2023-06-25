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

#include <libsolidity/ast/experimental/Type.h>
#include <liblangutil/Exceptions.h>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace solidity::frontend::experimental
{

class TypeEnvironment
{
public:
	TypeEnvironment(TypeSystem& _typeSystem): m_typeSystem(_typeSystem) {}
	TypeEnvironment(TypeEnvironment const&) = delete;
	TypeEnvironment& operator=(TypeEnvironment const&) = delete;
	TypeEnvironment clone() const;
	Type resolve(Type _type) const;
	Type resolveRecursive(Type _type) const;
	Type fresh(Type _type);
	struct TypeMismatch { Type a; Type b; };
	struct SortMismatch { Type type; Sort sort; };
	using UnificationFailure = std::variant<TypeMismatch, SortMismatch>;
	[[nodiscard]] std::vector<UnificationFailure> unify(Type _a, Type _b);
	Sort sort(Type _type) const;
	bool typeEquals(Type _lhs, Type _rhs) const;
	TypeSystem& typeSystem() { return m_typeSystem; }
	TypeSystem const& typeSystem() const { return m_typeSystem; }
private:
	TypeEnvironment(TypeEnvironment&& _env): m_typeSystem(_env.m_typeSystem), m_typeVariables(std::move(_env.m_typeVariables)) {}
	[[nodiscard]] std::vector<TypeEnvironment::UnificationFailure> instantiate(TypeVariable _variable, Type _type);
	TypeSystem& m_typeSystem;
	std::map<size_t, Type> m_typeVariables;
};

class TypeSystem
{
public:
	struct TypeConstructorInfo
	{
		std::string name;
		std::vector<Arity> arities;
		size_t arguments() const
		{
			solAssert(!arities.empty());
			return arities.front().argumentSorts.size();
		}
	};
	struct TypeClassInfo
	{
		Type typeVariable;
		std::map<std::string, Type> functions;
	};
	TypeSystem();
	TypeSystem(TypeSystem const&) = delete;
	TypeSystem const& operator=(TypeSystem const&) = delete;
	Type type(TypeConstructor _typeConstructor, std::vector<Type> _arguments) const;
	std::string typeName(TypeConstructor _typeConstructor) const
	{
		// TODO: proper error handling
		return m_typeConstructors.at(_typeConstructor).name;
	}
	void declareTypeConstructor(TypeConstructor _typeConstructor, std::string _name, size_t _arguments);
	size_t constructorArguments(TypeConstructor _typeConstructor) const
	{
		// TODO: error handling
		return m_typeConstructors.at(_typeConstructor).arguments();
	}
	TypeConstructorInfo const& constructorInfo(TypeConstructor _typeConstructor) const
	{
		// TODO: error handling
		return m_typeConstructors.at(_typeConstructor);
	}
	TypeClassInfo const* typeClassInfo(TypeClass _class) const
	{
		return util::valueOrNullptr(m_typeClasses, _class);
	}

	[[nodiscard]] std::optional<std::string> declareTypeClass(TypeClass _class, Type _typeVariable, std::map<std::string, Type> _functions);
	[[nodiscard]] std::optional<std::string> instantiateClass(Type _instanceVariable, Arity _arity, std::map<std::string, Type> _functions);

	Type freshTypeVariable(Sort _sort);
	Type freshKindVariable(Sort _sort);

	TypeEnvironment const& env() const { return m_globalTypeEnvironment; }
	TypeEnvironment& env() { return m_globalTypeEnvironment; }

	Type freshVariable(Sort _sort);
private:
	size_t m_numTypeVariables = 0;
	std::map<TypeConstructor, TypeConstructorInfo> m_typeConstructors;
	std::map<TypeClass, TypeClassInfo> m_typeClasses;
	TypeEnvironment m_globalTypeEnvironment{*this};
};

}
