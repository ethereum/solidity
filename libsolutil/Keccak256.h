// SPDX-License-Identifier: GPL-3.0
/** @file SHA3.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * The FixedHash fixed-size "hash" container type.
 */

#pragma once

#include <libsolutil/FixedHash.h>

#include <string>

namespace solidity::util
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
