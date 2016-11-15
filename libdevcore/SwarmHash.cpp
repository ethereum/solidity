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
/** @file SwarmHash.cpp
 */

#include <libdevcore/SwarmHash.h>

#include <libdevcore/SHA3.h>

using namespace std;
using namespace dev;


bytes toLittleEndian(size_t _size)
{
	bytes encoded(8);
	for (size_t i = 0; i < 8; ++i)
		encoded[i] = (_size >> (8 * i)) & 0xff;
	return encoded;
}

h256 swarmHashSimple(bytesConstRef _data, size_t _size)
{
	return keccak256(toLittleEndian(_size) + _data.toBytes());
}

h256 dev::swarmHash(bytes const& _input)
{
	bytes data = _input;
	size_t lastChunkSize = 0;
	size_t level = 0;
	do
	{
		bytes innerNodes;
		size_t i = 0;
		do
		{
			size_t bytes = std::min<size_t>(0x1000, data.size() - i);
			size_t size = bytes << (7 * level);
			if (i + 0x1000 >= data.size())
			{
				// last node
				size = level == 0 ? bytes : ((bytes - 32) << (7 * level)) + lastChunkSize;
				lastChunkSize = size;
			}
			innerNodes += swarmHashSimple(bytesConstRef(_input.data() + i, bytes), size).asBytes();
			i += 0x1000;
		}
		while (i < data.size());
		data = std::move(innerNodes);
		level++;
	}
	while (data.size() > 32);
	return h256(data);
}
