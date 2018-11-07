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

NameDispenser::NameDispenser(set<YulString> _usedNames):
	m_usedNames(std::move(_usedNames))
{
}

YulString NameDispenser::newName(YulString _nameHint, YulString _context)
{
	// Shortening rules: Use a suffix of _prefix and a prefix of _context.
	YulString prefix = _nameHint;

	if (!_context.empty())
		prefix = YulString{_context.str().substr(0, 10) + "_" + prefix.str()};

	return newNameInternal(prefix);
}

YulString NameDispenser::newNameInternal(YulString _nameHint)
{
	YulString name = _nameHint;
	while (name.empty() || m_usedNames.count(name))
	{
		m_counter++;
		name = YulString(_nameHint.str() + "_" + to_string(m_counter));
	}
	m_usedNames.emplace(name);
	return name;
}
