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


#include <libsolidity/experimental/ast/TypeSystemHelper.h>
#include <libsolidity/ast/AST.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>

#include <libsolutil/Visitor.h>

#include <range/v3/to_container.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/reverse.hpp>

#include <fmt/format.h>

using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;

/*std::optional<TypeConstructor> experimental::typeConstructorFromTypeName(Analysis const& _analysis, TypeName const& _typeName)
{
	if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
	{
		if (auto constructor = typeConstructorFromToken(_analysis, elementaryTypeName->typeName().token()))
			return *constructor;
	}
	else if (auto const* userDefinedType = dynamic_cast<UserDefinedTypeName const*>(&_typeName))
	{
		if (auto const* referencedDeclaration = userDefinedType->pathNode().annotation().referencedDeclaration)
			return _analysis.annotation<TypeRegistration>(*referencedDeclaration).typeConstructor;
	}
	return nullopt;
}*/
/*
std::optional<TypeConstructor> experimental::typeConstructorFromToken(Analysis const& _analysis, langutil::Token _token)
{
	TypeSystem const& typeSystem = _analysis.typeSystem();
	switch (_token)
	{
	case Token::Void:
		return typeSystem.builtinConstructor(BuiltinType::Void);
	case Token::Fun:
		return typeSystem.builtinConstructor(BuiltinType::Function);
	case Token::Unit:
		return typeSystem.builtinConstructor(BuiltinType::Unit);
	case Token::Pair:
		return typeSystem.builtinConstructor(BuiltinType::Pair);
	case Token::Word:
		return typeSystem.builtinConstructor(BuiltinType::Word);
	case Token::IntegerType:
		return typeSystem.builtinConstructor(BuiltinType::Integer);
	case Token::Bool:
		return typeSystem.builtinConstructor(BuiltinType::Bool);
	default:
		return nullopt;
	}
}*/

std::optional<BuiltinClass> experimental::builtinClassFromToken(langutil::Token _token)
{
	switch (_token)
	{
	case Token::Integer:
		return BuiltinClass::Integer;
	case Token::Mul:
		return BuiltinClass::Mul;
	case Token::Add:
		return BuiltinClass::Add;
	case Token::Equal:
		return BuiltinClass::Equal;
	case Token::LessThan:
		return BuiltinClass::Less;
	case Token::LessThanOrEqual:
		return BuiltinClass::LessOrEqual;
	case Token::GreaterThan:
		return BuiltinClass::Greater;
	case Token::GreaterThanOrEqual:
		return BuiltinClass::GreaterOrEqual;
	default:
		return std::nullopt;
	}
}
/*
std::optional<TypeClass> experimental::typeClassFromTypeClassName(TypeClassName const& _typeClass)
{
	return std::visit(util::GenericVisitor{
		[&](ASTPointer<IdentifierPath> _path) -> optional<TypeClass> {
			auto classDefinition = dynamic_cast<TypeClassDefinition const*>(_path->annotation().referencedDeclaration);
			if (!classDefinition)
				return nullopt;
			return TypeClass{classDefinition};
		},
		[&](Token _token) -> optional<TypeClass> {
			return typeClassFromToken(_token);
		}
	}, _typeClass.name());
}
*/
experimental::Type TypeSystemHelpers::tupleType(std::vector<Type> _elements) const
{
	if (_elements.empty())
		return typeSystem.type(PrimitiveType::Unit, {});
	if (_elements.size() == 1)
		return _elements.front();
	Type result = _elements.back();
	for (Type type: _elements | ranges::views::reverse | ranges::views::drop_exactly(1))
		result = typeSystem.type(PrimitiveType::Pair, {type, result});
	return result;
}

std::vector<experimental::Type> TypeSystemHelpers::destTupleType(Type _tupleType) const
{
	if (!isTypeConstant(_tupleType))
		return {_tupleType};
	TypeConstructor pairConstructor = typeSystem.constructor(PrimitiveType::Pair);
	auto [constructor, arguments] = destTypeConstant(_tupleType);
	if (constructor == typeSystem.constructor(PrimitiveType::Unit))
		return {};
	if (constructor != pairConstructor)
		return {_tupleType};
	solAssert(arguments.size() == 2);

	std::vector<Type> result;
	result.emplace_back(arguments.front());
	Type tail = arguments.back();
	while (true)
	{
		if (!isTypeConstant(tail))
			break;
		auto [tailConstructor, tailArguments] = destTypeConstant(tail);
		if (tailConstructor != pairConstructor)
			break;
		solAssert(tailArguments.size() == 2);
		result.emplace_back(tailArguments.front());
		tail = tailArguments.back();
	}
	result.emplace_back(tail);
	return result;
}

experimental::Type TypeSystemHelpers::sumType(std::vector<Type> _elements) const
{
	if (_elements.empty())
		return typeSystem.type(PrimitiveType::Void, {});
	if (_elements.size() == 1)
		return _elements.front();
	Type result = _elements.back();
	for (Type type: _elements | ranges::views::reverse | ranges::views::drop_exactly(1))
		result = typeSystem.type(PrimitiveType::Sum, {type, result});
	return result;
}

std::vector<experimental::Type> TypeSystemHelpers::destSumType(Type _tupleType) const
{
	if (!isTypeConstant(_tupleType))
		return {_tupleType};
	TypeConstructor sumConstructor = typeSystem.constructor(PrimitiveType::Sum);
	auto [constructor, arguments] = destTypeConstant(_tupleType);
	if (constructor == typeSystem.constructor(PrimitiveType::Void))
		return {};
	if (constructor != sumConstructor)
		return {_tupleType};
	solAssert(arguments.size() == 2);

	std::vector<Type> result;
	result.emplace_back(arguments.front());
	Type tail = arguments.back();
	while (true)
	{
		if (!isTypeConstant(tail))
			break;
		auto [tailConstructor, tailArguments] = destTypeConstant(tail);
		if (tailConstructor != sumConstructor)
			break;
		solAssert(tailArguments.size() == 2);
		result.emplace_back(tailArguments.front());
		tail = tailArguments.back();
	}
	result.emplace_back(tail);
	return result;
}

std::tuple<TypeConstructor, std::vector<experimental::Type>> TypeSystemHelpers::destTypeConstant(Type _type) const
{
	using ResultType = std::tuple<TypeConstructor, std::vector<Type>>;
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

bool TypeSystemHelpers::isPrimitiveType(Type _type, PrimitiveType _primitiveType) const
{
	if (!isTypeConstant(_type))
		return false;
	auto constructor = std::get<0>(destTypeConstant(_type));
	return constructor == typeSystem.constructor(_primitiveType);
}

experimental::Type TypeSystemHelpers::functionType(experimental::Type _argType, experimental::Type _resultType) const
{
	return typeSystem.type(PrimitiveType::Function, {_argType, _resultType});
}

std::tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destTypeConstant(_functionType);
	solAssert(constructor == typeSystem.constructor(PrimitiveType::Function));
	solAssert(arguments.size() == 2);
	return std::make_tuple(arguments.front(), arguments.back());
}

bool TypeSystemHelpers::isFunctionType(Type _type) const
{
	return isPrimitiveType(_type, PrimitiveType::Function);
}

experimental::Type TypeSystemHelpers::typeFunctionType(experimental::Type _argType, experimental::Type _resultType) const
{
	return typeSystem.type(PrimitiveType::TypeFunction, {_argType, _resultType});
}

std::tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destTypeFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destTypeConstant(_functionType);
	solAssert(constructor == typeSystem.constructor(PrimitiveType::TypeFunction));
	solAssert(arguments.size() == 2);
	return std::make_tuple(arguments.front(), arguments.back());
}

bool TypeSystemHelpers::isTypeFunctionType(Type _type) const
{
	return isPrimitiveType(_type, PrimitiveType::TypeFunction);
}

std::vector<experimental::Type> TypeEnvironmentHelpers::typeVars(Type _type) const
{
	std::set<size_t> indices;
	std::vector<Type> typeVars;
	auto typeVarsImpl = [&](Type _type, auto _recurse) -> void {
		std::visit(util::GenericVisitor{
			[&](TypeConstant const& _type) {
				for (auto arg: _type.arguments)
					_recurse(arg, _recurse);
			},
			[&](TypeVariable const& _var) {
				if (indices.emplace(_var.index()).second)
					typeVars.emplace_back(_var);
			},
			[](std::monostate) { solAssert(false); }
		}, env.resolve(_type));
	};
	typeVarsImpl(_type, typeVarsImpl);
	return typeVars;

}


std::string TypeSystemHelpers::sortToString(Sort _sort) const
{
	switch (_sort.classes.size())
	{
	case 0:
		return "()";
	case 1:
		return typeSystem.typeClassName(*_sort.classes.begin());
	default:
	{
		std::stringstream stream;
		stream << "(";
		for (auto typeClass: _sort.classes | ranges::views::drop_last(1))
			stream << typeSystem.typeClassName(typeClass) << ", ";
		stream << typeSystem.typeClassName(*_sort.classes.rbegin()) << ")";
		return stream.str();
	}
	}
}

std::string TypeEnvironmentHelpers::canonicalTypeName(Type _type) const
{
	return visit(util::GenericVisitor{
		[&](TypeConstant _type) -> std::string {
			std::stringstream stream;
			stream << env.typeSystem().constructorInfo(_type.constructor).canonicalName;
			if (!_type.arguments.empty())
			{
				stream << "$";
				for (auto type: _type.arguments | ranges::views::drop_last(1))
					stream << canonicalTypeName(type) << "$";
				stream << canonicalTypeName(_type.arguments.back());
				stream << "$";
			}
			return stream.str();
		},
		[](TypeVariable) -> std::string {
			solAssert(false);
		},
		[](std::monostate) -> std::string {
			solAssert(false);
		},
	}, env.resolve(_type));
}

std::string TypeEnvironmentHelpers::typeToString(Type const& _type) const
{
	std::map<TypeConstructor, std::function<std::string(std::vector<Type>)>> formatters{
		{env.typeSystem().constructor(PrimitiveType::Function), [&](auto const& _args) {
			solAssert(_args.size() == 2);
			return fmt::format("{} -> {}", typeToString(_args.front()), typeToString(_args.back()));
		}},
		{env.typeSystem().constructor(PrimitiveType::Unit), [&](auto const& _args) {
			solAssert(_args.size() == 0);
			return "()";
		}},
		{env.typeSystem().constructor(PrimitiveType::Pair), [&](auto const& _arguments) {
			auto tupleTypes = TypeSystemHelpers{env.typeSystem()}.destTupleType(_arguments.back());
			std::string result = "(";
			result += typeToString(_arguments.front());
			for (auto type: tupleTypes)
				result += ", " + typeToString(type);
			result += ")";
			return result;
		}},
	};
	return std::visit(util::GenericVisitor{
		[&](TypeConstant const& _type) {
			if (auto* formatter = util::valueOrNullptr(formatters, _type.constructor))
				return (*formatter)(_type.arguments);
			std::stringstream stream;
			stream << env.typeSystem().constructorInfo(_type.constructor).name;
			if (!_type.arguments.empty())
			{
				stream << "(";
				for (auto type: _type.arguments | ranges::views::drop_last(1))
					stream << typeToString(type) << ", ";
				stream << typeToString(_type.arguments.back());
				stream << ")";
			}
			return stream.str();
		},
		[&](TypeVariable const& _type) {
			std::stringstream stream;
			std::string varName;
			size_t index = _type.index();
			varName += static_cast<char>('a' + (index%26));
			while (index /= 26)
				varName += static_cast<char>('a' + (index%26));
			reverse(varName.begin(), varName.end());
			stream << '\'' << varName;
			switch (_type.sort().classes.size())
			{
			case 0:
				break;
			case 1:
				stream << ":" << env.typeSystem().typeClassName(*_type.sort().classes.begin());
				break;
			default:
				stream << ":(";
				for (auto typeClass: _type.sort().classes | ranges::views::drop_last(1))
					stream << env.typeSystem().typeClassName(typeClass) << ", ";
				stream << env.typeSystem().typeClassName(*_type.sort().classes.rbegin());
				stream << ")";
				break;
			}
			return stream.str();
		},
		[](std::monostate) -> std::string { solAssert(false); }
	}, env.resolve(_type));
}
