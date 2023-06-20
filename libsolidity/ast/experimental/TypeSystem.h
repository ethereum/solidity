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

struct AtomicType;
struct GenericTypeVariable;
struct FreeTypeVariable;

using Type = std::variant<AtomicType, GenericTypeVariable, FreeTypeVariable>;

enum class BuiltinType
{
	Void,
	Function,
	Unit,
	Pair,
	Word
};

struct AtomicType
{
	using Constructor = std::variant<BuiltinType, Declaration const*>;
	Constructor constructor;
	std::vector<Type> arguments;

};

struct FreeTypeVariable
{
	TypeSystem const& parent() const { return *m_parent; }
	uint64_t index() const { return m_index; }
private:
	friend class TypeSystem;
	TypeSystem const* m_parent = nullptr;
	uint64_t m_index = 0;
	FreeTypeVariable(TypeSystem const& _parent, uint64_t _index): m_parent(&_parent), m_index(_index) {}
};

struct GenericTypeVariable
{
	TypeSystem const& parent() const { return *m_parent; }
	uint64_t index() const { return m_index; }
private:
	friend class TypeSystem;
	TypeSystem const* m_parent = nullptr;
	uint64_t m_index = 0;
	GenericTypeVariable(TypeSystem const& _parent, uint64_t _index): m_parent(&_parent), m_index(_index) {}
};

class TypeEnvironment
{
public:
	TypeEnvironment(TypeSystem& _parent): m_parent(_parent) {}
	TypeEnvironment(TypeEnvironment const& _env) = delete;
	TypeEnvironment& operator=(TypeEnvironment const& _env) = delete;
	std::unique_ptr<TypeEnvironment> fresh() const;
	void assignType(Declaration const* _declaration, Type _typeAssignment);
	std::optional<Type> lookup(Declaration const* _declaration);
private:
	TypeSystem& m_parent;
	std::map<Declaration const*, Type> m_types;
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
	void instantiate(GenericTypeVariable _variable, Type _type);
	void validate(GenericTypeVariable _variable) const;
	Type resolve(Type _type) const;
	std::string typeToString(Type const& _type) const;
	Type freshTypeVariable();
	Type fresh(Type _type);
	void unify(Type _a, Type _b);
private:
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
	std::tuple<AtomicType::Constructor, std::vector<Type>> destAtomicType(Type _functionType) const;
	std::tuple<Type, Type> destFunctionType(Type _functionType) const;
};

}