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

#pragma once

#include <libdevcore/AnsiColorized.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace solidity
{
namespace test
{

#define soltestAssert(CONDITION, DESCRIPTION) \
	do \
	{ \
		if (!(CONDITION)) \
			BOOST_THROW_EXCEPTION(runtime_error(DESCRIPTION)); \
	} \
	while (false)


/**
 * Representation of a notice, warning or error that can occur while
 * formatting and therefore updating an interactive function call test.
 */
struct FormatError
{
	enum Type
	{
		Notice,
		Warning,
		Error
	};

	explicit FormatError(Type _type, std::string _message):
		type(_type),
		message(std::move(_message))
	{}

	Type type;
	std::string message;
};
using FormatErrors = std::vector<FormatError>;

/**
 * Utility class that collects notices, warnings and errors and is able
 * to format them for ANSI colorized output during the interactive update
 * process in isoltest.
 * Its purpose is to help users of isoltest to automatically
 * update test files and always keep track of what is happening.
 */
class ErrorReporter
{
public:
	explicit ErrorReporter() {}

	/// Adds a new FormatError of type Notice with the given message.
	void notice(std::string _notice)
	{
		m_errors.push_back(FormatError{FormatError::Notice, std::move(_notice)});
	}

	/// Adds a new FormatError of type Warning with the given message.
	void warning(std::string _warning)
	{
		m_errors.push_back(FormatError{FormatError::Warning, std::move(_warning)});
	}

	/// Adds a new FormatError of type Error with the given message.
	void error(std::string _error)
	{
		m_errors.push_back(FormatError{FormatError::Error, std::move(_error)});
	}

	/// Prints all errors depending on their type using ANSI colorized output.
	/// It will be used to print notices, warnings and errors during the
	/// interactive update process.
	std::string format(std::string const& _linePrefix, bool _formatted)
	{
		std::stringstream os;
		for (auto const& error: m_errors)
		{
			switch (error.type)
			{
			case FormatError::Notice:

				break;
			case FormatError::Warning:
				AnsiColorized(
					os,
					_formatted,
					{formatting::YELLOW}
				) << _linePrefix << "Warning: " << error.message << std::endl;
				break;
			case FormatError::Error:
				AnsiColorized(
					os,
					_formatted,
					{formatting::RED}
				) << _linePrefix << "Error: " << error.message << std::endl;
				break;
			}
		}
		return os.str();
	}

private:
	FormatErrors m_errors;
};

}
}
}
