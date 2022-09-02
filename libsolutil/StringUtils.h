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

/// Decodes a URI with respect to %XX notation.
/// No URI-validity verification is performed but simply the URI decoded into non-escaping characters.
std::string decodeURI(std::string const& _uri);

/// Encodes a string into a URI conform notation.
std::string encodeURI(std::string const& _uri);

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

/// Same as @ref formatNumberReadable but only for unsigned numbers
template <class T>
inline std::string formatUnsignedNumberReadable (
	T const& _value,
	bool _useTruncation = false
)
{
	static_assert(
		std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed,
		"only unsigned types or bigint supported"
	); //bigint does not carry sign bit on shift

	// smaller numbers return as decimal
	if (_value <= 0x1000000)
		return _value.str();

	HexCase hexcase = HexCase::Mixed;
	HexPrefix prefix = HexPrefix::Add;

	// when multiple trailing zero bytes, format as N * 2**x
	int i = 0;
	T v = _value;
	for (; (v & 0xff) == 0; v >>= 8)
		++i;
	if (i > 2)
	{
		// 0x100 yields 2**8 (N is 1 and redundant)
		if (v == 1)
			return "2**" + std::to_string(i * 8);
		return toHex(toCompactBigEndian(v), prefix, hexcase) +
			" * 2**" +
			std::to_string(i * 8);
	}

	// when multiple trailing FF bytes, format as N * 2**x - 1
	i = 0;
	for (v = _value; (v & 0xff) == 0xff; v >>= 8)
		++i;
	if (i > 2)
	{
		// 0xFF yields 2**8 - 1 (v is 0 in that case)
		if (v == 0)
			return "2**" + std::to_string(i * 8) + " - 1";
		return toHex(toCompactBigEndian(T(v + 1)), prefix, hexcase) +
			" * 2**" + std::to_string(i * 8) +
			" - 1";
	}

	std::string str = toHex(toCompactBigEndian(_value), prefix, hexcase);
	if (_useTruncation)
	{
		// return as interior-truncated hex.
		size_t len = str.size();

		if (len < 24)
			return str;

		size_t const initialChars = (prefix == HexPrefix::Add) ? 6 : 4;
		size_t const finalChars = 4;
		size_t numSkipped = len - initialChars - finalChars;

		return str.substr(0, initialChars) +
			"...{+" +
			std::to_string(numSkipped) +
			" more}..." +
			str.substr(len-finalChars, len);
	}

	// otherwise, show whole value.
	return str;
}

/// Formats large numbers to be easily readable by humans.
/// Returns decimal representation for smaller numbers; hex for large numbers.
/// "Special" numbers, powers-of-two and powers-of-two minus 1, are returned in
/// formulaic form like 0x01 * 2**24 - 1.
/// @a T can be any integer variable, will typically be u160, u256 or bigint.
/// @param _value to be formatted
/// @param _useTruncation if true, internal truncation is also applied,
/// like  0x5555...{+56 more}...5555
/// @example formatNumberReadable((u256)0x7ffffff) = "0x08 * 2**24"
/// @example formatNumberReadable(-57896044618658097711785492504343953926634992332820282019728792003956564819968) = -0x80 * 2**248
template <class T>
inline std::string formatNumberReadable(
	T const& _value,
	bool _useTruncation = false
)
{
	static_assert(
		std::numeric_limits<T>::is_integer,
		"only integer numbers are supported"
	);

	if (_value >= 0)
	{
		bigint const _v = bigint(_value);
		return formatUnsignedNumberReadable(_v, _useTruncation);
	}
	else
	{
		bigint const _abs_value = bigint(-1) * _value;
		return "-" + formatUnsignedNumberReadable(_abs_value, _useTruncation);
	}
}

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

// Checks if character is printable using classic "C" locale
/// @param _c character to be checked
/// @return true if _c is a printable character, false otherwise.
inline bool isPrint(char _c)
{
	return isprint(_c, std::locale::classic());
}

}
