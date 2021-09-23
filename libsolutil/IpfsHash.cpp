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

#include <libsolutil/IpfsHash.h>

#include <libsolutil/Exceptions.h>
#include <libsolutil/picosha2.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;

namespace
{
bytes varintEncoding(size_t _n)
{
	bytes encoded;
	while (_n > 0x7f)
	{
		encoded.emplace_back(uint8_t(0x80 | (_n & 0x7f)));
		_n >>= 7;
	}
	encoded.emplace_back(_n);
	return encoded;
}

bytes encodeByteArray(bytes const& _data)
{
	return bytes{0x0a} + varintEncoding(_data.size()) + _data;
}

bytes encodeHash(bytes const& _data)
{
	return bytes{0x12, 0x20} + picosha2::hash256(_data);
}

bytes encodeLinkData(bytes const& _data)
{
	return bytes{0x12} + varintEncoding(_data.size()) + _data;
}

string base58Encode(bytes const& _data)
{
	static string const alphabet{"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"};
	bigint data(toHex(_data, HexPrefix::Add));
	string output;
	while (data)
	{
		output += alphabet[static_cast<size_t>(data % alphabet.size())];
		data /= alphabet.size();
	}
	reverse(output.begin(), output.end());
	return output;
}

struct Chunk
{
	Chunk() = default;
	Chunk(bytes _hash, size_t _size, size_t _blockSize):
		hash(std::move(_hash)),
		size(_size),
		blockSize(_blockSize)
	{}

	bytes hash = {};
	size_t size = 0;
	size_t blockSize = 0;
};

using Chunks = vector<Chunk>;

Chunk combineLinks(Chunks& _links)
{
	bytes data = {};
	bytes lengths = {};
	Chunk chunk = {};
	for (Chunk& link: _links)
	{
		chunk.size += link.size;
		chunk.blockSize += link.blockSize;

		data += encodeLinkData(
			bytes {0x0a} +
			varintEncoding(link.hash.size()) +
			std::move(link.hash) +
			bytes{0x12, 0x00, 0x18} +
			varintEncoding(link.blockSize)
		);

		lengths += bytes{0x20} + varintEncoding(link.size);
	}

	bytes blockData = data + encodeByteArray(bytes{0x08, 0x02, 0x18} + varintEncoding(chunk.size) + lengths);

	chunk.blockSize += blockData.size();
	chunk.hash = encodeHash(blockData);

	return chunk;
}

Chunks buildNextLevel(Chunks& _currentLevel)
{
	size_t const maxChildNum = 174;

	Chunks nextLevel;
	Chunks links;

	for (Chunk& chunk: _currentLevel)
	{
		links.emplace_back(std::move(chunk.hash), chunk.size, chunk.blockSize);
		if (links.size() == maxChildNum)
		{
			nextLevel.emplace_back(combineLinks(links));
			links = {};
		}
	}
	if (!links.empty())
		nextLevel.emplace_back(combineLinks(links));

	return nextLevel;
}

/// Builds a tree starting from the bottom level where nodes are data nodes.
/// Data nodes should be calculated and passed as the only level in chunk levels
/// Each next level is calculated as following:
///   - Pick up to maxChildNum (174) nodes until a whole level is added, group them and pass to the node in the next level
///   - Do this until the current level has only one node, return the hash in that node
bytes groupChunksBottomUp(Chunks _currentLevel)
{
	// when we reach root it will be the only node in that level
	while (_currentLevel.size() != 1)
		_currentLevel = buildNextLevel(_currentLevel);

	// top level's only node stores the hash for file
	return _currentLevel.front().hash;
}
}

bytes solidity::util::ipfsHash(string _data)
{
	size_t const maxChunkSize = 1024 * 256;
	size_t chunkCount = _data.length() / maxChunkSize + (_data.length() % maxChunkSize > 0 ? 1 : 0);
	chunkCount = chunkCount == 0 ? 1 : chunkCount;

	Chunks allChunks;

	for (size_t chunkIndex = 0; chunkIndex < chunkCount; chunkIndex++)
	{
		bytes chunkBytes = asBytes(
			_data.substr(chunkIndex * maxChunkSize, min(maxChunkSize, _data.length() - chunkIndex * maxChunkSize))
		);

		bytes lengthAsVarint = varintEncoding(chunkBytes.size());

		bytes protobufEncodedData;
		// Type: File
		protobufEncodedData += bytes{0x08, 0x02};
		if (!chunkBytes.empty())
		{
			// Data (length delimited bytes)
			protobufEncodedData += bytes{0x12};
			protobufEncodedData += lengthAsVarint;
			protobufEncodedData += chunkBytes;
		}
		// filesize: length as varint
		protobufEncodedData += bytes{0x18} + lengthAsVarint;

		// PBDag:
		// Data: (length delimited bytes)
		bytes blockData = encodeByteArray(protobufEncodedData);

		// Multihash: sha2-256, 256 bits
		allChunks.emplace_back(
			encodeHash(blockData),
			chunkBytes.size(),
			blockData.size()
		);
	}

	return groupChunksBottomUp(std::move(allChunks));
}

string solidity::util::ipfsHashBase58(string _data)
{
	return base58Encode(ipfsHash(std::move(_data)));
}
