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
/** @file UTF8.cpp
 * @author Alex Beregszaszi
 * @date 2016
 *
 * UTF-8 related helpers
 */

#include "UTF8.h"


namespace dev
{

namespace utf8
{


bool validate(std::string input, int &invalidPosition)
{
	const int length = input.length();
	bool valid = true;
	int i = 0;

	for (; i < length; i++)
	{
		if ((unsigned char)input[i] < 0x80)
			continue;

		int count = 0;
		switch(input[i] & 0xe0) {
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

		for (int j = 0; j < count; j++)
		{
			i++;
			if ((input[i] & 0xc0) != 0x80)
			{
				valid = false;
				break;
			}
		}
	}

	if (valid)
		return true;

	invalidPosition = i;
	return false;
}


}

}
