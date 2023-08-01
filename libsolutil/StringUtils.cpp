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

#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string/trim.hpp>

#include <sstream>
#include <string>
#include <vector>

using namespace solidity;
using namespace solidity::util;

bool solidity::util::stringWithinDistance(std::string const& _str1, std::string const& _str2, size_t _maxDistance, size_t _lenThreshold)
{
	if (_str1 == _str2)
		return true;

	size_t n1 = _str1.size();
	size_t n2 = _str2.size();
	if (_lenThreshold > 0 && n1 * n2 > _lenThreshold)
		return false;

	size_t distance = stringDistance(_str1, _str2);

	// if distance is not greater than _maxDistance, and distance is strictly less than length of both names, they can be considered similar
	// this is to avoid irrelevant suggestions
	return distance <= _maxDistance && distance < n1 && distance < n2;
}

size_t solidity::util::stringDistance(std::string const& _str1, std::string const& _str2)
{
	size_t n1 = _str1.size();
	size_t n2 = _str2.size();
	// Optimize by storing only last 2 rows and current row. So first index is considered modulo 3
	// This is a two-dimensional array of size 3 x (n2 + 1).
	std::vector<size_t> dp(3 * (n2 + 1));

	// In this dp formulation of Damerau–Levenshtein distance we are assuming that the strings are 1-based to make base case storage easier.
	// So index accesser to _name1 and _name2 have to be adjusted accordingly
	for (size_t i1 = 0; i1 <= n1; ++i1)
		for (size_t i2 = 0; i2 <= n2; ++i2)
		{
			size_t x = 0;
			if (std::min(i1, i2) == 0) // base case
				x = std::max(i1, i2);
			else
			{
				size_t left = dp[(i1 - 1) % 3 + i2 * 3];
				size_t up = dp[(i1 % 3) + (i2 - 1) * 3];
				size_t upleft = dp[((i1 - 1) % 3) + (i2 - 1) * 3];
				// deletion and insertion
				x = std::min(left + 1, up + 1);
				if (_str1[i1-1] == _str2[i2-1])
					// same chars, can skip
					x = std::min(x, upleft);
				else
					// different chars so try substitution
					x = std::min(x, upleft + 1);

				// transposing
				if (i1 > 1 && i2 > 1 && _str1[i1 - 1] == _str2[i2 - 2] && _str1[i1 - 2] == _str2[i2 - 1])
					x = std::min(x, dp[((i1 - 2) % 3) + (i2 - 2) * 3] + 1);
			}
			dp[(i1 % 3) + i2 * 3] = x;
		}

	return dp[(n1 % 3) + n2 * 3];
}

std::string solidity::util::quotedAlternativesList(std::vector<std::string> const& suggestions)
{
	std::vector<std::string> quotedSuggestions;

	for (auto& suggestion: suggestions)
		quotedSuggestions.emplace_back("\"" + suggestion + "\"");

	return joinHumanReadable(quotedSuggestions, ", ", " or ");
}

std::string solidity::util::suffixedVariableNameList(std::string const& _baseName, size_t _startSuffix, size_t _endSuffix)
{
	std::string result;
	if (_startSuffix < _endSuffix)
	{
		result = _baseName + std::to_string(_startSuffix++);
		while (_startSuffix < _endSuffix)
			result += ", " + _baseName + std::to_string(_startSuffix++);
	}
	else if (_endSuffix < _startSuffix)
	{
		result = _baseName + std::to_string(_endSuffix++);
		while (_endSuffix < _startSuffix)
			result = _baseName + std::to_string(_endSuffix++) + ", " + result;
	}
	return result;
}

namespace
{

/// Try to format as N * 2**x
std::optional<std::string> tryFormatPowerOfTwo(bigint const& _value)
{
	bigint prefix = _value;

	// when multiple trailing zero bytes, format as N * 2**x
	int i = 0;
	for (; (prefix & 0xff) == 0; prefix >>= 8)
		++i;
	if (i <= 2)
		return std::nullopt;

	// 0x100 yields 2**8 (N is 1 and redundant)
	if (prefix == 1)
		return {fmt::format("2**{}", i * 8)};
	else if ((prefix & (prefix - 1)) == 0)
	{
		int j = 0;
		for (; (prefix & 0x1) == 0; prefix >>= 1)
			j++;
		return {fmt::format("2**{}", i * 8 + j)};
	}
	else
		return {fmt::format(
			"{} * 2**{}",
			toHex(toCompactBigEndian(prefix), HexPrefix::Add, HexCase::Mixed),
			i * 8
		)};
}

}

std::string solidity::util::formatNumberReadable(bigint const& _value, bool _useTruncation)
{
	bool const isNegative = _value < 0;
	bigint const absValue = isNegative ? (bigint(-1) * _value) : bigint(_value);
	std::string const sign = isNegative ? "-" : "";

	// smaller numbers return as decimal
	if (absValue <= 0x1000000)
		return sign + absValue.str();

	if (auto result = tryFormatPowerOfTwo(absValue))
		return {sign + *result};
	else if (auto result = tryFormatPowerOfTwo(absValue + 1))
		return {sign + *result + (isNegative ? " + 1" : " - 1")};

	std::string str = toHex(toCompactBigEndian(absValue), HexPrefix::Add, HexCase::Mixed);

	if (_useTruncation)
	{
		// return as interior-truncated hex.
		size_t len = str.size();

		if (len < 24)
			return sign + str;

		size_t const initialChars = 6;
		size_t const finalChars = 4;
		size_t numSkipped = len - initialChars - finalChars;

		return fmt::format(
			"{}{}...{{+{} more}}...{}",
			sign,
			str.substr(0, initialChars),
			numSkipped,
			str.substr(len-finalChars, len)
		);
	}

	return sign + str;
}

std::string solidity::util::prefixLines(
	std::string const& _input,
	std::string const& _prefix,
	bool _trimPrefix
)
{
	std::ostringstream output;
	printPrefixed(output, _input, _prefix, _trimPrefix, false /* _ensureFinalNewline */);
	return output.str();
}

void solidity::util::printPrefixed(
	std::ostream& _output,
	std::string const& _input,
	std::string const& _prefix,
	bool _trimPrefix,
	bool _ensureFinalNewline
)
{
	std::istringstream input(_input);
	std::string line;
	while (std::getline(input, line))
	{
		if (line.empty() && _trimPrefix)
			_output << boost::trim_right_copy(_prefix);
		else
			_output << _prefix << line;
		if (!input.eof() || _ensureFinalNewline)
			_output << '\n';
	}
}
