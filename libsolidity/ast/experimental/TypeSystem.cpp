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

std::string TypeEnvironment::typeToString(Type const& _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const& _type) {
			std::stringstream stream;
			auto printTypeArguments = [&]() {
				if (!_type.arguments.empty())
				{
					stream << "(";
					for (auto type: _type.arguments | ranges::views::drop_last(1))
						stream << typeToString(type) << ", ";
					stream << typeToString(_type.arguments.back());
					stream << ")";
				}
			};
			std::visit(util::GenericVisitor{
				[&](Declaration const* _declaration) {
					stream << m_typeSystem.typeName(_declaration);
					printTypeArguments();
				},
				[&](BuiltinType _builtinType) {
					switch (_builtinType)
					{
					case BuiltinType::Function:
						solAssert(_type.arguments.size() == 2);
						stream << fmt::format("{} -> {}", typeToString(_type.arguments.front()), typeToString(_type.arguments.back()));
						break;
					case BuiltinType::Unit:
						solAssert(_type.arguments.empty());
						stream << "()";
						break;
					case BuiltinType::Pair:
					{
						auto tupleTypes = TypeSystemHelpers{m_typeSystem}.destTupleType(_type);
						stream << "(";
						for (auto type: tupleTypes | ranges::views::drop_last(1))
							stream << typeToString(type) << ", ";
						stream << typeToString(tupleTypes.back()) << ")";
						break;
					}
					case BuiltinType::Type:
					{
						solAssert(_type.arguments.size() == 1);
						stream << "TYPE(" << typeToString(_type.arguments.front()) << ")";
						break;
					}
					default:
						stream << m_typeSystem.typeName(_builtinType);
						printTypeArguments();
						break;
					}
				}
			}, _type.constructor);
			return stream.str();
		},
		[](TypeVariable const& _type) {
			std::stringstream stream;
			stream << (_type.generic() ? '?' : '\'') << "var" << _type.index();
			switch (_type.sort().classes.size())
			{
			case 0:
				break;
			case 1:
				stream << ":" << _type.sort().classes.begin()->toString();
				break;
			default:
				stream << ":(";
				for (auto typeClass: _type.sort().classes | ranges::views::drop_last(1))
					stream << typeClass.toString() << ", ";
				stream << _type.sort().classes.rbegin()->toString();
				stream << ")";
				break;
			}
			return stream.str();
		},
	}, resolve(_type));
}

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
					Type newVar = m_typeSystem.freshVariable(_left.generic() && _right.generic(), _left.sort() + _right.sort());
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

TypeEnvironment TypeEnvironment::clone() const
{
	TypeEnvironment result{m_typeSystem};
	result.m_typeVariables = m_typeVariables;
	return result;
}

TypeSystem::TypeSystem()
{
	Sort kindSort{{TypeClass{BuiltinClass::Kind}}};
	Sort typeSort{{TypeClass{BuiltinClass::Type}}};
	m_typeConstructors[BuiltinType::Type] = TypeConstructorInfo{
		"type",
		{Arity{vector<Sort>{{typeSort}}, TypeClass{BuiltinClass::Kind}}}
	};
	m_typeConstructors[BuiltinType::Sort] = TypeConstructorInfo{
		"constraint",
		{Arity{vector<Sort>{{kindSort}}, TypeClass{BuiltinClass::Constraint}}}
	};
	m_typeConstructors[BuiltinType::Function] = TypeConstructorInfo{
		"fun",
		{
			Arity{vector<Sort>{{typeSort, typeSort}}, TypeClass{BuiltinClass::Type}},
		}
	};
}

experimental::Type TypeSystem::freshVariable(bool _generic, Sort _sort)
{
	uint64_t index = m_numTypeVariables++;
	return TypeVariable(index, std::move(_sort), _generic);
}

experimental::Type TypeSystem::freshTypeVariable(bool _generic, Sort _sort)
{
	_sort.classes.emplace(TypeClass{BuiltinClass::Type});
	return freshVariable(_generic, _sort);
}

experimental::Type TypeSystem::freshKindVariable(bool _generic, Sort _sort)
{
	_sort.classes.emplace(TypeClass{BuiltinClass::Kind});
	return freshVariable(_generic, _sort);
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

void TypeSystem::declareTypeClass(TypeConstructor _classDeclaration, std::string _name)
{
	bool newlyInserted = m_typeConstructors.emplace(std::make_pair(_classDeclaration, TypeConstructorInfo{
		_name,
		{Arity{vector<Sort>{}, TypeClass{BuiltinClass::Kind}}}
	})).second;
	// TODO: proper error handling.
	solAssert(newlyInserted, "Type class already declared.");

}

experimental::Type TypeSystem::type(TypeConstructor _constructor, std::vector<Type> _arguments) const
{
	// TODO: proper error handling
	auto const& info = m_typeConstructors.at(_constructor);
	solAssert(info.arguments() == _arguments.size(), "Invalid arity.");
	return TypeConstant{_constructor, _arguments};
}

experimental::Type TypeEnvironment::fresh(Type _type, bool _generalize)
{
	std::unordered_map<uint64_t, Type> mapping;
	auto freshImpl = [&](Type _type, bool _generalize, auto _recurse) -> Type {
		return std::visit(util::GenericVisitor{
			[&](TypeConstant const& _type) -> Type {
				return TypeConstant{
					_type.constructor,
					_type.arguments | ranges::views::transform([&](Type _argType) {
						return _recurse(_argType, _generalize, _recurse);
					}) | ranges::to<vector<Type>>
				};
			},
			[&](TypeVariable const& _var) -> Type {
				if (_generalize || _var.generic())
				{
					if (auto* mapped = util::valueOrNullptr(mapping, _var.index()))
					{
						auto* typeVariable = get_if<TypeVariable>(mapped);
						solAssert(typeVariable);
						// TODO: can there be a mismatch?
						solAssert(typeVariable->sort() == _var.sort());
						return *mapped;
					}
					return mapping[_var.index()] = m_typeSystem.freshTypeVariable(true, _var.sort());
				}
				else
					return _type;
			},
		}, resolve(_type));
	};
	return freshImpl(_type, _generalize, freshImpl);
}

void TypeSystem::instantiateClass(TypeConstructor _typeConstructor, Arity _arity)
{
	// TODO: proper error handling
	auto& typeConstructorInfo = m_typeConstructors.at(_typeConstructor);
	solAssert(_arity.argumentSorts.size() == typeConstructorInfo.arguments(), "Invalid arity.");
	typeConstructorInfo.arities.emplace_back(_arity);
}

experimental::Type TypeSystemHelpers::tupleType(vector<Type> _elements) const
{
	if (_elements.empty())
		return typeSystem.type(BuiltinType::Unit, {});
	if (_elements.size() == 1)
		return _elements.front();
	Type result = _elements.back();
	for (Type type: _elements | ranges::views::reverse | ranges::views::drop_exactly(1))
		result = typeSystem.type(BuiltinType::Pair, {type, result});
	return result;
}

vector<experimental::Type> TypeSystemHelpers::destTupleType(Type _tupleType) const
{
	if (!isTypeConstant(_tupleType))
		return {_tupleType};
	auto [constructor, arguments] = destTypeConstant(_tupleType);
	if (auto const* builtinType = get_if<BuiltinType>(&constructor))
	{
		if (*builtinType == BuiltinType::Unit)
			return {};
		else if (*builtinType != BuiltinType::Pair)
			return {_tupleType};
	}
	else
		return {_tupleType};
	solAssert(arguments.size() == 2);

	vector<Type> result;
	result.emplace_back(arguments.front());
	Type tail = arguments.back();
	while(true)
	{
		if (!isTypeConstant(tail))
			break;
		auto [tailConstructor, tailArguments] = destTypeConstant(tail);
		auto const* builtinType = get_if<BuiltinType>(&tailConstructor);
		if(!builtinType || *builtinType != BuiltinType::Pair)
			break;
		solAssert(tailArguments.size() == 2);
		result.emplace_back(tailArguments.front());
		tail = tailArguments.back();
	}
	result.emplace_back(tail);
	return result;
}

experimental::Type TypeSystemHelpers::functionType(experimental::Type _argType, experimental::Type _resultType) const
{
	return typeSystem.type(BuiltinType::Function, {_argType, _resultType});
}

tuple<TypeConstructor, vector<experimental::Type>> TypeSystemHelpers::destTypeConstant(Type _type) const
{
	using ResultType = tuple<TypeConstructor, vector<Type>>;
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const& _type) -> ResultType {
			return std::make_tuple(_type.constructor, _type.arguments);
		},
		[](auto const&) -> ResultType {
			solAssert(false);
		}
	}, _type);
}

bool TypeSystemHelpers::isTypeConstant(Type _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const&) -> bool {
			return true;
		},
		[](auto const&) -> bool {
			return false;
		}
	}, _type);
}

tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destTypeConstant(_functionType);
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	solAssert(builtinType && *builtinType == BuiltinType::Function);
	solAssert(arguments.size() == 2);
	return make_tuple(arguments.front(), arguments.back());
}

bool TypeSystemHelpers::isFunctionType(Type _type) const
{
	if (!isTypeConstant(_type))
		return false;
	auto constructor = get<0>(destTypeConstant(_type));
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	return builtinType && *builtinType == BuiltinType::Function;
}

vector<experimental::Type> TypeSystemHelpers::typeVars(Type _type) const
{
	vector<Type> typeVars;
	auto typeVarsImpl = [&](Type _type, auto _recurse) -> void {
		std::visit(util::GenericVisitor{
			[&](TypeConstant const& _type) {
				for (auto arg: _type.arguments)
					_recurse(arg, _recurse);
			},
			[&](TypeVariable const& _var) {
				typeVars.emplace_back(_var);
			},
// TODO: move to env helpers?
		}, typeSystem.env().resolve(_type));
	};
	typeVarsImpl(_type, typeVarsImpl);
	return typeVars;

}

experimental::Type TypeSystemHelpers::kindType(Type _type) const
{
	return typeSystem.type(BuiltinType::Type, {_type});
}

experimental::Type TypeSystemHelpers::destKindType(Type _type) const
{
	auto [constructor, arguments] = destTypeConstant(_type);
	solAssert(constructor == TypeConstructor{BuiltinType::Type});
	solAssert(arguments.size() == 1);
	return arguments.front();
}

bool TypeSystemHelpers::isKindType(Type _type) const
{
	if (!isTypeConstant(_type))
		return false;
	auto constructor = get<0>(destTypeConstant(_type));
	return constructor == TypeConstructor{BuiltinType::Type};
}

std::string TypeSystemHelpers::sortToString(Sort _sort) const
{
	switch (_sort.classes.size())
	{
	case 0:
		return "()";
	case 1:
		return _sort.classes.begin()->toString();
	default:
	{
		std::stringstream stream;
		stream << "(";
		for (auto typeClass: _sort.classes | ranges::views::drop_last(1))
			stream << typeClass.toString() << ", ";
		stream << _sort.classes.rbegin()->toString() << ")";
		return stream.str();
	}
	}
}