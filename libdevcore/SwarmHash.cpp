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
/** @file SwarmHash.cpp
 */

#include <libdevcore/SwarmHash.h>

#include <libdevcore/Keccak256.h>

using namespace std;
using namespace dev;

namespace
{

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

h256 swarmHashIntermediate(string const& _input, size_t _offset, size_t _length)
{
	bytesConstRef ref;
	bytes innerNodes;
	if (_length <= 0x1000)
		ref = bytesConstRef(_input).cropped(_offset, _length);
	else
	{
		size_t maxRepresentedSize = 0x1000;
		while (maxRepresentedSize * (0x1000 / 32) < _length)
			maxRepresentedSize *= (0x1000 / 32);
		for (size_t i = 0; i < _length; i += maxRepresentedSize)
		{
			size_t size = std::min(maxRepresentedSize, _length - i);
			innerNodes += swarmHashIntermediate(_input, _offset + i, size).asBytes();
		}
		ref = bytesConstRef(&innerNodes);
	}
	return swarmHashSimple(ref, _length);
}

}

h256 dev::swarmHash(string const& _input)
{
	return swarmHashIntermediate(_input, 0, _input.size());
}
