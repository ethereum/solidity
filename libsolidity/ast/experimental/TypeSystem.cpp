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
		[&](AtomicType const& _type) {
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
		[](FreeTypeVariable const& _type) {
			return fmt::format("free[{}]", _type.index());
		},
		[](GenericTypeVariable const& _type) {
			return fmt::format("var[{}]", _type.index());
		},
	}, resolve(_type));
}

void TypeEnvironment::assignType(Declaration const* _declaration, Type _typeAssignment)
{
	auto&& [type, newlyInserted] = m_types.emplace(std::piecewise_construct, std::forward_as_tuple(_declaration), std::forward_as_tuple(std::move(_typeAssignment)));
	if (!newlyInserted)
	{
		m_parent.unify(type->second, _typeAssignment);
	}
}
std::optional<experimental::Type> TypeEnvironment::lookup(Declaration const* _declaration)
{
	if (m_types.count(_declaration))
		return m_types[_declaration];
	return std::nullopt;
}


void TypeSystem::unify(Type _a, Type _b)
{
	auto unificationFailure = [&]() {
		solAssert(false, fmt::format("cannot unify {} and {}", typeToString(_a), typeToString(_b)));
	};
	_a = resolve(_a);
	_b = resolve(_b);
	std::visit(util::GenericVisitor{
		[&](GenericTypeVariable _left, GenericTypeVariable _right) {
			validate(_left);
			validate(_right);
			if (_left.index() != _right.index())
				instantiate(_left, _right);
		},
		[&](GenericTypeVariable _var, auto) {
			instantiate(_var, _b);
		},
		[&](auto, GenericTypeVariable _var) {
			instantiate(_var, _a);
		},
		[&](AtomicType _atomicLeft, AtomicType _atomicRight) {
		  if(_atomicLeft.constructor != _atomicRight.constructor)
			  unificationFailure();
		  if (_atomicLeft.arguments.size() != _atomicRight.arguments.size())
			  unificationFailure();
		   for (auto&& [left, right]: ranges::zip_view(_atomicLeft.arguments, _atomicRight.arguments))
			  unify(left, right);
		},
		[&](auto, auto) {
			unificationFailure();
		}
	}, _a, _b);
}

unique_ptr<TypeEnvironment> TypeEnvironment::fresh() const
{
	auto newEnv = make_unique<TypeEnvironment>(m_parent);
	for(auto [decl,type]: m_types)
		newEnv->m_types.emplace(decl, type);
	return newEnv;

}

experimental::Type TypeSystem::freshTypeVariable()
{
	uint64_t index = m_typeVariables.size();
	m_typeVariables.emplace_back(std::nullopt);
	return GenericTypeVariable(*this, index);
}

experimental::Type TypeSystem::freshFreeType()
{
	uint64_t index = m_freeTypes.size();
	m_freeTypes.emplace_back(std::nullopt);
	return FreeTypeVariable(*this, index);
}

void TypeSystem::instantiate(GenericTypeVariable _variable, Type _type)
{
	validate(_variable);
	solAssert(!m_typeVariables.at(_variable.index()).has_value());
	solAssert(_variable.m_parent == this);
	m_typeVariables[_variable.index()] = _type;
}

experimental::Type TypeSystem::resolve(Type _type) const
{
	Type result = _type;
	while(auto const* var = std::get_if<GenericTypeVariable>(&result))
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
	return AtomicType{_builtinType, _arguments};
}

void TypeSystem::validate(GenericTypeVariable _variable) const
{
	solAssert(_variable.m_parent == this);
	solAssert(_variable.index() < m_typeVariables.size());
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

experimental::Type TypeSystem::fresh(Type _type)
{
	return std::visit(util::GenericVisitor{
		[&](AtomicType const& _type) -> Type {
			return AtomicType{
				_type.constructor,
				_type.arguments | ranges::view::transform([&](Type _argType) {
					return fresh(_argType);
				}) | ranges::to<vector<Type>>
			};
		},
		[&](FreeTypeVariable const&) {
			return _type;
		},
		[&](GenericTypeVariable const&) {
			return freshTypeVariable();
		},
	}, resolve(_type));
}

tuple<AtomicType::Constructor, vector<experimental::Type>> TypeSystemHelpers::destAtomicType(Type _functionType) const
{
	using ResultType = tuple<AtomicType::Constructor, vector<Type>>;
	return std::visit(util::GenericVisitor{
		[&](AtomicType const& _type) -> ResultType {
			return std::make_tuple(_type.constructor, _type.arguments);
		},
		[](auto) -> ResultType {
			solAssert(false);
		}
	}, _functionType);
}


tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destAtomicType(_functionType);
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	solAssert(builtinType && *builtinType == BuiltinType::Function);
	solAssert(arguments.size() == 2);
	return make_tuple(arguments.front(), arguments.back());
}
