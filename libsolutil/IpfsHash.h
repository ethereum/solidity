// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolutil/Common.h>

#include <string>

namespace solidity::util
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
