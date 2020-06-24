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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014, 2015
 * Parses a given docstring into pieces introduced by tags.
 */

#pragma once

#include <libsolidity/ast/ASTAnnotations.h>
#include <string>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::frontend
{

class DocStringParser
{
public:
	/// Parse the given @a _docString and stores the parsed components internally.
	void parse(
		std::string const& _docString,
		langutil::SourceLocation const& _location,
		langutil::ErrorReporter& _errorReporter
	);

	std::multimap<std::string, DocTag> const& tags() const { return m_docTags; }

private:
	using iter = std::string::const_iterator;
	void resetUser();
	void resetDev();

	iter parseDocTagLine(iter _pos, iter _end, bool _appending);
	iter parseDocTagParam(iter _pos, iter _end);
	iter appendDocTagParam(iter _pos, iter _end);
	void parseDocString(std::string const& _string);
	iter appendDocTag(iter _pos, iter _end);
	/// Parses the doc tag named @a _tag, adds it to m_docTags and returns the position
	/// after the tag.
	iter parseDocTag(iter _pos, iter _end, std::string const& _tag);

	void docstringParsingError(langutil::ErrorId _error, std::string const& _description);

	void docstringParsingError(
		std::string::const_iterator const& _subRangeBegin,
		std::string::const_iterator const& _subRangeEnd,
		langutil::ErrorId _error,
		std::string const& _description
	);

	/// Creates and inserts a new tag and adjusts m_lastTag.
	void newTag(std::string const& _tagName);

	/// Mapping tag name -> content.
	std::multimap<std::string, DocTag> m_docTags;
	DocTag* m_lastTag = nullptr;
	langutil::ErrorReporter* m_errorReporter = nullptr;

	std::string const* m_docString = nullptr;
	langutil::SourceLocation m_location{};
};

}
