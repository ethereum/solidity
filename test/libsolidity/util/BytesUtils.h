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
	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the boolean number literal. Throws if conversion fails.
	bytes convertBoolean(std::string const& _literal);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the decimal number literal. Throws if conversion fails.
	bytes convertNumber(std::string const& _literal);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the hex literal. Throws if conversion fails.
	bytes convertHexNumber(std::string const& _literal);

	/// Tries to convert \param _literal to an unpadded `bytes`
	/// representation of the string literal. Throws if conversion fails.
	bytes convertString(std::string const& _literal);

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// an unsigned value.
	std::string formatUnsigned(bytes const& _bytes) const;

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a signed value.
	std::string formatSigned(bytes const& _bytes) const;

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a boolean value.
	std::string formatBoolean(bytes const& _bytes) const;

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a hex value.
	std::string formatHex(bytes const& _bytes) const;

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a hexString value.
	std::string formatHexString(bytes const& _bytes) const;

	/// Converts \param _bytes to a soltest-compliant and human-readable
	/// string representation of a byte array which is assumed to hold
	/// a string value.
	std::string formatString(bytes const& _bytes) const;
};

}
}
}
