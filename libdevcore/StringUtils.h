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
/** @file StringUtils.h
 * @author Balajiganapathi S <balajiganapathi.s@gmail.com>
 * @date 2017
 *
 * String routines
 */

#pragma once

#include <string>

namespace dev
{

/// Calculates the Damerau–Levenshtein distance between @a _name1 and @a _name2 and
/// @returns true if that distance is not greater than @a _maxDistance
bool stringWithinDistance(std::string const& _name1, std::string const& _name2, size_t _maxDistance);

}
