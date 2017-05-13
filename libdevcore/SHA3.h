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

#include <string>
#include "FixedHash.h"

namespace dev
{

// Keccak-256 convenience routines.

/// Calculate Keccak-256 hash of the given input and load it into the given output.
/// @returns false if o_output.size() != 32.
bool keccak256(bytesConstRef _input, bytesRef o_output);

/// Calculate Keccak-256 hash of the given input, returning as a 256-bit hash.
inline h256 keccak256(bytesConstRef _input) { h256 ret; keccak256(_input, ret.ref()); return ret; }

/// Calculate Keccak-256 hash of the given input, returning as a 256-bit hash.
inline h256 keccak256(bytes const& _input) { return keccak256(bytesConstRef(&_input)); }

/// Calculate Keccak-256 hash of the given input (presented as a binary-filled string), returning as a 256-bit hash.
inline h256 keccak256(std::string const& _input) { return keccak256(bytesConstRef(_input)); }

/// Calculate Keccak-256 hash of the given input (presented as a FixedHash), returns a 256-bit hash.
template<unsigned N> inline h256 keccak256(FixedHash<N> const& _input) { return keccak256(_input.ref()); }

/// Calculate Keccak-256 hash of the given input, possibly interpreting it as nibbles, and return the hash as a string filled with binary data.
inline std::string keccak256(std::string const& _input, bool _isNibbles) { return asString((_isNibbles ? keccak256(fromHex(_input)) : keccak256(bytesConstRef(&_input))).asBytes()); }

/// Calculate Keccak-256 MAC
inline void keccak256mac(bytesConstRef _secret, bytesConstRef _plain, bytesRef _output) { keccak256(_secret.toBytes() + _plain.toBytes()).ref().populate(_output); }

}
