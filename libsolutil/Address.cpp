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
// SPDX-License-Identifier: GPL-3.0

#include <libsolutil/Address.h>
#include <libsolutil/Exceptions.h>

using namespace std;

namespace solidity::util
{

/// @returns a h160 representation of an address string
/// or an error message in case the address string is not a valid address
Result<h160> validateAddress(string _addrString)
{
	string message;
	h160 address;

	if (_addrString.substr(0, 2) != "0x")
		message = "Library address \"" + _addrString + "\" is not prefixed with \"0x\".\n"
			"Note that the address must be prefixed with \"0x\".";
	else
	{
		string addrString = _addrString.substr(2);

		if (addrString.length() != 40)
			message =
				"Invalid library address length " +
				to_string(addrString.length()) +
				" instead of 40 characters.";
		else
		{
			try
			{
				address = util::h160(_addrString);
			}
			catch (util::BadHexCharacter const&)
			{
				message = "Invalid library address (\"" + _addrString + "\") supplied.";
			}
		}

		if (message.empty())
		{
			if (!passesAddressChecksum(addrString, false))
				message =
					"Invalid checksum for library address \"" + _addrString + "\".\n"
					"The correct checksum is \"" + getChecksummedAddress(addrString) + "\".";
		}
	}

	if (!message.empty())
		return Result<h160>::err(message);

	return address;
}

}
