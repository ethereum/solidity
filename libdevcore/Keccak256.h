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
/** @file SHA3.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * The FixedHash fixed-size "hash" container type.
 */

#pragma once

#include <libdevcore/FixedHash.h>

#include <string>

namespace dev
{

/// Calculate Keccak-256 hash of the given input, returning as a 256-bit hash.
h256 keccak256(bytesConstRef _input);

/// Calculate Keccak-256 hash of the given input, returning as a 256-bit hash.
inline h256 keccak256(bytes const& _input) { return keccak256(bytesConstRef(&_input)); }

/// Calculate Keccak-256 hash of the given input (presented as a binary-filled string), returning as a 256-bit hash.
inline h256 keccak256(std::string const& _input) { return keccak256(bytesConstRef(_input)); }

/// Calculate Keccak-256 hash of the given input (presented as a FixedHash), returns a 256-bit hash.
template<unsigned N> inline h256 keccak256(FixedHash<N> const& _input) { return keccak256(_input.ref()); }

}
