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

#include <libsolutil/Assertions.h>
#include <libsolutil/Exceptions.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/facilities/overload.hpp>

namespace solidity::smtutil
{

struct SMTLogicError: virtual util::Exception {};

/// Assertion that throws an SMTLogicError containing the given description if it is not met.
#if !BOOST_PP_VARIADICS_MSVC
#define smtAssert(...) BOOST_PP_OVERLOAD(smtAssert_,__VA_ARGS__)(__VA_ARGS__)
#else
#define smtAssert(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(smtAssert_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define smtAssert_1(CONDITION) \
	smtAssert_2(CONDITION, "")

#define smtAssert_2(CONDITION, DESCRIPTION) \
	assertThrowWithDefaultDescription( \
		CONDITION, \
		::solidity::smtutil::SMTLogicError, \
		DESCRIPTION, \
		"SMT assertion failed" \
	)

}
