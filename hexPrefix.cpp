/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Foobar is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Main test functions.
 */

#include "TrieCommon.h"
using namespace std;
using namespace eth;

int hexPrefixTest()
{
	/*
	 * Hex-prefix Notation. First nibble has flags: oddness = 2^0 & termination = 2^1
	 * [0,0,1,2,3,4,5]   0x10012345
	 * [0,1,2,3,4,5]     0x00012345
	 * [1,2,3,4,5]       0x112345
	 * [0,0,1,2,3,4]     0x00001234
	 * [0,1,2,3,4]       0x101234
	 * [1,2,3,4]         0x001234
	 * [0,0,1,2,3,4,5,T] 0x30012345
	 * [0,0,1,2,3,4,T]   0x20001234
	 * [0,1,2,3,4,5,T]   0x20012345
	 * [1,2,3,4,5,T]     0x312345
	 * [1,2,3,4,T]       0x201234
	 */
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4, 5}, false)) == "10012345");
	assert(asHex(hexPrefixEncode({0, 1, 2, 3, 4, 5}, false)) == "00012345");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4, 5}, false)) == "112345");
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4}, false)) == "00001234");
	assert(asHex(hexPrefixEncode({0, 1, 2, 3, 4}, false)) == "101234");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4}, false)) == "001234");
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4, 5}, true)) == "30012345");
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4}, true)) == "20001234");
	assert(asHex(hexPrefixEncode({0, 1, 2, 3, 4, 5}, true)) == "20012345");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4, 5}, true)) == "312345");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4}, true)) == "201234");

	return 0;
}

