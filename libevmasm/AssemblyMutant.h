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
 * Contains code for mutant, its description and location.
 */

#pragma once

#include <libevmasm/Assembly.h>
#include <libevmasm/LinkerMutant.h>

namespace dev
{

namespace eth
{

class AssemblyMutant : public Assembly
{
public:
	AssemblyMutant(
			eth::Assembly const& _assembly, 
			std::string const& _description, 
			SourceLocation const& _genLocation) :
		Assembly(_assembly), m_description(_description), m_genLocation(_genLocation) 
	{}

	AssemblyMutant const& subMutant(size_t _sub) const;

	LinkerMutant const& assembleMutant() const;

	std::string const& description() const { return m_description; }
	SourceLocation const& genLocation() const { return m_genLocation; }
private:
	std::string m_description;
	SourceLocation m_genLocation;
};

}
}
