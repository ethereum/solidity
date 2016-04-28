/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Base58.cpp
 * Adapted from code found on https://github.com/bitcoin/bitcoin/blob/master/src/base58.cpp
 * Licenced under The MIT License.
 * @author The Bitcoin core developers (original)
 * @author Gav Wood <i@gavwood.com> (minor modifications and reformating)
 * @date 2015
 */

#include "Base58.h"
using namespace std;
using namespace dev;

std::string dev::AlphabetIPFS("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");
std::string dev::AlphabetFlickr("123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ");

bytes dev::fromBase58(string const& _s, string const& _alphabet)
{
	auto index = _s.begin();

	// Skip and count leading '1's.
	int zeroes = 0;
	while (*index == _alphabet[0])
	{
		zeroes++;
		index++;
	}

	// Allocate enough space in big-endian base256 representation.
	// log(58) / log(256), rounded up.
	bytes ret((_s.size() - zeroes) * 733 / 1000 + 1);

	// Process the characters.
	while (index != _s.end())
	{
		// Decode base58 character
		size_t carry = _alphabet.find(*index);
		if (carry == string::npos)
			throw invalid_argument("Invalid character in base-58 string");
		// Apply "ret = ret * 58 + ch".
		for (auto it = ret.rbegin(); it != ret.rend(); it++)
		{
			carry += 58 * (*it);
			*it = carry % 256;
			carry /= 256;
		}
		assert(carry == 0);
		index++;
	}

	// Skip leading zeroes.
	while (!ret.front())
		ret.erase(ret.begin());

	// Re-insert zeroes.
	for (int i = 0; i < zeroes; ++i)
		ret.insert(ret.begin(), 0);

	return ret;
}

string dev::toBase58(bytesConstRef _d, string const& _alphabet)
{
	auto begin = _d.data();
	auto end = _d.data() + _d.size();

	// Skip & count leading zeroes.
	int zeroes = 0;
	for (; begin != end && !*begin; begin++, zeroes++) {}

	// Allocate enough space in big-endian base58 representation.
	// log(256) / log(58), rounded up.
	std::vector<unsigned char> b58((end - begin) * 138 / 100 + 1);

	// Process the bytes.
	while (begin != end)
	{
		int carry = *begin;
		// Apply "b58 = b58 * 256 + ch".
		for (auto it = b58.rbegin(); it != b58.rend(); it++)
		{
			carry += 256 * (*it);
			*it = carry % 58;
			carry /= 58;
		}
		assert(!carry);
		begin++;
	}

	// Skip leading zeroes in base58 result.
	auto it = b58.begin();
	while (it != b58.end() && !*it)
		it++;

	// Translate the result into a string.
	std::string ret;
	ret.reserve(zeroes + (b58.end() - it));
	ret.assign(zeroes, '1');
	while (it != b58.end())
		ret += _alphabet[*(it++)];

	return ret;
}

