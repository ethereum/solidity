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

#include <test/libsolidity/util/ContractABIUtils.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <liblangutil/Common.h>

#include <libdevcore/StringUtils.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <iomanip>
#include <memory>
#include <regex>
#include <stdexcept>

using namespace dev;
using namespace langutil;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;
using namespace soltest;

bytes BytesUtils::alignLeft(bytes _bytes)
{
	soltestAssert(_bytes.size() <= 32, "");
	size_t size = _bytes.size();
	return std::move(_bytes) + bytes(32 - size, 0);
}

bytes BytesUtils::alignRight(bytes _bytes)
{
	soltestAssert(_bytes.size() <= 32, "");
	return bytes(32 - _bytes.size(), 0) + std::move(_bytes);
}

bytes BytesUtils::applyAlign(
	Parameter::Alignment _alignment,
	ABIType& _abiType,
	bytes _bytes
)
{
	if (_alignment != Parameter::Alignment::None)
		_abiType.alignDeclared = true;

	switch (_alignment)
	{
	case Parameter::Alignment::Left:
		_abiType.align = ABIType::AlignLeft;
		return alignLeft(std::move(_bytes));
	case Parameter::Alignment::Right:
	default:
		_abiType.align = ABIType::AlignRight;
		return alignRight(std::move(_bytes));
	}
}

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

string BytesUtils::formatUnsigned(bytes const& _bytes)
{
	stringstream os;

	soltestAssert(!_bytes.empty() && _bytes.size() <= 32, "");

	return fromBigEndian<u256>(_bytes).str();
}

string BytesUtils::formatSigned(bytes const& _bytes)
{
	stringstream os;

	soltestAssert(!_bytes.empty() && _bytes.size() <= 32, "");

	if (*_bytes.begin() & 0x80)
		os << u2s(fromBigEndian<u256>(_bytes));
	else
		os << fromBigEndian<u256>(_bytes);

	return os.str();
}

string BytesUtils::formatBoolean(bytes const& _bytes)
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

string BytesUtils::formatHex(bytes const& _bytes)
{
	soltestAssert(!_bytes.empty() && _bytes.size() <= 32, "");
	u256 value = fromBigEndian<u256>(_bytes);

	return toCompactHexWithPrefix(value);
}

string BytesUtils::formatHexString(bytes const& _bytes)
{
	stringstream os;

	os << "hex\"" << toHex(_bytes) << "\"";

	return os.str();
}

string BytesUtils::formatString(bytes const& _bytes, size_t _cutOff)
{
	stringstream os;

	os << "\"";
	for (size_t i = 0; i < min(_cutOff, _bytes.size()); ++i)
	{
		auto const v = _bytes[i];
		switch (v)
		{
			case '\0':
				os << "\\0";
				break;
			case '\n':
				os << "\\n";
				break;
			default:
				if (isprint(v))
					os << v;
				else
					os << "\\x" << setw(2) << setfill('0') << hex << v;

		}
	}
	os << "\"";

	return os.str();
}

string BytesUtils::formatRawBytes(
	bytes const& _bytes,
	dev::solidity::test::ParameterList const& _parameters,
	string _linePrefix)
{
	stringstream os;
	ParameterList parameters;
	auto it = _bytes.begin();

	if (_bytes.size() != ContractABIUtils::encodingSize(_parameters))
		parameters = ContractABIUtils::defaultParameters(ceil(_bytes.size() / 32));
	else
		parameters = _parameters;

	for (auto const& parameter: parameters)
	{
		bytes byteRange{it, it + static_cast<long>(parameter.abiType.size)};

		os << _linePrefix << byteRange;
		if (&parameter != &parameters.back())
			os << endl;

		it += static_cast<long>(parameter.abiType.size);
	}

	return os.str();
}

string BytesUtils::formatBytes(
	bytes const& _bytes,
	ABIType const& _abiType
)
{
	stringstream os;

	switch (_abiType.type)
	{
	case ABIType::UnsignedDec:
		// Check if the detected type was wrong and if this could
		// be signed. If an unsigned was detected in the expectations,
		// but the actual result returned a signed, it would be formatted
		// incorrectly.
		if (*_bytes.begin() & 0x80)
			os << formatSigned(_bytes);
		else
			os << formatUnsigned(_bytes);
		break;
	case ABIType::SignedDec:
		os << formatSigned(_bytes);
		break;
	case ABIType::Boolean:
		os << formatBoolean(_bytes);
		break;
	case ABIType::Hex:
		os << formatHex(_bytes);
		break;
	case ABIType::HexString:
		os << formatHexString(_bytes);
		break;
	case ABIType::String:
		os << formatString(_bytes, _bytes.size() - countRightPaddedZeros(_bytes));
		break;
	case ABIType::Failure:
		break;
	case ABIType::None:
		break;
	}
	return os.str();
}

string BytesUtils::formatBytesRange(
	bytes _bytes,
	dev::solidity::test::ParameterList const& _parameters,
	bool _highlight
)
{
	stringstream os;
	ParameterList parameters;
	auto it = _bytes.begin();

	if (_bytes.size() != ContractABIUtils::encodingSize(_parameters))
		parameters = ContractABIUtils::defaultParameters(ceil(_bytes.size() / 32));
	else
		parameters = _parameters;


	for (auto const& parameter: parameters)
	{
		bytes byteRange{it, it + static_cast<long>(parameter.abiType.size)};

		if (!parameter.matchesBytes(byteRange))
			AnsiColorized(
				os,
				_highlight,
				{dev::formatting::RED_BACKGROUND}
			) << formatBytes(byteRange, parameter.abiType);
		else
			os << parameter.rawString;

		if (&parameter != &parameters.back())
			os << ", ";

		it += static_cast<long>(parameter.abiType.size);
	}

	return os.str();
}

size_t BytesUtils::countRightPaddedZeros(bytes const& _bytes)
{
	return find_if(
		_bytes.rbegin(),
		_bytes.rend(),
		[](uint8_t b) { return b != '\0'; }
	) - _bytes.rbegin();
}

