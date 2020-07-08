// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity scanner.
 */

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;

char CharStream::advanceAndGet(size_t _chars)
{
	if (isPastEndOfInput())
		return 0;
	m_position += _chars;
	if (isPastEndOfInput())
		return 0;
	return m_source[m_position];
}

char CharStream::rollback(size_t _amount)
{
	solAssert(m_position >= _amount, "");
	m_position -= _amount;
	return get();
}

char CharStream::setPosition(size_t _location)
{
	solAssert(_location <= m_source.size(), "Attempting to set position past end of source.");
	m_position = _location;
	return get();
}

string CharStream::lineAtPosition(int _position) const
{
	// if _position points to \n, it returns the line before the \n
	using size_type = string::size_type;
	size_type searchStart = min<size_type>(m_source.size(), size_type(_position));
	if (searchStart > 0)
		searchStart--;
	size_type lineStart = m_source.rfind('\n', searchStart);
	if (lineStart == string::npos)
		lineStart = 0;
	else
		lineStart++;
	string line = m_source.substr(
		lineStart,
		min(m_source.find('\n', lineStart), m_source.size()) - lineStart
	);
	if (!line.empty() && line.back() == '\r')
		line.pop_back();
	return line;
}

tuple<int, int> CharStream::translatePositionToLineColumn(int _position) const
{
	using size_type = string::size_type;
	using diff_type = string::difference_type;
	size_type searchPosition = min<size_type>(m_source.size(), size_type(_position));
	int lineNumber = static_cast<int>(count(m_source.begin(), m_source.begin() + diff_type(searchPosition), '\n'));
	size_type lineStart;
	if (searchPosition == 0)
		lineStart = 0;
	else
	{
		lineStart = m_source.rfind('\n', searchPosition - 1);
		lineStart = lineStart == string::npos ? 0 : lineStart + 1;
	}
	return tuple<int, int>(lineNumber, searchPosition - lineStart);
}
