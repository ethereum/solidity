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
#include <utility>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libevmasm/SourceLocation.h>

namespace dev
{
namespace solidity
{

class ErrorReporter;

class DocStringParser
{
public:
	explicit DocStringParser(std::string _sourceUnitName,
							 SourceReferenceFormatter::ScannerFromSourceNameFun _scannerFromSource):
		m_sourceUnitName(std::move(_sourceUnitName)), m_scannerFromSourceFunction(std::move(_scannerFromSource)),
		m_lines(0), m_currentLine(0) {}

	/// Parse the given @a _docString and stores the parsed components internally.
	/// @returns false on error and appends the error to @a _errors.
	bool parse(std::string const& _docString, SourceLocation const& _location, ErrorReporter& _errorReporter);

	std::multimap<std::string, DocTag> const& tags() const { return m_docTags; }

private:
	using iter = std::string::const_iterator;

	iter parseDocTagLine(iter _pos, iter _end, bool _appending, bool _preserveNewLines = false);
	iter parseDocTagParam(iter _pos, iter _end);

	iter appendDocTag(iter _pos, iter _end, bool _preserveNewLines = false);
	/// Parses the doc tag named @a _tag, adds it to m_docTags and returns the position
	/// after the tag.
	iter parseDocTag(iter _pos, iter _end, std::string const& _tag);

	/// Creates and inserts a new tag and adjusts m_lastTag.
	void newTag(std::string const& _tagName);

	void appendError(std::string const& _description);

	/// Mapping tag name -> content.
	std::multimap<std::string, DocTag> m_docTags;
	DocTag* m_lastTag = nullptr;
	ErrorReporter* m_errorReporter = nullptr;
	bool m_errorsOccurred = false;

	std::string m_sourceUnitName;
	SourceReferenceFormatter::ScannerFromSourceNameFun m_scannerFromSourceFunction;
	SourceLocation m_location;
	size_t m_lines;
	size_t m_currentLine;
};

} //solidity NS
} // dev NS
