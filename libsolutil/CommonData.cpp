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
/** @file CommonData.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libsolutil/Assertions.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Exceptions.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>

using namespace solidity;
using namespace solidity::util;

namespace
{

static char const* upperHexChars = "0123456789ABCDEF";
static char const* lowerHexChars = "0123456789abcdef";

}

std::string solidity::util::toHex(uint8_t _data, HexCase _case)
{
	assertThrow(_case != HexCase::Mixed, BadHexCase, "Mixed case can only be used for byte arrays.");

	char const* chars = _case == HexCase::Upper ? upperHexChars : lowerHexChars;

	return std::string{
		chars[(unsigned(_data) / 16) & 0xf],
		chars[unsigned(_data) & 0xf]
	};
}

std::string solidity::util::toHex(bytes const& _data, HexPrefix _prefix, HexCase _case)
{
	std::string ret(_data.size() * 2 + (_prefix == HexPrefix::Add ? 2 : 0), 0);

	size_t i = 0;
	if (_prefix == HexPrefix::Add)
	{
		ret[i++] = '0';
		ret[i++] = 'x';
	}

	// Mixed case will be handled inside the loop.
	char const* chars = _case == HexCase::Upper ? upperHexChars : lowerHexChars;
	size_t rix = _data.size() - 1;
	for (uint8_t c: _data)
	{
		// switch hex case every four hexchars
		if (_case == HexCase::Mixed)
			chars = (rix-- & 2) == 0 ? lowerHexChars : upperHexChars;

		ret[i++] = chars[(static_cast<size_t>(c) >> 4ul) & 0xfu];
		ret[i++] = chars[c & 0xfu];
	}
	assertThrow(i == ret.size(), Exception, "");

	return ret;
}

int solidity::util::fromHex(char _i, WhenError _throw)
{
	if (_i >= '0' && _i <= '9')
		return _i - '0';
	if (_i >= 'a' && _i <= 'f')
		return _i - 'a' + 10;
	if (_i >= 'A' && _i <= 'F')
		return _i - 'A' + 10;
	if (_throw == WhenError::Throw)
		assertThrow(false, BadHexCharacter, std::to_string(_i));
	else
		return -1;
}

bytes solidity::util::fromHex(std::string const& _s, WhenError _throw)
{
	if (_s.empty())
		return {};

	unsigned s = (_s.size() >= 2 && _s[0] == '0' && _s[1] == 'x') ? 2 : 0;
	std::vector<uint8_t> ret;
	ret.reserve((_s.size() - s + 1) / 2);

	if (_s.size() % 2)
	{
		int h = fromHex(_s[s++], _throw);
		if (h != -1)
			ret.push_back(static_cast<uint8_t>(h));
		else
			return bytes();
	}
	for (unsigned i = s; i < _s.size(); i += 2)
	{
		int h = fromHex(_s[i], _throw);
		int l = fromHex(_s[i + 1], _throw);
		if (h != -1 && l != -1)
			ret.push_back(static_cast<uint8_t>(h * 16 + l));
		else
			return bytes();
	}
	return ret;
}


bool solidity::util::passesAddressChecksum(std::string const& _str, bool _strict)
{
	std::string s = _str.substr(0, 2) == "0x" ? _str : "0x" + _str;

	if (s.length() != 42)
		return false;

	if (!_strict && (
		s.find_first_of("abcdef") == std::string::npos ||
		s.find_first_of("ABCDEF") == std::string::npos
	))
		return true;

	return s == solidity::util::getChecksummedAddress(s);
}

std::string solidity::util::getChecksummedAddress(std::string const& _addr)
{
	std::string s = _addr.substr(0, 2) == "0x" ? _addr.substr(2) : _addr;
	assertThrow(s.length() == 40, InvalidAddress, "");
	assertThrow(s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos, InvalidAddress, "");

	h256 hash = keccak256(boost::algorithm::to_lower_copy(s, std::locale::classic()));

	std::string ret = "0x";
	for (unsigned i = 0; i < 40; ++i)
	{
		char addressCharacter = s[i];
		uint8_t nibble = hash[i / 2u] >> (4u * (1u - (i % 2u))) & 0xf;
		if (nibble >= 8)
			ret += toUpper(addressCharacter);
		else
			ret += toLower(addressCharacter);
	}
	return ret;
}

bool solidity::util::isValidHex(std::string const& _string)
{
	if (_string.substr(0, 2) != "0x")
		return false;
	if (_string.find_first_not_of("0123456789abcdefABCDEF", 2) != std::string::npos)
		return false;
	return true;
}

bool solidity::util::isValidDecimal(std::string const& _string)
{
	if (_string.empty())
		return false;
	if (_string == "0")
		return true;
	// No leading zeros
	if (_string.front() == '0')
		return false;
	if (_string.find_first_not_of("0123456789") != std::string::npos)
		return false;
	return true;
}

std::string solidity::util::formatAsStringOrNumber(std::string const& _value)
{
	assertThrow(_value.length() <= 32, StringTooLong, "String to be formatted longer than 32 bytes.");

	for (auto const& c: _value)
		if (c <= 0x1f || c >= 0x7f || c == '"')
			return "0x" + h256(_value, h256::AlignLeft).hex();

	return escapeAndQuoteString(_value);
}


std::string solidity::util::escapeAndQuoteString(std::string const& _input)
{
	std::string out;

	for (char c: _input)
		if (c == '\\')
			out += "\\\\";
		else if (c == '"')
			out += "\\\"";
		else if (c == '\n')
			out += "\\n";
		else if (c == '\r')
			out += "\\r";
		else if (c == '\t')
			out += "\\t";
		else if (!isPrint(c))
		{
			std::ostringstream o;
			o << "\\x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned)(unsigned char)(c);
			out += o.str();
		}
		else
			out += c;

	return "\"" + std::move(out) + "\"";
}
