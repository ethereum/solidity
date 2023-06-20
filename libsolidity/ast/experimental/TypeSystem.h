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

enum class BuiltinType
{
	Void,
	Function,
	Unit,
	Pair,
	Word
};

struct TypeExpression
{
	using Constructor = std::variant<BuiltinType, Declaration const*>;
	Constructor constructor;
	std::vector<Type> arguments;

};

struct TypeVariable
{
	TypeSystem const& parent() const { return *m_parent; }
	uint64_t index() const { return m_index; }
	bool generic() const { return m_generic; }
private:
	friend class TypeSystem;
	TypeSystem const* m_parent = nullptr;
	uint64_t m_index = 0;
	bool m_generic = false;
	TypeVariable(TypeSystem const& _parent, uint64_t _index, bool _generic): m_parent(&_parent), m_index(_index), m_generic(_generic) {}
};

class TypeSystem
{
public:
	TypeSystem() {}
	TypeSystem(TypeSystem const&) = delete;
	TypeSystem const& operator=(TypeSystem const&) = delete;
	void declareBuiltinType(BuiltinType _builtinType, std::string _name, uint64_t _arity);
	Type builtinType(BuiltinType _builtinType, std::vector<Type> _arguments) const;
	std::string builtinTypeName(BuiltinType _builtinType) const
	{
		return m_builtinTypes.at(_builtinType).name;
	}
	Type freshFreeType();
	Type resolve(Type _type) const;
	std::string typeToString(Type const& _type) const;
	Type freshTypeVariable(bool _generic);
	Type fresh(Type _type, bool _generalize);
	struct UnificationFailure { Type a; Type b; };
	[[nodiscard]] std::vector<UnificationFailure> unify(Type _a, Type _b);
private:
	void instantiate(TypeVariable _variable, Type _type);
	void validate(TypeVariable _variable) const;
	std::vector<std::optional<Type>> m_freeTypes;
	struct TypeConstructorInfo
	{
		std::string name;
		uint64_t arity = 0;
	};
	std::map<BuiltinType, TypeConstructorInfo> m_builtinTypes;
	std::vector<std::optional<Type>> m_typeVariables;
};

struct TypeSystemHelpers
{
	TypeSystem& typeSystem;
	Type tupleType(std::vector<Type> _elements) const;
	Type functionType(Type _argType, Type _resultType) const;
	std::tuple<TypeExpression::Constructor, std::vector<Type>> destTypeExpression(Type _functionType) const;
	std::tuple<Type, Type> destFunctionType(Type _functionType) const;
};

}