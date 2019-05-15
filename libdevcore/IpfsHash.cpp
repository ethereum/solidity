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

#include <libdevcore/IpfsHash.h>

#include <libdevcore/Exceptions.h>
#include <libdevcore/picosha2.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;

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

string base58Encode(bytes const& _data)
{
	static string const alphabet{"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"};
	bigint data(toHex(_data, HexPrefix::Add));
	string output;
	while (data)
	{
		output += alphabet[size_t(data % alphabet.size())];
		data /= alphabet.size();
	}
	reverse(output.begin(), output.end());
	return output;
}
}

bytes dev::ipfsHash(string _data)
{
	if (_data.length() >= 1024 * 256)
		BOOST_THROW_EXCEPTION(
			DataTooLong() <<
			errinfo_comment("Ipfs hash for large (chunked) files not yet implemented.")
		);

	bytes lengthAsVarint = varintEncoding(_data.size());

	bytes protobufEncodedData;
	// Type: File
	protobufEncodedData += bytes{0x08, 0x02};
	if (!_data.empty())
	{
		// Data (length delimited bytes)
		protobufEncodedData += bytes{0x12};
		protobufEncodedData += lengthAsVarint;
		protobufEncodedData += asBytes(std::move(_data));
	}
	// filesize: length as varint
	protobufEncodedData += bytes{0x18} + lengthAsVarint;

	// PBDag:
	// Data: (length delimited bytes)
	size_t protobufLength = protobufEncodedData.size();
	bytes blockData = bytes{0x0a} + varintEncoding(protobufLength) + std::move(protobufEncodedData);
	// TODO Handle "large" files with multiple blocks

	// Multihash: sha2-256, 256 bits
	bytes hash = bytes{0x12, 0x20} + picosha2::hash256(std::move(blockData));
	return hash;
}

string dev::ipfsHashBase58(string _data)
{
	return base58Encode(ipfsHash(std::move(_data)));
}
