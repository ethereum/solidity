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
/** @file AssemblyMutant.cpp
 * @author Danil Nemirovsky <danil.nemirovsky@gmail.com>
 * @date 2016
 */

#include <libevmasm/AssemblyMutant.h>

using namespace dev;
using namespace dev::eth;
using namespace std;

AssemblyMutant const& AssemblyMutant::subMutant(size_t _sub) const
{
	return * new AssemblyMutant(sub(_sub),m_description, m_genLocation);
}

LinkerMutant const& AssemblyMutant::assembleMutant() const
{
	return * new LinkerMutant(assemble(), m_description, m_genLocation);
}
