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
/**
 * Optimiser component that can create new unique names.
 */

#include <libyul/optimiser/NameDispenser.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

string NameDispenser::newName(string const& _prefix)
{
	string name = _prefix;
	size_t suffix = 0;
	while (name.empty() || m_usedNames.count(name))
	{
		suffix++;
		name = _prefix + "_" + to_string(suffix);
	}
	m_usedNames.insert(name);
	return name;
}
