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
#include <libsolidity/ast/AST.h>
#include <memory>

using namespace std;
using namespace dev;
using namespace dev::solidity;

ErrorReporter& ErrorReporter::operator=(ErrorReporter const& _errorReporter)
{
	if (&_errorReporter == this)
		return *this;
	m_errorList = _errorReporter.m_errorList;
	return *this;
}


void ErrorReporter::warning(string const& _description)
{
	error(Error::Type::Warning, SourceLocation(), _description);
}

void ErrorReporter::warning(
	SourceLocation const& _location,
	string const& _description
)
{
	error(Error::Type::Warning, _location, _description);
}

void ErrorReporter::warning(
	SourceLocation const& _location,
	string const& _description,
	SecondarySourceLocation const& _secondaryLocation
)
{
	error(Error::Type::Warning, _location, _secondaryLocation, _description);
}

void ErrorReporter::error(Error::Type _type, SourceLocation const& _location, string const& _description)
{
	if (checkForExcessiveErrors(_type))
		return;

	auto err = make_shared<Error>(_type);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_comment(_description);

	m_errorList.push_back(err);
}

void ErrorReporter::error(Error::Type _type, SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	if (checkForExcessiveErrors(_type))
		return;

	auto err = make_shared<Error>(_type);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_secondarySourceLocation(_secondaryLocation) <<
		errinfo_comment(_description);

	m_errorList.push_back(err);
}

bool ErrorReporter::checkForExcessiveErrors(Error::Type _type)
{
	if (_type == Error::Type::Warning)
	{
		m_warningCount++;

		if (m_warningCount == c_maxWarningsAllowed)
		{
			auto err = make_shared<Error>(Error::Type::Warning);
			*err << errinfo_comment("There are more than 256 warnings. Ignoring the rest.");
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
			*err << errinfo_comment("There are more than 256 errors. Aborting.");
			m_errorList.push_back(err);
			BOOST_THROW_EXCEPTION(FatalError());
		}
	}

	return false;
}

void ErrorReporter::fatalError(Error::Type _type, SourceLocation const& _location, string const& _description)
{
	error(_type, _location, _description);
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

void ErrorReporter::declarationError(SourceLocation const& _location, SecondarySourceLocation const&_secondaryLocation, string const& _description)
{
	error(
		Error::Type::DeclarationError,
		_location,
		_secondaryLocation,
		_description
	);
}

void ErrorReporter::declarationError(SourceLocation const& _location, string const& _description)
{
	error(
		Error::Type::DeclarationError,
		_location,
		_description
	);
}

void ErrorReporter::fatalDeclarationError(SourceLocation const& _location, std::string const& _description)
{
	fatalError(
		Error::Type::DeclarationError,
		_location,
		_description);
}

void ErrorReporter::parserError(SourceLocation const& _location, string const& _description)
{
	error(
		Error::Type::ParserError,
		_location,
		_description
	);
}

void ErrorReporter::fatalParserError(SourceLocation const& _location, string const& _description)
{
	fatalError(
		Error::Type::ParserError,
		_location,
		_description
	);
}

void ErrorReporter::syntaxError(SourceLocation const& _location, string const& _description)
{
	error(
		Error::Type::SyntaxError,
		_location,
		_description
	);
}

void ErrorReporter::typeError(SourceLocation const& _location, SecondarySourceLocation const& _secondaryLocation, string const& _description)
{
	error(
		Error::Type::TypeError,
		_location,
		_secondaryLocation,
		_description
	);
}

void ErrorReporter::typeError(SourceLocation const& _location, string const& _description)
{
	error(
		Error::Type::TypeError,
		_location,
		_description
	);
}


void ErrorReporter::fatalTypeError(SourceLocation const& _location, string const& _description)
{
	fatalError(Error::Type::TypeError,
		_location,
		_description
	);
}

void ErrorReporter::docstringParsingError(string const& _description)
{
	error(
		Error::Type::DocstringParsingError,
		SourceLocation(),
		_description
	);
}
