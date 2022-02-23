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

#include <libsolidity/parsing/DocStringParser.h>

#include <libsolidity/ast/AST.h>

#include <liblangutil/Common.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <range/v3/algorithm/find_first_of.hpp>
#include <range/v3/algorithm/find_if_not.hpp>
#include <range/v3/view/subrange.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{

string::const_iterator skipLineOrEOS(
	string::const_iterator _nlPos,
	string::const_iterator _end
)
{
	return (_nlPos == _end) ? _end : ++_nlPos;
}

string::const_iterator firstNonIdentifier(
	string::const_iterator _pos,
	string::const_iterator _end
)
{
	auto currPos = _pos;
	if (currPos == _pos && isIdentifierStart(*currPos))
	{
		currPos++;
		currPos = ranges::find_if_not(ranges::make_subrange(currPos, _end), isIdentifierPart);
	}
	return currPos;
}

string::const_iterator firstWhitespaceOrNewline(
	string::const_iterator _pos,
	string::const_iterator _end
)
{
	return ranges::find_first_of(ranges::make_subrange(_pos, _end), " \t\n");
}


string::const_iterator skipWhitespace(
	string::const_iterator _pos,
	string::const_iterator _end
)
{
	auto isWhitespace = [](char const& c) { return (c == ' ' || c == '\t'); };
	return ranges::find_if_not(ranges::make_subrange(_pos, _end), isWhitespace);
}

}

multimap<string, DocTag> DocStringParser::parse()
{
	m_lastTag = nullptr;
	m_docTags = {};

	solAssert(m_node.text(), "");
	iter currPos = m_node.text()->begin();
	iter end = m_node.text()->end();

	while (currPos != end)
	{
		iter tagPos = find(currPos, end, '@');
		iter nlPos = find(currPos, end, '\n');

		if (tagPos != end && tagPos < nlPos)
		{
			// we found a tag
			iter tagNameEndPos = firstWhitespaceOrNewline(tagPos, end);
			string tagName{tagPos + 1, tagNameEndPos};
			iter tagDataPos = (tagNameEndPos != end) ? tagNameEndPos + 1 : tagNameEndPos;
			currPos = parseDocTag(tagDataPos, end, tagName);
		}
		else if (!!m_lastTag) // continuation of the previous tag
			currPos = parseDocTagLine(currPos, end, true);
		else if (currPos != end)
		{
			// if it begins without a tag then consider it as @notice
			if (currPos == m_node.text()->begin())
			{
				currPos = parseDocTag(currPos, end, "notice");
				continue;
			}
			else if (nlPos == end) //end of text
				break;
			// else skip the line if a newline was found and we get here
			currPos = nlPos + 1;
		}
	}
	return move(m_docTags);
}

DocStringParser::iter DocStringParser::parseDocTagLine(iter _pos, iter _end, bool _appending)
{
	solAssert(!!m_lastTag, "");
	auto nlPos = find(_pos, _end, '\n');
	if (_appending && _pos != _end && *_pos != ' ' && *_pos != '\t')
		m_lastTag->content += " ";
	else if (!_appending)
		_pos = skipWhitespace(_pos, _end);
	copy(_pos, nlPos, back_inserter(m_lastTag->content));
	return skipLineOrEOS(nlPos, _end);
}

DocStringParser::iter DocStringParser::parseDocTagParam(iter _pos, iter _end)
{
	// find param name start
	auto nameStartPos = skipWhitespace(_pos, _end);
	if (nameStartPos == _end)
	{
		m_errorReporter.docstringParsingError(3335_error, m_node.location(), "No param name given");
		return _end;
	}
	auto nameEndPos = firstNonIdentifier(nameStartPos, _end);
	auto paramName = string(nameStartPos, nameEndPos);

	auto descStartPos = skipWhitespace(nameEndPos, _end);
	auto nlPos = find(descStartPos, _end, '\n');

	if (descStartPos == nlPos)
	{
		m_errorReporter.docstringParsingError(9942_error, m_node.location(), "No description given for param " + paramName);
		return _end;
	}

	auto paramDesc = string(descStartPos, nlPos);
	newTag("param");
	m_lastTag->paramName = paramName;
	m_lastTag->content = paramDesc;

	return skipLineOrEOS(nlPos, _end);
}

DocStringParser::iter DocStringParser::parseDocTag(iter _pos, iter _end, string const& _tag)
{
	// TODO: need to check for @(start of a tag) between here and the end of line
	// for all cases.
	if (!m_lastTag || _tag != "")
	{
		if (_tag == "param")
			return parseDocTagParam(_pos, _end);
		else
		{
			newTag(_tag);
			return parseDocTagLine(_pos, _end, false);
		}
	}
	else
		return parseDocTagLine(_pos, _end, true);
}

void DocStringParser::newTag(string const& _tagName)
{
	m_lastTag = &m_docTags.insert(make_pair(_tagName, DocTag()))->second;
}
