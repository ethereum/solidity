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

bool TypeClass::operator<(TypeClass const& _rhs) const
{
	return declaration->id() < _rhs.declaration->id();
}

bool Sort::operator==(Sort const& _rhs) const
{
	if (classes.size() != _rhs.classes.size())
		return false;
	for (auto [lhs, rhs]: ranges::zip_view(classes, _rhs.classes))
		if (lhs != rhs)
			return false;
	return true;
}

bool Sort::operator<(Sort const& _rhs) const
{
	for (auto c: classes)
		if (!_rhs.classes.count(c))
			return false;
	return true;
}

Sort Sort::operator+(Sort const& _rhs) const
{
	Sort result { classes };
	result.classes += _rhs.classes;
	return result;
}

bool TypeExpression::operator<(TypeExpression const& _rhs) const
{
	if (constructor < _rhs.constructor)
		return true;
	if (_rhs.constructor < constructor)
		return false;
	solAssert(arguments.size() == _rhs.arguments.size());
	for(auto [lhs, rhs]: ranges::zip_view(arguments, _rhs.arguments))
	{
		if (lhs < rhs)
			return true;
		if (rhs < lhs)
			return false;
	}
	return false;
}

std::string experimental::canonicalTypeName(Type _type)
{
	return std::visit(util::GenericVisitor{
		[&](TypeExpression const& _type) {
			std::stringstream stream;
			auto printTypeArguments = [&]() {
				if (!_type.arguments.empty())
				{
					stream << "$";
					for (auto type: _type.arguments | ranges::views::drop_last(1))
						stream << canonicalTypeName(type) << "$";
					stream << canonicalTypeName(_type.arguments.back());
					stream << "$";
				}
			};
			std::visit(util::GenericVisitor{
				[&](Declaration const* _declaration) {
					printTypeArguments();
					// TODO: canonical name
					stream << _declaration->name();
				},
				[&](BuiltinType _builtinType) {
					printTypeArguments();
					switch(_builtinType)
					{
					case BuiltinType::Void:
						stream << "void";
					break;
					case BuiltinType::Function:
						stream << "fun";
						break;
					case BuiltinType::Unit:
						stream << "unit";
						break;
					case BuiltinType::Pair:
						stream << "pair";
						break;
					case BuiltinType::Word:
						stream << "word";
						break;
					case BuiltinType::Integer:
						stream << "integer";
						break;
					}
				}
			}, _type.constructor);
			return stream.str();
		},
		[](TypeVariable const&)-> string {
			solAssert(false);
		},
	}, _type);
}

std::string TypeEnvironment::typeToString(Type const& _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeExpression const& _type) {
			std::stringstream stream;
			auto printTypeArguments = [&]() {
				if (!_type.arguments.empty())
				{
					stream << "(";
					for (auto type: _type.arguments | ranges::views::drop_last(1))
						stream << typeToString(type) << ", ";
					stream << typeToString(_type.arguments.back());
					stream << ") ";
				}
			};
			std::visit(util::GenericVisitor{
				[&](Declaration const* _declaration) {
					printTypeArguments();
					stream << m_typeSystem.typeName(_declaration);
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
					default:
						printTypeArguments();
						stream << m_typeSystem.typeName(_builtinType);
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
				stream << ":" << _type.sort().classes.begin()->declaration->name();
				break;
			default:
				stream << ":{";
				for (auto typeClass: _type.sort().classes | ranges::views::drop_last(1))
					stream << typeClass.declaration->name() << ", ";
				stream << _type.sort().classes.rbegin()->declaration->name();
				stream << "}";
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
		failures.emplace_back(UnificationFailure{_a, _b});
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
				if (_left.sort() < _right.sort())
					instantiate(_left, _right);
				else if (_right.sort() < _left.sort())
					instantiate(_right, _left);
				else
				{
					Type newVar = m_typeSystem.freshTypeVariable(_left.generic() && _right.generic(), _left.sort() + _right.sort());
					instantiate(_left, newVar);
					instantiate(_right, newVar);
				}
			}
		},
		[&](TypeVariable _var, auto) {
			instantiate(_var, _b);
		},
		[&](auto, TypeVariable _var) {
			instantiate(_var, _a);
		},
		[&](TypeExpression _left, TypeExpression _right) {
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

experimental::Type TypeSystem::freshTypeVariable(bool _generic, Sort const& _sort)
{
	uint64_t index = m_numTypeVariables++;
	return TypeVariable(index, _sort, _generic);
}

void TypeEnvironment::instantiate(TypeVariable _variable, Type _type)
{
	solAssert(m_typeVariables.emplace(_variable.index(), _type).second);
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

void TypeSystem::declareBuiltinType(BuiltinType _builtinType, std::string _name, uint64_t _arguments)
{
	declareTypeConstructor(_builtinType, _name, _arguments);
}

void TypeSystem::declareTypeConstructor(TypeExpression::Constructor _typeConstructor, std::string _name, size_t _arguments)
{
	bool newlyInserted = m_typeConstructors.emplace(std::make_pair(_typeConstructor, TypeConstructorInfo{
		_name,
		_arguments,
		{}
	})).second;
	// TODO: proper error handling.
	solAssert(newlyInserted, "Type constructor already declared.");
}

experimental::Type TypeSystem::builtinType(BuiltinType _builtinType, std::vector<Type> _arguments) const
{
	// TODO: proper error handling
	auto const& info = m_typeConstructors.at(_builtinType);
	solAssert(info.arguments == _arguments.size(), "Invalid arity.");
	return TypeExpression{_builtinType, _arguments};
}

experimental::Type TypeEnvironment::fresh(Type _type, bool _generalize)
{
	std::unordered_map<uint64_t, Type> mapping;
	auto freshImpl = [&](Type _type, bool _generalize, auto _recurse) -> Type {
		return std::visit(util::GenericVisitor{
			[&](TypeExpression const& _type) -> Type {
				return TypeExpression{
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

void TypeSystem::instantiateClass(TypeExpression::Constructor _typeConstructor, Arity _arity)
{
	// TODO: proper error handling
	auto& typeConstructorInfo = m_typeConstructors.at(_typeConstructor);
	solAssert(_arity._argumentSorts.size() == typeConstructorInfo.arguments, "Invalid arity.");
	typeConstructorInfo.arities.emplace_back(_arity);
}

experimental::Type TypeSystemHelpers::tupleType(vector<Type> _elements) const
{
	if (_elements.empty())
		return typeSystem.builtinType(BuiltinType::Unit, {});
	if (_elements.size() == 1)
		return _elements.front();
	Type result = _elements.back();
	for (Type type: _elements | ranges::views::reverse | ranges::views::drop_exactly(1))
		result = typeSystem.builtinType(BuiltinType::Pair, {type, result});
	return result;
}

vector<experimental::Type> TypeSystemHelpers::destTupleType(Type _tupleType) const
{
	auto [constructor, arguments] = destTypeExpression(_tupleType);
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
		auto const* tailTypeExpression = get_if<TypeExpression>(&tail);
		if (!tailTypeExpression)
			break;

		auto [tailConstructor, tailArguments] = destTypeExpression(tail);
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
	return typeSystem.builtinType(BuiltinType::Function, {_argType, _resultType});
}

tuple<TypeExpression::Constructor, vector<experimental::Type>> TypeSystemHelpers::destTypeExpression(Type _type) const
{
	using ResultType = tuple<TypeExpression::Constructor, vector<Type>>;
	return std::visit(util::GenericVisitor{
		[&](TypeExpression const& _type) -> ResultType {
			return std::make_tuple(_type.constructor, _type.arguments);
		},
		[](auto) -> ResultType {
			solAssert(false);
		}
	}, _type);
}

tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destTypeExpression(_functionType);
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	solAssert(builtinType && *builtinType == BuiltinType::Function);
	solAssert(arguments.size() == 2);
	return make_tuple(arguments.front(), arguments.back());
}

vector<experimental::Type> TypeSystemHelpers::typeVars(Type _type) const
{
	vector<Type> typeVars;
	auto typeVarsImpl = [&](Type _type, auto _recurse) -> void {
		std::visit(util::GenericVisitor{
			[&](TypeExpression const& _type) {
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