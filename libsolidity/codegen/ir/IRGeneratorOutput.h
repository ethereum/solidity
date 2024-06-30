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

#include <libsolidity/ast/AST.h>

#include <libyul/Object.h>

#include <functional>
#include <memory>
#include <set>
#include <string>

namespace solidity::frontend
{

/// Container for storing Yul source code produced by the IR-based code generator.
/// Allows for easy manipulation of the Yul object structure without the need to reparse the source.
/// Carries information about dependencies but does not include their sources verbatim - they need
/// to be provided separately to be able to print the complete object.
struct IRGeneratorOutput
{
	using DependencyResolver = std::function<IRGeneratorOutput const&(ContractDefinition const*)>;

	struct Creation {
		std::string name;
		std::string code;
		std::shared_ptr<yul::ObjectDebugData const> debugData;
		std::set<ContractDefinition const*, ASTNode::CompareByID> dependencies;
	} creation;

	struct Deployed {
		std::string name;
		std::string code;
		std::shared_ptr<yul::ObjectDebugData const> debugData;
		std::set<ContractDefinition const*, ASTNode::CompareByID> dependencies;
		std::shared_ptr<yul::Data> metadata;

		// TMP: docstring
		std::set<std::string> qualifiedDataNames(DependencyResolver const& _dependencyResolver) const;
	} deployed;

	bool isValid() const;

	/// Combines all the components into IR source code. The result represents a complete Yul object
	/// hierarchy and can be parsed by @a yul::ObjectParser.
	/// @return Complete IR source, which can be considered the canonical text output of the generator.
	/// @param _dependencyResolver A function that can provide access to @a IRGeneratorOutput
	///     corresponding to every dependency contract in the hierarchy, including those found
	///     (recursively) in the @a IRGeneratorOutput instances returned by the function.
	std::string toPrettyString(DependencyResolver const& _dependencyResolver) const;

	// TMP: docstring
	/// @returns the set of names of data objects accessible from within the code of
	/// this object, including the name of object itself
	/// Handles all names containing dots as reserved identifiers, not accessible as data.
	std::set<std::string> qualifiedDataNames(DependencyResolver const& _dependencyResolver) const;

private:
	/// Implementation detail of @a toPrettyString(). The only difference is that the output is not
	/// properly indented.
	std::string toString(DependencyResolver const& _dependencyResolver) const;
};

}
