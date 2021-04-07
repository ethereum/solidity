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

#include <test/tools/ossfuzz/ValueGenerator.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Keccak256.h>

#include <boost/preprocessor.hpp>

#include <regex>
#include <iostream>

using namespace std;

/// Convenience macros
/// Returns a valid Solidity integer width w such that 8 <= w <= 256.
#define INTWIDTH(z, n, _ununsed) BOOST_PP_MUL(BOOST_PP_ADD(n, 1), 8)
/// Using declaration that aliases long boost multiprecision types with
/// s(u)<width> where <width> is a valid Solidity integer width and "s"
/// stands for "signed" and "u" for "unsigned".
#define USINGDECL(z, n, sign) \
	using BOOST_PP_CAT(BOOST_PP_IF(sign, s, u), INTWIDTH(z, n,)) =             \
	boost::multiprecision::number<                                             \
		boost::multiprecision::cpp_int_backend<                                \
			INTWIDTH(z, n,),                                                   \
			INTWIDTH(z, n,),                                                   \
			BOOST_PP_IF(                                                       \
				sign,                                                          \
				boost::multiprecision::signed_magnitude,                       \
				boost::multiprecision::unsigned_magnitude                      \
			),                                                                 \
			boost::multiprecision::unchecked,                                  \
			void                                                               \
		>                                                                      \
	>;
/// Instantiate the using declarations for signed and unsigned integer types.
BOOST_PP_REPEAT(32, USINGDECL, 1)
BOOST_PP_REPEAT(32, USINGDECL, 0)
/// Case implementation that returns an integer value of the specified type.
/// For signed integers, we divide by two because the range for boost multiprecision
/// types is double that of Solidity integer types. Example, 8-bit signed boost
/// number range is [-255, 255] but Solidity `int8` range is [-128, 127]
#define CASEIMPL(z, n, sign)                                                   \
	case INTWIDTH(z, n,):                                                      \
		stream << BOOST_PP_IF(                                                 \
			sign,                                                              \
			integerValue<                                                      \
				BOOST_PP_CAT(                                                  \
					BOOST_PP_IF(sign, s, u),                                   \
					INTWIDTH(z, n,)                                            \
                )>(_counter) / 2,                                              \
			integerValue<                                                      \
				BOOST_PP_CAT(                                                  \
					BOOST_PP_IF(sign, s, u),                                   \
					INTWIDTH(z, n,)                                            \
                )>(_counter)                                                   \
        );                                                                     \
		break;
/// Switch implementation that instantiates case statements for (un)signed
/// Solidity integer types.
#define SWITCHIMPL(sign)                                                       \
	ostringstream stream;                                                      \
	switch (_intWidth)                                                         \
	{                                                                          \
	BOOST_PP_REPEAT(32, CASEIMPL, sign)	                                       \
	}	                                                                       \
	return stream.str();

namespace
{
template<typename V>
V integerValue(size_t _counter)
{
	V value = V(
		u256(solidity::util::keccak256(solidity::util::h256(_counter))) %
		u256(boost::math::tools::max_value<V>())
	);
	if (boost::multiprecision::is_signed_number<V>::value && value % 2 == 0)
		return value * (-1);
	else
		return value;
}

string signedIntegerValue(size_t _counter, size_t _intWidth)
{
	SWITCHIMPL(1)
}

string unsignedIntegerValue(size_t _counter, size_t _intWidth)
{
	SWITCHIMPL(0)
}

string integerValue(size_t _counter, size_t _intWidth, bool _signed)
{
	if (_signed)
		return signedIntegerValue(_counter, _intWidth);
	else
		return unsignedIntegerValue(_counter, _intWidth);
}

string fixedBytes(
	size_t _numBytes,
	size_t _counter,
	bool _isHexLiteral
)
{
	solAssert(
		_numBytes > 0 && _numBytes <= 32,
		"Proto ABIv2 fuzzer: Too short or too long a cropped string"
	);

	// Number of masked nibbles is twice the number of bytes for a
	// hex literal of _numBytes bytes. For a string literal, each nibble
	// is treated as a character.
	size_t numMaskNibbles = _isHexLiteral ? _numBytes * 2 : _numBytes;

	// Start position of substring equals totalHexStringLength - numMaskNibbles
	// totalHexStringLength = 64 + 2 = 66
	// e.g., 0x12345678901234567890123456789012 is a total of 66 characters
	//      |---------------------^-----------|
	//      <--- start position---><--numMask->
	//      <-----------total length --------->
	// Note: This assumes that maskUnsignedIntToHex() invokes toHex(..., HexPrefix::Add)
	size_t startPos = 66 - numMaskNibbles;
	// Extracts the least significant numMaskNibbles from the result
	// of maskUnsignedIntToHex().
	return solidity::util::toHex(
		u256(solidity::util::keccak256(solidity::util::h256(_counter))) &
		u256("0x" + std::string(numMaskNibbles, 'f')),
		solidity::util::HexPrefix::Add
	).substr(startPos, numMaskNibbles);
}
}

std::string ValueGenerator::addressLiteral()
{
	auto iter = m_addressSelector.begin();
	std::uniform_int_distribution<size_t> dist(0, m_addressSelector.size() - 1);
	std::advance(iter, dist(m_rand));
	return "0x" + iter->first.hex();
}

std::string ValueGenerator::functionLiteral()
{
	std::string contractAddress;
	std::string functionSelector;
	for (auto const& item: m_addressSelector)
		if (!item.second.empty())
		{
			contractAddress = "0x" + item.first.hex();
			auto selectors = item.second;
			std::uniform_int_distribution<size_t> dist(0, selectors.size() - 1);
			functionSelector = "0x" + selectors[dist(m_rand)];
			return contractAddress + ":" + functionSelector;
		}
	// If no function found, simply output a (valid address, invalid function) pair.
	return addressLiteral() +
		":" +
		"0x" +
		fixedBytes(static_cast<size_t>(FixedBytesWidth::Bytes4), m_rand(), true);
}

void ValueGenerator::initialiseType(TypeInfo& _t)
{
	switch (_t.type)
	{
	case Type::Boolean:
		_t.value += m_bernoulli(m_rand) ? "true" : "false";
		break;
	case Type::Integer:
		_t.value += integerValue(m_rand(), static_cast<size_t>(_t.intType.width), true);
		break;
	case Type::UInteger:
		_t.value += integerValue(m_rand(), static_cast<size_t>(_t.intType.width), false);
		break;
	case Type::String:
		_t.value += "0xdeadbeef";
		break;
	case Type::Bytes:
	{
		// Bytes can contain between 1--32 bytes.
		size_t bytesWidth = (m_rand() % 32) + 1;
		_t.value += "0x" + fixedBytes(bytesWidth, m_rand(), true);
		break;
	}
	case Type::FixedBytes:
		_t.value += "0x" + fixedBytes(static_cast<size_t>(_t.fixedByteWidth), m_rand(), true);
		break;
	case Type::Address:
		_t.value += addressLiteral();
		break;
	case Type::Function:
		_t.value += functionLiteral();
		break;
	default:
		solAssert(false, "Value Generator: Invalid value type.");
	}
}

void ValueGenerator::initialiseTuple(TypeInfo& _tuple)
{
	_tuple.value += "(";
	std::string separator;
	for (auto& c: _tuple.tupleInfo)
	{
		_tuple.value += separator + c.value;
//		if (c.arrayInfo.empty())
//		{
//			if (c.type == Type::Tuple)
//				initialiseTuple(c);
//			else
//				initialiseType(c);
//		}
//		else
//		{
//			initialiseArray(c.arrayInfo, c);
//			cout << c.arrayInfo.size() << endl;
//			cout << c.arrayInfo.back().numElements << endl;
//			cout << c.value << endl;
//		}
		if (separator.empty())
			separator = ",";
	}
	_tuple.value += ")";
}

void ValueGenerator::initialiseArray(
	ArrayInfo& _arrayInfo,
	TypeInfo& _typeInfo
)
{
#if 0
	cout << "Init 1D array" << endl;
	cout << _typeInfo.value << endl;
#endif
	_typeInfo.value += "[";
	std::string separator;
	for (size_t j = 0; j < _arrayInfo.numElements; j++)
	{
		_typeInfo.value += separator;
		if (_typeInfo.type == Type::Tuple) {
//			cout << "Tuple inside array" << endl;
			initialiseTuple(_typeInfo);
		}
		else
			initialiseType(_typeInfo);
		if (separator.empty())
			separator = ",";
	}
	_typeInfo.value += "]";
#if 0
	cout << _typeInfo.value << endl;
#endif
}

void ValueGenerator::initialiseArray(
	vector<ArrayInfo>& _arrayInfo,
	TypeInfo& _typeInfo
)
{
	if (_arrayInfo.size() == 1)
		initialiseArray(_arrayInfo[0], _typeInfo);
	else
	{
		vector<ArrayInfo> copy = _arrayInfo;
		auto k = copy.back();
		copy.pop_back();
		_typeInfo.value += "[";
		std::string separator;
		for (size_t i = 0; i < k.numElements; i++)
		{
			_typeInfo.value += separator;
			initialiseArray(copy, _typeInfo);
			if (separator.empty())
				separator = ",";
		}
		_typeInfo.value += "]";
	}
}

void ValueGenerator::typeHelper(Json::Value const& _type, TypeInfo& _typeInfo)
{
	std::string jsonTypeString = _type["type"].asString();
	/*
	 * Index | Match description
	 * 0 | Entire type string e.g., bool[1][2][]
	 * 1 | Type string e.g., bool, uint256, address etc.
	 * 2 | Base type e.g., uint, int, bytes
	 * 3 | Type width e.g., 256, 64, 1...32
	 * 4 | First array bracket e.g., [1] in uint256[1][2][3]
	 * 5 | First array dimension e.g., 1 in uint256[1][2][3]
	 * 6 | Second array bracket e.g., [2] in uint256[1][2][3]
	 * 7 | Second array dimension e.g., 2 in uint256[1][2][3]
	 * 8 | Third array bracket e.g., [3] in uint256[1][2][3]
	 * 9 | Third array dimension e.g., 3 in uint256[1][2][3]
	 */
	regex r = regex(
		"(bool|(uint|int|bytes)(\\d+)|address|bytes|string|function|tuple)"
		"(\\[(\\d+)?\\])?(\\[(\\d+)?\\])?(\\[(\\d+)?\\])?"
	);
	smatch matches;
	auto match = regex_search(jsonTypeString, matches, r);
	solAssert(match, "Value generator: Regex match failed.");
	solAssert(
		!matches[1].str().empty(),
		"Value generator: Invalid type"
	);
	auto typeString = matches[1].str();
	size_t width = 0;
	if (matches[3].matched)
		width = stoul(matches[3].str());

	if (typeString.find("bool") != string::npos)
	{
		_typeInfo.type = Type::Boolean;
		_typeInfo.name = "bool";
	}
	else if (typeString.find("function") != string::npos)
	{
		_typeInfo.type = Type::Function;
		_typeInfo.name = "function";
	}
	else if (typeString.find("address") != string::npos)
	{
		_typeInfo.type = Type::Address;
		_typeInfo.name = "address";
	}
	else if (typeString.find("tuple") != string::npos)
	{
		_typeInfo.type = Type::Tuple;
		tuple(_type["components"], _typeInfo);
	}
	else if (typeString.find("bytes") != string::npos)
	{
		if (matches[3].matched)
		{
			solAssert(
				width >= 1 && width <= 32,
				"Value generator: Invalid fixed bytes type."
			);
			_typeInfo.type = Type::FixedBytes;
			_typeInfo.fixedByteWidth = static_cast<FixedBytesWidth>(width);
			_typeInfo.name = "bytes" + matches[3].str();
		}
		else
		{
			solAssert(width == 0, "Value generator: Invalid width.");
			_typeInfo.type = Type::Bytes;
			_typeInfo.name = "bytes";
		}
	}
	else if (typeString.find("string") != string::npos)
	{
		_typeInfo.type = Type::String;
		_typeInfo.name = "string";
	}
	else
	{
		std::string baseType = matches[2].str();
		solAssert(
			baseType == "int" || baseType == "uint",
			"Value generator: Invalid integer type."
		);
		solAssert(
			width >=8 && width <= 256 && (width % 8 == 0),
			"Value generator: Invalid integer width."
		);
		if (baseType == "int")
		{
			_typeInfo.type = Type::Integer;
			_typeInfo.intType = {true, static_cast<IntegerWidth>(width)};
			_typeInfo.name = "int" + matches[3].str();
		}
		else
		{
			_typeInfo.type = Type::UInteger;
			_typeInfo.intType = {false, static_cast<IntegerWidth>(width)};
			_typeInfo.name = "uint" + matches[3].str();
		}
	}

	for (unsigned i = 4; i < 10; i += 2)
	{
		if (matches[i].matched)
		{
			size_t arraySize;
			if (matches[i + 1].matched)
			{
				std::string arraySizeString = matches[i + 1].str();
				arraySize = stoul(arraySizeString);
				_typeInfo.arrayInfo.push_back({true, arraySize});
				_typeInfo.name += "[" + arraySizeString + "]";
			}
			else
			{
				// TODO: Assign pseudo randomly chosen dynamic size.
				arraySize = 2;
				_typeInfo.arrayInfo.push_back({false, arraySize});
				_typeInfo.name += "[]";
			}
		}
	}
	if (_typeInfo.arrayInfo.empty())
	{
		if (_typeInfo.type == Type::Tuple) {
//			cout << "Init tuple" << endl;
			initialiseTuple(_typeInfo);
		}
		else
			initialiseType(_typeInfo);
	}
	else
	{
//		cout << "Init array" << endl;
		initialiseArray(_typeInfo.arrayInfo, _typeInfo);
	}
}

void ValueGenerator::tuple(Json::Value const& _type, TypeInfo& _typeInfo)
{
	_typeInfo.name += "(";
	for (auto component = _type.begin(); component != _type.end();)
	{
		TypeInfo componentType;
		typeHelper(*component, componentType);
		_typeInfo.tupleInfo.emplace_back(componentType);
		_typeInfo.name += _typeInfo.tupleInfo.back().name;
		if (++component != _type.end())
		{
			_typeInfo.name += ",";
		}
	}
	_typeInfo.name += ")";
}

ValueGenerator::TypeInfo ValueGenerator::type(Json::Value const& _value)
{
	solAssert(
		_value.isMember("type"),
		"Value generator: Invalid function input parameter type."
	);
	TypeInfo result;
	typeHelper(_value, result);
	return result;
}

std::pair<std::string, std::string> ValueGenerator::type()
{
	std::string typeString = "(";
	std::string valueString = "(";
	std::string separator = "";
	for (auto const& param: m_type)
	{
		auto l = type(param);
		typeString += separator + l.name;
		valueString += separator + l.value;
		if (separator.empty())
			separator = ",";
	}
	return {typeString + ")", valueString + ")"};
}
