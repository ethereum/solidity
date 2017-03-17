
#include <libsolidity/parsing/DocStringParser.h>
#include <libsolidity/interface/Utils.h>

#include <boost/range/irange.hpp>
#include <boost/range/algorithm.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;


namespace
{

string::const_iterator skipLineOrEOS(
	string::const_iterator _nlPos,
	string::const_iterator _end
)
{
	return (_nlPos == _end) ? _end : ++_nlPos;
}

string::const_iterator firstSpaceOrTab(
	string::const_iterator _pos,
	string::const_iterator _end
)
{
	return boost::range::find_first_of(make_pair(_pos, _end), " \t");
}

string::const_iterator firstWhitespaceOrNewline(
	string::const_iterator _pos,
	string::const_iterator _end
)
{
	return boost::range::find_first_of(make_pair(_pos, _end), " \t\n");
}


string::const_iterator skipWhitespace(
	string::const_iterator _pos,
	string::const_iterator _end
)
{
	auto currPos = _pos;
	while (currPos != _end && (*currPos == ' ' || *currPos == '\t'))
		currPos += 1;
	return currPos;
}

}

bool DocStringParser::parse(string const& _docString, ErrorList& _errors)
{
	m_errors = &_errors;
	m_errorsOccurred = false;
	m_lastTag = nullptr;

	auto currPos = _docString.begin();
	auto end = _docString.end();

	while (currPos != end)
	{
		auto tagPos = find(currPos, end, '@');
		auto nlPos = find(currPos, end, '\n');

		if (tagPos != end && tagPos < nlPos)
		{
			// we found a tag
			auto tagNameEndPos = firstWhitespaceOrNewline(tagPos, end);
			if (tagNameEndPos == end)
			{
				appendError("End of tag " + string(tagPos, tagNameEndPos) + "not found");
				break;
			}

			currPos = parseDocTag(tagNameEndPos + 1, end, string(tagPos + 1, tagNameEndPos));
		}
		else if (!!m_lastTag) // continuation of the previous tag
			currPos = appendDocTag(currPos, end);
		else if (currPos != end)
		{
			// if it begins without a tag then consider it as @notice
			if (currPos == _docString.begin())
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
	return !m_errorsOccurred;
}

DocStringParser::iter DocStringParser::parseDocTagLine(iter _pos, iter _end, bool _appending)
{
	solAssert(!!m_lastTag, "");
	auto nlPos = find(_pos, _end, '\n');
	if (_appending && _pos < _end && *_pos != ' ' && *_pos != '\t')
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
		appendError("No param name given");
		return _end;
	}
	auto nameEndPos = firstSpaceOrTab(nameStartPos, _end);
	if (nameEndPos == _end)
	{
		appendError("End of param name not found: " + string(nameStartPos, _end));
		return _end;
	}
	auto paramName = string(nameStartPos, nameEndPos);

	auto descStartPos = skipWhitespace(nameEndPos, _end);
	if (descStartPos == _end)
	{
		appendError("No description given for param " + paramName);
		return _end;
	}

	auto nlPos = find(descStartPos, _end, '\n');
	auto paramDesc = string(descStartPos, nlPos);
	newTag("param");
	m_lastTag->paramName = paramName;
	m_lastTag->content = paramDesc;

	return skipLineOrEOS(nlPos, _end);
}

DocStringParser::iter DocStringParser::parseDocTag(iter _pos, iter _end, string const& _tag)
{
	// LTODO: need to check for @(start of a tag) between here and the end of line
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
		return appendDocTag(_pos, _end);
}

DocStringParser::iter DocStringParser::appendDocTag(iter _pos, iter _end)
{
	solAssert(!!m_lastTag, "");
	return parseDocTagLine(_pos, _end, true);
}

void DocStringParser::newTag(string const& _tagName)
{
	m_lastTag = &m_docTags.insert(make_pair(_tagName, DocTag()))->second;
}

void DocStringParser::appendError(string const& _description)
{
	auto err = make_shared<Error>(Error::Type::DocstringParsingError);
	*err << errinfo_comment(_description);
	m_errors->push_back(err);
	m_errorsOccurred = true;
}
