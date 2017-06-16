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
/** @file UTF8.cpp
 * @author Alex Beregszaszi
 * @date 2016
 *
 * UTF-8 related helpers
 */

#include "UTF8.h"


namespace dev
{

namespace
{

/// Validate byte sequence against Unicode chapter 3 Table 3-7.
bool isWellFormed(unsigned char byte1, unsigned char byte2)
{
	switch (byte1)
	{
	case 0xc0 ... 0xc1:
		return false;
	case 0xc2 ... 0xdf:
		break;
	case 0xe0:
		if (byte2 < 0xa0)
			return false;
		break;
	case 0xe1 ... 0xec:
		break;
	case 0xed:
		if (byte2 > 0x9f)
			return false;
		break;
	case 0xee ... 0xef:
		break;
	case 0xf0:
		if (byte2 < 0x90)
			return false;
		break;
	case 0xf1 ... 0xf3:
		break;
	case 0xf4:
		if (byte2 > 0x8f)
			return false;
		break;
	case 0xf5 ... 0xf7:
	default:
		/// Technically anything below 0xc0 or above 0xf7 is
		/// not possible to encode using Table 3-6 anyway.
		return false;
	}
	return true;
}

}

bool validateUTF8(std::string const& _input, size_t& _invalidPosition)
{
	const size_t length = _input.length();
	bool valid = true;
	size_t i = 0;

	for (; i < length; i++)
	{
		// Check for Unicode Chapter 3 Table 3-6 conformity.
		if ((unsigned char)_input[i] < 0x80)
			continue;

		size_t count = 0;
		switch(_input[i] & 0xf0) {
			case 0xc0: count = 1; break;
			case 0xe0: count = 2; break;
			case 0xf0: count = 3; break;
			default: break;
		}

		if (count == 0)
		{
			valid = false;
			break;
		}

		if ((i + count) >= length)
		{
			valid = false;
			break;
		}

		for (size_t j = 0; j < count; j++)
		{
			i++;
			if ((_input[i] & 0xc0) != 0x80)
			{
				valid = false;
				break;
			}

			// Check for Unicode Chapter 3 Table 3-7 conformity.
			if ((j == 0) && !isWellFormed(_input[i - 1], _input[i]))
			{
				valid = false;
				break;
			}
		}
	}

	if (valid)
		return true;

	_invalidPosition = i;
	return false;
}

}
