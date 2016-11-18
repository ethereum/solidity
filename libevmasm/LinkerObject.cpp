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
/** @file LinkerObject.cpp
 * @author Christian R <c@ethdev.com>
 * @date 2015
 */

#include <libevmasm/LinkerObject.h>
#include <libdevcore/CommonData.h>

using namespace dev;
using namespace dev::eth;
using namespace std;

void LinkerObject::append(LinkerObject const& _other)
{
	for (auto const& ref: _other.linkReferences)
		linkReferences[ref.first + bytecode.size()] = ref.second;
	bytecode += _other.bytecode;
}

void LinkerObject::link(map<string, h160> const& _libraryAddresses)
{
	std::map<size_t, std::string> remainingRefs;
	for (auto const& linkRef: linkReferences)
	{
		auto it = _libraryAddresses.find(linkRef.second);
		if (it == _libraryAddresses.end())
			remainingRefs.insert(linkRef);
		else
			it->second.ref().copyTo(ref(bytecode).cropped(linkRef.first, 20));
	}
	linkReferences.swap(remainingRefs);
}

string LinkerObject::toHex() const
{
	string hex = dev::toHex(bytecode);
	for (auto const& ref: linkReferences)
	{
		size_t pos = ref.first * 2;
		string const& name = ref.second;
		hex[pos] = hex[pos + 1] = hex[pos + 38] = hex[pos + 39] = '_';
		for (size_t i = 0; i < 36; ++i)
			hex[pos + 2 + i] = i < name.size() ? name[i] : '_';
	}
	return hex;
}
