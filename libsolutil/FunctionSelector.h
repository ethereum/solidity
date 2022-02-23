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

#include <libsolutil/Keccak256.h>
#include <libsolutil/FixedHash.h>

#include <string>

namespace solidity::util
{

/// @returns the ABI selector for a given function signature, as a 32 bit number.
inline uint32_t selectorFromSignature32(std::string const& _signature)
{
	return uint32_t(FixedHash<4>::Arith(util::FixedHash<4>(util::keccak256(_signature))));
}

/// @returns the ABI selector for a given function signature, as a u256 (left aligned) number.
inline u256 selectorFromSignature(std::string const& _signature)
{
	return u256(selectorFromSignature32(_signature)) << (256 - 32);
}


}
