// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2017
 * Metadata processing helpers.
 */

#include <libsolutil/CommonData.h>

#include <map>
#include <optional>
#include <string>

namespace solidity::test
{

/// Returns only the CBOR metadata.
bytes onlyMetadata(bytes const& _bytecode);

/// Returns the bytecode with the metadata hash stripped out.
bytes bytecodeSansMetadata(bytes const& _bytecode);

/// Returns the bytecode with the metadata hash stripped out.
/// Throws exception on invalid hex string.
std::string bytecodeSansMetadata(std::string const& _bytecode);

/// Parse CBOR metadata into a map. Expects the input CBOR to be a
/// fixed length map, with each key being a string. The values
/// are parsed as follows:
/// - strings into strings
/// - bytes into hex strings
/// - booleans into "true"/"false" strings
/// - everything else is invalid
std::optional<std::map<std::string, std::string>> parseCBORMetadata(bytes const& _metadata);

/// Expects a serialised metadata JSON and returns true if the
/// content is valid metadata.
bool isValidMetadata(std::string const& _metadata);

} // end namespaces
