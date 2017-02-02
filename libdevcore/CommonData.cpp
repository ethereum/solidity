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
/** @file CommonData.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libdevcore/CommonData.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/SHA3.h>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace dev;

std::string dev::escaped(std::string const& _s, bool _all)
{
	static const map<char, char> prettyEscapes{{'\r', 'r'}, {'\n', 'n'}, {'\t', 't'}, {'\v', 'v'}};
	std::string ret;
	ret.reserve(_s.size() + 2);
	ret.push_back('"');
	for (auto i: _s)
		if (i == '"' && !_all)
			ret += "\\\"";
		else if (i == '\\' && !_all)
			ret += "\\\\";
		else if (prettyEscapes.count(i) && !_all)
		{
			ret += '\\';
			ret += prettyEscapes.find(i)->second;
		}
		else if (i < ' ' || _all)
		{
			ret += "\\x";
			ret.push_back("0123456789abcdef"[(uint8_t)i / 16]);
			ret.push_back("0123456789abcdef"[(uint8_t)i % 16]);
		}
		else
			ret.push_back(i);
	ret.push_back('"');
	return ret;
}

int dev::fromHex(char _i, WhenError _throw)
{
	if (_i >= '0' && _i <= '9')
		return _i - '0';
	if (_i >= 'a' && _i <= 'f')
		return _i - 'a' + 10;
	if (_i >= 'A' && _i <= 'F')
		return _i - 'A' + 10;
	if (_throw == WhenError::Throw)
		BOOST_THROW_EXCEPTION(BadHexCharacter() << errinfo_invalidSymbol(_i));
	else
		return -1;
}

bytes dev::fromHex(std::string const& _s, WhenError _throw)
{
	unsigned s = (_s.size() >= 2 && _s[0] == '0' && _s[1] == 'x') ? 2 : 0;
	std::vector<uint8_t> ret;
	ret.reserve((_s.size() - s + 1) / 2);

	if (_s.size() % 2)
	{
		int h = fromHex(_s[s++], WhenError::DontThrow);
		if (h != -1)
			ret.push_back(h);
		else if (_throw == WhenError::Throw)
			BOOST_THROW_EXCEPTION(BadHexCharacter());
		else
			return bytes();
	}
	for (unsigned i = s; i < _s.size(); i += 2)
	{
		int h = fromHex(_s[i], WhenError::DontThrow);
		int l = fromHex(_s[i + 1], WhenError::DontThrow);
		if (h != -1 && l != -1)
			ret.push_back((byte)(h * 16 + l));
		else if (_throw == WhenError::Throw)
			BOOST_THROW_EXCEPTION(BadHexCharacter());
		else
			return bytes();
	}
	return ret;
}


bool dev::passesAddressChecksum(string const& _str, bool _strict)
{
	string s = _str.substr(0, 2) == "0x" ? _str.substr(2) : _str;

	if (s.length() != 40)
		return false;

	if (!_strict && (
		_str.find_first_of("abcdef") == string::npos ||
		_str.find_first_of("ABCDEF") == string::npos
	))
		return true;

	h256 hash = keccak256(boost::algorithm::to_lower_copy(s, std::locale::classic()));
	for (size_t i = 0; i < 40; ++i)
	{
		char addressCharacter = s[i];
		bool lowerCase;
		if ('a' <= addressCharacter && addressCharacter <= 'f')
			lowerCase = true;
		else if ('A' <= addressCharacter && addressCharacter <= 'F')
			lowerCase = false;
		else
			continue;
		unsigned nibble = (unsigned(hash[i / 2]) >> (4 * (1 - (i % 2)))) & 0xf;
		if ((nibble >= 8) == lowerCase)
			return false;
	}
	return true;
}
