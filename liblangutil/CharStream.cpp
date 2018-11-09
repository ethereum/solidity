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

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

using namespace std;
using namespace dev;
using namespace langutil;

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

string CharStream::lineAtPosition(int _position) const
{
	// if _position points to \n, it returns the line before the \n
	using size_type = string::size_type;
	size_type searchStart = min<size_type>(m_source.size(), _position);
	if (searchStart > 0)
		searchStart--;
	size_type lineStart = m_source.rfind('\n', searchStart);
	if (lineStart == string::npos)
		lineStart = 0;
	else
		lineStart++;
	return m_source.substr(lineStart, min(m_source.find('\n', lineStart),
										  m_source.size()) - lineStart);
}

tuple<int, int> CharStream::translatePositionToLineColumn(int _position) const
{
	using size_type = string::size_type;
	size_type searchPosition = min<size_type>(m_source.size(), _position);
	int lineNumber = count(m_source.begin(), m_source.begin() + searchPosition, '\n');
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


