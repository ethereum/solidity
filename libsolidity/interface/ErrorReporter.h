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

#include <libsolidity/interface/Exceptions.h>
#include <libevmasm/SourceLocation.h>

namespace dev
{
namespace solidity
{

class ASTNode;

class ErrorReporter
{
public:

	ErrorReporter(ErrorList& _errors):
		m_errorList(_errors) { }

	ErrorReporter& operator=(ErrorReporter const& _errorReporter);

	void warning(std::string const& _description = std::string());

	void warning(
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string()
	);

	void error(
		Error::Type _type,
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string()
	);

	void declarationError(
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation = SecondarySourceLocation(),
		std::string const& _description = std::string()
	);

	void declarationError(
		SourceLocation const& _location,
		std::string const& _description = std::string()
	);

	void fatalDeclarationError(SourceLocation const& _location, std::string const& _description);

	void parserError(SourceLocation const& _location, std::string const& _description);

	void fatalParserError(SourceLocation const& _location, std::string const& _description);

	void syntaxError(SourceLocation const& _location, std::string const& _description);

	void typeError(SourceLocation const& _location, std::string const& _description);

	void fatalTypeError(SourceLocation const& _location, std::string const& _description);

	void docstringParsingError(std::string const& _location);

	ErrorList const& errors() const;

	void clear();

private:
	void error(Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(Error::Type _type,
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string());

	ErrorList& m_errorList;
};


}
}

