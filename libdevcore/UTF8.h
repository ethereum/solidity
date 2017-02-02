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
/** @file UTF8.h
 * @author Alex Beregszaszi
 * @date 2016
 *
 * UTF-8 related helpers
 */

#pragma once

#include <string>

namespace dev
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
