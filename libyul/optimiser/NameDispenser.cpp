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

    // If the name itself has a suffix supplied by user, get rid of it
    // so that we have no conflicts and the order of suffix is in order.
    std::string nameHintStr = _nameHint.str();
    size_t underscore = nameHintStr.find_last_of("_");
    if (underscore != std::string::npos && underscore != 0)
    {
        try
        {
            std::stoi(nameHintStr.substr(underscore + 1));
            prefix = YulString(nameHintStr.substr(0, underscore));
        } catch (std::invalid_argument &e) {
            // dont do any conversion
        }
    }

	if (!_context.empty())
		prefix = YulString{_context.str().substr(0, 10) + "_" + prefix.str()};

	return newNameInternal(prefix);
}

YulString NameDispenser::newNameInternal(YulString _nameHint)
{
	YulString name = _nameHint;

    if (m_usedNames.count(name) && !m_counters.count(name.id()))
       m_counters.emplace(name.id(), 0); 

    if (m_counters.count(name.id()))
        name = YulString(_nameHint.prefix(), ++m_counters.at(name.id()));
    else
        m_counters.emplace(name.id(), 0);
    m_usedNames.emplace(name);
    return name;
}
