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
/** @file StringUtils.h
 * @author Balajiganapathi S <balajiganapathi.s@gmail.com>
 * @date 2017
 *
 * String routines
 */

#pragma once

#include <string>
#include <vector>

#include <libdevcore/CommonData.h>

namespace dev
{

// Calculates the Damerau–Levenshtein distance between _str1 and _str2 and returns true if that distance is not greater than _maxDistance
// if _lenThreshold > 0 and the product of the strings length is greater than _lenThreshold, the function will return false
bool stringWithinDistance(std::string const& _str1, std::string const& _str2, size_t _maxDistance, size_t _lenThreshold = 0);
// Calculates the Damerau–Levenshtein distance between _str1 and _str2
size_t stringDistance(std::string const& _str1, std::string const& _str2);
// Return a string having elements of suggestions as quoted, alternative suggestions. e.g. "a", "b" or "c"
std::string quotedAlternativesList(std::vector<std::string> const& suggestions);

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
/// @a T will typically by unsigned, u160, u256 or bigint.
/// @param _value to be formatted
/// @param _useTruncation if true, internal truncation is also applied,
/// like  0x5555...{+56 more}...5555
/// @example formatNumber((u256)0x7ffffff)
template <class T>
inline std::string formatNumberReadable(
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
		int len = str.size();

		if (len < 24)
			return str;

		int const initialChars = (prefix == HexPrefix::Add) ? 6 : 4;
		int const finalChars = 4;
		int numSkipped = len - initialChars - finalChars;

		return str.substr(0, initialChars) +
			"...{+" +
			std::to_string(numSkipped) +
			" more}..." +
			str.substr(len-finalChars, len);
	}

	// otherwise, show whole value.
	return str;
}

}
