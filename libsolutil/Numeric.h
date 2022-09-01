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
/**
 * Definition of u256 and similar types and helper functions.
 */

#pragma once

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>

#include <boost/version.hpp>
#if (BOOST_VERSION < 106500)
#error "Unsupported Boost version. At least 1.65 required."
#endif

// TODO: do this only conditionally as soon as a boost version with gcc 12 support is released.
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overread"
#pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
#endif
#include <boost/multiprecision/cpp_int.hpp>
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 12)
#pragma GCC diagnostic pop
#endif

#include <limits>

namespace solidity
{

// Numeric types.
using bigint = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>>;
using u256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;
using s256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void>>;

/// Interprets @a _u as a two's complement signed number and returns the resulting s256.
inline s256 u2s(u256 _u)
{
	static bigint const c_end = bigint(1) << 256;
	if (boost::multiprecision::bit_test(_u, 255))
		return s256(-(c_end - _u));
	else
		return s256(_u);
}

/// @returns the two's complement signed representation of the signed number _u.
inline u256 s2u(s256 _u)
{
	static bigint const c_end = bigint(1) << 256;
	if (_u >= 0)
		return u256(_u);
	else
		return u256(c_end + _u);
}

inline u256 exp256(u256 _base, u256 _exponent)
{
	using boost::multiprecision::limb_type;
	u256 result = 1;
	while (_exponent)
	{
		if (boost::multiprecision::bit_test(_exponent, 0))
			result *= _base;
		_base *= _base;
		_exponent >>= 1;
	}
	return result;
}

/// Checks whether _mantissa * (X ** _exp) fits into 4096 bits,
/// where X is given indirectly via _log2OfBase = log2(X).
bool fitsPrecisionBaseX(bigint const& _mantissa, double _log2OfBase, uint32_t _exp);


// Big-endian to/from host endian conversion functions.

/// Converts a templated integer value to the big-endian byte-stream represented on a templated collection.
/// The size of the collection object will be unchanged. If it is too small, it will not represent the
/// value properly, if too big then the additional elements will be zeroed out.
/// @a Out will typically be either std::string or bytes.
/// @a T will typically by unsigned, u160, u256 or bigint.
template <class T, class Out>
inline void toBigEndian(T _val, Out& o_out)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	for (auto i = o_out.size(); i != 0; _val >>= 8, i--)
	{
		T v = _val & (T)0xff;
		o_out[i - 1] = (typename Out::value_type)(uint8_t)v;
	}
}

/// Converts a big-endian byte-stream represented on a templated collection to a templated integer value.
/// @a In will typically be either std::string or bytes.
/// @a T will typically by unsigned, u256 or bigint.
template <class T, class In>
inline T fromBigEndian(In const& _bytes)
{
	T ret = (T)0;
	for (auto i: _bytes)
		ret = (T)((ret << 8) | (uint8_t)(typename std::make_unsigned<typename In::value_type>::type)i);
	return ret;
}
inline bytes toBigEndian(u256 _val) { bytes ret(32); toBigEndian(_val, ret); return ret; }

/// Convenience function for toBigEndian.
/// @returns a byte array just big enough to represent @a _val.
template <class T>
inline bytes toCompactBigEndian(T _val, unsigned _min = 0)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	unsigned i = 0;
	for (T v = _val; v; ++i, v >>= 8) {}
	bytes ret(std::max<unsigned>(_min, i), 0);
	toBigEndian(_val, ret);
	return ret;
}

/// Convenience function for conversion of a u256 to hex
inline std::string toHex(u256 val)
{
	return util::toHex(toBigEndian(val));
}

template <class T>
inline std::string toCompactHexWithPrefix(T _value)
{
	return "0x" + util::toHex(toCompactBigEndian(_value, 1));
}

/// Returns decimal representation for small numbers and hex for large numbers.
inline std::string formatNumber(bigint const& _value)
{
	if (_value < 0)
		return "-" + formatNumber(-_value);
	if (_value > 0x1000000)
		return "0x" + util::toHex(toCompactBigEndian(_value, 1));
	else
		return _value.str();
}

inline std::string formatNumber(u256 const& _value)
{
	if (_value > 0x1000000)
		return toCompactHexWithPrefix(_value);
	else
		return _value.str();
}


// Algorithms for string and string-like collections.

/// Determine bytes required to encode the given integer value. @returns 0 if @a _i is zero.
template <class T>
inline unsigned numberEncodingSize(T _i)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	unsigned i = 0;
	for (; _i != 0; ++i, _i >>= 8) {}
	return i;
}

}
