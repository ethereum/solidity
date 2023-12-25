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
 * String abstraction that avoids copies.
 */

#pragma once

#include <fmt/format.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace solidity::yul
{

// xxhash64 implementation
class XXHash64
{
public:
	// 0 is a valid seed
	explicit XXHash64(uint64_t seed)
	{
		state[0] = seed + Prime1 + Prime2;
		state[1] = seed + Prime2;
		state[2] = seed;
		state[3] = seed - Prime1;
		bufferSize = 0;
		totalLength = 0;
	}

	/// add a chunk of bytes
	/** @param  input  pointer to a continuous block of data
		@param  length number of bytes
		@return false if parameters are invalid / zero **/
	bool add(const void* input, uint64_t length)
	{
		// no data ?
		if (!input || length == 0)
			return false;
		totalLength += length;
		// byte-wise access
		const unsigned char* data = (const unsigned char*) input;

		// unprocessed old data plus new data still fit in temporary buffer ?
		if (bufferSize + length < MaxBufferSize)
		{
			// just add new data
			while (length-- > 0)
				buffer[bufferSize++] = *data++;
			return true;
		}
		// point beyond last byte
		const unsigned char* stop = data + length;
		const unsigned char* stopBlock = stop - MaxBufferSize;
		// some data left from previous update ?
		if (bufferSize > 0)
		{
			// make sure temporary buffer is full (16 bytes)
			while (bufferSize < MaxBufferSize)
				buffer[bufferSize++] = *data++;

			// process these 32 bytes (4x8)
			process(buffer, state[0], state[1], state[2], state[3]);
		}

		// copying state to local variables helps optimizer A LOT
		uint64_t s0 = state[0], s1 = state[1], s2 = state[2], s3 = state[3];
		// 32 bytes at once
		while (data <= stopBlock)
		{
			// local variables s0..s3 instead of state[0]..state[3] are much faster
			process(data, s0, s1, s2, s3);
			data += 32;
		}
		// copy back
		state[0] = s0;
		state[1] = s1;
		state[2] = s2;
		state[3] = s3;

		// copy remainder to temporary buffer
		bufferSize = static_cast<uint64_t>(stop - data);
		for (uint64_t i = 0; i < bufferSize; i++)
			buffer[i] = data[i];

		// done
		return true;
	}

	uint64_t hash() const
	{
		// fold 256 bit state into one single 64 bit value
		uint64_t result;
		if (totalLength >= MaxBufferSize)
		{
			result = rotateLeft(state[0], 1) + rotateLeft(state[1], 7) + rotateLeft(state[2], 12)
					 + rotateLeft(state[3], 18);
			result = (result ^ processSingle(0, state[0])) * Prime1 + Prime4;
			result = (result ^ processSingle(0, state[1])) * Prime1 + Prime4;
			result = (result ^ processSingle(0, state[2])) * Prime1 + Prime4;
			result = (result ^ processSingle(0, state[3])) * Prime1 + Prime4;
		}
		else
		{
			// internal state wasn't set in add(), therefore original seed is still stored in state2
			result = state[2] + Prime5;
		}

		result += totalLength;

		// process remaining bytes in temporary buffer
		const unsigned char* data = buffer;
		// point beyond last byte
		const unsigned char* stop = data + bufferSize;

		// at least 8 bytes left ? => eat 8 bytes per step
		for (; data + 8 <= stop; data += 8)
			result = rotateLeft(result ^ processSingle(0, *(uint64_t*) data), 27) * Prime1 + Prime4;

		// 4 bytes left ? => eat those
		if (data + 4 <= stop)
		{
			result = rotateLeft(result ^ (*(uint32_t*) data) * Prime1, 23) * Prime2 + Prime3;
			data += 4;
		}

		// take care of remaining 0..3 bytes, eat 1 byte per step
		while (data != stop)
			result = rotateLeft(result ^ (*data++) * Prime5, 11) * Prime1;

		// mix bits
		result ^= result >> 33;
		result *= Prime2;
		result ^= result >> 29;
		result *= Prime3;
		result ^= result >> 32;
		return result;
	}

private:
	static constexpr uint64_t Prime1 = 11400714785074694791ULL;
	static constexpr uint64_t Prime2 = 14029467366897019727ULL;
	static constexpr uint64_t Prime3 = 1609587929392839161ULL;
	static constexpr uint64_t Prime4 = 9650029242287828579ULL;
	static constexpr uint64_t Prime5 = 2870177450012600261ULL;

	// store up to 31 bytes between function calls
	static constexpr uint64_t MaxBufferSize = 31 + 1;

	uint64_t state[4];
	unsigned char buffer[MaxBufferSize];
	uint64_t bufferSize;
	uint64_t totalLength;

	/// rotate bits, should compile to a single CPU instruction (ROL)
	static uint64_t rotateLeft(uint64_t x, unsigned char bits) { return (x << bits) | (x >> (64 - bits)); }

	/// process a single 64 bit value
	static uint64_t processSingle(uint64_t previous, uint64_t input)
	{
		return rotateLeft(previous + input * Prime2, 31) * Prime1;
	}

	/// process a block of 4x4 bytes, this is the main part of the XXHash32 algorithm
	static void process(const void* data, uint64_t& state0, uint64_t& state1, uint64_t& state2, uint64_t& state3)
	{
		const auto* block = static_cast<const uint64_t*>(data);
		state0 = processSingle(state0, block[0]);
		state1 = processSingle(state1, block[1]);
		state2 = processSingle(state2, block[2]);
		state3 = processSingle(state3, block[3]);
	}
};

/// Repository for YulStrings.
/// Owns the string data for all YulStrings, which can be referenced by a Handle.
/// A Handle consists of an ID (that depends on the insertion order of YulStrings and is potentially
/// non-deterministic) and a deterministic string hash.
class YulStringRepository
{
public:
	struct Handle
	{
		size_t id;
		std::uint64_t hash;
	};

	static YulStringRepository& instance()
	{
		static YulStringRepository inst;
		return inst;
	}

	Handle stringToHandle(std::string const& _string)
	{
		if (_string.empty())
			return {0, emptyHash()};
		std::uint64_t h = hash(_string);
		auto range = m_hashToID.equal_range(h);
		for (auto it = range.first; it != range.second; ++it)
			if (*m_strings[it->second] == _string)
				return Handle{it->second, h};
		m_strings.emplace_back(std::make_shared<std::string>(_string));
		size_t id = m_strings.size() - 1;
		m_hashToID.emplace_hint(range.second, std::make_pair(h, id));

		return Handle{id, h};
	}
	std::string const& idToString(size_t _id) const { return *m_strings.at(_id); }

	/// xxhash64 implementation
	static std::uint64_t hash(std::string const& v)
	{
		static constexpr uint64_t seed = 0u;
		XXHash64 hasher(seed);
		hasher.add(v.c_str(), v.length());
		return hasher.hash();
	}

	/// empty value of xxhash64 when seed and length is 0
	static constexpr std::uint64_t emptyHash() { return 17241709254077376921ULL; }
	/// Clear the repository.
	/// Use with care - there cannot be any dangling YulString references.
	/// If references need to be cleared manually, register the callback via
	/// resetCallback.
	static void reset()
	{
		for (auto const& cb: resetCallbacks())
			cb();
		instance() = YulStringRepository{};
	}
	/// Struct that registers a reset callback as a side-effect of its construction.
	/// Useful as static local variable to register a reset callback once.
	struct ResetCallback
	{
		ResetCallback(std::function<void()> _fun)
		{
			YulStringRepository::resetCallbacks().emplace_back(std::move(_fun));
		}
	};

private:
	YulStringRepository() = default;
	YulStringRepository(YulStringRepository const&) = delete;
	YulStringRepository(YulStringRepository&&) = default;
	YulStringRepository& operator=(YulStringRepository const& _rhs) = delete;
	YulStringRepository& operator=(YulStringRepository&& _rhs) = default;

	static std::vector<std::function<void()>>& resetCallbacks()
	{
		static std::vector<std::function<void()>> callbacks;
		return callbacks;
	}

	std::vector<std::shared_ptr<std::string>> m_strings = {std::make_shared<std::string>()};
	std::unordered_multimap<std::uint64_t, size_t> m_hashToID = {{emptyHash(), 0}};
};

/// Wrapper around handles into the YulString repository.
/// Equality of two YulStrings is determined by comparing their ID.
/// The <-operator depends on the string hash and is not consistent
/// with string comparisons (however, it is still deterministic).
class YulString
{
public:
	YulString() = default;
	explicit YulString(std::string const& _s): m_handle(YulStringRepository::instance().stringToHandle(_s)) {}
	YulString(YulString const&) = default;
	YulString(YulString&&) = default;
	YulString& operator=(YulString const&) = default;
	YulString& operator=(YulString&&) = default;

	/// This is not consistent with the string <-operator!
	/// First compares the string hashes. If they are equal
	/// it checks for identical IDs (only identical strings have
	/// identical IDs and identical strings do not compare as "less").
	/// If the hashes are identical and the strings are distinct, it
	/// falls back to string comparison.
	bool operator<(YulString const& _other) const
	{
		if (m_handle.hash < _other.m_handle.hash) return true;
		if (_other.m_handle.hash < m_handle.hash) return false;
		if (m_handle.id == _other.m_handle.id) return false;
		return str() < _other.str();
	}
	/// Equality is determined based on the string ID.
	bool operator==(YulString const& _other) const { return m_handle.id == _other.m_handle.id; }
	bool operator!=(YulString const& _other) const { return m_handle.id != _other.m_handle.id; }

	bool empty() const { return m_handle.id == 0; }
	std::string const& str() const
	{
		return YulStringRepository::instance().idToString(m_handle.id);
	}

	uint64_t hash() const { return m_handle.hash; }

private:
	/// Handle of the string. Assumes that the empty string has ID zero.
	YulStringRepository::Handle m_handle{0, YulStringRepository::emptyHash()};
};

inline YulString operator"" _yulstring(char const* _string, std::size_t _size)
{
	return YulString(std::string(_string, _size));
}

}

namespace fmt
{
template<>
struct formatter<solidity::yul::YulString>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& _context)
	{
		return _context.begin();
	}

	template<typename FormatContext>
	auto format(solidity::yul::YulString _value, FormatContext& _context)
	{
		return format_to(_context.out(), "{}", _value.str());
	}
};
}
namespace std
{
template<> struct hash<solidity::yul::YulString>
{
	size_t operator()(solidity::yul::YulString const& x) const
	{
		return static_cast<size_t>(x.hash());
	}
};
}
