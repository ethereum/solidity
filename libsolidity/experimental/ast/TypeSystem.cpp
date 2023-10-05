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


#include <libsolidity/experimental/ast/TypeSystem.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>
#include <libsolidity/ast/AST.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/Visitor.h>

#include <range/v3/to_container.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

#include <fmt/format.h>

#include <unordered_map>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;

std::vector<TypeEnvironment::UnificationFailure> TypeEnvironment::unify(Type _a, Type _b)
{
	std::vector<UnificationFailure> failures;

	auto unificationFailure = [&]() {
		failures.emplace_back(UnificationFailure{TypeMismatch{_a, _b}});
	};

	_a = resolve(_a);
	_b = resolve(_b);
	std::visit(util::GenericVisitor{
		[&](GenericTypeVariable const& _left, GenericTypeVariable const& _right) {
			if (_left.index() == _right.index())
				solAssert(_left.sort() == _right.sort());
			else
			{
				if (_left.sort() <= _right.sort())
					failures += instantiate(_left, _right);
				else if (_right.sort() <= _left.sort())
					failures += instantiate(_right, _left);
				else
				{
					Type newVar = m_typeSystem.freshGenericVariable(_left.sort() + _right.sort());
					failures += instantiate(_left, newVar);
					failures += instantiate(_right, newVar);
				}
			}
		},
		[&](GenericTypeVariable const& _var, auto const&) {
			failures += instantiate(_var, _b);
		},
		[&](auto const&, GenericTypeVariable const& _var) {
			failures += instantiate(_var, _a);
		},
		[&](TypeConstant const& _left, TypeConstant const& _right) {
			if (_left.constructor != _right.constructor)
				return unificationFailure();
			if (_left.arguments.size() != _right.arguments.size())
				return unificationFailure();
			for (auto&& [left, right]: ranges::zip_view(_left.arguments, _right.arguments))
				failures += unify(left, right);
		},
		[&](auto const&, auto const&) {
			unificationFailure();
		}
	}, _a, _b);
	return failures;
}

bool TypeEnvironment::typeEquals(Type _lhs, Type _rhs) const
{
	return std::visit(util::GenericVisitor{
		[&](GenericTypeVariable const& _left, GenericTypeVariable const& _right) {
			if (_left.index() == _right.index())
			{
				solAssert(_left.sort() == _right.sort());
				return true;
			}
			return false;
		},
		[&](TypeConstant const& _left, TypeConstant const& _right) {
			if (_left.constructor != _right.constructor)
				return false;
			if (_left.arguments.size() != _right.arguments.size())
				return false;
			for (auto&& [left, right]: ranges::zip_view(_left.arguments, _right.arguments))
				if (!typeEquals(left, right))
					return false;
			return true;
		},
		[&](auto const&, auto const&) {
			return false;
		}
	}, resolve(_lhs), resolve(_rhs));
}

TypeEnvironment TypeEnvironment::clone() const
{
	TypeEnvironment result{m_typeSystem};
	result.m_genericTypeVariables = m_genericTypeVariables;
	return result;
}

TypeSystem::TypeSystem()
{
	auto declarePrimitiveClass = [&](std::string _name) {
		return std::visit(util::GenericVisitor{
			[](std::string _error) -> TypeClass {
				solAssert(false, _error);
			},
			[](TypeClass _class) -> TypeClass { return _class; }
		}, declareTypeClass(freshGenericVariable({}), _name, nullptr));
	};

	m_primitiveTypeClasses.emplace(PrimitiveClass::Type, declarePrimitiveClass("type"));

	for (auto [type, name, arity]: std::initializer_list<std::tuple<PrimitiveType, char const*, size_t>>{
		{PrimitiveType::TypeFunction, "tfun", 2},
		{PrimitiveType::Function, "fun", 2},
		{PrimitiveType::Itself, "itself", 1},
		{PrimitiveType::Void, "void", 0},
		{PrimitiveType::Unit, "unit", 0},
		{PrimitiveType::Pair, "pair", 2},
		{PrimitiveType::Sum, "sum", 2},
		{PrimitiveType::Word, "word", 0},
		{PrimitiveType::Integer, "integer", 0},
		{PrimitiveType::Bool, "bool", 0},
	})
		m_primitiveTypeConstructors.emplace(type, declareTypeConstructor(name, name, arity, nullptr));

	TypeClass classType = primitiveClass(PrimitiveClass::Type);
	//TypeClass classKind = primitiveClass(PrimitiveClass::Kind);
	Sort typeSort{{classType}};
	m_typeConstructors.at(m_primitiveTypeConstructors.at(PrimitiveType::TypeFunction).m_index).arities = {Arity{std::vector<Sort>{{typeSort},{typeSort}}, classType}};
	m_typeConstructors.at(m_primitiveTypeConstructors.at(PrimitiveType::Function).m_index).arities = {Arity{std::vector<Sort>{{typeSort, typeSort}}, classType}};
	m_typeConstructors.at(m_primitiveTypeConstructors.at(PrimitiveType::Itself).m_index).arities = {Arity{std::vector<Sort>{{typeSort, typeSort}}, classType}};
}

GenericTypeVariable TypeSystem::freshGenericVariable(Sort _sort)
{
	size_t index = m_numGenericTypeVariables++;
	return GenericTypeVariable(index, std::move(_sort));
}

GenericTypeVariable TypeSystem::freshGenericTypeVariable(Sort _sort)
{
	_sort.classes.emplace(primitiveClass(PrimitiveClass::Type));
	return freshGenericVariable(_sort);
}

std::vector<TypeEnvironment::UnificationFailure> TypeEnvironment::instantiate(GenericTypeVariable _variable, Type _type)
{
	for (auto const& typeVar: TypeEnvironmentHelpers{*this}.typeVars(_type))
		if (auto const* genericTypeVar = std::get_if<GenericTypeVariable>(&typeVar))
			if (genericTypeVar->index() == _variable.index())
				return {UnificationFailure{RecursiveUnification{_variable, _type}}};

	Sort typeSort = sort(_type);
	if (!(_variable.sort() <= typeSort))
		return {UnificationFailure{SortMismatch{_type, _variable.sort() - typeSort}}};

	auto [_, inserted] = m_genericTypeVariables.emplace(_variable.index(), _type);
	solAssert(inserted);
	return {};
}

experimental::Type TypeEnvironment::resolve(Type _type) const
{
	Type result = _type;
	while (auto const* var = std::get_if<GenericTypeVariable>(&result))
		if (Type const* resolvedType = util::valueOrNullptr(m_genericTypeVariables, var->index()))
			result = *resolvedType;
		else
			break;
	return result;
}

experimental::Type TypeEnvironment::resolveRecursive(Type _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const& _typeConstant) -> Type {
			return TypeConstant{
				_typeConstant.constructor,
				_typeConstant.arguments | ranges::views::transform([&](Type const& _argType) {
					return resolveRecursive(_argType);
				}) | ranges::to<std::vector<Type>>
			};
		},
		[](GenericTypeVariable const& _genericTypeVar) -> Type {
			return _genericTypeVar;
		},
		[](std::monostate const& _nothing) -> Type {
			return _nothing;
		}
	}, resolve(_type));
}

Sort TypeEnvironment::sort(Type _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const& _expression) -> Sort
		{
			auto const& constructorInfo = m_typeSystem.constructorInfo(_expression.constructor);
			auto argumentSorts = _expression.arguments | ranges::views::transform([&](Type _argumentType) {
				return sort(resolve(_argumentType));
			}) | ranges::to<std::vector<Sort>>;
			Sort sort;
			for (auto const& arity: constructorInfo.arities)
			{
				solAssert(arity.argumentSorts.size() == argumentSorts.size());
				bool hasArity = true;
				for (auto&& [argumentSort, arityArgumentSort]: ranges::zip_view(argumentSorts, arity.argumentSorts))
				{
					if (!(arityArgumentSort <= argumentSort))
					{
						hasArity = false;
						break;
					}
				}

				if (hasArity)
					sort.classes.insert(arity.typeClass);
			}
			return sort;
		},
		[](GenericTypeVariable const& _variable) -> Sort { return _variable.sort(); },
		[](std::monostate) -> Sort { solAssert(false); }
	}, _type);
}

TypeConstructor TypeSystem::declareTypeConstructor(std::string _name, std::string _canonicalName, size_t _argumentCount, Declaration const* _declaration)
{
	solAssert(m_canonicalTypeNames.insert(_canonicalName).second, "Duplicate canonical type name.");
	Sort baseSort{{primitiveClass(PrimitiveClass::Type)}};
	size_t index = m_typeConstructors.size();
	m_typeConstructors.emplace_back(TypeConstructorInfo{
		_name,
		_canonicalName,
		{Arity{std::vector<Sort>{_argumentCount, baseSort}, primitiveClass(PrimitiveClass::Type)}},
		_declaration
	});
	TypeConstructor constructor{index};
	if (_argumentCount > 0)
	{
		std::vector<Sort> argumentSorts;
		std::generate_n(std::back_inserter(argumentSorts), _argumentCount, [&](){ return Sort{{primitiveClass(PrimitiveClass::Type)}}; });
		std::vector<Type> argumentTypes;
		std::generate_n(std::back_inserter(argumentTypes), _argumentCount, [&](){ return freshGenericVariable({}); });
		auto error = instantiateClass(type(constructor, argumentTypes), Arity{argumentSorts, primitiveClass(PrimitiveClass::Type)});
		solAssert(!error, *error);
	}
	else
	{
		auto error = instantiateClass(type(constructor, {}), Arity{{}, primitiveClass(PrimitiveClass::Type)});
		solAssert(!error, *error);
	}
	return constructor;
}

std::variant<TypeClass, std::string> TypeSystem::declareTypeClass(GenericTypeVariable _typeVariable, std::string _name, Declaration const* _declaration)
{
	size_t index = m_typeClasses.size();
	m_typeClasses.emplace_back(TypeClassInfo{
		_typeVariable,
		_name,
		_declaration
	});
	TypeClass typeClass{index};

	return typeClass;
}

experimental::Type TypeSystem::type(TypeConstructor _constructor, std::vector<Type> _arguments) const
{
	// TODO: proper error handling
	auto const& info = m_typeConstructors.at(_constructor.m_index);
	solAssert(
		info.argumentCount() == _arguments.size(),
		fmt::format("Type constructor '{}' accepts {} type arguments (got {}).", constructorInfo(_constructor).name, info.argumentCount(), _arguments.size())
	);
	return TypeConstant{_constructor, _arguments};
}

experimental::Type TypeEnvironment::fresh(Type _type)
{
	std::unordered_map<size_t, GenericTypeVariable> genericFreshReplacements;

	auto freshImpl = [&](Type const& _type, auto _recurse) -> Type {
		return std::visit(util::GenericVisitor{
			[&](TypeConstant const& _type) -> Type {
				return TypeConstant{
					_type.constructor,
					_type.arguments | ranges::views::transform([&](Type const& _argType) {
						return _recurse(_argType, _recurse);
					}) | ranges::to<std::vector<Type>>
				};
			},
			[&](GenericTypeVariable const& _genericVar) -> Type {
				if (GenericTypeVariable const* typeVariable = util::valueOrNullptr(genericFreshReplacements, _genericVar.index()))
				{
					// TODO: can there be a mismatch?
					solAssert(typeVariable->sort() == _genericVar.sort());
					return *typeVariable;
				}

				GenericTypeVariable freshVariable = m_typeSystem.freshGenericTypeVariable(_genericVar.sort());
				genericFreshReplacements.emplace(_genericVar.index(), freshVariable);
				return freshVariable;
			},
			[](std::monostate) -> Type { solAssert(false); }
		}, resolve(_type));
	};

	return freshImpl(_type, freshImpl);
}

std::optional<std::string> TypeSystem::instantiateClass(Type _instanceVariable, Arity _arity)
{
	if (!TypeSystemHelpers{*this}.isTypeConstant(_instanceVariable))
		return "Invalid instance variable.";
	auto [typeConstructor, typeArguments] = TypeSystemHelpers{*this}.destTypeConstant(_instanceVariable);
	auto& typeConstructorInfo = m_typeConstructors.at(typeConstructor.m_index);
	if (_arity.argumentSorts.size() != typeConstructorInfo.argumentCount())
		return "Invalid arity.";
	if (typeArguments.size() != typeConstructorInfo.argumentCount())
		return "Invalid arity.";

	typeConstructorInfo.arities.emplace_back(_arity);

	return std::nullopt;
}
