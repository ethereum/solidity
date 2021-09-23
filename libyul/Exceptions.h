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
/**
 * Exceptions in Yul.
 */

#pragma once

#include <libsolutil/Exceptions.h>
#include <libsolutil/Assertions.h>

#include <libyul/YulString.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/facilities/overload.hpp>

namespace solidity::yul
{

struct YulException: virtual util::Exception {};
struct OptimizerException: virtual YulException {};
struct CodegenException: virtual YulException {};
struct YulAssertion: virtual YulException {};

struct StackTooDeepError: virtual YulException
{
	StackTooDeepError(YulString _variable, int _depth, std::string const& _message):
		variable(_variable), depth(_depth)
	{
		*this << util::errinfo_comment(_message);
	}
	StackTooDeepError(YulString _functionName, YulString _variable, int _depth, std::string const& _message):
		functionName(_functionName), variable(_variable), depth(_depth)
	{
		*this << util::errinfo_comment(_message);
	}
	YulString functionName;
	YulString variable;
	int depth;
};

/// Assertion that throws an YulAssertion containing the given description if it is not met.
#if !BOOST_PP_VARIADICS_MSVC
#define yulAssert(...) BOOST_PP_OVERLOAD(yulAssert_,__VA_ARGS__)(__VA_ARGS__)
#else
#define yulAssert(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(yulAssert_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define yulAssert_1(CONDITION) \
	yulAssert_2(CONDITION, "")

#define yulAssert_2(CONDITION, DESCRIPTION) \
	assertThrowWithDefaultDescription( \
		CONDITION, \
		::solidity::yul::YulAssertion, \
		DESCRIPTION, \
		"Yul assertion failed" \
	)

}
