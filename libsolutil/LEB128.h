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


#include <libsolutil/Common.h>

namespace solidity::util
{

inline bytes lebEncode(uint64_t _n)
{
	bytes encoded;
	while (_n > 0x7f)
	{
		encoded.emplace_back(uint8_t(0x80 | (_n & 0x7f)));
		_n >>= 7;
	}
	encoded.emplace_back(_n);
	return encoded;
}

// signed right shift is an arithmetic right shift
static_assert((-1 >> 1) == -1, "Arithmetic shift not supported.");

inline bytes lebEncodeSigned(int64_t _n)
{
	// Based on https://github.com/llvm/llvm-project/blob/master/llvm/include/llvm/Support/LEB128.h
	bytes result;
	bool more;
	do
	{
		uint8_t v = _n & 0x7f;
		_n >>= 7;
		more = !((((_n == 0) && ((v & 0x40) == 0)) || ((_n == -1) && ((v & 0x40) != 0))));
		if (more)
			v |= 0x80; // Mark this byte to show that more bytes will follow.
		result.emplace_back(v);
	}
	while (more);
	return result;
}

}
