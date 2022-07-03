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

#include <boost/preprocessor.hpp>

#include <sstream>

/// Convenience macros
/// Returns a valid Solidity integer width w such that 8 <= w <= 256.
#define INTWIDTH(z, n, _ununsed) BOOST_PP_MUL(BOOST_PP_ADD(n, 1), 8)
/// Using declaration that aliases long boost multiprecision types with
/// s(u)<width> where <width> is a valid Solidity integer width and "s"
/// stands for "signed" and "u" for "unsigned".
#define USINGDECL(z, n, sign) \
	using BOOST_PP_CAT(BOOST_PP_IF(sign, s, u), INTWIDTH(z, n,)) =             \
	boost::multiprecision::number<                                             \
		boost::multiprecision::cpp_int_backend<                                \
			INTWIDTH(z, n,),                                                   \
			INTWIDTH(z, n,),                                                   \
			BOOST_PP_IF(                                                       \
				sign,                                                          \
				boost::multiprecision::signed_magnitude,                       \
				boost::multiprecision::unsigned_magnitude                      \
			),                                                                 \
			boost::multiprecision::unchecked,                                  \
			void                                                               \
		>                                                                      \
	>;
/// Instantiate the using declarations for signed and unsigned integer types.
BOOST_PP_REPEAT(32, USINGDECL, 1)
BOOST_PP_REPEAT(32, USINGDECL, 0)
/// Case implementation that returns an integer value of the specified type.
/// For signed integers, we divide by two because the range for boost multiprecision
/// types is double that of Solidity integer types. Example, 8-bit signed boost
/// number range is [-255, 255] but Solidity `int8` range is [-128, 127]
#define CASEIMPL(z, n, sign)                                                   \
	case INTWIDTH(z, n,):                                                      \
		stream << BOOST_PP_IF(                                                 \
			sign,                                                              \
			integerValue<                                                      \
				BOOST_PP_CAT(                                                  \
					BOOST_PP_IF(sign, s, u),                                   \
					INTWIDTH(z, n,)                                            \
                )>(_counter) / 2,                                              \
			integerValue<                                                      \
				BOOST_PP_CAT(                                                  \
					BOOST_PP_IF(sign, s, u),                                   \
					INTWIDTH(z, n,)                                            \
                )>(_counter)                                                   \
        );                                                                     \
		break;
/// Switch implementation that instantiates case statements for (un)signed
/// Solidity integer types.
#define SWITCHIMPL(sign)                                                       \
	std::ostringstream stream;                                                      \
	switch (_intWidth)                                                         \
	{                                                                          \
	BOOST_PP_REPEAT(32, CASEIMPL, sign)	                                       \
	}	                                                                       \
	return stream.str();

namespace solidity::test::fuzzer
{
struct LiteralGeneratorUtil
{
	template<typename V>
	V integerValue(size_t _counter);

	std::string signedIntegerValue(size_t _counter, size_t _intWidth)
	{
		SWITCHIMPL(1)
	}

	std::string unsignedIntegerValue(size_t _counter, size_t _intWidth)
	{
		SWITCHIMPL(0)
	}

	std::string integerValue(size_t _counter, size_t _intWidth, bool _signed);
	std::string fixedBytes(size_t _numBytes, size_t _counter, bool _isHexLiteral);
};
}
