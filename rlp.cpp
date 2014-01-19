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
/** @file rlp.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * RLP test functions.
 */

#include <RLP.h>
using namespace std;
using namespace eth;

int rlpTest()
{
	// int of value 15
	assert(RLP("\x0f") == 15);
	assert(asString(rlp(15)) == "\x0f");

	// 3-character string
	assert(RLP("\x43""dog") == "dog");
	assert(asString(rlp("dog")) == "\x43""dog");

	// 2-item list
	string twoItemListString = "\x82\x0f\x43""dog";
	RLP twoItemList(twoItemListString);
	assert(twoItemList.itemCount() == 2);
	assert(twoItemList[0] == 15);
	assert(twoItemList[1] == "dog");
	assert(asString(rlpList(15, "dog")) == "\x82\x0f\x43""dog");

	// 1-byte (8-bit) int
	assert(RLP("\x18\x45") == 69);
	assert(asString(rlp(69)) == "\x18\x45");

	// 2-byte (16-bit) int
	assert(RLP("\x19\x01\x01") == 257);
	assert(asString(rlp(257)) == "\x19\x01\x01");

	// 32-byte (256-bit) int
	assert(RLP("\x37\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f") == bigint("0x100102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));
	assert(asString(rlp(bigint("0x100102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"))) == "\x37\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f");

	// 33-byte (264-bit) int
	assert(RLP("\x38\x21\x20\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f") == bigint("0x20100102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"));
	assert(asString(rlp(bigint("0x20100102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"))) == "\x38\x21\x20\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f");

	// 56-character string.
	assert(RLP("\x78\x38""Lorem ipsum dolor sit amet, consectetur adipisicing elit") == "Lorem ipsum dolor sit amet, consectetur adipisicing elit");
	assert(asString(rlp("Lorem ipsum dolor sit amet, consectetur adipisicing elit")) == "\x78\x38""Lorem ipsum dolor sit amet, consectetur adipisicing elit");

	return 0;
}

