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

#include <fmt/format.h>

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

struct SumType;
struct TupleType;
struct FunctionType;
struct WordType;
struct UserDefinedType;
struct TypeVariable;
struct FreeType;

using Type = std::variant<SumType, TupleType, FunctionType, WordType, UserDefinedType, TypeVariable, FreeType>;

struct SumType
{
	std::vector<Type const*> alternatives;
	std::string toString() const
	{
		return "sum";
	}
};

struct TupleType
{
	std::vector<Type const*> components;
	std::string toString() const
	{
		return "tuple";
	}
};

struct FunctionType
{
	Type const* codomain = nullptr;
	Type const* domain = nullptr;
	std::string toString() const
	{
		return "fun";
	}
};

struct WordType
{
	std::string toString() const
	{
		return "word";
	}
};

struct UserDefinedType
{
	Declaration const* declaration = nullptr;
	std::vector<Type const*> arguments;
	std::string toString() const
	{
		return "user_defined_type";
	}
};

struct TypeVariable
{
	std::string toString() const
	{
		return fmt::format("var<{}>", m_index);
	}
private:
	uint64_t index() const { return m_index; }
	friend class TypeSystem;
	uint64_t m_index = 0;
	TypeVariable(uint64_t _index): m_index(_index) {}
};

struct FreeType
{
	uint64_t index = 0;
	std::string toString() const
	{
		return fmt::format("free<{}>", index);
	}
};

inline std::string typeToString(Type const& _type)
{
	return std::visit([](auto _type) { return _type.toString(); }, _type);
}

class TypeEnvironment
{
public:
	void assignType(TypeSystem& _typeSystem, Declaration const* _declaration, Type _typeAssignment);
	Type lookup(TypeSystem& _typeSystem, Declaration const* _declaration);
	Type freshFreeType();
	void unify(TypeSystem& _typeSystem, Type _a, Type _b);
private:
	uint64_t m_numFreeTypes = 0;
	std::map<Declaration const*, Type> m_types;
};


class TypeSystem
{
public:
	TypeSystem() {}
	TypeSystem(TypeSystem const&) = delete;
	TypeSystem const& operator=(TypeSystem const&) = delete;
	Type freshTypeVariable()
	{
		uint64_t index = m_typeVariables.size();
		m_typeVariables.emplace_back(std::nullopt);
		return TypeVariable(index);
	}
	void instantiate(TypeVariable _variable, Type _type)
	{
		solAssert(_variable.index() < m_typeVariables.size());
		solAssert(!m_typeVariables.at(_variable.index()).has_value());
		m_typeVariables[_variable.index()] = _type;
	}
	Type resolve(Type _type)
	{
		Type result = _type;
		while(auto const* var = std::get_if<TypeVariable>(&result))
			if (auto value = m_typeVariables.at(var->index()))
				result = *value;
			else
				break;
		return result;
	}
private:
	std::vector<std::optional<Type>> m_typeVariables;
};

}