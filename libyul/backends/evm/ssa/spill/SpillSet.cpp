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

#include <libyul/backends/evm/ssa/spill/SpillSet.h>

#include <libyul/backends/evm/ssa/stack/Shuffler.h>

#include <libyul/backends/evm/ssa/Stack.h>
#include <libyul/backends/evm/ssa/StackLayout.h>

#include <range/v3/view/zip.hpp>

#include <deque>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::spill;

namespace
{

/// Build the symbolic stack right after `_value`'s operation completes by replaying the recorded shuffles
/// and operation effects from the block's `stackIn`
StackData computeOperationOut(
	SSACFG const& _cfg,
	SSACFGStackLayout const& _layout,
	InstId const _value
)
{
	InstId const producer = _cfg.isProjection(_value) ? _cfg.inst(_value).inputs.front() : _value;

	SSACFG::BlockId const block = _cfg.inst(producer).block;
	auto const& blockLayout = _layout[block];
	yulAssert(blockLayout, fmt::format("producer {}'s block has no layout", producer));

	auto const& instructions = _cfg.block(block).instructions;
	yulAssert(blockLayout->operationShuffles.size() == instructions.size());
	StackData opOutStack = blockLayout->stackIn;
	for (auto const& [id, shuffle]: ranges::views::zip(instructions, blockLayout->operationShuffles))
	{
		if (!_cfg.isOperation(id))
			continue;
		replay(opOutStack, shuffle);

		SSACFG::Inst const& inst = _cfg.inst(id);
		// a call that can continue also consumes its return label, which sits right below the inputs
		std::size_t consumedSlots = inst.inputs.size();
		if (inst.opcode == InstOpcode::Call && _cfg.callPayload(id).canContinue)
			++consumedSlots;
		yulAssert(opOutStack.size() >= consumedSlots, "operation input layout smaller than consumed slot count");
		for (std::size_t i = 0; i < consumedSlots; ++i)
			opOutStack.pop_back();
		_cfg.forEachOutput(id, [&](InstId const output) {
			opOutStack.push_back(StackSlot::makeValue(_cfg, output));
		});

		if (id == producer)
			return opOutStack;
	}
	yulAssert(false, fmt::format("producer {} not found in its block's instructions", producer));
	solidity::util::unreachable();
}

/// The symbolic stack the Emitter faces at `_value`'s definition, where its `mstore` fires. Three cases:
/// - a phi: the merged value is materialized on its defining block's `stackIn`, so a single store there covers every incoming edge;
/// - a function argument: it has no producer operation and lives on the function entry stack, where CodeTransform emits `mstore` while the args are still laid out;
/// - any other value: it sits on its producer's `operationOut`.
StackData defStackFor(
	SSACFG const& _cfg,
	SSACFGStackLayout const& _layout,
	InstId const _value
)
{
	if (_cfg.isPhi(_value))
	{
		SSACFG::BlockId const block = _cfg.inst(_value).block;
		yulAssert(block.hasValue(), fmt::format("phi {} has no defining block", _value));
		auto const& blockLayout = _layout[block];
		yulAssert(blockLayout, fmt::format("phi {}'s defining block has no layout", _value));
		return blockLayout->stackIn;
	}
	if (_cfg.isFunctionArg(_value))
	{
		auto const& entryLayout = _layout[_cfg.entry];
		yulAssert(entryLayout, "entry block has no layout for function-arg def-site");
		return entryLayout->stackIn;
	}
	return computeOperationOut(_cfg, _layout, _value);
}

}

void SpillSet::closeUnderReachabilityConstraints(SSACFG const& _cfg, SSACFGStackLayout const& _layout, SpillStoreTraces* _storeTraces)
{
	if (_storeTraces)
		_storeTraces->clear();

	// work queue over variables that are marked for spillage
	std::deque<SpillKey> queue;
	for (SpillKey const key: spilledValues())
		queue.push_back(key);

	while (!queue.empty())
	{
		SpillKey const key = queue.front();
		queue.pop_front();

		InstId const value = key.value();
		StackData const defStack = defStackFor(_cfg, _layout, value);
		ensureDefSiteFeasible(key, defStack, queue, _storeTraces);
	}
}

void SpillSet::ensureDefSiteFeasible(
	SpillKey const _key,
	StackData const& _defStack,
	std::deque<SpillKey>& _workQueue,
	SpillStoreTraces* _storeTraces)
{
	// predicate = spill set minus the owner; the shuffle accumulates discovered culprits here.
	SpillSet spillSetWithoutOwner = without(_key);
	// [... defStack ..., _key]
	StackData const target = [&]{
		StackData result;
		result.reserve(_defStack.size() + 1);
		result.insert(result.end(), _defStack.begin(), _defStack.end());
		result.push_back(_key);
		return result;
	}();
	StackData workStack = _defStack;
	stack::ShuffleResult result = stack::shuffle(workStack, target, spillSetWithoutOwner);
	yulAssert(
		result.status == stack::ShuffleResult::Status::Admissible,
		fmt::format("def-site store for {} infeasible even after spilling siblings (status={})", _key, static_cast<int>(result.status))
	);

	// - if `_key` is reachable, it can be just DUPed and there shouldn't have been a stack too deep with it
	// - if `_key` is unreachable, there are > reachable stack depth distinct slots strictly above it and the
	//   shuffler heuristics should not pick anything that is already too deep as culprit
	yulAssert(!spillSetWithoutOwner.isSpilled(_key), "spill-aware shuffle reported the owner as its own blocker");

	if (_storeTraces)
	{
		// the `mstore` consuming the variable from the top concludes the def-site trace
		result.trace.push_back(ShuffleOp::store(_key));
		(*_storeTraces)[_key.value()] = std::move(result.trace);
	}

	for (SpillKey const culprit: spillSetWithoutOwner.spilledValues())
	{
		if (isSpilled(culprit))
			continue;
		add(culprit);
		_workQueue.push_back(culprit);
	}
}

SpillSet SpillSet::without(SpillKey const _key) const
{
	SpillSet result = *this;
	result.m_values.erase(_key);
	return result;
}
