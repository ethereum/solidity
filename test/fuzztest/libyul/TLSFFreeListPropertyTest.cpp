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

#include <libyul/backends/evm/ssa/util/TLSFFreeList.h>

#include <libsolutil/Visitor.h>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <range/v3/algorithm/mismatch.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

using namespace solidity::yul::ssa::util;

namespace solidity::yul::test
{

struct TombstoneDescriptor
{
	using ElementValue = std::uint64_t;
	static ElementValue make() noexcept { return 0; }
	static bool isTombstone(ElementValue const _slot) noexcept { return _slot == 0; }
};

using FreeList = TLSFFreeList<TombstoneDescriptor::ElementValue, TombstoneDescriptor>;

struct Allocate { FreeList::Length length; };
struct Deallocate { std::size_t block; };
using Operation = std::variant<Allocate, Deallocate>;

static FreeList::Length constexpr maxLength = 256;
static std::size_t constexpr maxOperations = 512;

static void MatchesReferenceAndCoalesces(std::vector<Operation> const& _operations)
{
	FreeList freeList;
	std::vector<std::uint64_t> reference;
	std::vector<std::pair<FreeList::Index, FreeList::Length>> liveBlocks;
	std::uint64_t nextID = 1;

	util::GenericVisitor const perform{
		[&](Allocate const& _allocate)
		{
			std::size_t const oldSize = freeList.size();
			FreeList::Index const start = freeList.allocate(_allocate.length);
			// the pool grows only on a miss, by appending exactly the requested block
			ASSERT_TRUE(freeList.size() == oldSize || freeList.size() == oldSize + _allocate.length)
				<< "pool grew from " << oldSize << " to " << freeList.size() << " for length " << _allocate.length;
			// slots the pool grew by are free
			reference.resize(freeList.size());
			// also keeps the slot accesses below in bounds
			ASSERT_LE(std::size_t{start} + _allocate.length, freeList.size()) << "block at " << start << " exceeds the pool";
			std::uint64_t const id = nextID++;
			for (FreeList::Index slot = start; slot < start + _allocate.length; ++slot)
			{
				ASSERT_EQ(reference[slot], 0u) << "block at " << start << " overlaps a live block in slot " << slot;
				freeList[slot] = reference[slot] = id;
			}
			liveBlocks.emplace_back(start, _allocate.length);
		},
		[&](Deallocate const& _deallocate)
		{
			if (liveBlocks.empty())
				return;
			auto const blockIt = liveBlocks.begin() + static_cast<std::ptrdiff_t>(_deallocate.block % liveBlocks.size());
			auto const [start, length] = *blockIt;
			freeList.deallocate(start, length);
			std::fill_n(reference.begin() + start, length, 0);
			liveBlocks.erase(blockIt);
		}
	};
	auto const matchesReference = [&]() -> testing::AssertionResult
	{
		if (freeList.size() != reference.size())
			return testing::AssertionFailure() << "pool has " << freeList.size() << " slots, model has " << reference.size();
		auto const [actual, expected] = ranges::mismatch(freeList.data(), reference);
		if (actual != freeList.data().end() || expected != reference.end())
			return testing::AssertionFailure() << "slot " << expected - reference.begin() << " deviates from the model";
		// coalescing is immediate, so every maximal run of free slots is exactly one free block
		std::size_t freeRuns = 0;
		for (std::size_t slot = 0; slot < reference.size(); ++slot)
			if (reference[slot] == 0 && (slot == 0 || reference[slot - 1] != 0))
				++freeRuns;
		if (freeList.numFreeBlocks() != freeRuns)
			return testing::AssertionFailure() << freeList.numFreeBlocks() << " free blocks for " << freeRuns << " runs of free slots";
		return testing::AssertionSuccess();
	};

	for (Operation const& operation: _operations)
	{
		// assertions inside the visitor only return from it
		ASSERT_NO_FATAL_FAILURE(std::visit(perform, operation));
		ASSERT_TRUE(matchesReference());
	}
}

FUZZ_TEST(TLSFFreeListProperty, MatchesReferenceAndCoalesces)
	.WithDomains(
		fuzztest::VectorOf(
			fuzztest::VariantOf(
				fuzztest::StructOf<Allocate>(fuzztest::InRange<FreeList::Length>(1, maxLength)),
				fuzztest::StructOf<Deallocate>(fuzztest::Arbitrary<std::size_t>())
			)
		).WithMaxSize(maxOperations)
	);

}
