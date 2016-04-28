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
/** @file TrieCommon.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "TrieCommon.h"

namespace dev
{

/*
 * Hex-prefix Notation. First nibble has flags: oddness = 2^0 & termination = 2^1
 * NOTE: the "termination marker" and "leaf-node" specifier are completely equivalent.
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

std::string hexPrefixEncode(bytes const& _hexVector, bool _leaf, int _begin, int _end)
{
	unsigned begin = _begin;
	unsigned end = _end < 0 ? _hexVector.size() + 1 + _end : _end;
	bool odd = ((end - begin) % 2) != 0;

	std::string ret(1, ((_leaf ? 2 : 0) | (odd ? 1 : 0)) * 16);
	if (odd)
	{
		ret[0] |= _hexVector[begin];
		++begin;
	}
	for (unsigned i = begin; i < end; i += 2)
		ret += _hexVector[i] * 16 + _hexVector[i + 1];
	return ret;
}

std::string hexPrefixEncode(bytesConstRef _data, bool _leaf, int _beginNibble, int _endNibble, unsigned _offset)
{
	unsigned begin = _beginNibble + _offset;
	unsigned end = (_endNibble < 0 ? ((int)(_data.size() * 2 - _offset) + 1) + _endNibble : _endNibble) + _offset;
	bool odd = (end - begin) & 1;

	std::string ret(1, ((_leaf ? 2 : 0) | (odd ? 1 : 0)) * 16);
	ret.reserve((end - begin) / 2 + 1);

	unsigned d = odd ? 1 : 2;
	for (auto i = begin; i < end; ++i, ++d)
	{
		byte n = nibble(_data, i);
		if (d & 1)	// odd
			ret.back() |= n;		// or the nibble onto the back
		else
			ret.push_back(n << 4);	// push the nibble on to the back << 4
	}
	return ret;
}

std::string hexPrefixEncode(bytesConstRef _d1, unsigned _o1, bytesConstRef _d2, unsigned _o2, bool _leaf)
{
	unsigned begin1 = _o1;
	unsigned end1 = _d1.size() * 2;
	unsigned begin2 = _o2;
	unsigned end2 = _d2.size() * 2;

	bool odd = (end1 - begin1 + end2 - begin2) & 1;

	std::string ret(1, ((_leaf ? 2 : 0) | (odd ? 1 : 0)) * 16);
	ret.reserve((end1 - begin1 + end2 - begin2) / 2 + 1);

	unsigned d = odd ? 1 : 2;
	for (auto i = begin1; i < end1; ++i, ++d)
	{
		byte n = nibble(_d1, i);
		if (d & 1)	// odd
			ret.back() |= n;		// or the nibble onto the back
		else
			ret.push_back(n << 4);	// push the nibble on to the back << 4
	}
	for (auto i = begin2; i < end2; ++i, ++d)
	{
		byte n = nibble(_d2, i);
		if (d & 1)	// odd
			ret.back() |= n;		// or the nibble onto the back
		else
			ret.push_back(n << 4);	// push the nibble on to the back << 4
	}
	return ret;
}

byte uniqueInUse(RLP const& _orig, byte except)
{
	byte used = 255;
	for (unsigned i = 0; i < 17; ++i)
		if (i != except && !_orig[i].isEmpty())
		{
			if (used == 255)
				used = (byte)i;
			else
				return 255;
		}
	return used;
}

}
