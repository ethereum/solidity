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
 * @file Assertions.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 *
 * Assertion handling.
 */

#pragma once

#include <libsolutil/Exceptions.h>

#include <string>

namespace solidity::util
{

#if defined(_MSC_VER)
#define ETH_FUNC __FUNCSIG__
#elif defined(__GNUC__)
#define ETH_FUNC __PRETTY_FUNCTION__
#else
#define ETH_FUNC __func__
#endif

#if defined(__GNUC__)
// GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or above)
[[noreturn]] inline __attribute__((always_inline)) void unreachable()
{
	__builtin_unreachable();
}

#elif defined(_MSC_VER) // MSVC

[[noreturn]] __forceinline void unreachable()
{
	__assume(false);
}

#else

[[noreturn]] inline void unreachable()
{
	solThrow(Exception, "Unreachable");
}
#endif

namespace assertions
{

inline std::string stringOrDefault(std::string _string, std::string _defaultString)
{
	// NOTE: Putting this in a function rather than directly in a macro prevents the string from
	// being evaluated multiple times if it's not just a literal.
	return (!_string.empty() ? _string : _defaultString);
}

}

/// Base macro that can be used to implement assertion macros.
/// Throws an exception containing the given description if the condition is not met.
/// Allows you to provide the default description for the case where the user of your macro does
/// not provide any.
/// The second parameter must be an exception class (rather than an instance).
#define assertThrowWithDefaultDescription(_condition, _exceptionType, _description, _defaultDescription) \
	do \
	{ \
		if (!(_condition)) \
			solThrow( \
				_exceptionType, \
				::solidity::util::assertions::stringOrDefault((_description), (_defaultDescription)) \
			); \
	} \
	while (false)

/// Assertion that throws an exception containing the given description if it is not met.
/// Use it as assertThrow(1 == 1, ExceptionType, "Mathematics is wrong.");
/// The second parameter must be an exception class (rather than an instance).
#define assertThrow(_condition, _exceptionType, _description) \
	assertThrowWithDefaultDescription((_condition), _exceptionType, (_description), "Assertion failed")

}
