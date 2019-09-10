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

#pragma once

namespace langutil
{

inline bool isDecimalDigit(char c)
{
	return '0' <= c && c <= '9';
}

inline bool isHexDigit(char c)
{
	return
		isDecimalDigit(c) ||
		('a' <= c && c <= 'f') ||
		('A' <= c && c <= 'F');
}

inline bool isLineTerminator(char c)
{
	return c == '\n';
}

inline bool isWhiteSpace(char c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

inline bool isIdentifierStart(char c)
{
	return c == '_' || c == '$' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

inline bool isIdentifierPart(char c)
{
	return isIdentifierStart(c) || isDecimalDigit(c);
}

inline int hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else return -1;
}
}
