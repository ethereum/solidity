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
/** @file StringUtils.h
 * @author Balajiganapathi S <balajiganapathi.s@gmail.com>
 * @date 2017
 *
 * String routines
 */

#pragma once

#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>

#include <fmt/format.h>

#include <algorithm>
#include <limits>
#include <locale>
#include <string>
#include <vector>

namespace solidity::util
{

// Calculates the Damerau–Levenshtein distance between _str1 and _str2 and returns true if that distance is not greater than _maxDistance
// if _lenThreshold > 0 and the product of the strings length is greater than _lenThreshold, the function will return false
bool stringWithinDistance(std::string const& _str1, std::string const& _str2, size_t _maxDistance, size_t _lenThreshold = 0);
// Calculates the Damerau–Levenshtein distance between _str1 and _str2
size_t stringDistance(std::string const& _str1, std::string const& _str2);
// Return a string having elements of suggestions as quoted, alternative suggestions. e.g. "a", "b" or "c"
std::string quotedAlternativesList(std::vector<std::string> const& suggestions);

/// @returns a string containing a comma-separated list of variable names consisting of @a _baseName suffixed
/// with increasing integers in the range [@a _startSuffix, @a _endSuffix), if @a _startSuffix < @a _endSuffix,
/// and with decreasing integers in the range [@a _endSuffix, @a _startSuffix), if @a _endSuffix < @a _startSuffix.
/// If @a _startSuffix == @a _endSuffix, the empty string is returned.
std::string suffixedVariableNameList(std::string const& _baseName, size_t _startSuffix, size_t _endSuffix);

/// Joins collection of strings into one string with separators between, last separator can be different.
/// @param _list collection of strings to join
/// @param _separator defaults to ", "
/// @param _lastSeparator (optional) will be used to separate last two strings instead of _separator
/// @example join(vector<string>{"a", "b", "c"}, "; ", " or ") == "a; b or c"
template<class T>
std::string joinHumanReadable
(
	T const& _list,
	std::string const& _separator = ", ",
	std::string const& _lastSeparator = ""
)
{
	auto const itEnd = end(_list);

	std::string result;

	for (auto it = begin(_list); it != itEnd; )
	{
		std::string element = *it;
		bool first = (it == begin(_list));
		++it;
		if (!first)
		{
			if (it == itEnd && !_lastSeparator.empty())
				result += _lastSeparator; // last iteration
			else
				result += _separator;
		}
		result += std::move(element);
	}

	return result;
}

/// Joins collection of strings just like joinHumanReadable, but prepends the separator
/// unless the collection is empty.
template<class T>
std::string joinHumanReadablePrefixed
(
	T const& _list,
	std::string const& _separator = ", ",
	std::string const& _lastSeparator = ""
)
{
	if (begin(_list) == end(_list))
		return {};
	else
		return _separator + joinHumanReadable(_list, _separator, _lastSeparator);
}

/// Formats large numbers to be easily readable by humans.
/// Returns decimal representation for smaller numbers; hex for large numbers.
/// "Special" numbers, powers-of-two and powers-of-two minus 1, are returned in
/// formulaic form like 0x01 * 2**24 - 1.
/// @a T can be any integer type, will typically be u160, u256 or bigint.
/// @param _value to be formatted
/// @param _useTruncation if true, internal truncation is also applied,
/// like  0x5555...{+56 more}...5555
/// @example formatNumberReadable((u256)0x7ffffff) = "2**27 - 1"
/// @example formatNumberReadable(-57896044618658097711785492504343953926634992332820282019728792003956564819968) = -2**255
std::string formatNumberReadable(bigint const& _value, bool _useTruncation = false);

/// Safely converts an unsigned integer as string into an unsigned int type.
///
/// @return the converted number or nullopt in case of an failure (including if it would not fit).
inline std::optional<unsigned> toUnsignedInt(std::string const& _value)
{
	try
	{
		auto const ulong = stoul(_value);
		if (ulong > std::numeric_limits<unsigned>::max())
			return std::nullopt;
		return static_cast<unsigned>(ulong);
	}
	catch (...)
	{
		return std::nullopt;
	}
}

/// Converts parameter _c to its lowercase equivalent if c is an uppercase letter and has a lowercase equivalent. It uses the classic "C" locale semantics.
/// @param _c value to be converted
/// @return the converted value
inline char toLower(char _c)
{
	return tolower(_c, std::locale::classic());
}

/// Converts parameter _c to its uppercase equivalent if c is an lowercase letter and has a uppercase equivalent. It uses the classic "C" locale semantics.
/// @param _c value to be converted
/// @return the converted value
inline char toUpper(char _c)
{
	return toupper(_c, std::locale::classic());
}

/// Converts parameter _s to its lowercase equivalent. It uses the classic "C" locale semantics.
/// @param _s value to be converted
/// @return the converted value
inline std::string toLower(std::string _s)
{
	std::transform(_s.begin(), _s.end(), _s.begin(), [](char _c) {
		return toLower(_c);
	});
	return _s;
}

/// Checks whether _c is a decimal digit character. It uses the classic "C" locale semantics.
/// @param _c character to be checked
/// @return true if _c is a decimal digit character, false otherwise
inline bool isDigit(char _c)
{
	return isdigit(_c, std::locale::classic());
}

/// Checks if character is printable using classic "C" locale
/// @param _c character to be checked
/// @return true if _c is a printable character, false otherwise.
inline bool isPrint(char _c)
{
	return isprint(_c, std::locale::classic());
}

/// Adds a prefix to every line in the input.
/// @see printPrefixed()
std::string prefixLines(
	std::string const& _input,
	std::string const& _prefix,
	bool _trimPrefix = true
);

/// Prints to a stream, adding a prefix to every line in the input.
/// Assumes \n as the line separator.
/// @param _trimPrefix If true, the function avoids introducing trailing whitespace on empty lines.
///     This is achieved by removing trailing spaces from the prefix on such lines.
///     Note that tabs and newlines are not removed, only spaces are.
/// @param _finalNewline If true, an extra \n will be printed at the end of @a _input if it does
///     not already end with one.
void printPrefixed(
	std::ostream& _output,
	std::string const& _input,
	std::string const& _prefix,
	bool _trimPrefix = true,
	bool _ensureFinalNewline = true
);

/// Adds a standard indent of 4 spaces to every line in the input.
/// Assumes \n as the line separator.
/// @param _indentEmptyLines If true, the indent will be applied to empty lines as well, resulting
///     such lines containing trailing whitespace.
inline std::string indent(std::string const& _input, bool _indentEmptyLines = false)
{
	return prefixLines(_input, "    ", !_indentEmptyLines);
}

}
