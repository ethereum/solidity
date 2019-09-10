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

#include <libdevcore/UTF8.h>

namespace dev
{
namespace
{

/// Validate byte sequence against Unicode chapter 3 Table 3-7.
bool isWellFormed(unsigned char byte1, unsigned char byte2)
{
	if (byte1 == 0xc0 || byte1 == 0xc1)
		return false;
	else if (byte1 >= 0xc2 && byte1 <= 0xdf)
		return true;
	else if (byte1 == 0xe0)
	{
		if (byte2 < 0xa0)
			return false;
		else
			return true;
	}
	else if (byte1 >= 0xe1 && byte1 <= 0xec)
		return true;
	else if (byte1 == 0xed)
	{
		if (byte2 > 0x9f)
			return false;
		else
			return true;
	}
	else if (byte1 == 0xee || byte1 == 0xef)
		return true;
	else if (byte1 == 0xf0)
	{
		if (byte2 < 0x90)
			return false;
		else
			return true;
	}
	else if (byte1 >= 0xf1 && byte1 <= 0xf3)
		return true;
	else if (byte1 == 0xf4)
	{
		if (byte2 > 0x8f)
			return false;
		else
			return true;
	}
	/// 0xf5 .. 0xf7 is disallowed
	/// Technically anything below 0xc0 or above 0xf7 is
	/// not possible to encode using Table 3-6 anyway.
	return false;
}

}

bool validateUTF8(unsigned char const* _input, size_t _length, size_t& _invalidPosition)
{
	bool valid = true;
	size_t i = 0;

	for (; i < _length; i++)
	{
		// Check for Unicode Chapter 3 Table 3-6 conformity.
		if (_input[i] < 0x80)
			continue;

		size_t count = 0;
		if (_input[i] >= 0xc0 && _input[i] <= 0xdf)
			count = 1;
		else if (_input[i] >= 0xe0 && _input[i] <= 0xef)
			count = 2;
		else if (_input[i] >= 0xf0 && _input[i] <= 0xf7)
			count = 3;

		if (count == 0)
		{
			valid = false;
			break;
		}

		if ((i + count) >= _length)
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

bool validateUTF8(std::string const& _input, size_t& _invalidPosition)
{
	return validateUTF8(reinterpret_cast<unsigned char const*>(_input.c_str()), _input.length(), _invalidPosition);
}

}
