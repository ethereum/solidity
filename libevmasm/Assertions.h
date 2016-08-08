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
/**
 * @file Assertions.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 *
 * Assertion handling.
 */

#pragma once

#include "Exceptions.h"
#include "debugbreak.h"

namespace dev
{

#if defined(_MSC_VER)
#define ETH_FUNC __FUNCSIG__
#elif defined(__GNUC__)
#define ETH_FUNC __PRETTY_FUNCTION__
#else
#define ETH_FUNC __func__
#endif

#define asserts(A) ::dev::assertAux(A, #A, __LINE__, __FILE__, ETH_FUNC)
#define assertsEqual(A, B) ::dev::assertEqualAux(A, B, #A, #B, __LINE__, __FILE__, ETH_FUNC)

inline bool assertAux(bool _a, char const* _aStr, unsigned _line, char const* _file, char const* _func)
{
	bool ret = _a;
	if (!ret)
	{
		std::cerr << "Assertion failed:" << _aStr << " [func=" << _func << ", line=" << _line << ", file=" << _file << "]" << std::endl;
#if ETH_DEBUG
		debug_break();
#endif
	}
	return !ret;
}

template<class A, class B>
inline bool assertEqualAux(A const& _a, B const& _b, char const* _aStr, char const* _bStr, unsigned _line, char const* _file, char const* _func)
{
	bool ret = _a == _b;
	if (!ret)
	{
		std::cerr << "Assertion failed: " << _aStr << " == " << _bStr << " [func=" << _func << ", line=" << _line << ", file=" << _file << "]" << std::endl;
		std::cerr << "   Fail equality: " << _a << "==" << _b << std::endl;
#if ETH_DEBUG
		debug_break();
#endif
	}
	return !ret;
}

/// Assertion that throws an exception containing the given description if it is not met.
/// Use it as assertThrow(1 == 1, ExceptionType, "Mathematics is wrong.");
/// Do NOT supply an exception object as the second parameter.
#define assertThrow(_condition, _ExceptionType, _description) \
	::dev::assertThrowAux<_ExceptionType>(_condition, _description, __LINE__, __FILE__, ETH_FUNC)

using errinfo_comment = boost::error_info<struct tag_comment, std::string>;

template <class _ExceptionType>
inline void assertThrowAux(
	bool _condition,
	::std::string const& _errorDescription,
	unsigned _line,
	char const* _file,
	char const* _function
)
{
	if (!_condition)
		::boost::throw_exception(
			_ExceptionType() <<
			::dev::errinfo_comment(_errorDescription) <<
			::boost::throw_function(_function) <<
			::boost::throw_file(_file) <<
			::boost::throw_line(_line)
		);
}

template <class _ExceptionType>
inline void assertThrowAux(
	void const* _pointer,
	::std::string const& _errorDescription,
	unsigned _line,
	char const* _file,
	char const* _function
)
{
	assertThrowAux<_ExceptionType>(_pointer != nullptr, _errorDescription, _line, _file, _function);
}

}
