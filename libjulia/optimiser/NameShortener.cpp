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
 * Optimiser component that makes all identifiers unique.
 */

#include <libjulia/optimiser/NameShortener.h>

#include <libjulia/optimiser/NameCollector.h>

#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmScope.h>

#include <libsolidity/interface/Exceptions.h>

using namespace std;
using namespace dev;
using namespace dev::julia;

NameShortener::NameShortener(Block const& _ast, size_t _maxSize)
{
	set<string> names = NameCollector(_ast).names();
	for (auto const& name: names)
	{
		if (name.size() <= _maxSize)
			m_translations[name] = name;
		else
		{
			string prefix = name.substr(0, _maxSize);
			string replacement = prefix;
			size_t suffix = 0;
			while (replacement.empty() || names.count(replacement))
			{
				suffix++;
				replacement = prefix + "_" + std::to_string(suffix);
			}
			cout << "Translating " << name << " to " << replacement << endl;
			m_translations[name] = replacement;
			names.insert(replacement);
		}
	}
}

string NameShortener::translateIdentifier(string const& _name)
{
	return m_translations.at(_name);
}
