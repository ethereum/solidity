// SPDX-License-Identifier: GPL-3.0
/** @file UTF8.h
 * @author Alex Beregszaszi
 * @date 2016
 *
 * UTF-8 related helpers
 */

#pragma once

#include <string>

namespace solidity::util
{

/// Validate an input for UTF8 encoding
/// @returns false if it is invalid and the first invalid position in invalidPosition
bool validateUTF8(std::string const& _input, size_t& _invalidPosition);

inline bool validateUTF8(std::string const& _input)
{
	size_t invalidPos;
	return validateUTF8(_input, invalidPos);
}

}
