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

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/info_tuple.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <exception>
#include <string>

namespace solidity::util
{

/// Base class for all exceptions.
struct Exception: virtual std::exception, virtual boost::exception
{
	char const* what() const noexcept override;

	/// @returns "FileName:LineNumber" referring to the point where the exception was thrown.
	std::string lineInfo() const;

	/// @returns the errinfo_comment of this exception.
	std::string const* comment() const noexcept;
};

/// Throws an exception with a given description and extra information about the location the
/// exception was thrown from.
/// @param _exceptionType The type of the exception to throw (not an instance).
/// @param _description The message that describes the error.
#define solThrow(_exceptionType, _description) \
	::boost::throw_exception( \
		_exceptionType() << \
		::solidity::util::errinfo_comment((_description)) << \
		::boost::throw_function(ETH_FUNC) << \
		::boost::throw_file(__FILE__) << \
		::boost::throw_line(__LINE__) \
	)

/// Throws an exception if condition is not met with a given description and extra information about the location the
/// exception was thrown from.
/// @param _condition if condition is not met, specified exception will be thrown.
/// @param _exceptionType The type of the exception to throw (not an instance).
/// @param _description The message that describes the error.
#define solRequire(_condition, _exceptionType, _description) \
	if (!(_condition)) \
		solThrow(_exceptionType, (_description))

/// Defines an exception type that's meant to signal a specific condition and be caught rather than
/// unwind the stack all the way to the top-level exception handler and interrupt the program.
/// As such it does not carry a message - the code catching it is expected to handle it without
/// letting it escape.
#define DEV_SIMPLE_EXCEPTION(X) struct X: virtual ::solidity::util::Exception { const char* what() const noexcept override { return #X; } }

DEV_SIMPLE_EXCEPTION(InvalidAddress);
DEV_SIMPLE_EXCEPTION(BadHexCharacter);
DEV_SIMPLE_EXCEPTION(BadHexCase);
DEV_SIMPLE_EXCEPTION(FileNotFound);
DEV_SIMPLE_EXCEPTION(NotAFile);
DEV_SIMPLE_EXCEPTION(DataTooLong);
DEV_SIMPLE_EXCEPTION(StringTooLong);
DEV_SIMPLE_EXCEPTION(ElementNotFound);

// error information to be added to exceptions
using errinfo_comment = boost::error_info<struct tag_comment, std::string>;

}
