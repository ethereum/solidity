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

#include <libsolidity/experimental/ast/Type.h>
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

class TypeEnvironment
{
public:
	struct TypeMismatch
	{
		Type a;
		Type b;
	};

	struct SortMismatch {
		Type type;
		Sort sort;
	};

	struct RecursiveUnification
	{
		Type var;
		Type type;
	};

	using UnificationFailure = std::variant<
		TypeMismatch,
		SortMismatch,
		RecursiveUnification
	>;

	TypeEnvironment(TypeSystem& _typeSystem): m_typeSystem(_typeSystem) {}
	TypeEnvironment(TypeEnvironment const&) = delete;
	TypeEnvironment& operator=(TypeEnvironment const&) = delete;
	TypeEnvironment clone() const;

	Type resolve(Type _type) const;
	Type resolveRecursive(Type _type) const;
	Type fresh(Type _type);
	[[nodiscard]] std::vector<UnificationFailure> unify(Type _a, Type _b);
	Sort sort(Type _type) const;
	bool typeEquals(Type _lhs, Type _rhs) const;

	TypeSystem& typeSystem() { return m_typeSystem; }
	TypeSystem const& typeSystem() const { return m_typeSystem; }

	bool isFixedTypeVar(Type const& _typeVar) const;
	bool isGenericTypeVar(Type const& _typeVar) const;
	void fixTypeVars(std::vector<Type> const& _typeVars);

private:
	TypeEnvironment(TypeEnvironment&& _env):
		m_typeSystem(_env.m_typeSystem),
		m_typeVariables(std::move(_env.m_typeVariables))
	{}

	[[nodiscard]] std::vector<TypeEnvironment::UnificationFailure> instantiate(TypeVariable _variable, Type _type);

	TypeSystem& m_typeSystem;

	/// For each @a TypeVariable (identified by its index) stores the type is has been successfully
	/// unified with. Used for type resolution. Note that @a Type may itself be a type variable
	/// or may contain type variables so resolution must be recursive.
	std::map<size_t, Type> m_typeVariables;

	/// Type variables marked as fixed free type variables (as opposed to generic type variables).
	/// Identified by their indices.
	std::set<size_t> m_fixedTypeVariables;
};

class TypeSystem
{
public:
	struct TypeConstructorInfo
	{
		std::string name;
		std::string canonicalName;
		std::vector<Arity> arities;
		Declaration const* typeDeclaration = nullptr;
		size_t arguments() const
		{
			solAssert(!arities.empty());
			return arities.front().argumentSorts.size();
		}
	};
	struct TypeClassInfo
	{
		Type typeVariable;
		std::string name;
		Declaration const* classDeclaration = nullptr;
	};
	TypeSystem();
	TypeSystem(TypeSystem const&) = delete;
	TypeSystem const& operator=(TypeSystem const&) = delete;
	Type type(PrimitiveType _typeConstructor, std::vector<Type> _arguments) const
	{
		return type(m_primitiveTypeConstructors.at(_typeConstructor), std::move(_arguments));
	}
	Type type(TypeConstructor _typeConstructor, std::vector<Type> _arguments) const;
	std::string typeName(TypeConstructor _typeConstructor) const
	{
		// TODO: proper error handling
		return m_typeConstructors.at(_typeConstructor.m_index).name;
	}
	std::string canonicalName(TypeConstructor _typeConstructor) const
	{
		// TODO: proper error handling
		return m_typeConstructors.at(_typeConstructor.m_index).canonicalName;
	}
	TypeConstructor declareTypeConstructor(std::string _name, std::string _canonicalName, size_t _arguments, Declaration const* _declaration);
	TypeConstructor constructor(PrimitiveType _type) const
	{
		return m_primitiveTypeConstructors.at(_type);
	}
	TypeClass primitiveClass(PrimitiveClass _class) const
	{
		return m_primitiveTypeClasses.at(_class);
	}
	size_t constructorArguments(TypeConstructor _typeConstructor) const
	{
		// TODO: error handling
		return m_typeConstructors.at(_typeConstructor.m_index).arguments();
	}
	TypeConstructorInfo const& constructorInfo(TypeConstructor _typeConstructor) const
	{
		// TODO: error handling
		return m_typeConstructors.at(_typeConstructor.m_index);
	}
	TypeConstructorInfo const& constructorInfo(PrimitiveType _typeConstructor) const
	{
		return constructorInfo(constructor(_typeConstructor));
	}

	std::variant<TypeClass, std::string> declareTypeClass(std::string _name, Declaration const* _declaration, bool _primitive = false);
	[[nodiscard]] std::optional<std::string> instantiateClass(Type _instanceVariable, Arity _arity);

	Type freshTypeVariable(Sort _sort);

	TypeEnvironment const& env() const { return m_globalTypeEnvironment; }
	TypeEnvironment& env() { return m_globalTypeEnvironment; }

	Type freshVariable(Sort _sort);
	std::string typeClassName(TypeClass _class) const { return m_typeClasses.at(_class.m_index).name; }
	Declaration const* typeClassDeclaration(TypeClass _class) const { return m_typeClasses.at(_class.m_index).classDeclaration; }
	Type typeClassVariable(TypeClass _class) const
	{
		return m_typeClasses.at(_class.m_index).typeVariable;
	}

	TypeClassInfo const& typeClassInfo(TypeClass _class) const
	{
		return m_typeClasses.at(_class.m_index);
	}

private:
	friend class TypeEnvironment;
	size_t m_numTypeVariables = 0;
	std::map<PrimitiveType, TypeConstructor> m_primitiveTypeConstructors;
	std::map<PrimitiveClass, TypeClass> m_primitiveTypeClasses;
	std::set<std::string> m_canonicalTypeNames;
	std::vector<TypeConstructorInfo> m_typeConstructors;
	std::vector<TypeClassInfo> m_typeClasses;
	TypeEnvironment m_globalTypeEnvironment{*this};
};

}
