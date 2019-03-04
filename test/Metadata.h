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
/**
 * @date 2017
 * Metadata processing helpers.
 */

#include <libdevcore/CommonData.h>

#include <boost/optional.hpp>

#include <map>
#include <string>

namespace dev
{
namespace test
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
boost::optional<std::map<std::string, std::string>> parseCBORMetadata(bytes const& _metadata);

/// Expects a serialised metadata JSON and returns true if the
/// content is valid metadata.
bool isValidMetadata(std::string const& _metadata);

}
} // end namespaces
