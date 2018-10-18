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
#include <libdevcore/Keccak256.h>

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
		if (h160 const* address = matchLibrary(linkRef.second, _libraryAddresses))
			copy(address->data(), address->data() + 20, bytecode.begin() + linkRef.first);
		else
			remainingRefs.insert(linkRef);
	linkReferences.swap(remainingRefs);
}

string LinkerObject::toHex() const
{
	string hex = dev::toHex(bytecode);
	for (auto const& ref: linkReferences)
	{
		size_t pos = ref.first * 2;
		string hash = libraryPlaceholder(ref.second);
		hex[pos] = hex[pos + 1] = hex[pos + 38] = hex[pos + 39] = '_';
		for (size_t i = 0; i < 36; ++i)
			hex[pos + 2 + i] = hash.at(i);
	}
	return hex;
}

string LinkerObject::libraryPlaceholder(string const& _libraryName)
{
	return "$" + keccak256(_libraryName).hex().substr(0, 34) + "$";
}

h160 const*
LinkerObject::matchLibrary(
	string const& _linkRefName,
	map<string, h160> const& _libraryAddresses
)
{
	auto it = _libraryAddresses.find(_linkRefName);
	if (it != _libraryAddresses.end())
		return &it->second;
	// If the user did not supply a fully qualified library name,
	// try to match only the simple library name
	size_t colon = _linkRefName.find(':');
	if (colon == string::npos)
		return nullptr;
	it = _libraryAddresses.find(_linkRefName.substr(colon + 1));
	if (it != _libraryAddresses.end())
		return &it->second;
	return nullptr;
}
