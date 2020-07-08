// SPDX-License-Identifier: GPL-3.0
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
}

namespace solidity::frontend
{

class DocStringParser
{
public:
	/// Parse the given @a _docString and stores the parsed components internally.
	void parse(std::string const& _docString, langutil::ErrorReporter& _errorReporter);

	std::multimap<std::string, DocTag> const& tags() const { return m_docTags; }

private:
	using iter = std::string::const_iterator;
	void resetUser();
	void resetDev();

	iter parseDocTagLine(iter _pos, iter _end, bool _appending);
	iter parseDocTagParam(iter _pos, iter _end);
	iter appendDocTagParam(iter _pos, iter _end);
	void parseDocString(std::string const& _string);
	/// Parses the doc tag named @a _tag, adds it to m_docTags and returns the position
	/// after the tag.
	iter parseDocTag(iter _pos, iter _end, std::string const& _tag);

	/// Creates and inserts a new tag and adjusts m_lastTag.
	void newTag(std::string const& _tagName);

	/// Mapping tag name -> content.
	std::multimap<std::string, DocTag> m_docTags;
	DocTag* m_lastTag = nullptr;
	langutil::ErrorReporter* m_errorReporter = nullptr;
};

}
