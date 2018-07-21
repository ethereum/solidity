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
/** @file Common.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Very common stuff (i.e. that every other header needs except vector_ref.h).
 */

#pragma once

// way too many unsigned to size_t warnings in 32 bit build
#ifdef _M_IX86
#pragma warning(disable:4244)
#endif

#if _MSC_VER && _MSC_VER < 1900
#define _ALLOW_KEYWORD_MACROS
#define noexcept throw()
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:3682) //call through incomplete class
#endif

#include <libdevcore/vector_ref.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // defined(__GNUC__)

// See https://github.com/ethereum/libweb3core/commit/90680a8c25bfb48b24371b4abcacde56c181517c
// See https://svn.boost.org/trac/boost/ticket/11328
// Bob comment - perhaps we should just HARD FAIL here with Boost-1.58.00?
// It is quite old now, and requiring end-users to use a newer Boost release is probably not unreasonable.
#include <boost/version.hpp>
#if (BOOST_VERSION == 105800)
	#include "boost_multiprecision_number_compare_bug_workaround.hpp"
#endif // (BOOST_VERSION == 105800)

#include <boost/multiprecision/cpp_int.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__)

#include <map>
#include <vector>
#include <functional>
#include <string>

using byte = uint8_t;

namespace dev
{

// Binary data types.
using bytes = std::vector<byte>;
using bytesRef = vector_ref<byte>;
using bytesConstRef = vector_ref<byte const>;

// Numeric types.
using bigint = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>>;
using u256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;
using s256 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void>>;
using u160 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<160, 160, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

// Map types.
using StringMap = std::map<std::string, std::string>;

// String types.
using strings = std::vector<std::string>;

/// Interprets @a _u as a two's complement signed number and returns the resulting s256.
inline s256 u2s(u256 _u)
{
	static const bigint c_end = bigint(1) << 256;
	if (boost::multiprecision::bit_test(_u, 255))
		return s256(-(c_end - _u));
	else
		return s256(_u);
}

/// @returns the two's complement signed representation of the signed number _u.
inline u256 s2u(s256 _u)
{
	static const bigint c_end = bigint(1) << 256;
    if (_u >= 0)
		return u256(_u);
    else
		return u256(c_end + _u);
}

inline std::ostream& operator<<(std::ostream& os, bytes const& _bytes)
{
	std::ostringstream ss;
	ss << std::hex;
	std::copy(_bytes.begin(), _bytes.end(), std::ostream_iterator<int>(ss, ","));
	std::string result = ss.str();
	result.pop_back();
	os << "[" + result + "]";
	return os;
}

/// RAII utility class whose destructor calls a given function.
class ScopeGuard
{
public:
	explicit ScopeGuard(std::function<void(void)> _f): m_f(_f) {}
	~ScopeGuard() { m_f(); }

private:
	std::function<void(void)> m_f;
};

}
