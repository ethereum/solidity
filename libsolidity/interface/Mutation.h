/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Danil Nemirovsky <danil.nemirovsky@gmail.com>
 * @date 2016
 * Contains bytecode for contract and its mutants.
 */

#pragma once

#include <libevmasm/LinkerObject.h>

namespace dev
{

namespace solidity
{

class Mutation
{
public:
    /// @returns bytecode of ordinary contract
    bytes bytecodeOrdinary() const;
    /// @returns a hex representation of the bytecode of the given object, replacing unlinked
	/// addresses by placeholders.
	std::string hexOrdinary() const;
	/// Links the given libraries by replacing their uses in the code and removes them from the references.
	void link(std::map<std::string, h160> const& _libraryAddresses);
    /// @returns Map from offsets in bytecode to library identifiers. The addresses starting at those offsets
	/// need to be replaced by the actual addresses by the linker.
	std::map<size_t, std::string> linkReferencesOrdinary() const;
    /// set ordinary contract
    void setOrdinary(eth::LinkerObject _object);
private:
    eth::LinkerObject m_ordinary;
    std::map<std::string const, eth::LinkerObject> m_mutants;
};

}
}
