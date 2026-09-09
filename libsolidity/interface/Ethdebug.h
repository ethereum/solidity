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
 * The ethdebug resources of a contract: the type documents and pointer
 * templates of ethdebug/format/info/resources, derived from analysis results.
 */

#pragma once

#include <libevmasm/EthdebugSchema.h>

#include <map>
#include <string>

namespace solidity::frontend
{
class ContractDefinition;
}

namespace solidity::frontend::ethdebug
{

/// The two resource tables of ethdebug/format/info/resources.
struct Resources
{
	/// Type documents keyed by the compiler's type identifier. Composed types
	/// reference their component types by that identifier.
	std::map<std::string, evmasm::ethdebug::schema::type::Type> types;
	/// The pointer of every state variable as a template, keyed by a name
	/// derived from the AST IDs of the contract and the variable. The
	/// templates of mappings expect their keys as parameters.
	std::map<std::string, evmasm::ethdebug::schema::pointer::Template> pointers;

	/// Adds the tables of @a _other, replacing entries with the same key.
	void merge(Resources _other);
};

/// The resources of @a _contract: the types of its state variables and of the
/// parameters and return variables of every function and modifier compiled
/// into it, and the storage and transient storage pointers of its state
/// variables. @a _sourceIndices maps source unit names to the source IDs of
/// the ethdebug compilation record, which definition locations refer to.
Resources resources(ContractDefinition const& _contract, std::map<std::string, unsigned> const& _sourceIndices);

}
