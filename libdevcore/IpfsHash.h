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

#pragma once

#include <libdevcore/Common.h>

#include <string>

namespace dev
{

/// Compute the "ipfs hash" of a file with the content @a _data.
/// The output will be the multihash of the UnixFS protobuf encoded data.
/// As hash function it will use sha2-256.
/// The effect is that the hash should be identical to the one produced by
/// the command `ipfs add <filename>`.
bytes ipfsHash(std::string _data);

/// Compute the "ipfs hash" as above, but encoded in base58 as used by ipfs / bitcoin.
std::string ipfsHashBase58(std::string _data);

}
