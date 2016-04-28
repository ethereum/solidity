/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
	  claim that you wrote the original source code. If you use this source code
	  in a product, an acknowledgment in the product documentation would be
	  appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
	  misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/
/// Adapted from code found on http://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
/// Originally by René Nyffenegger.
/// DEVified by Gav Wood.
#pragma once

#include <string>
#include "Common.h"
#include "FixedHash.h"

namespace dev
{

std::string toBase64(bytesConstRef _in);
bytes fromBase64(std::string const& _in);

template <size_t N> inline std::string toBase36(FixedHash<N> const& _h)
{
	static char const* c_alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	typename FixedHash<N>::Arith a = _h;
	std::string ret;
	for (; a > 0; a /= 36)
	{
		unsigned r = (unsigned)(a - a / 36 * 36); // boost's % is broken
		ret = c_alphabet[r] + ret;
	}
	return ret;
}

template <size_t N> inline FixedHash<N> fromBase36(std::string const& _h)
{
	typename FixedHash<N>::Arith ret = 0;
	for (char c: _h)
		ret = ret * 36 + (c < 'A' ? c - '0' : (c - 'A' + 10));
	return ret;
}

}
