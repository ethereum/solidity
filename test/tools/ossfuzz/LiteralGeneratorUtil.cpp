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

#include <test/tools/ossfuzz/LiteralGeneratorUtil.h>

#include <liblangutil/Exceptions.h>

using namespace solidity::test::fuzzer;
using namespace std;

template<typename V>
V LiteralGeneratorUtil::integerValue(size_t _counter)
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

string LiteralGeneratorUtil::integerValue(size_t _counter, size_t _intWidth, bool _signed)
{
	if (_signed)
		return signedIntegerValue(_counter, _intWidth);
	else
		return unsignedIntegerValue(_counter, _intWidth);
}

string LiteralGeneratorUtil::fixedBytes(size_t _numBytes, size_t _counter, bool _isHexLiteral)
{
	solAssert(
		_numBytes > 0 && _numBytes <= 32,
		"Literal Generator: Too short or too long a cropped string"
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
