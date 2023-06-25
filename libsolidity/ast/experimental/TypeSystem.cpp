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


#include <libsolidity/ast/experimental/TypeSystem.h>
#include <libsolidity/ast/experimental/TypeSystemHelper.h>
#include <libsolidity/ast/AST.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/Visitor.h>

#include <range/v3/to_container.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

#include <fmt/format.h>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;

vector<TypeEnvironment::UnificationFailure> TypeEnvironment::unify(Type _a, Type _b)
{
	vector<UnificationFailure> failures;
	auto unificationFailure = [&]() {
		failures.emplace_back(UnificationFailure{TypeMismatch{_a, _b}});
	};
	_a = resolve(_a);
	_b = resolve(_b);
	std::visit(util::GenericVisitor{
		[&](TypeVariable _left, TypeVariable _right) {
			if (_left.index() == _right.index())
			{
				if (_left.sort() != _right.sort())
					unificationFailure();
			}
			else
			{
				if (_left.sort() <= _right.sort())
					failures += instantiate(_left, _right);
				else if (_right.sort() <= _left.sort())
					failures += instantiate(_right, _left);
				else
				{
					Type newVar = m_typeSystem.freshVariable(_left.sort() + _right.sort());
					failures += instantiate(_left, newVar);
					failures += instantiate(_right, newVar);
				}
			}
		},
		[&](TypeVariable _var, auto) {
			failures += instantiate(_var, _b);
		},
		[&](auto, TypeVariable _var) {
			failures += instantiate(_var, _a);
		},
		[&](TypeConstant _left, TypeConstant _right) {
		  if(_left.constructor != _right.constructor)
			  return unificationFailure();
		  if (_left.arguments.size() != _right.arguments.size())
			  return unificationFailure();
		   for (auto&& [left, right]: ranges::zip_view(_left.arguments, _right.arguments))
			  failures += unify(left, right);
		},
		[&](auto, auto) {
			unificationFailure();
		}
	}, _a, _b);
	return failures;
}

bool TypeEnvironment::typeEquals(Type _lhs, Type _rhs) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeVariable _left, TypeVariable _right) {
			if (_left.index() == _right.index())
			{
				solAssert(_left.sort() == _right.sort());
				return true;
			}
			return false;
		},
		[&](TypeConstant _left, TypeConstant _right) {
		  if(_left.constructor != _right.constructor)
			  return false;
		  if (_left.arguments.size() != _right.arguments.size())
			  return false;
		   for (auto&& [left, right]: ranges::zip_view(_left.arguments, _right.arguments))
			  if (!typeEquals(left, right))
				  return false;
		   return true;
		},
		[&](auto, auto) {
			return false;
		}
	}, resolve(_lhs), resolve(_rhs));
}

TypeEnvironment TypeEnvironment::clone() const
{
	TypeEnvironment result{m_typeSystem};
	result.m_typeVariables = m_typeVariables;
	return result;
}

TypeSystem::TypeSystem()
{
	Sort typeSort{{TypeClass{BuiltinClass::Type}}};
	m_typeConstructors[BuiltinType::TypeFunction] = TypeConstructorInfo{
		"tfun",
		{Arity{vector<Sort>{{typeSort},{typeSort}}, TypeClass{BuiltinClass::Kind}}}
	};
	m_typeConstructors[BuiltinType::Function] = TypeConstructorInfo{
		"fun",
		{
			Arity{vector<Sort>{{typeSort, typeSort}}, TypeClass{BuiltinClass::Type}},
		}
	};
}

experimental::Type TypeSystem::freshVariable(Sort _sort)
{
	uint64_t index = m_numTypeVariables++;
	return TypeVariable(index, std::move(_sort));
}

experimental::Type TypeSystem::freshTypeVariable(Sort _sort)
{
	_sort.classes.emplace(TypeClass{BuiltinClass::Type});
	return freshVariable(_sort);
}

experimental::Type TypeSystem::freshKindVariable(Sort _sort)
{
	_sort.classes.emplace(TypeClass{BuiltinClass::Kind});
	return freshVariable(_sort);
}

vector<TypeEnvironment::UnificationFailure> TypeEnvironment::instantiate(TypeVariable _variable, Type _type)
{
	Sort typeSort = sort(_type);
	if (!(_variable.sort() <= typeSort))
	{
		return {UnificationFailure{SortMismatch{_type, _variable.sort() - typeSort}}};
	}
	solAssert(m_typeVariables.emplace(_variable.index(), _type).second);
	return {};
}

experimental::Type TypeEnvironment::resolve(Type _type) const
{
	Type result = _type;
	while(auto const* var = std::get_if<TypeVariable>(&result))
		if (Type const* resolvedType = util::valueOrNullptr(m_typeVariables, var->index()))
			result = *resolvedType;
		else
			break;
	return result;
}

experimental::Type TypeEnvironment::resolveRecursive(Type _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const& _type) -> Type {
			return TypeConstant{
				_type.constructor,
				_type.arguments | ranges::views::transform([&](Type _argType) {
					return resolveRecursive(_argType);
				}) | ranges::to<vector<Type>>
			};
		},
		[&](TypeVariable const&) -> Type {
			return _type;
		},
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
			}) | ranges::to<vector<Sort>>;
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
		[](TypeVariable const& _variable) -> Sort { return _variable.sort(); },
	}, _type);
}

void TypeSystem::declareTypeConstructor(TypeConstructor _typeConstructor, std::string _name, size_t _arguments)
{
	Sort baseSort{{TypeClass{BuiltinClass::Type}}};
	bool newlyInserted = m_typeConstructors.emplace(std::make_pair(_typeConstructor, TypeConstructorInfo{
		_name,
		{Arity{vector<Sort>{_arguments, baseSort}, TypeClass{BuiltinClass::Type}}}
	})).second;
	// TODO: proper error handling.
	solAssert(newlyInserted, "Type constructor already declared.");
}

std::optional<std::string> TypeSystem::declareTypeClass(TypeClass _class, Type _typeVariable, std::map<std::string, Type> _functions)
{
	TypeVariable const* typeVariable = get_if<TypeVariable>(&_typeVariable);
	if (!typeVariable)
		return "Invalid type variable.";
	for (auto [functionName, functionType]: _functions)
	{
		auto typeVars = TypeEnvironmentHelpers{m_globalTypeEnvironment}.typeVars(functionType);
		if (typeVars.empty())
			return "Function " + functionName + " does not depend on class variable.";
		if (typeVars.size() > 2)
			return "Function " + functionName + " depends on multiple type variables.";
		if (get<TypeVariable>(typeVars.front()).index() != typeVariable->index())
			return "Function " + functionName + " depends on invalid type variable.";
	}

	if (!m_typeClasses.emplace(std::make_pair(_class, TypeClassInfo{
		_typeVariable,
		std::move(_functions)
	})).second)
		return "Type class already declared";
	return nullopt;

}

experimental::Type TypeSystem::type(TypeConstructor _constructor, std::vector<Type> _arguments) const
{
	// TODO: proper error handling
	auto const& info = m_typeConstructors.at(_constructor);
	solAssert(info.arguments() == _arguments.size(), "Invalid arity.");
	return TypeConstant{_constructor, _arguments};
}

experimental::Type TypeEnvironment::fresh(Type _type)
{
	std::unordered_map<uint64_t, Type> mapping;
	auto freshImpl = [&](Type _type, auto _recurse) -> Type {
		return std::visit(util::GenericVisitor{
			[&](TypeConstant const& _type) -> Type {
				return TypeConstant{
					_type.constructor,
					_type.arguments | ranges::views::transform([&](Type _argType) {
						return _recurse(_argType, _recurse);
					}) | ranges::to<vector<Type>>
				};
			},
			[&](TypeVariable const& _var) -> Type {
				if (auto* mapped = util::valueOrNullptr(mapping, _var.index()))
				{
					auto* typeVariable = get_if<TypeVariable>(mapped);
					solAssert(typeVariable);
					// TODO: can there be a mismatch?
					solAssert(typeVariable->sort() == _var.sort());
					return *mapped;
				}
				return mapping[_var.index()] = m_typeSystem.freshTypeVariable(_var.sort());
			},
		}, resolve(_type));
	};
	return freshImpl(_type, freshImpl);
}

std::optional<std::string> TypeSystem::instantiateClass(Type _instanceVariable, Arity _arity, map<string, Type> _functionTypes)
{
	if (!TypeSystemHelpers{*this}.isTypeConstant(_instanceVariable))
		return "Invalid instance variable.";
	auto [typeConstructor, typeArguments] = TypeSystemHelpers{*this}.destTypeConstant(_instanceVariable);
	auto& typeConstructorInfo = m_typeConstructors.at(typeConstructor);
	if (_arity.argumentSorts.size() != typeConstructorInfo.arguments())
		return "Invalid arity.";
	if (typeArguments.size() != typeConstructorInfo.arguments())
		return "Invalid arity.";

	auto const* classInfo = typeClassInfo(_arity.typeClass);
	if (!classInfo)
		return "Unknown class.";

	TypeEnvironment newEnv = m_globalTypeEnvironment.clone();

	std::set<size_t> typeVariables;

	Type classVariable = classInfo->typeVariable;
	if (!newEnv.unify(classVariable, _instanceVariable).empty())
		// TODO: error reporting
		return "Unification of class and instance variable failed.";

	for (auto [name, classFunctionType]: classInfo->functions)
	{
		if (!_functionTypes.count(name))
			return "Missing function: " + name;
		Type instanceFunctionType = _functionTypes.at(name);
		_functionTypes.erase(name);

		if (!newEnv.typeEquals(instanceFunctionType, classFunctionType))
			return "Type mismatch for function " + name + " " + TypeEnvironmentHelpers{newEnv}.typeToString(instanceFunctionType) + " != " + TypeEnvironmentHelpers{newEnv}.typeToString(classFunctionType);
	}

	typeConstructorInfo.arities.emplace_back(_arity);

	if (!_functionTypes.empty())
	{
		// TODO: list function names.
		return "Additional functions in class instantiation.";
	}

	return nullopt;
}
