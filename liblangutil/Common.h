// SPDX-License-Identifier: GPL-3.0

#pragma once

namespace solidity::langutil
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
