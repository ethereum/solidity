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
/** @file FixedHash.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * The FixedHash fixed-size "hash" container type.
 */

#pragma once

#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>

#include <boost/functional/hash.hpp>
#include <boost/io/ios_state.hpp>

#include <array>
#include <cstdint>
#include <algorithm>

namespace solidity::util
{

/// Fixed-size raw-byte array container type, with an API optimised for storing hashes.
/// Transparently converts to/from the corresponding arithmetic type; this will
/// assume the data contained in the hash is big-endian.
template <unsigned N>
class FixedHash
{
public:
	/// The corresponding arithmetic type.
	using Arith = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<N * 8, N * 8, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

	/// The size of the container.
	enum { size = N };
	static_assert(N != 0);

	/// Method to convert from a string.
	enum ConstructFromStringType { FromHex, FromBinary };

	/// Method to convert from a string.
	enum ConstructFromHashType { AlignLeft, AlignRight, FailIfDifferent };

	/// Construct an empty hash.
	explicit FixedHash() { m_data.fill(0); }

	/// Construct from another hash, filling with zeroes or cropping as necessary.
	template <unsigned M> explicit FixedHash(FixedHash<M> const& _h, ConstructFromHashType _t = AlignLeft)
	{
		m_data.fill(0);
		unsigned c = std::min(M, N);
		for (unsigned i = 0; i < c; ++i)
			m_data[_t == AlignRight ? N - 1 - i : i] = _h[_t == AlignRight ? M - 1 - i : i];
	}

	/// Convert from the corresponding arithmetic type.
	FixedHash(Arith const& _arith) { toBigEndian(_arith, m_data); }

	/// Explicitly construct, copying from a byte array.
	explicit FixedHash(bytes const& _array, ConstructFromHashType _sizeMismatchBehavior = FailIfDifferent)
	{
		if (_array.size() == N)
			memcpy(m_data.data(), _array.data(), _array.size());
		else
		{
			m_data.fill(0);
			if (_sizeMismatchBehavior != FailIfDifferent)
			{
				auto bytesToCopy = std::min<size_t>(_array.size(), N);
				for (size_t i = 0; i < bytesToCopy; ++i)
					if (_sizeMismatchBehavior == AlignRight)
						m_data[N - 1 - i] = _array[_array.size() - 1 - i];
					else
						m_data[i] = _array[i];
			}
		}
	}

	/// Explicitly construct, copying from a byte array.
	explicit FixedHash(bytesConstRef _b, ConstructFromHashType _t = FailIfDifferent)
	{
		if (_b.size() == N)
			memcpy(m_data.data(), _b.data(), std::min<size_t>(_b.size(), N));
		else
		{
			m_data.fill(0);
			if (_t != FailIfDifferent)
			{
				auto c = std::min<size_t>(_b.size(), N);
				for (size_t i = 0; i < c; ++i)
					if (_t == AlignRight)
						m_data[N - 1 - i] = _b[_b.size() - 1 - i];
					else
						m_data[i] = _b[i];
			}
		}
	}

	/// Explicitly construct, copying from a string.
	explicit FixedHash(std::string const& _s, ConstructFromStringType _t = FromHex, ConstructFromHashType _ht = FailIfDifferent):
		FixedHash(_t == FromHex ? fromHex(_s, WhenError::Throw) : solidity::util::asBytes(_s), _ht)
	{}

	/// Convert to arithmetic type.
	operator Arith() const { return fromBigEndian<Arith>(m_data); }

	// The obvious comparison operators.
	bool operator==(FixedHash const& _c) const { return m_data == _c.m_data; }
	bool operator!=(FixedHash const& _c) const { return m_data != _c.m_data; }
	/// Required to sort objects of this type or use them as map keys.
	bool operator<(FixedHash const& _c) const {
		for (unsigned i = 0; i < N; ++i)
		{
			if (m_data[i] < _c.m_data[i])
				return true;
			else if (m_data[i] > _c.m_data[i])
				return false;
		}
		return false;
	}

	/// @returns a particular byte from the hash.
	uint8_t& operator[](unsigned _i) { return m_data[_i]; }
	/// @returns a particular byte from the hash.
	uint8_t operator[](unsigned _i) const { return m_data[_i]; }

	/// @returns the hash as a user-readable hex string.
	std::string hex() const { return toHex(asBytes()); }

	/// @returns a mutable byte vector_ref to the object's data.
	bytesRef ref() { return bytesRef(m_data.data(), N); }

	/// @returns a constant byte vector_ref to the object's data.
	bytesConstRef ref() const { return bytesConstRef(m_data.data(), N); }

	/// @returns a mutable byte pointer to the object's data.
	uint8_t* data() { return m_data.data(); }

	/// @returns a constant byte pointer to the object's data.
	uint8_t const* data() const { return m_data.data(); }

	/// @returns a copy of the object's data as a byte vector.
	bytes asBytes() const { return bytes(data(), data() + N); }

private:
	std::array<uint8_t, N> m_data;		///< The binary data.
};

/// Stream I/O for the FixedHash class.
template <unsigned N>
inline std::ostream& operator<<(std::ostream& _out, FixedHash<N> const& _h)
{
	boost::io::ios_all_saver guard(_out);
	_out << std::noshowbase << std::hex << std::setfill('0');
	for (unsigned i = 0; i < N; ++i)
		_out << std::setw(2) << (int)_h[i];
	_out << std::dec;
	return _out;
}

// Common types of FixedHash.
using h256 = FixedHash<32>;
using h160 = FixedHash<20>;

}
