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
#include <libsolidity/ast/AST.h>
#include <libsolutil/Visitor.h>

#include <range/v3/to_container.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/reverse.hpp>

#include <fmt/format.h>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;

std::optional<TypeConstructor> experimental::typeConstructorFromTypeName(TypeName const& _typeName)
{
	if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
	{
		if (auto constructor = typeConstructorFromToken(elementaryTypeName->typeName().token()))
			return *constructor;
	}
	else if (auto const* userDefinedType = dynamic_cast<UserDefinedTypeName const*>(&_typeName))
	{
		if (auto const* referencedDeclaration = userDefinedType->pathNode().annotation().referencedDeclaration)
			return referencedDeclaration;
	}
	return nullopt;
}

std::optional<TypeConstructor> experimental::typeConstructorFromToken(langutil::Token _token)
{
	switch(_token)
	{
	case Token::Void:
		return BuiltinType::Void;
	case Token::Fun:
		return BuiltinType::Function;
	case Token::Unit:
		return BuiltinType::Unit;
	case Token::Pair:
		return BuiltinType::Pair;
	case Token::Word:
		return BuiltinType::Word;
	case Token::Integer:
		return BuiltinType::Integer;
	case Token::Bool:
		return BuiltinType::Bool;
	default:
		return nullopt;
	}
}

std::optional<TypeClass> experimental::typeClassFromToken(langutil::Token _token)
{
	switch (_token)
	{
	case Token::Integer:
		return TypeClass{BuiltinClass::Integer};
	case Token::Mul:
		return TypeClass{BuiltinClass::Mul};
	case Token::Add:
		return TypeClass{BuiltinClass::Add};
	case Token::Equal:
		return TypeClass{BuiltinClass::Equal};
	case Token::LessThan:
		return TypeClass{BuiltinClass::Less};
	case Token::LessThanOrEqual:
		return TypeClass{BuiltinClass::LessOrEqual};
	case Token::GreaterThan:
		return TypeClass{BuiltinClass::Greater};
	case Token::GreaterThanOrEqual:
		return TypeClass{BuiltinClass::GreaterOrEqual};
	default:
		return nullopt;
	}
}

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

experimental::Type TypeSystemHelpers::functionType(experimental::Type _argType, experimental::Type _resultType) const
{
	return typeSystem.type(BuiltinType::Function, {_argType, _resultType});
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

experimental::Type TypeSystemHelpers::typeFunctionType(experimental::Type _argType, experimental::Type _resultType) const
{
	return typeSystem.type(BuiltinType::TypeFunction, {_argType, _resultType});
}

tuple<experimental::Type, experimental::Type> TypeSystemHelpers::destTypeFunctionType(Type _functionType) const
{
	auto [constructor, arguments] = destTypeConstant(_functionType);
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	solAssert(builtinType && *builtinType == BuiltinType::TypeFunction);
	solAssert(arguments.size() == 2);
	return make_tuple(arguments.front(), arguments.back());
}

bool TypeSystemHelpers::isTypeFunctionType(Type _type) const
{
	if (!isTypeConstant(_type))
		return false;
	auto constructor = get<0>(destTypeConstant(_type));
	auto const* builtinType = get_if<BuiltinType>(&constructor);
	return builtinType && *builtinType == BuiltinType::TypeFunction;
}

vector<experimental::Type> TypeEnvironmentHelpers::typeVars(Type _type) const
{
	set<size_t> indices;
	vector<Type> typeVars;
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
// TODO: move to env helpers?
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

std::string TypeEnvironmentHelpers::canonicalTypeName(Type _type) const
{
	return visit(util::GenericVisitor{
		[&](TypeConstant const& _type) {
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
					if (auto const* typeDeclarationAnnotation = dynamic_cast<TypeDeclarationAnnotation const*>(&_declaration->annotation()))
						stream << *typeDeclarationAnnotation->canonicalName;
					else
					// TODO: canonical name
						stream << _declaration->name();
				},
				[&](BuiltinType _builtinType) {
					printTypeArguments();
					switch(_builtinType)
					{
					case BuiltinType::Type:
						stream << "type";
						break;
					case BuiltinType::Sort:
						stream << "sort";
						break;
					case BuiltinType::Void:
						stream << "void";
						break;
					case BuiltinType::Function:
						stream << "fun";
						break;
					case BuiltinType::TypeFunction:
						stream << "tfun";
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
					case BuiltinType::Bool:
						stream << "bool";
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
	}, env.resolve(_type));
}

std::string TypeEnvironmentHelpers::typeToString(Type const& _type) const
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
					stream << env.typeSystem().typeName(_declaration);
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
						auto tupleTypes = TypeSystemHelpers{env.typeSystem()}.destTupleType(_type);
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
						stream << env.typeSystem().typeName(_builtinType);
						printTypeArguments();
						break;
					}
				}
			}, _type.constructor);
			return stream.str();
		},
		[](TypeVariable const& _type) {
			std::stringstream stream;
			stream << "'var" << _type.index();
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
	}, env.resolve(_type));
}
