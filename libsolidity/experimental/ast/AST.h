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
#include <liblangutil/SourceLocation.h>
#include <libyul/ASTForward.h>

#include <string>
#include <optional>
#include <variant>

namespace solidity::frontend::experimental
{

struct TermBase
{
	langutil::SourceLocation location;
	std::optional<int64_t> legacyId;
	Type type;
};


struct Application;
struct Lambda;
struct InlineAssembly;
struct Reference: TermBase
{
	size_t index = std::numeric_limits<size_t>::max();
	std::string name;
};

enum class BuiltinConstant
{
	// TODO: sort
	Unit,
	Pair,
	Fun,
	Constrain,
	NamedTerm,
	TypeDeclaration,
	TypeDefinition,
	TypeClassDefinition,
	TypeClassInstantiation,
	FunctionDeclaration,
	FunctionDefinition,
	ContractDefinition,
	VariableDeclaration,
	VariableDefinition,
	Block,
	ReturnStatement,
	RegularStatement,
	ChainStatements,
	Assign,
	MemberAccess,
	Mul,
	Add,
	Void,
	Word,
	Integer,
	Bool,
	Undefined,
	Equal
};

struct Constant: TermBase
{
	std::variant<std::string, BuiltinConstant> name;
};

using Term = std::variant<Application, Lambda, InlineAssembly, Reference, Constant>;

struct InlineAssembly: TermBase
{
	yul::Block const& block;
	std::map<yul::Identifier const*, std::unique_ptr<Term>> references;
};

struct Application: TermBase
{
	std::unique_ptr<Term> expression;
	std::unique_ptr<Term> argument;
};


struct Lambda: TermBase
{
	std::unique_ptr<Term> argument;
	std::unique_ptr<Term> value;
};

template<typename T, typename = void>
struct LocationGetter
{
	langutil::SourceLocation operator()(T const&) const { return {}; }
};
template<typename T>
struct LocationGetter<T, std::enable_if_t<std::is_base_of_v<TermBase, T>>>
{
	langutil::SourceLocation operator()(T const& _t) const
	{
		return _t.location;
	}
};
template<>
struct LocationGetter<Term>
{
	langutil::SourceLocation operator()(Term const& _term) const
	{
		return std::visit([](auto const& _expression) -> langutil::SourceLocation {
			return LocationGetter<std::remove_const_t<std::remove_reference_t<decltype(_expression)>>>{}(_expression);
		}, _term);
	}
};

inline TermBase& termBase(Term& _term)
{
	return std::visit([](auto& _term) -> TermBase& { return static_cast<TermBase&>(_term); }, _term);
}
inline TermBase const& termBase(Term const& _term)
{
	return std::visit([](auto const& _term) -> TermBase const& { return static_cast<TermBase const&>(_term); }, _term);
}


template<typename T>
langutil::SourceLocation locationOf(T const& _t)
{
	return LocationGetter<T>{}(_t);
}


struct AST
{
	std::map<frontend::TypeDefinition const*, std::unique_ptr<Term>, ASTCompareByID<frontend::TypeDefinition>> typeDefinitions;
	std::map<frontend::TypeClassDefinition const*, std::unique_ptr<Term>, ASTCompareByID<frontend::TypeClassDefinition>> typeClasses;
	std::map<frontend::TypeClassInstantiation const*, std::unique_ptr<Term>, ASTCompareByID<frontend::TypeClassInstantiation>> typeClassInstantiations;
	std::map<frontend::FunctionDefinition const*, std::unique_ptr<Term>, ASTCompareByID<frontend::FunctionDefinition>> functions;
	std::map<frontend::ContractDefinition const*, std::unique_ptr<Term>, ASTCompareByID<frontend::ContractDefinition>> contracts;
	std::vector<frontend::ASTNode const*> nodeById;
};

}
