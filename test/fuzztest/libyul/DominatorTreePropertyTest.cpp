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

/// Property test for the SSA CFG dominator tree
///
/// For a random SSA CFG the DominatorTree is cross-checked against the definition of dominance, i.e.,
/// a block d dominates a reachable block v iff d == v or v is no longer reachable from the entry once d is removed.
/// The oracle is implemented by one reachability search per removed block.
/// - `DepthFirstSpanningTree::reachable` agrees with an independent reachability search,
/// - `dominates(a, b)` agrees with the oracle for every pair of reachable blocks;
/// - `immediateDominator(b)` is a strict dominator of `b` that every strict dominator of `b` dominates
///    and the entry block is its own immediate dominator;
/// - queries involving an unreachable block are rejected.

#include <libyul/backends/evm/ssa/SSACFG.h>
#include <libyul/backends/evm/ssa/analysis/DepthFirstSpanningTree.h>
#include <libyul/backends/evm/ssa/analysis/DominatorTree.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/Exceptions.h>

#include <liblangutil/EVMVersion.h>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <vector>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::analysis;

namespace solidity::yul::test
{

namespace
{

using NodeID = std::uint32_t;

struct BlockExit
{
	std::uint8_t numSuccessors;
	NodeID first;  // first exit node if numSuccessors >= 1
	NodeID second;  // second exit node if numSuccessors == 2
};

constexpr std::uint32_t maxBlocks = 64;

struct TestGraph
{
	explicit TestGraph(std::vector<BlockExit> const& _exits):
		cfg(EVMDialect::strictAssemblyForEVM(langutil::EVMVersion::current())),
		successors(_exits.size())
	{
		auto const numBlocks = static_cast<NodeID>(_exits.size());
		for (NodeID i = 0; i < numBlocks; ++i)
			cfg.makeBlock(nullptr);

		for (NodeID i = 0; i < numBlocks; ++i)
		{
			auto const& [numSuccessors, first, second] = _exits[i];
			auto& block = cfg.block(SSACFG::BlockId{i});
			if (numSuccessors == 1)
			{
				block.exit = SSACFG::BasicBlock::Jump{SSACFG::BlockId{first}};
				successors[i] = {first};
			}
			else if (numSuccessors == 2)
			{
				block.exit = SSACFG::BasicBlock::ConditionalJump{{}, SSACFG::BlockId{first}, SSACFG::BlockId{second}};
				successors[i] = {first, second};
			}
			for (NodeID const successor: successors[i])
				cfg.block(SSACFG::BlockId{successor}).entries.push_back(SSACFG::BlockId{i});
		}
	}

	SSACFG cfg;
	std::vector<std::vector<NodeID>> successors;
};

}

static void DominatorTreeMatchesDefinition(std::vector<BlockExit> const& _exits)
{
	auto const numBlocks = static_cast<NodeID>(_exits.size());
	TestGraph const graph(_exits);
	auto const& successors = graph.successors;

	DepthFirstSpanningTree const dfsTree(graph.cfg);
	DominatorTree const dominatorTree(dfsTree);

	// Blocks reachable from the entry without passing through `_removed`.
	auto const reachableWithout = [&](std::optional<NodeID> const& _removed) {
		std::vector<std::uint8_t> reached(numBlocks, false);
		if (_removed == 0u)
			return reached;
		std::vector<NodeID> stack{0};
		reached[0] = true;
		while (!stack.empty())
		{
			NodeID const u = stack.back();
			stack.pop_back();
			for (NodeID const v: successors[u])
				if (!reached[v] && v != _removed)
				{
					reached[v] = true;
					stack.push_back(v);
				}
		}
		return reached;
	};

	std::vector<std::uint8_t> const reachable = reachableWithout(std::nullopt);
	for (NodeID i = 0; i < numBlocks; ++i)
		ASSERT_EQ(dfsTree.reachable(SSACFG::BlockId{i}), static_cast<bool>(reachable[i])) << "block " << i;

	// dom[v][d] = whether d dominates v.
	std::vector dom(numBlocks, std::vector<std::uint8_t>(numBlocks, false));
	for (NodeID d = 0; d < numBlocks; ++d)
	{
		std::vector<std::uint8_t> const reachedWithoutD = reachableWithout(d);
		for (NodeID v = 0; v < numBlocks; ++v)
			dom[v][d] = reachable[v] && !reachedWithoutD[v];
	}

	for (NodeID a = 0; a < numBlocks; ++a)
	{
		if (!reachable[a])
			continue;
		for (NodeID b = 0; b < numBlocks; ++b)
			if (reachable[b])
				ASSERT_EQ(dominatorTree.dominates(SSACFG::BlockId{a}, SSACFG::BlockId{b}), static_cast<bool>(dom[b][a]))
					<< "does " << a << " dominate " << b << "?";
	}

	ASSERT_EQ(dominatorTree.immediateDominator(SSACFG::BlockId{0}).value, 0u);
	for (NodeID b = 1; b < numBlocks; ++b)
	{
		if (!reachable[b])
			continue;
		NodeID const idom = dominatorTree.immediateDominator(SSACFG::BlockId{b}).value;
		ASSERT_LT(idom, numBlocks) << "block " << b;
		ASSERT_NE(idom, b) << "block " << b;
		ASSERT_TRUE(dom[b][idom]) << "idom " << idom << " of block " << b << " does not dominate it";
		for (NodeID d = 0; d < numBlocks; ++d)
			if (d != b && dom[b][d])
				ASSERT_TRUE(dom[idom][d])
					<< "strict dominator " << d << " of block " << b << " does not dominate its idom " << idom;
	}

	for (NodeID u = 0; u < numBlocks; ++u)
		if (!reachable[u])
		{
			EXPECT_THROW(
				dominatorTree.dominates(SSACFG::BlockId{u}, SSACFG::BlockId{0}),
				YulAssertion
			);
			EXPECT_THROW(
				dominatorTree.dominates(SSACFG::BlockId{0}, SSACFG::BlockId{u}),
				YulAssertion
			);
			EXPECT_THROW(dominatorTree.immediateDominator(SSACFG::BlockId{u}), YulAssertion);
		}
}

FUZZ_TEST(DominatorTreeProperty, DominatorTreeMatchesDefinition)
	.WithDomains(
		fuzztest::FlatMap(
			[](NodeID const _numBlocks) {
				return fuzztest::VectorOf(
					fuzztest::StructOf<BlockExit>(
						fuzztest::ElementOf<std::uint8_t>({0, 1, 1, 2, 2, 2}),
						fuzztest::InRange<NodeID>(0, _numBlocks - 1),
						fuzztest::InRange<NodeID>(0, _numBlocks - 1)
					)
				).WithSize(_numBlocks);
			},
			fuzztest::InRange<NodeID>(1, maxBlocks)
		)
	);

}
