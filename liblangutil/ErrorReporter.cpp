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
 * Error helper class.
 */

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceLocation.h>
#include <memory>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;

ErrorId solidity::langutil::operator"" _error(unsigned long long _error) { return ErrorId{ _error }; }

ErrorReporter& ErrorReporter::operator=(ErrorReporter const& _errorReporter)
{
	if (&_errorReporter == this)
		return *this;
	m_errorList = _errorReporter.m_errorList;
	return *this;
}

void ErrorReporter::warning(ErrorId _error, string const& _description)
{
	error(_error, Error::Type::Warning, SourceLocation(), _description);
}

void ErrorReporter::warning(
	ErrorId _error,
	SourceLocation const& _location,
	string const& _description
)
{
	error(_error, Error::Type::Warning, _location, _description);
}

void ErrorReporter::warning(
	ErrorId _error,
	SourceLocation const& _location,
	string const& _description,
	SecondarySourceLocation const& _secondaryLocation
)
{
	error(_error, Error::Type::Warning, _location, _secondaryLocation, _description);
}

void ErrorReporter::error(ErrorId, Error::Type _type, SourceLocation const& _location, string const& _description)
{
	if (checkForExcessiveErrors(_type))
		return;

	auto err = make_shared<Error>(_type);
	*err <<
		errinfo_sourceLocation(_location) <<
		util::errinfo_comment(_description);

	m_errorList.push_back(err);
}

void ErrorReporter::error(ErrorId, Error::Type _type, SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	if (checkForExcessiveErrors(_type))
		return;

	auto err = make_shared<Error>(_type);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_secondarySourceLocation(_secondaryLocation) <<
		util::errinfo_comment(_description);

	m_errorList.push_back(err);
}

bool ErrorReporter::hasExcessiveErrors() const
{
	return m_errorCount > c_maxErrorsAllowed;
}

bool ErrorReporter::checkForExcessiveErrors(Error::Type _type)
{
	if (_type == Error::Type::Warning)
	{
		m_warningCount++;

		if (m_warningCount == c_maxWarningsAllowed)
		{
			auto err = make_shared<Error>(Error::Type::Warning);
			*err << util::errinfo_comment("There are more than 256 warnings. Ignoring the rest.");
			m_errorList.push_back(err);
		}

		if (m_warningCount >= c_maxWarningsAllowed)
			return true;
	}
	else
	{
		m_errorCount++;

		if (m_errorCount > c_maxErrorsAllowed)
		{
			auto err = make_shared<Error>(Error::Type::Warning);
			*err << util::errinfo_comment("There are more than 256 errors. Aborting.");
			m_errorList.push_back(err);
			BOOST_THROW_EXCEPTION(FatalError());
		}
	}

	return false;
}

void ErrorReporter::fatalError(ErrorId _error, Error::Type _type, SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	error(_error, _type, _location, _secondaryLocation, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

void ErrorReporter::fatalError(ErrorId _error, Error::Type _type, SourceLocation const& _location, string const& _description)
{
	error(_error, _type, _location, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

ErrorList const& ErrorReporter::errors() const
{
	return m_errorList;
}

void ErrorReporter::clear()
{
	m_errorList.clear();
}

void ErrorReporter::declarationError(ErrorId _error, SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	error(
		_error,
		Error::Type::DeclarationError,
		_location,
		_secondaryLocation,
		_description
	);
}

void ErrorReporter::declarationError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	error(
		_error,
		Error::Type::DeclarationError,
		_location,
		_description
	);
}

void ErrorReporter::fatalDeclarationError(ErrorId _error, SourceLocation const& _location, std::string const& _description)
{
	fatalError(
		_error,
		Error::Type::DeclarationError,
		_location,
		_description);
}

void ErrorReporter::parserError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	error(
		_error,
		Error::Type::ParserError,
		_location,
		_description
	);
}

void ErrorReporter::fatalParserError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	fatalError(
		_error,
		Error::Type::ParserError,
		_location,
		_description
	);
}

void ErrorReporter::syntaxError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	error(
		_error,
		Error::Type::SyntaxError,
		_location,
		_description
	);
}

void ErrorReporter::typeError(ErrorId _error, SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	error(
		_error,
		Error::Type::TypeError,
		_location,
		_secondaryLocation,
		_description
	);
}

void ErrorReporter::typeError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	error(
		_error,
		Error::Type::TypeError,
		_location,
		_description
	);
}


void ErrorReporter::fatalTypeError(ErrorId _error, SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	fatalError(
		_error,
		Error::Type::TypeError,
		_location,
		_secondaryLocation,
		_description
	);
}

void ErrorReporter::fatalTypeError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	fatalError(
		_error,
		Error::Type::TypeError,
		_location,
		_description
	);
}

void ErrorReporter::docstringParsingError(ErrorId _error, string const& _description)
{
	error(
		_error,
		Error::Type::DocstringParsingError,
		SourceLocation(),
		_description
	);
}

void ErrorReporter::docstringParsingError(ErrorId _error, SourceLocation const& _location, string const& _description)
{
	error(
		_error,
		Error::Type::DocstringParsingError,
		_location,
		_description
	);
}
