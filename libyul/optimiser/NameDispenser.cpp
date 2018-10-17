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

#include <libyul/optimiser/NameCollector.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

NameDispenser::NameDispenser(Block const& _ast):
	NameDispenser(NameCollector(_ast).names())
{
}

NameDispenser::NameDispenser(set<string> _usedNames):
	m_usedNames(std::move(_usedNames))
{
}

string NameDispenser::newName(string const& _nameHint, string const& _context)
{
	// Shortening rules: Use a suffix of _prefix and a prefix of _context.
	string prefix = _nameHint;

	if (!_context.empty())
		prefix = _context.substr(0, 10) + "_" + prefix;

	return newNameInternal(prefix);
}

string NameDispenser::newNameInternal(string const& _nameHint)
{
	size_t suffix = 0;
	string name = _nameHint;
	while (name.empty() || m_usedNames.count(name))
	{
		suffix++;
		name = _nameHint + "_" + to_string(suffix);
	}
	m_usedNames.insert(name);
	return name;
}
