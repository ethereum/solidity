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

#include <test/libsolidity/util/BytesUtils.h>

#include <liblangutil/Common.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <memory>
#include <regex>
#include <stdexcept>

using namespace dev;
using namespace langutil;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;
using namespace soltest;

bytes BytesUtils::convertBoolean(string const& _literal)
{
	if (_literal == "true")
		return bytes{true};
	else if (_literal == "false")
		return bytes{false};
	else
		throw Error(Error::Type::ParserError, "Boolean literal invalid.");
}

bytes BytesUtils::convertNumber(string const& _literal)
{
	try
	{
		return toCompactBigEndian(u256{_literal});
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Number encoding invalid.");
	}
}

bytes BytesUtils::convertHexNumber(string const& _literal)
{
	try
	{
		if (_literal.size() % 2)
			throw Error(Error::Type::ParserError, "Hex number encoding invalid.");
		else
			return fromHex(_literal);
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Hex number encoding invalid.");
	}
}

bytes BytesUtils::convertString(string const& _literal)
{
	try
	{
		return asBytes(_literal);
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "String encoding invalid.");
	}
}

string BytesUtils::formatUnsigned(bytes const& _bytes) const
{
	stringstream os;

	if (*_bytes.begin() & 0x80)
		os << u2s(fromBigEndian<u256>(_bytes));
	else
		os << fromBigEndian<u256>(_bytes);

	return os.str();
}

string BytesUtils::formatSigned(bytes const& _bytes) const
{
	stringstream os;

	if (*_bytes.begin() & 0x80)
		os << u2s(fromBigEndian<u256>(_bytes));
	else
		os << fromBigEndian<u256>(_bytes);

	return os.str();
}

string BytesUtils::formatBoolean(bytes const& _bytes) const
{
	stringstream os;
	u256 result = fromBigEndian<u256>(_bytes);

	if (result == 0)
		os << "false";
	else if (result == 1)
		os << "true";
	else
		os << result;

	return os.str();
}

string BytesUtils::formatHex(bytes const& _bytes) const
{
	stringstream os;

	string hex{toHex(_bytes, HexPrefix::Add)};
	boost::algorithm::replace_all(hex, "00", "");
	os << hex;

	return os.str();
}

string BytesUtils::formatHexString(bytes const& _bytes) const
{
	stringstream os;

	os << "hex\"" << toHex(_bytes) << "\"";

	return os.str();
}

string BytesUtils::formatString(bytes const& _bytes) const
{
	stringstream os;

	os << "\"";
	bool expectZeros = false;
	for (auto const& v: _bytes)
	{
		if (expectZeros && v != 0)
			return {};
		if (v == 0) expectZeros = true;
		else
		{
			if (!isprint(v) || v == '"')
				return {};
			os << v;
		}
	}
	os << "\"";

	return os.str();
}

bytes BytesUtils::alignLeft(bytes _bytes) const
{
	return std::move(_bytes) + bytes(32 - _bytes.size(), 0);
}

bytes BytesUtils::alignRight(bytes _bytes) const
{
	return bytes(32 - _bytes.size(), 0) + std::move(_bytes);
}

bytes BytesUtils::applyAlign(
	Parameter::Alignment _alignment,
	ABIType& _abiType,
	bytes _bytes
) const
{
	if (_alignment != Parameter::Alignment::None)
		_abiType.alignDeclared = true;

	switch (_alignment)
	{
	case Parameter::Alignment::Left:
		_abiType.align = ABIType::AlignLeft;
		return alignLeft(std::move(_bytes));
	case Parameter::Alignment::Right:
		_abiType.align = ABIType::AlignRight;
		return alignRight(std::move(_bytes));
	default:
		_abiType.align = ABIType::AlignRight;
		return alignRight(std::move(_bytes));
	}
}
