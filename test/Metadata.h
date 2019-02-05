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
#include <string>

namespace dev
{
namespace test
{

/// Returns the bytecode with the metadata hash stripped out.
bytes bytecodeSansMetadata(bytes const& _bytecode);

/// Returns the bytecode with the metadata hash stripped out.
/// Throws exception on invalid hex string.
std::string bytecodeSansMetadata(std::string const& _bytecode);

/// Expects a serialised metadata JSON and returns true if the
/// content is valid metadata.
bool isValidMetadata(std::string const& _metadata);

}
} // end namespaces
