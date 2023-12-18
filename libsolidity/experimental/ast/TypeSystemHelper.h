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

#include <libsolidity/experimental/ast/TypeSystem.h>
#include <libsolidity/ast/ASTForward.h>
#include <liblangutil/Token.h>

namespace solidity::frontend::experimental
{
class Analysis;
enum class BuiltinClass;
//std::optional<TypeConstructor> typeConstructorFromTypeName(Analysis const& _analysis, TypeName const& _typeName);
//std::optional<TypeConstructor> typeConstructorFromToken(Analysis const& _analysis, langutil::Token _token);
//std::optional<TypeClass> typeClassFromTypeClassName(TypeClassName const& _typeClass);
std::optional<BuiltinClass> builtinClassFromToken(langutil::Token _token);

struct TypeSystemHelpers
{
	TypeSystem const& typeSystem;
	std::tuple<TypeConstructor, std::vector<Type>> destTypeConstant(Type _type) const;
	bool isTypeConstant(Type _type) const;
	bool isPrimitiveType(Type _type, PrimitiveType _primitiveType) const;
	Type tupleType(std::vector<Type> _elements) const;
	std::vector<Type> destTupleType(Type _tupleType) const;
	Type sumType(std::vector<Type> _elements) const;
	std::vector<Type> destSumType(Type _tupleType) const;
	Type functionType(Type _argType, Type _resultType) const;
	std::tuple<Type, Type> destFunctionType(Type _functionType) const;
	bool isFunctionType(Type _type) const;
	Type typeFunctionType(Type _argType, Type _resultType) const;
	std::tuple<Type, Type> destTypeFunctionType(Type _functionType) const;
	bool isTypeFunctionType(Type _type) const;
	std::string sortToString(Sort _sort) const;
};

struct TypeEnvironmentHelpers
{
	TypeEnvironment const& env;
	std::string typeToString(Type const& _type) const;
	std::string canonicalTypeName(Type _type) const;
	std::vector<Type> typeVars(Type _type) const;
};

}
