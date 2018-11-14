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

#include <string>
#include <libsolidity/ast/ASTAnnotations.h>

namespace langutil
{
class ErrorReporter;
}

namespace dev
{
namespace solidity
{

class DocStringParser
{
public:
	/// Parse the given @a _docString and stores the parsed components internally.
	/// @returns false on error and appends the error to @a _errors.
	bool parse(std::string const& _docString, langutil::ErrorReporter& _errorReporter);

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

	/// Creates and inserts a new tag and adjusts m_lastTag.
	void newTag(std::string const& _tagName);

	void appendError(std::string const& _description);

	/// Mapping tag name -> content.
	std::multimap<std::string, DocTag> m_docTags;
	DocTag* m_lastTag = nullptr;
	langutil::ErrorReporter* m_errorReporter = nullptr;
	bool m_errorsOccurred = false;
};

} //solidity NS
} // dev NS
