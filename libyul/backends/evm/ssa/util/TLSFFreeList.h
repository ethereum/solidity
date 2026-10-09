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
 * Two-Level Segregated Fit (TLSF) slot pool optimized for uint32_t. Owns a collection of T and grows it on miss.
 * Constant-time `allocate` via bitmap-indexed FL/SL bins, constant-time `deallocate` with immediate coalescing.
 *
 * Reference: M. Masmano, I. Ripoll, A. Crespo, J. Real, "TLSF: a New Dynamic
 * Memory Allocator for Real-Time Systems", ECRTS 2004.
 */

#pragma once

#include <libyul/Exceptions.h>

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <vector>

namespace solidity::yul::ssa::util
{

template<typename TS, typename T>
concept TombstoneTrait = requires(T const& _t)
{
	{ TS::make() } -> std::same_as<T>;
	{ TS::isTombstone(_t) } -> std::convertible_to<bool>;
};

template<typename T, TombstoneTrait<T> Tombstone>
class TLSFFreeList
{
public:
	using Index = std::uint32_t;
	using Length = std::uint32_t;

	explicit TLSFFreeList(std::size_t const _initialCapacity = 0)
	{
		for (auto& row: m_binHead)
			row.fill(EMPTY);
		m_data.reserve(_initialCapacity);
		m_headers.reserve(_initialCapacity);
	}
	TLSFFreeList(TLSFFreeList const&) = delete;
	TLSFFreeList(TLSFFreeList&&) = default;
	TLSFFreeList& operator=(TLSFFreeList const&) = delete;
	TLSFFreeList& operator=(TLSFFreeList&&) = default;
	~TLSFFreeList() = default;

	/// Returns the start index of a contiguous run of `_length` slots
	Index allocate(Length const _length)
	{
		yulAssert(_length >= 1);
		yulAssert(_length <= MAX_ALLOCATE_LENGTH, "length exceeds safe range (would overflow round-up in mappingForRequest)");
		yulAssert(m_data.size() < EMPTY - _length, "container can't grow anymore");
		Index const blockStart = findFreeBlock(_length);
		// couldn't find anything, we should grow
		if (blockStart == EMPTY)
			return growStorage(_length);

		yulAssert(hasHeader(blockStart));
		BlockHeader& block = m_headers[blockStart];
		yulAssert(block.isFree && block.length >= _length);
		// unlink formerly free block from its segregated free list.
		unlinkFromBin(blockStart);

		// trailing remainder goes back into bins
		if (block.length > _length)
			split(blockStart, _length);

		// mark block as used
		block.isFree = false;
		yulAssert(static_cast<std::size_t>(blockStart) + _length <= m_data.size());
		for (Length k = 0; k < _length; ++k)
			yulAssert(Tombstone::isTombstone(m_data[blockStart + k]));
		return blockStart;
	}

	/// Releases `[_start, _start + _length)` to the pool, overwriting the slots with `Tombstone::make()` and
	/// immediately coalescing with adjacent free physical neighbors
	void deallocate(Index const _start, Length const _length)
	{
		yulAssert(_length >= 1);
		yulAssert(static_cast<std::size_t>(_start) + _length <= m_data.size());
		yulAssert(hasHeader(_start), "deallocate: no header at start index");
		BlockHeader& self = m_headers[_start];
		yulAssert(!self.isFree, "deallocate: block is already free");
		yulAssert(self.length == _length, "deallocate: length mismatch with recorded block length");

		for (Length k = 0; k < _length; ++k)
			m_data[_start + k] = Tombstone::make();

		// merge (§7.2 "Coalesce blocks", §8): check both physical neighbors via the boundary-tag links, absorb if free
		Index mergedStart = _start;
		Length mergedLength = _length;
		Index endOfMerged = _start + _length; // address right after the merged block
		Index prevPhys = self.prevPhys;

		// Right physical neighbor
		if (endOfMerged < m_data.size())
		{
			yulAssert(hasHeader(endOfMerged), "deallocate: missing right neighbor");
			BlockHeader const& right = m_headers[endOfMerged];
			if (right.isFree)
			{
				// coalesce (see §7.2)
				Length const rLen = right.length;
				unlinkFromBin(endOfMerged);
				mergedLength += rLen;
				// unset the header
				m_headers[endOfMerged] = {};
				endOfMerged += rLen;
			}
		}

		// Left physical neighbor
		if (prevPhys != EMPTY)
		{
			yulAssert(hasHeader(prevPhys), "deallocate: missing left neighbor");
			BlockHeader const& left = m_headers[prevPhys];
			if (left.isFree)
			{
				Length const lLen = left.length;
				Index const lPrev = left.prevPhys;
				unlinkFromBin(prevPhys);
				mergedStart = prevPhys;
				mergedLength += lLen;
				prevPhys = lPrev;
				// coalesced into left, remove this header
				m_headers[_start] = {};
			}
		}

		m_headers[mergedStart] = BlockHeader {
			.length = mergedLength,
			.prevPhys = prevPhys,
			.isFree = true,
			.prevFreeInBin = EMPTY,
			.nextFreeInBin = EMPTY
		};

		// Update the physical successor's `prevPhys`
		if (endOfMerged < m_data.size())
			m_headers[endOfMerged].prevPhys = mergedStart;
		else
			m_tailBlock = mergedStart;

		// insert free block into corresponding bin (see §7.2 "Insert a free block").
		linkIntoBin(mergedStart, mergedLength);
	}

	T& operator[](Index const _i) { return m_data[_i]; }
	T const& operator[](Index const _i) const { return m_data[_i]; }

	std::size_t size() const noexcept { return m_data.size(); }

	std::vector<T> const& data() const noexcept { return m_data; }

	bool noFreeIntervals() const noexcept
	{
		// Bit `fl` of m_flBitmap is set iff some bin in row `fl` is non-empty;
		// row 0 covers the small-size linear range, so a single bitmap check
		// is sufficient.
		return m_flBitmap == 0;
	}

	/// Number of free (coalesced) blocks
	std::size_t numFreeBlocks() const noexcept { return m_numFreeBlocks; }

private:
	/// Sentinel for an empty/unset index
	static constexpr Index EMPTY = std::numeric_limits<Index>::max();

	/// Number of bits for the second level index (SLI)
	static constexpr std::uint32_t SL_BITS = 5;

	/// Actual number of bins per FL class (2^SL_BITS)
	static constexpr std::uint32_t SL_COUNT = std::uint32_t{1} << SL_BITS;

	/// First level index count. Denotes bins `[2^0, 2^1), ..., [2^(FLI-1), 2^FLI)`. Using 32 for `std::uint32_t`
	static constexpr std::uint32_t FL_COUNT = 32;

	/// Considering that
	///		fl_index = floor(log2(size))
	///			being the position of the leading 1 in `size`s binary representation,
	///		sl_index = (size >> (fl_index - SL_BITS)) - 2^SL_BITS
	///			being the index of which of the linear 2^SL_BITS below fl_index the size falls into,
	///	this protects against `fl_index - SL_BITS` becoming negative
	static constexpr Length SMALL_THRESHOLD = SL_COUNT;

	/// An upper bound on `allocate(_length)`. Above this, the round-up `_length + (subBinSpan - 1)` in
	/// `mappingForRequest` can overflow `Length`
	static constexpr Length MAX_ALLOCATE_LENGTH = std::numeric_limits<Length>::max() - (Length{1} << (FL_COUNT - 1 - SL_BITS)) + Length{1};

	/// Per-block header describing a contiguous run of slots in `m_data`. Can be allocated or free. They are
	/// densely packed.
	struct BlockHeader
	{
		Length length{0}; // length of the run
		Index prevPhys{EMPTY}; // begin index of left-hand side neighbor in block linked list used for coalescing
		bool isFree{false}; // whether this run of slots is free
		// indices to link free blocks of the same length class in a segregated free list
		Index prevFreeInBin{EMPTY};
		Index nextFreeInBin{EMPTY};
	};

	struct BinIdx { std::uint32_t fl; std::uint32_t sl; };

	/// Rounded-up variant of the paper's mapping function (§7.2). Used on allocate so the chosen bin
	/// guarantees `block_size >= _length`.
	static BinIdx mappingForRequest(Length const _length) noexcept
	{
		if (_length < SMALL_THRESHOLD)
			return {std::uint32_t{0}, static_cast<std::uint32_t>(_length)};
		// computes `floor(log2(_length))`, the position of the left-most bit that is 1
		auto const fl = static_cast<std::uint32_t>(std::bit_width(_length) - 1);
		// width of a second-level bin under fl
		//	fl_width / SL_COUNT = 2^fl / 2^SL_BITS = 2^(fl - SL_BITS)
		Length const subBinSpan = Length{1} << (fl - SL_BITS);
		// round up length to the next bin to guarantee that any block in the new bucket satisfies that its size >= _length
		Length const rounded = _length + (subBinSpan - 1);
		return mappingForExact(rounded);
	}

	/// Paper's exact mapping function (§7.2). Picks the `(fl,sl)`-bin whose size range contains _length.
	static BinIdx mappingForExact(Length const _length) noexcept
	{
		if (_length < SMALL_THRESHOLD)
			return {std::uint32_t{0}, static_cast<std::uint32_t>(_length)};
		auto const fl = static_cast<std::uint32_t>(std::bit_width(_length) - 1);
		std::uint32_t const shift = fl - SL_BITS;
		auto const sl = static_cast<std::uint32_t>((_length >> shift) - (Length{1} << SL_BITS));
		yulAssert(fl < FL_COUNT);
		yulAssert(sl < SL_COUNT);
		return {fl, sl};
	}

	/// `search_suitable_block` from the paper (§7.2 "Get a free block"):
	///   1) compute (f, s) from the requested size
	///   2) try `m_binHead[f][s]`; if non-empty, return its first block
	///   3) otherwise find the next non-empty list in the bitmap structure
	Index findFreeBlock(Length const _length) const noexcept
	{
		// bit mask with the lower _numUnsetBits` being set to zero
		static auto constexpr upperBitMask = [](std::uint32_t const _numUnsetBits)
		{
			if (_numUnsetBits >= 32)
				return std::uint32_t{0};
			return ~std::uint32_t{0} << _numUnsetBits;
		};

		BinIdx const target = mappingForRequest(_length);

		// Try the segregated list at (target.fl, target.sl). Mask out bins that correspond to too small lengths.
		std::uint32_t const slMaskAtFl = m_slBitmap[target.fl] & upperBitMask(target.sl);
		if (slMaskAtFl != 0)
		{
			// smallest qualifying bin
			auto const sl = static_cast<std::uint32_t>(std::countr_zero(slMaskAtFl));
			return m_binHead[target.fl][sl];
		}
		// Search the next non-empty FL class via the first-level bitmap
		std::uint32_t const flMask = m_flBitmap & upperBitMask(target.fl + 1);
		if (flMask == 0)
			return EMPTY;
		// smallest fl with at least one nonempty sl bin
		auto const fl = static_cast<std::uint32_t>(std::countr_zero(flMask));
		std::uint32_t const slMask = m_slBitmap[fl];
		yulAssert(slMask != 0);
		// smallest non-empty sl bin
		auto const sl = static_cast<std::uint32_t>(std::countr_zero(slMask));
		return m_binHead[fl][sl];
	}

	/// Paper §7.2 "Insert a free block": push onto the head of the segregated double-linked list for the block's
	/// exact size class, and mark the bin non-empty in the bitmaps
	void linkIntoBin(Index const _start, Length const _length)
	{
		BlockHeader& self = m_headers[_start];
		self.isFree = true;
		BinIdx const bin = mappingForExact(_length);
		Index& head = m_binHead[bin.fl][bin.sl];
		Index const oldHead = head;
		self.prevFreeInBin = EMPTY;
		self.nextFreeInBin = oldHead;
		if (oldHead != EMPTY)
			m_headers[oldHead].prevFreeInBin = _start;
		head = _start;
		setBinBitmap(bin, true);
		++m_numFreeBlocks;
	}

	/// Paper §8 `remove(found_block)`: splice the block out of its segregated double-linked list and clear
	/// the bin's bitmap bits if it became empty
	void unlinkFromBin(Index const _start)
	{
		BlockHeader& self = m_headers[_start];
		yulAssert(self.isFree);
		BinIdx const bin = mappingForExact(self.length);
		Index& head = m_binHead[bin.fl][bin.sl];
		Index const prev = self.prevFreeInBin;
		Index const next = self.nextFreeInBin;
		if (prev != EMPTY)
			m_headers[prev].nextFreeInBin = next;
		else
		{
			yulAssert(head == _start);
			head = next;
		}
		if (next != EMPTY)
			m_headers[next].prevFreeInBin = prev;
		self.prevFreeInBin = EMPTY;
		self.nextFreeInBin = EMPTY;
		self.isFree = false;
		if (head == EMPTY)
			setBinBitmap(bin, false);
		yulAssert(m_numFreeBlocks > 0);
		--m_numFreeBlocks;
	}

	/// Paper §7.4 "Bitmaps are used to track non-empty lists": each FL class has a 32-bit `m_slBitmap[fl]` indicating
	/// which SL sub-bins are non-empty, and a 32-bit `m_flBitmap` indicating which FL classes have any non-empty SL.
	void setBinBitmap(BinIdx const _bin, bool const _on)
	{
		std::uint32_t const slMask = std::uint32_t{1} << _bin.sl;
		std::uint32_t const flMask = std::uint32_t{1} << _bin.fl;
		if (_on)
		{
			m_slBitmap[_bin.fl] |= slMask;
			m_flBitmap |= flMask;
		}
		else
		{
			m_slBitmap[_bin.fl] &= ~slMask;
			if (m_slBitmap[_bin.fl] == 0)
				m_flBitmap &= ~flMask;
		}
	}

	/// Paper §8 `split(found_block, size)` followed by `insert(remaining_block)`: chop off the leading `_length`
	/// slots for the caller and re-link the trailing remainder as a fresh free block
	void split(Index const _start, Length const _length)
	{
		BlockHeader& self = m_headers[_start];
		yulAssert(self.length > _length);
		Length const originalLength = self.length;
		Length const remainderLength = originalLength - _length;
		Index const remainderStart = _start + _length;
		Index const oldEnd = _start + originalLength;

		self.length = _length;

		// `remainderStart` becomes a proper block head
		m_headers[remainderStart] = BlockHeader{
			.length = remainderLength,
			.prevPhys = _start,
			.isFree = false,
			.prevFreeInBin = EMPTY,
			.nextFreeInBin = EMPTY
		};
		if (oldEnd < m_data.size())
			m_headers[oldEnd].prevPhys = remainderStart;
		else
			m_tailBlock = remainderStart;

		linkIntoBin(remainderStart, remainderLength);
	}

	/// Extend storage and append a fresh used block at the end
	Index growStorage(Length const _length)
	{
		auto const start = static_cast<Index>(m_data.size());
		for (Length k = 0; k < _length; ++k)
			m_data.emplace_back(Tombstone::make());
		m_headers.resize(m_data.size());
		// link up new tail
		m_headers[start] = BlockHeader{
			.length = _length,
			.prevPhys = m_tailBlock,
			.isFree = false,
			.prevFreeInBin = EMPTY,
			.nextFreeInBin = EMPTY
		};
		m_tailBlock = start;
		return start;
	}

	/// Yields true if `_s` points to a valid header
	bool hasHeader(Index const _s) const noexcept
	{
		return _s < m_headers.size() && m_headers[_s].length != 0;
	}

	/// The actual data
	std::vector<T> m_data;

	/// Block headers, parallel to `m_data` (one entry per slot). Contains only valid data for indices pointing to
	/// actual block heads
	std::vector<BlockHeader> m_headers;

	/// Tail of the physical-address-ordered block list (`empty` when empty)
	Index m_tailBlock{EMPTY};

	/// Heads of the segregated free lists (paper §7 figure 1)
	std::array<std::array<Index, SL_COUNT>, FL_COUNT> m_binHead{};

	///	The bit `fl`is set iff some SL bin under that FL class is non-empty;
	std::uint32_t m_flBitmap{0};

	/// For first level bin `fl`, `m_slBitmap[fl]` bit `sl` is set iff bin `(fl, sl)` is non-empty
	std::array<std::uint32_t, FL_COUNT> m_slBitmap{};

	/// Number of blocks linked into the bins, i.e., of free blocks
	std::size_t m_numFreeBlocks{0};
};

}
