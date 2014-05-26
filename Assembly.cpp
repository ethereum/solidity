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
/** @file Assembly.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Assembly.h"

#include <libethcore/CommonEth.h>

using namespace std;
using namespace eth;

void Assembly::append(Assembly const& _a)
{
	for (AssemblyItem i: _a.m_items)
	{
		if (i.type() == Tag || i.type() == PushTag)
			i.m_data += m_usedTags;
		m_items.push_back(i);
	}
	for (auto const& i: _a.m_data)
		m_data.insert(i);
}

ostream& Assembly::streamOut(ostream& _out) const
{
	for (auto const& i: m_items)
	{
	}
	return _out;
}

bytes Assembly::assemble() const
{
	bytes ret;
	return ret;
}
