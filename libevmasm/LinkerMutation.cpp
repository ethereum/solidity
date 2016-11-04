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
/** @file LinkerMutation.cpp
 * @author Danil Nemirovsky <danil.nemirovsky@gmail.com>
 * @date 2016
 */

#include <libevmasm/LinkerMutation.h>

using namespace dev;
using namespace dev::eth;
using namespace std;

void LinkerMutation::link(map<string, h160> const& _libraryAddresses)
{
    m_ordinary.link(_libraryAddresses);
}

void LinkerMutation::ordinary(eth::LinkerObject const& _object)
{
    m_ordinary = _object;
}

eth::LinkerObject const& LinkerMutation::ordinary() const
{
	return m_ordinary;
}
