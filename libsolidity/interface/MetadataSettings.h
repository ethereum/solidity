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
#pragma once

#include <map>
#include <optional>
#include <string>

namespace solidity::frontend
{

/// Known metadata hashing methods.
enum class MetadataHash
{
	IPFS,
	Bzzr1,
	None,
};

/// Map from MetadataHash enum values to their string representations.
inline std::map<MetadataHash, std::string> const metadataHashes = {
	{MetadataHash::IPFS, "ipfs"},
	{MetadataHash::Bzzr1, "bzzr1"},
	{MetadataHash::None, "none"},
};

/// @param _metadataHash The metadata hash string to convert.
/// @return The corresponding MetadataHash enum value, or std::nullopt for unknown strings.
inline std::optional<MetadataHash> metadataHashFromString(std::string const& _metadataHash)
{
	for (auto const& [value, name]: metadataHashes)
		if (name == _metadataHash)
			return value;
	return std::nullopt;
}

/// @param _metadataHash The metadata hash enum value to convert to string.
/// @return The string representation of the metadata hash.
inline std::string metadataHashToString(MetadataHash _metadataHash)
{
	return metadataHashes.at(_metadataHash);
}

}

