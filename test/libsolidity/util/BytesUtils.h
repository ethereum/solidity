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

#pragma once

#include <test/libsolidity/util/SoltestTypes.h>

#include <libdevcore/CommonData.h>

namespace dev
{
namespace solidity
{
namespace test
{

/**
 * Utility class that aids conversions from parsed strings to an
 * isoltest-internal, ABI-based bytes representation and vice-versa.
 */
class BytesUtils
{
public:
	/// Left-aligns and pads given _bytes and returns a new
	/// bytes array.
	static bytes alignLeft(bytes _bytes);

	/// Right-aligns and pads given _bytes and returns a new
	/// bytes array.
	static bytes alignRight(bytes _bytes);

	/// Applies given _alignment to _bytes and returns a new
	/// bytes array.
	/// TODO: Remove abiType reference from parameter list
	/// and return the new alignment instead.
	static bytes applyAlign(
		Parameter::Alignment _alignment,
		ABIType& _abiType,
		bytes _bytes
	);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the boolean number literal. Throws if conversion fails.
	static bytes convertBoolean(std::string const& _literal);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the decimal number literal. Throws if conversion fails.
	static bytes convertNumber(std::string const& _literal);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the hex literal. Throws if conversion fails.
	static bytes convertHexNumber(std::string const& _literal);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the string literal. Throws if conversion fails.
	static bytes convertString(std::string const& _literal);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// an unsigned value.
	static std::string formatUnsigned(bytes const& _bytes);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a signed value.
	static std::string formatSigned(bytes const& _bytes);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a boolean value.
	static std::string formatBoolean(bytes const& _bytes);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a hex value.
	static std::string formatHex(bytes const& _bytes);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a hexString value.
	static std::string formatHexString(bytes const& _bytes);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a string value.
	static std::string formatString(bytes const& _bytes, size_t _cutOff);

	static std::string formatString(bytes const& _bytes)
	{
		return formatString(_bytes, _bytes.size());
	}

	/// Used to print returned bytes from function calls to the commandline.
	/// Returns a string representation of given _bytes in ranges of 32 bytes.
	/// If _withSignature is true, the first 4 bytes will be formatted separately.
	static std::string formatRawBytes(
		bytes const& _bytes,
		std::string _linePrefix = "",
		bool _withSignature = false
	);

	/// Formats given _bytes with type information passed in _abiType.
	static std::string formatBytes(bytes const& _bytes, ABIType const& _abiType);

	/// Formats given _bytes with type information passed in _abiTypes.
	/// Prints obtained result if it does not match the expectation
	/// and prints the expected result otherwise.
	/// Highlights parameter only if it does not match.
	static std::string formatBytesRange(
		bytes _bytes,
		ParameterList const& _parameters,
		bool _highlight
	);
};

}
}
}
