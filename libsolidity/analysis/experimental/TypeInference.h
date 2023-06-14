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

#include <libsolidity/ast/ASTVisitor.h>

#include <liblangutil/ErrorReporter.h>

#include <range/v3/span.hpp>

namespace solidity::frontend::experimental
{

class GlobalTypeContext;

struct SumType;
struct TupleType;
struct FunctionType;
struct WordType;
struct UserDefinedType;
struct TypeVariable;
struct FreeType;

using Type = std::variant<SumType, TupleType, FunctionType, WordType, UserDefinedType, TypeVariable, FreeType>;

struct SumType
{
	std::vector<Type const*> alternatives;
};

struct TupleType
{
	std::vector<Type const*> components;
};

struct FunctionType
{
	Type const* codomain = nullptr;
	Type const* domain = nullptr;
};

struct WordType
{
};

struct UserDefinedType
{
	Declaration const* declaration = nullptr;
	std::vector<Type const*> arguments;
};

struct TypeVariable
{
	uint64_t index = 0;
};

struct FreeType
{
	uint64_t index = 0;
};

Type unify(Type _a, Type _b)
{
	
}

class TypeEnvironment
{
public:
	TypeEnvironment() {}
	void assignType(Declaration const* _declaration, Type _typeAssignment)
	{
		m_types.emplace(std::piecewise_construct, std::forward_as_tuple(_declaration), std::forward_as_tuple(std::move(_typeAssignment)));
	}
private:
	uint64_t m_numTypeVariables = 0;
	std::map<Declaration const*, Type> m_types;
};

class TypeInference: public ASTConstVisitor
{
public:
	TypeInference(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}
private:
	bool visit(Block const&) override { return true; }
	bool visit(VariableDeclarationStatement const&) override { return true; }
	bool visit(VariableDeclaration const& _variableDeclaration) override;

	bool visitNode(ASTNode const& _node) override;

private:
	langutil::ErrorReporter& m_errorReporter;
	TypeEnvironment m_env;
};

}