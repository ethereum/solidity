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
/**
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Error reporting helper class.
 */

#pragma once

#include <libsolutil/CommonData.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>
#include <libsolutil/StringUtils.h>

#include <boost/range/adaptor/filtered.hpp>

namespace solidity::langutil
{

/**
 * Unique identifiers are used to tag and track individual error cases.
 * They are passed as the first parameter of error reporting functions.
 * Suffix _error helps to find them in the sources.
 * The struct ErrorId prevents incidental calls like typeError(3141) instead of typeError(3141_error).
 * To create a new ID, one can add 0000_error and then run "python ./scripts/correct_error_ids.py"
 * from the root of the repo.
 */
struct ErrorId { unsigned long long error = 0; };
ErrorId operator"" _error(unsigned long long error);

class ErrorReporter
{
public:

	explicit ErrorReporter(ErrorList& _errors):
		m_errorList(_errors) { }

	ErrorReporter(ErrorReporter const& _errorReporter) noexcept:
		m_errorList(_errorReporter.m_errorList) { }

	ErrorReporter& operator=(ErrorReporter const& _errorReporter);

	void append(ErrorList const& _errorList)
	{
		m_errorList += _errorList;
	}

	void warning(ErrorId _error, std::string const& _description);

	void warning(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void warning(
		ErrorId _error,
		SourceLocation const& _location,
		std::string const& _description,
		SecondarySourceLocation const& _secondaryLocation
	);

	void error(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location,
		std::string const& _description
	);

	void declarationError(
		ErrorId _error,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description
	);

	void declarationError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void fatalDeclarationError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void parserError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void fatalParserError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void syntaxError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	void typeError(
		ErrorId _error,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation = SecondarySourceLocation(),
		std::string const& _description = std::string()
	);

	void typeError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	template <typename... Strings>
	void typeErrorConcatenateDescriptions(ErrorId _error, SourceLocation const& _location, Strings const&... _descriptions)
	{
		std::initializer_list<std::string> const descs = { _descriptions... };
		solAssert(descs.size() > 0, "Need error descriptions!");

		auto filterEmpty = boost::adaptors::filtered([](std::string const& _s) { return !_s.empty(); });

		std::string errorStr = util::joinHumanReadable(descs | filterEmpty, " ");

		error(_error, Error::Type::TypeError, _location, errorStr);
	}

	void fatalTypeError(ErrorId _error, SourceLocation const& _location, std::string const& _description);
	void fatalTypeError(ErrorId _error, SourceLocation const& _location, SecondarySourceLocation const& _secondLocation, std::string const& _description);

	void docstringParsingError(ErrorId _error, std::string const& _description);
	void docstringParsingError(ErrorId _error, SourceLocation const& _location, std::string const& _description);

	ErrorList const& errors() const;

	void clear();

	/// @returns true iff there is any error (ignores warnings).
	bool hasErrors() const
	{
		return m_errorCount > 0;
	}

	/// @returns the number of errors (ignores warnings).
	unsigned errorCount() const
	{
		return m_errorCount;
	}

	// @returns true if the maximum error count has been reached.
	bool hasExcessiveErrors() const;

private:
	void error(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(
		ErrorId _error,
		Error::Type _type,
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string());

	// @returns true if error shouldn't be stored
	bool checkForExcessiveErrors(Error::Type _type);

	ErrorList& m_errorList;

	unsigned m_errorCount = 0;
	unsigned m_warningCount = 0;

	unsigned const c_maxWarningsAllowed = 256;
	unsigned const c_maxErrorsAllowed = 256;
};

}
