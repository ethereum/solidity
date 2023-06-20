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

std::string TypeSystem::typeToString(Type const& _type) const
{
	return std::visit(util::GenericVisitor{
		[&](TypeExpression const& _type) {
			std::stringstream stream;
			if (!_type.arguments.empty())
			{
				stream << "(";
				for (auto type: _type.arguments | ranges::views::drop_last(1))
					stream << typeToString(type) << ", ";
				stream << typeToString(_type.arguments.back());
				stream << ") ";
			}
			stream << std::visit(util::GenericVisitor{
				[&](Declaration const* _declaration) {
					return _declaration->name();
				},
				[&](BuiltinType _builtinType) -> string {
					return builtinTypeName(_builtinType);
				}
			}, _type.constructor);
			return stream.str();
		},
		[](TypeVariable const& _type) {
			return fmt::format("var[{}]", _type.index());
		},
	}, resolve(_type));
}

vector<TypeSystem::UnificationFailure> TypeSystem::unify(Type _a, Type _b)
{
	vector<UnificationFailure> failures;
	auto unificationFailure = [&]() {
		failures.emplace_back(UnificationFailure{_a, _b});
	};
	_a = resolve(_a);
	_b = resolve(_b);
	std::visit(util::GenericVisitor{
		[&](TypeVariable _left, TypeVariable _right) {
			validate(_left);
			validate(_right);
			if (_left.index() != _right.index())
				instantiate(_left, _right);
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

experimental::Type TypeSystem::freshTypeVariable(bool _generic)
{
	uint64_t index = m_typeVariables.size();
	m_typeVariables.emplace_back(std::nullopt);
	return TypeVariable(*this, index, _generic);
}

void TypeSystem::instantiate(TypeVariable _variable, Type _type)
{
	validate(_variable);
	solAssert(!m_typeVariables.at(_variable.index()).has_value());
	solAssert(_variable.m_parent == this);
	m_typeVariables[_variable.index()] = _type;
}

experimental::Type TypeSystem::resolve(Type _type) const
{
	Type result = _type;
	while(auto const* var = std::get_if<TypeVariable>(&result))
		if (auto value = m_typeVariables.at(var->index()))
			result = *value;
		else
			break;
	return result;
}

void TypeSystem::declareBuiltinType(BuiltinType _builtinType, std::string _name, uint64_t _arity)
{
	solAssert(m_builtinTypes.count(_builtinType) == 0, "Builtin type already declared.");
	m_builtinTypes[_builtinType] = TypeConstructorInfo{
		_name,
		_arity
	};
}

experimental::Type TypeSystem::builtinType(BuiltinType _builtinType, std::vector<Type> _arguments) const
{
	auto const& info = m_builtinTypes.at(_builtinType);
	solAssert(info.arity == _arguments.size(), "Invalid arity.");
	return TypeExpression{_builtinType, _arguments};
}

void TypeSystem::validate(TypeVariable _variable) const
{
	solAssert(_variable.m_parent == this);
	solAssert(_variable.index() < m_typeVariables.size());
}

experimental::Type TypeSystem::fresh(Type _type, bool _generalize)
{
	return std::visit(util::GenericVisitor{
		[&](TypeExpression const& _type) -> Type {
			return TypeExpression{
				_type.constructor,
				_type.arguments | ranges::view::transform([&](Type _argType) {
					return fresh(_argType, _generalize);
				}) | ranges::to<vector<Type>>
			};
		},
		[&](TypeVariable const& _var) {
			if (_generalize || _var.generic())
				return freshTypeVariable(true);
			else
				return _type;
		},
	}, resolve(_type));
}

experimental::Type TypeSystemHelpers::tupleType(vector<Type> _elements) const
{
	if (_elements.empty())
		return typeSystem.builtinType(BuiltinType::Unit, {});
	if (_elements.size() == 1)
		return _elements.front();
	Type result = _elements.back();
	for (Type type: _elements | ranges::view::reverse | ranges::view::drop_exactly(1))
		result = typeSystem.builtinType(BuiltinType::Pair, {type, result});
	return result;
}

experimental::Type TypeSystemHelpers::functionType(experimental::Type _argType, experimental::Type _resultType) const
{
	return typeSystem.builtinType(BuiltinType::Function, {_argType, _resultType});
}

tuple<TypeExpression::Constructor, vector<experimental::Type>> TypeSystemHelpers::destTypeExpression(Type _functionType) const
{
	using ResultType = tuple<TypeExpression::Constructor, vector<Type>>;
	return std::visit(util::GenericVisitor{
		[&](TypeExpression const& _type) -> ResultType {
			return std::make_tuple(_type.constructor, _type.arguments);
		},
		[](auto) -> ResultType {
			solAssert(false);
		}
	}, _functionType);
}


tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destTypeExpression(_functionType);
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	solAssert(builtinType && *builtinType == BuiltinType::Function);
	solAssert(arguments.size() == 2);
	return make_tuple(arguments.front(), arguments.back());
}
