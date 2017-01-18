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
/** @file Assembly.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

namespace dev
{
namespace eth
{

/**
 * Binary object that potentially still needs to be linked (i.e. addresses of other contracts
 * need to be filled in).
 */
struct LinkerObject
{
	bytes bytecode;
	/// Map from offsets in bytecode to library identifiers. The addresses starting at those offsets
	/// need to be replaced by the actual addresses by the linker.
	std::map<size_t, std::string> linkReferences;

	/// Appends the bytecode of @a _other and incorporates its link references.
	void append(LinkerObject const& _other);

	/// Links the given libraries by replacing their uses in the code and removes them from the references.
	void link(std::map<std::string, h160> const& _libraryAddresses);

	/// @returns a hex representation of the bytecode of the given object, replacing unlinked
	/// addresses by placeholders.
	std::string toHex() const;

private:
	static h160 const* matchLibrary(
		std::string const& _linkRefName,
		std::map<std::string, h160> const& _libraryAddresses
	);
};

}
}
