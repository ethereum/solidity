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

#include <libsolidity/ast/experimental/TypeSystem.h>
#include <libsolidity/ast/ASTForward.h>
#include <liblangutil/Token.h>

namespace solidity::frontend::experimental
{

std::optional<TypeConstructor> typeConstructorFromTypeName(TypeName const& _typeName);
std::optional<TypeConstructor> typeConstructorFromToken(langutil::Token _token);
std::optional<TypeClass> typeClassFromTypeClassName(TypeClassName const& _typeClass);
std::optional<TypeClass> typeClassFromToken(langutil::Token _token);

struct TypeSystemHelpers
{
	TypeSystem const& typeSystem;
	std::tuple<TypeConstructor, std::vector<Type>> destTypeConstant(Type _type) const;
	bool isTypeConstant(Type _type) const;
	Type tupleType(std::vector<Type> _elements) const;
	std::vector<Type> destTupleType(Type _tupleType) const;
	Type functionType(Type _argType, Type _resultType) const;
	std::tuple<Type, Type> destFunctionType(Type _functionType) const;
	bool isFunctionType(Type _type) const;
	std::vector<Type> typeVars(Type _type) const;
	std::string sortToString(Sort _sort) const;
	Type kindType(Type _type) const;
	bool isKindType(Type _type) const;
	Type destKindType(Type _type) const;
};

}
