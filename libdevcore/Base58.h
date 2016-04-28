/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Base58.cpp
 * Adapted from code found on https://github.com/bitcoin/bitcoin/blob/master/src/base58.cpp
 * Licenced under The MIT License.
 * @author The Bitcoin core developers (original)
 * @author Gav Wood <i@gavwood.com> (minor modifications and reformating)
 * @date 2015
 */
#pragma once

#include <string>
#include "Common.h"
#include "FixedHash.h"

namespace dev
{

extern std::string AlphabetIPFS;
extern std::string AlphabetFlickr;

std::string toBase58(bytesConstRef _in, std::string const& _alphabet = AlphabetIPFS);
bytes fromBase58(std::string const& _in, std::string const& _alphabet = AlphabetIPFS);

}
