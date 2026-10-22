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
/**
 * The type documents of ethdebug/format/info/resources, derived from the
 * types of the frontend.
 */

#pragma once

#include <libsolidity/ast/ASTForward.h>

#include <libevmasm/EthdebugSchema.h>

#include <liblangutil/SourceLocation.h>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace solidity::frontend
{
class Type;
}

namespace solidity::frontend::ethdebug
{

/// Type documents under construction, together with the compilation's source
/// indices: definition locations refer to the same source IDs the ethdebug
/// compilation record uses.
class TypeRegistry
{
public:
	explicit TypeRegistry(std::map<std::string, unsigned> const& _sourceIndices);

	/// Registers the document of @a _type and of every type it composes, keyed
	/// by the compiler's type identifier. @a _type must not be unnameable.
	///
	/// A type document does not depend on where a value is stored, so the
	/// located variants of a reference type share one document, keyed by the
	/// identifier of the type as it is laid out in storage.
	void registerType(Type const& _type);

	/// Registers the types of the parameters and return variables of @a _callable.
	void registerCallableTypes(CallableDeclaration const& _callable);

	/// The type @a _type is documented as: reference types in storage, an
	/// array slice as the array it views.
	static Type const& canonical(Type const& _type);

	std::map<std::string, evmasm::ethdebug::schema::Type> takeDocuments() { return std::move(m_documents); }

	/// The source range of @a _location in the sources of the compilation, if
	/// it is in one of them.
	std::optional<evmasm::ethdebug::schema::materials::SourceRange> sourceRange(langutil::SourceLocation const& _location) const;

private:
	evmasm::ethdebug::schema::Type document(Type const& _type);
	evmasm::ethdebug::schema::Type::Wrapper wrapper(std::optional<std::string> _name, Type const& _type);
	std::vector<evmasm::ethdebug::schema::Type::Wrapper> wrappers(std::vector<Type const*> const& _types);
	evmasm::ethdebug::schema::Type::Definition definition(Declaration const& _declaration) const;

	std::map<std::string, unsigned> const& m_sourceIndices;
	std::map<std::string, evmasm::ethdebug::schema::Type> m_documents;
};

}
