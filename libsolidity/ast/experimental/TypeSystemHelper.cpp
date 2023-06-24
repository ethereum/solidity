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


#include <libsolidity/ast/experimental/TypeSystemHelper.h>

#include <libsolutil/Visitor.h>

#include <range/v3/to_container.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/reverse.hpp>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;

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