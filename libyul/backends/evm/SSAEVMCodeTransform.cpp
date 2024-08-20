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
#include <libyul/backends/evm/SSAEVMCodeTransform.h>

#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/backends/evm/SSAControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/StackLayoutGenerator.h>

#include <libyul/Utilities.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/cxx20.h>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take_last.hpp>

using namespace solidity;
using namespace solidity::yul;

namespace {

std::string ssaCfgVarToString(SSACFG const& _ssacfg, SSACFG::ValueId _var)
{
	if (_var.value == std::numeric_limits<size_t>::max())
		return std::string("INVALID");
	auto const& info = _ssacfg.valueInfo(_var);
	return std::visit(
		util::GenericVisitor{
			[&](SSACFG::UnreachableValue const&) -> std::string {
				return "[unreachable]";
			},
			[&](SSACFG::LiteralValue const& _literal) {
				std::stringstream str;
				str << _literal.value;
				return str.str();
			},
			[&](auto const&) {
				return "v" + std::to_string(_var.value);
			}
		},
		info
	);
}

void debugPrintCFG(SSACFG const& _ssacfg, LivenessData const& _liveness) {
		auto varToString = [&](SSACFG::ValueId _var) {
			return ssaCfgVarToString(_ssacfg, _var);
		};
		auto printGraph = [&](SSACFG::BlockId _root) {
			util::BreadthFirstSearch<SSACFG::BlockId> bfs{{{_root}}};
			bfs.run([&](SSACFG::BlockId _blockId, auto _addChild) {
						//yulAssert(builder.blockInfo(_blockId).sealed, "Some block isn't sealed.");
						auto const& block = _ssacfg.block(_blockId);
						std::cout << "Block b" << _blockId.value << ":" << std::endl;
						std::cout << "[ " << util::joinHumanReadable(_liveness[_blockId].liveIn | ranges::view::transform(varToString)) << " ] => [ "
								  <<  util::joinHumanReadable(_liveness[_blockId].liveOut | ranges::view::transform(varToString)) << " ]" << std::endl;
						for(auto phi: block.phis)
						{
							auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_ssacfg.valueInfo(phi));
							yulAssert(phiInfo);
							std::cout << "  " << varToString(phi) << " := phi(" << std::endl;
							yulAssert(_ssacfg.block(phiInfo->block).entries.size() == phiInfo->arguments.size());
							for (auto&& [entry, input]: ranges::zip_view(_ssacfg.block(phiInfo->block).entries, phiInfo->arguments))
								std::cout << "    b" << entry.value << " => " << varToString(input) << std::endl;
							std::cout << "  )" << std::endl;
						}
						yulAssert(block.operations.size() == _liveness[_blockId].operationLiveOuts.size());
						for (auto&& [op, liveOut]: ranges::zip_view(block.operations, _liveness[_blockId].operationLiveOuts))
						{
							std::cout << "  ";
							if (!op.outputs.empty())
								std::cout << util::joinHumanReadable(op.outputs | ranges::view::transform(varToString)) << " := ";

							std::visit(util::GenericVisitor{
										   [&](SSACFG::Call const& _call) {
											   std::cout << _call.function.get().name.str();
											   std::cout << "(";
											   std::cout << util::joinHumanReadable(op.inputs | ranges::view::reverse | ranges::view::transform(varToString));
											   std::cout << ")";
										   },
										   [&](SSACFG::BuiltinCall const& _call) {
											   std::cout << _call.builtin.get().name.str();
											   std::cout << "(";
											   std::cout << util::joinHumanReadable(op.inputs | ranges::view::reverse | ranges::view::transform(varToString));
											   std::cout << ")";
										   }
									   }, op.kind);
							std::cout << "    [LIVEOUT: " << util::joinHumanReadable(liveOut | ranges::view::transform(varToString)) << "]";
							std::cout << std::endl;
						}
						std::visit(util::GenericVisitor{
									   [&](SSACFG::BasicBlock::MainExit const&)
									   {
										   std::cout << "  [MAIN EXIT]" << std::endl;
									   },
									   [&](SSACFG::BasicBlock::Jump const& _jump)
									   {
										   std::cout << "  jump(b" << _jump.target.value << ")" << std::endl;
										   _addChild(_jump.target);
									   },
									   [&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
									   {
										   std::cout << "  jumpif(" << varToString(_conditionalJump.condition) << ":" << std::endl;
										   std::cout << "    true => b" << _conditionalJump.nonZero.value << std::endl;
										   std::cout << "    false => b" <<  _conditionalJump.zero.value << std::endl;
										   std::cout << "  )" << std::endl;
										   _addChild(_conditionalJump.nonZero);
										   _addChild(_conditionalJump.zero);
									   },
									   [&](SSACFG::BasicBlock::JumpTable const& _jumpTable)
									   {
										   std::cout << "  jumpv(" << varToString(_jumpTable.value) << ":" << std::endl;
										   for (auto [v, b]: _jumpTable.cases)
										   {
											   std::cout << "    " << v << " => b" << b.value << std::endl;
											   _addChild(b);
										   }
										   _addChild(_jumpTable.defaultCase);
										   std::cout << "    _ => b" << _jumpTable.defaultCase.value << std::endl;
										   std::cout << "  )" << std::endl;
									   },
									   [&](SSACFG::BasicBlock::FunctionReturn const& _return)
									   {
										   std::cout << "  leave(" << util::joinHumanReadable(_return.returnValues | ranges::view::transform(varToString)) << ")" << std::endl;
									   },
									   [&](SSACFG::BasicBlock::Terminated const&) { std::cout << "  [TERMINATED]" << std::endl; }
								   }, block.exit);

						std::cout << std::endl;
					});
		};
		printGraph(SSACFG::BlockId{0});
		for (auto function: _ssacfg.functions)
		{
			auto& info = _ssacfg.functionInfos.at(&function.get());
			std::cout << "FUNCTION " << function.get().name.str() << std::endl;
			printGraph(info.entry);
		}
}

// TODO: note: this is a naive calculation with very bad complexity that
// should be replaced by a proper algorithm exploiting the SSA properties.
// There are linear-time algorithms for this.
LivenessData calculateLiveness(SSACFG& _ssacfg)
{
	LivenessData blockData(_ssacfg.numBlocks());
	auto analyze = [&](SSACFG::BlockId _root) {
		while(true)
		{
			bool allSame = true;

			std::set<SSACFG::BlockId> visited;
			std::list<SSACFG::BlockId> toVisit{{_root}};

			std::vector<SSACFG::BlockId> dfsOrder;
			auto dfs = [&, visited = std::set<SSACFG::BlockId>{}](SSACFG::BlockId _block, auto& _recurse) mutable -> void {
				if (visited.count(_block))
					return;
				visited.emplace(_block);
				dfsOrder.emplace_back(_block);
				_ssacfg.block(_block).forEachExit([&](SSACFG::BlockId _exit) { return _recurse(_exit, _recurse); });
			};
			dfs(_root, dfs);

			for (SSACFG::BlockId block: dfsOrder | ranges::view::reverse)
			{
				auto previousLiveOut = blockData[block].liveOut;
				auto previousLiveIn = blockData[block].liveIn;
				std::set<SSACFG::ValueId> innerBlockLive;
				std::visit(util::GenericVisitor{
							   [&](SSACFG::BasicBlock::MainExit const&) {
							   },
							   [&](SSACFG::BasicBlock::Jump const&) {},
							   [&](SSACFG::BasicBlock::ConditionalJump const& _jump) { innerBlockLive.emplace(_jump.condition); },
							   [&](SSACFG::BasicBlock::JumpTable const& _jump) { innerBlockLive.emplace(_jump.value); },
							   [&](SSACFG::BasicBlock::FunctionReturn const& _return)
							   {
								   blockData[block].liveOut += _return.returnValues | ranges::to<std::set>;
							   },
							   [&](SSACFG::BasicBlock::Terminated const&)
							   {
							   },
						   }, _ssacfg.block(block).exit);
				_ssacfg.block(block).forEachExit([&](SSACFG::BlockId _exit) {
					std::optional<size_t> entryIndex;
					for (auto&& [i, exitEntries]: _ssacfg.block(_exit).entries | ranges::view::enumerate)
						if (exitEntries == block)
						{
							yulAssert(!entryIndex.has_value());
							entryIndex = i;
						}
					yulAssert(entryIndex.has_value());
					blockData[block].liveOut += blockData[_exit].liveIn;
					for (auto phi: _ssacfg.block(_exit).phis)
					{
						auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&_ssacfg.valueInfo(phi));
						yulAssert(phiInfo);
						blockData[block].liveOut.insert(phiInfo->arguments.at(*entryIndex));
					}
				});

				std::set<SSACFG::ValueId> invars = blockData[block].liveOut;
				invars += innerBlockLive;
				blockData[block].operationLiveOuts.resize(_ssacfg.block(block).operations.size());
				for(auto&& [op, opLiveOut]: ranges::zip_view(_ssacfg.block(block).operations, blockData[block].operationLiveOuts) | ranges::view::reverse)
				{
					opLiveOut = invars;
					invars -= op.outputs;
					invars += op.inputs;
				}
				for (auto phi: _ssacfg.block(block).phis)
				{
					auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_ssacfg.valueInfo(phi));
					yulAssert(phiInfo);
					invars.erase(phi);
				}
				// TODO: don't add them in the first place
				for (auto it = invars.begin(); it != invars.end();)
				{
					if (std::holds_alternative<SSACFG::LiteralValue>(_ssacfg.valueInfo(*it)))
						it = invars.erase(it);
					else
						++it;
				}
				// TODO: don't add them in the first place
				{
					auto& liveOut = blockData[block].liveOut;
					for (auto it = liveOut.begin(); it != liveOut.end();)
					{
						if (std::holds_alternative<SSACFG::LiteralValue>(_ssacfg.valueInfo(*it)))
							it = liveOut.erase(it);
						else
							++it;
					}
				}
				blockData[block].liveIn += invars;

				if (blockData[block].liveOut != previousLiveOut || blockData[block].liveIn != previousLiveIn)
					allSame = false;
			}

			// Continue until the sets all saturate. Horrible runtime
			// TODO: replace by proper SSA-based liveness analysis.
			if (allSame)
				break;
		}
	};

	analyze({SSACFG::BlockId{0}});
	for (auto function: _ssacfg.functions)
	{
		auto const& info = _ssacfg.functionInfos.at(&function.get());
		analyze(info.entry);
	}
	return blockData;
}

}

std::vector<StackTooDeepError> SSAEVMCodeTransform::run(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	EVMDialect const& _dialect,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions
)
{
	std::unique_ptr<SSACFG> ssacfg = SSAControlFlowGraphBuilder::build(_analysisInfo, _dialect, _block);
	auto const liveness = calculateLiveness(*ssacfg);
	debugPrintCFG(*ssacfg, liveness);

	SSAEVMCodeTransform optimizedCodeTransform(
		_assembly,
		_builtinContext,
		_useNamedLabelsForFunctions,
		*ssacfg,
		liveness
	);
	// Force main entry block to start from an empty stack.
	optimizedCodeTransform.blockData(SSACFG::BlockId{0}).stackIn = std::make_optional(std::vector<StackSlot>{});
	optimizedCodeTransform(SSACFG::BlockId{0});
	for (auto function: ssacfg->functions)
		optimizedCodeTransform(ssacfg->functionInfos.at(&function.get()));
	return std::move(optimizedCodeTransform.m_stackErrors);
}

SSAEVMCodeTransform::SSAEVMCodeTransform(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions,
	SSACFG const& _cfg,
	LivenessData const& _livenessData
):
	m_assembly(_assembly),
	m_builtinContext(_builtinContext),
	m_cfg(_cfg),
	m_livenessData(_livenessData),
	m_functionLabels([&](){
		std::map<SSACFG::FunctionInfo const*, AbstractAssembly::LabelID> functionLabels;
		std::set<YulString> assignedFunctionNames;
		for (auto function: m_cfg.functions)
		{
			auto const& functionInfo = m_cfg.functionInfos.at(&function.get());
			bool nameAlreadySeen = !assignedFunctionNames.insert(function.get().name).second;
			if (_useNamedLabelsForFunctions == UseNamedLabels::YesAndForceUnique)
				yulAssert(!nameAlreadySeen);
			bool useNamedLabel = _useNamedLabelsForFunctions != UseNamedLabels::Never && !nameAlreadySeen;
			functionLabels[&functionInfo] = useNamedLabel ?
				m_assembly.namedLabel(
					function.get().name.str(),
					function.get().arguments.size(),
					function.get().returns.size(),
					functionInfo.debugData ? functionInfo.debugData->astID : std::nullopt
				) :
				m_assembly.newLabelId();
		}
		return functionLabels;
	}()),
	m_blockData(_cfg.numBlocks())
{
}

AbstractAssembly::LabelID SSAEVMCodeTransform::getFunctionLabel(Scope::Function const& _function)
{
	return m_functionLabels.at(&m_cfg.functionInfos.at(&_function));
}

void SSAEVMCodeTransform::pop()
{
	yulAssert(!m_stack.empty());
	m_stack.pop_back();
	m_assembly.appendInstruction(evmasm::Instruction::POP);
}

void SSAEVMCodeTransform::swap(size_t _depth)
{
	m_assembly.appendInstruction(evmasm::swapInstruction(static_cast<unsigned>(_depth)));
	yulAssert(m_stack.size() > _depth);
	std::swap(m_stack[m_stack.size() - _depth - 1], m_stack.back());
}

void SSAEVMCodeTransform::operator()(SSACFG::BlockId _block)
{
	if (blockData(_block).generated)
		return;
	blockData(_block).generated = true;

	{
		// TODO: remove - only used for debugging
		m_currentBlock = _block;
	}

	ScopedSaveAndRestore stackSave{m_stack, {}};

	{
		auto& label = blockData(_block).label;
		if (!label)
			label = m_assembly.newLabelId();
		m_assembly.appendLabel(*label);
	}

	{
		auto& stackIn = blockData(_block).stackIn;
		yulAssert(stackIn, "No starting layout for block b" + std::to_string(_block.value));
		// if (!stackIn)
		//	stackIn = liveness(_block).liveIn | ranges::to<std::vector<StackSlot>>;
		m_stack = *stackIn | ranges::to<std::vector<StackSlot>>;
	}
	m_assembly.setStackHeight(static_cast<int>(m_stack.size()));
	std::cout << "Generate block b" << _block.value << ": " << stackToString(m_stack) << std::endl;

	yulAssert(m_cfg.block(_block).operations.size() == liveness(_block).operationLiveOuts.size());
	for (auto&& [op, liveOut]: ranges::zip_view(m_cfg.block(_block).operations, liveness(_block).operationLiveOuts))
		(*this)(op, liveOut);

	auto restrictStackToSet = [&](std::set<SSACFG::ValueId> liveOut) {
		auto originalLiveOut = liveOut;
		std::vector<size_t> freeSlots;
		for (auto&& [pos, slot]: m_stack | ranges::view::enumerate)
		{
			if (std::holds_alternative<AbstractAssembly::LabelID>(slot))
				freeSlots.emplace_back(pos);
			if (!liveOut.erase(std::get<SSACFG::ValueId>(slot)))
				freeSlots.emplace_back(pos);
		}
		while (!freeSlots.empty())
		{
			auto lastFreeSlot = freeSlots.back();
			freeSlots.pop_back();
			if (lastFreeSlot == m_stack.size() - 1)
				pop();
			else
			{
				auto slot = m_stack[lastFreeSlot];
				swap(m_stack.size() - lastFreeSlot - 1);
				yulAssert(slot == m_stack.back());
				pop();
			}
		}
		// Validate resulting m_stack is just an ordering of the block's live out set.
		bool same = true;
		if (originalLiveOut.size() != m_stack.size())
			same = false;
		if (same)
		{
			if (!((m_stack | ranges::to<std::set<StackSlot>>) == (originalLiveOut | ranges::to<std::set<StackSlot>>)))
				same = false;
		}
		if (!same) {
			std::cout << "CLEAN UP STACK " << stackToString(m_stack) << " <=> " << stackToString(originalLiveOut | ranges::to<std::vector<StackSlot>>) <<  " (current block: " << m_currentBlock.value << ")" << std::endl;
		}
		yulAssert(same);
		yulAssert(originalLiveOut.size() == m_stack.size());
		yulAssert((m_stack | ranges::to<std::set<StackSlot>>) == (originalLiveOut | ranges::to<std::set<StackSlot>>));
	};
	auto cleanUpStack = [&](std::optional<SSACFG::ValueId> _additional = std::nullopt)
	{
		auto liveOut = liveness(_block).liveOut;
		if (_additional)
			liveOut.emplace(*_additional);
		restrictStackToSet(liveOut);
	};
	auto transformStackWithTargetPhis = [&](SSACFG::BlockId _current, SSACFG::BlockId _target, std::vector<StackSlot> const& _stack, bool _direction) {
		auto const& targetInfo = m_cfg.block(_target);
		size_t phiArgumentIndex = [&]() {
			auto idx = util::findOffset(targetInfo.entries, _current);
			yulAssert(idx, "Current block not found as entry in one of the exits of the current block.");
			return *idx;
		}();
		std::map<StackSlot, StackSlot> translation;
		for (auto const& phi: m_cfg.block(_target).phis)
		{
			auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&m_cfg.valueInfo(phi));
			yulAssert(phiInfo);
			if (_direction)
				translation[phiInfo->arguments.at(phiArgumentIndex)] = phi;
			else
				translation[phi] = phiInfo->arguments.at(phiArgumentIndex);
		}

		return _stack | ranges::view::transform([&](StackSlot const& _slot) {
			return util::valueOrDefault(translation, _slot, _slot);
		}) | ranges::to<std::vector>;
	};
	auto currentStackToTargetStack = [&](SSACFG::BlockId _target) {
		auto result = transformStackWithTargetPhis(_block, _target, m_stack, true);
		// TODO: this is a hack... this should happen elsewhere somehow...
		auto const& targetInfo = m_cfg.block(_target);
		size_t phiArgumentIndex = [&]() {
			auto idx = util::findOffset(targetInfo.entries, _block);
			yulAssert(idx, "Current block not found as entry in one of the exits of the current block.");
			return *idx;
		}();
		for (auto const& phi: m_cfg.block(_target).phis)
		{
			auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&m_cfg.valueInfo(phi));
			yulAssert(phiInfo);
			if (!util::contains(m_stack, StackSlot{phi}))
			{
				bringUpSlot(phiInfo->arguments.at(phiArgumentIndex));
				result.emplace_back(phi);
			}
		}
		return result;
	};
	auto inSetOfBlock = [&](SSACFG::BlockId _target) {
		std::set<SSACFG::ValueId> result = liveness(_target).liveIn;
		auto const& targetInfo = m_cfg.block(_target);
		size_t phiArgumentIndex = [&]() {
			auto idx = util::findOffset(targetInfo.entries, _block);
			yulAssert(idx, "Current block not found as entry in one of the exits of the current block.");
			return *idx;
		}();
		for (auto phi: m_cfg.block(_target).phis)
		{
			auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&m_cfg.valueInfo(phi));
			yulAssert(phiInfo);
			auto slot = phiInfo->arguments.at(phiArgumentIndex);
			if (!std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(slot)))
				result.emplace(slot);
		}
		return result;
	};
	auto targetStackToCurrentStack = [&](SSACFG::BlockId _target) {
		yulAssert(blockData(_target).stackIn);
		auto const& targetInfo = m_cfg.block(_target);
		size_t phiArgumentIndex = [&]() {
			auto idx = util::findOffset(targetInfo.entries, _block);
			yulAssert(idx, "Current block not found as entry in one of the exits of the current block.");
			return *idx;
		}();
		auto stack = transformStackWithTargetPhis(_block, _target, *blockData(_target).stackIn, false);
		for (auto& slot: stack)
		{
			auto* value = std::get_if<SSACFG::ValueId>(&slot);
			if (!liveness(_target).liveIn.count(*value))
			{
				bool skip = true;
				for (auto phi: m_cfg.block(_target).phis)
				{
					auto const& phiInfo = std::get<SSACFG::PhiValue>(m_cfg.valueInfo(phi));

					if(*value == phiInfo.arguments.at(phiArgumentIndex))
					{
						skip = false;
						break;
					}
				}
				// TODO: simplify; get rid of skip
				if (skip)
				{
					std::cout << "Weird slot: " << stackSlotToString(slot) << std::endl;
				}
			}
		}
		return stack;
	};

	std::visit(util::GenericVisitor{
		[&](SSACFG::BasicBlock::MainExit const& _exit) {
			(void)_exit; // TODO: should this get debug data?
			m_assembly.appendInstruction(evmasm::Instruction::STOP);
		},
		[&](SSACFG::BasicBlock::Jump const& _jump){
			auto& targetLabel = blockData(_jump.target).label;
			if (!targetLabel)
				targetLabel = m_assembly.newLabelId();
			auto& targetStack = blockData(_jump.target).stackIn;
			if (targetStack)
				createExactStack(targetStackToCurrentStack(_jump.target));
			else
			{
				cleanUpStack();
				targetStack = currentStackToTargetStack(_jump.target);
			}
			m_assembly.appendJumpTo(*targetLabel);
			(*this)(_jump.target);
		},
		[&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump) {
			auto& nonZeroLayout = blockData(_conditionalJump.nonZero).stackIn;
			auto& nonZeroLabel = blockData(_conditionalJump.nonZero).label;
			if (!nonZeroLabel)
				nonZeroLabel = m_assembly.newLabelId();
			auto& zeroLayout = blockData(_conditionalJump.zero).stackIn;
			auto& zeroLabel = blockData(_conditionalJump.zero).label;
			if (!zeroLabel)
				zeroLabel = m_assembly.newLabelId();
			if (nonZeroLayout)
			{
				createExactStack(targetStackToCurrentStack(_conditionalJump.nonZero));
				bringUpSlot(_conditionalJump.condition);
				m_stack.pop_back();
			}
			else
			{
				cleanUpStack(_conditionalJump.condition);
				createStackTop({_conditionalJump.condition}, liveness(_block).liveOut);
				m_stack.pop_back();
				nonZeroLayout = currentStackToTargetStack(_conditionalJump.nonZero);
			}
			m_assembly.appendJumpToIf(*nonZeroLabel);

			if (zeroLayout)
				createExactStack(targetStackToCurrentStack(_conditionalJump.zero));
			else
			{
				std::cout << "STACK1: " << stackToString(m_stack) << std::endl;
				auto inset = inSetOfBlock(_conditionalJump.zero);
				std::cout << "INSET: " << stackToString(inset | ranges::to<std::vector<StackSlot>>) << std::endl;
				restrictStackToSet(inset);
				std::cout << "STACK2: " << stackToString(m_stack) << std::endl;
				zeroLayout = currentStackToTargetStack(_conditionalJump.zero);
				std::cout << "STACK3: " << stackToString(*zeroLayout) << std::endl;
				std::cout << "(to b" << _conditionalJump.zero.value << ")" << std::endl;
			}
			m_assembly.appendJumpTo(*zeroLabel);
			(*this)(_conditionalJump.zero);
			(*this)(_conditionalJump.nonZero);
		},
		[&](SSACFG::BasicBlock::JumpTable const&){ yulAssert(false, "Jump tables not yet implemented."); },
		[&](SSACFG::BasicBlock::FunctionReturn const& _return){
			// Need to be able to also swap up return label!
			yulAssert(static_cast<size_t>(m_assembly.stackHeight()) == m_stack.size());
			m_assembly.setStackHeight(static_cast<int>(m_stack.size()) + 1);
			if (_return.returnValues.empty())
				while (!m_stack.empty())
					pop();
			else
			{
				auto targetStack = _return.returnValues | ranges::view::drop_exactly(1) | ranges::to<std::vector<StackSlot>>;
				targetStack.emplace_back(_return.returnValues.front());
				createExactStack(targetStack);
				// Swap up return label.
				m_assembly.appendInstruction(evmasm::swapInstruction(static_cast<unsigned>(targetStack.size())));
			}
			m_assembly.appendJump(0, AbstractAssembly::JumpType::OutOfFunction);
			m_stack.clear();
		},
		[&](SSACFG::BasicBlock::Terminated const&){
			// TODO: assert that last instruction terminated.
			// To be sure just emit another INVALID - should be removed by optimizer.
			m_assembly.appendInstruction(evmasm::Instruction::INVALID);
		}
	}, m_cfg.block(_block).exit);
}

void SSAEVMCodeTransform::operator()(SSACFG::Operation const& _operation, std::set<SSACFG::ValueId> const& _liveOut)
{
	std::optional<AbstractAssembly::LabelID> returnLabel;
	if (auto const* call = std::get_if<SSACFG::Call>(&_operation.kind))
		if (call->canContinue)
			returnLabel = m_assembly.newLabelId();

	std::vector<StackSlot> requiredStackTop;
	if (returnLabel)
		requiredStackTop.emplace_back(*returnLabel);
	requiredStackTop += _operation.inputs;

	// TODO: avoid having to do this
	auto liveOut = _liveOut;
	for (auto it = liveOut.begin(); it != liveOut.end();)
	{
		if (std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(*it)))
			it = liveOut.erase(it);
		else
			++it;
	}
	for (auto output: _operation.outputs)
		liveOut.erase(output);

	createStackTop(requiredStackTop, liveOut);
	std::visit(util::GenericVisitor {
		[&](SSACFG::BuiltinCall const& _builtin) {
			m_assembly.setSourceLocation(originLocationOf(_builtin));
			static_cast<BuiltinFunctionForEVM const&>(_builtin.builtin.get()).generateCode(
				_builtin.call,
				m_assembly,
				m_builtinContext
			);
		},
		[&](SSACFG::Call const& _call) {
			m_assembly.setSourceLocation(originLocationOf(_call));
			m_assembly.appendJumpTo(
				getFunctionLabel(_call.function),
				static_cast<int>(_call.function.get().returns.size() - _call.function.get().arguments.size()) - (_call.canContinue ? 1 : 0),
				AbstractAssembly::JumpType::IntoFunction
			);
			if (returnLabel)
				m_assembly.appendLabel(*returnLabel);
	   },
	}, _operation.kind);

	for (size_t i = 0; i < _operation.inputs.size() + (returnLabel ? 1 : 0); ++i)
		m_stack.pop_back();
	for (auto value: _operation.outputs)
		m_stack.push_back(value);

}

void SSAEVMCodeTransform::operator()(SSACFG::FunctionInfo const& _functionInfo)
{
	m_assembly.appendLabel(getFunctionLabel(_functionInfo.function));
	// Force function entry block to start from initial function layout.
	blockData(_functionInfo.entry).stackIn = _functionInfo.arguments | ranges::view::transform([](auto&& _tuple) { return std::get<1>(_tuple); }) | ranges::to<std::vector<StackSlot>>;
	(*this)(_functionInfo.entry);
}

void SSAEVMCodeTransform::createStackTop(
	std::vector<StackSlot> const& _targetTop,
	std::set<SSACFG::ValueId> const& _liveOut
)
{


	std::cout << "Create Stack Top " << stackToString(_targetTop) << " from " << stackToString(m_stack) << " (live out: " << stackToString(_liveOut|ranges::to<std::vector<StackSlot>>) << ")" << " (Current block: " << m_currentBlock.value << ")" << std::endl;
	createExactStack((_liveOut | ranges::to<std::vector<StackSlot>>) + _targetTop);
	return;

	// Hack to keep stacks manageably small despite being otherwise wastefull.
	auto cleanUpStack = [&]() {
		auto liveOut = _liveOut;
		for (auto const& slot: _targetTop)
		{
			auto* value = std::get_if<SSACFG::ValueId>(&slot);
			if (value)
				liveOut.emplace(*value);
		}
		std::vector<size_t> freeSlots;
		for (auto&& [pos, slot]: m_stack | ranges::view::enumerate)
		{
			if (std::holds_alternative<AbstractAssembly::LabelID>(slot))
				freeSlots.emplace_back(pos);
			if (!liveOut.erase(std::get<SSACFG::ValueId>(slot)))
				freeSlots.emplace_back(pos);
		}
		while (!freeSlots.empty())
		{
			auto lastFreeSlot = freeSlots.back();
			freeSlots.pop_back();
			if (lastFreeSlot == m_stack.size() - 1)
				pop();
			else
			{
				auto slot = m_stack[lastFreeSlot];
				swap(m_stack.size() - lastFreeSlot - 1);
				yulAssert(slot == m_stack.back());
				pop();
			}
		}
	};
	cleanUpStack();

	std::cout << "(After Hack) Create Stack Top " << stackToString(_targetTop) << " from " << stackToString(m_stack) << " (live out: " << stackToString(_liveOut|ranges::to<std::vector<StackSlot>>) << ")"  << " (Current block: " << m_currentBlock.value << ")" << std::endl;

	// Naive version that ignores the current _liveOut set and generates a fresh copy of all elements in _targetTop
	// ignoring the fact that for some elements in m_stack this may be the last use.
	// TODO: more optimal version
	for (auto slot: _targetTop)
		std::visit(util::GenericVisitor{
			[&](SSACFG::ValueId _value) {
				if (
					auto const* literal = std::get_if<SSACFG::LiteralValue>(&m_cfg.valueInfo(_value));
					!literal || literal->value != 0 || !m_assembly.evmVersion().hasPush0()
				)
					if (auto depth = util::findOffset(m_stack | ranges::view::reverse, _value))
					{
						if (*depth < 16)
						{
							m_assembly.appendInstruction(evmasm::dupInstruction(static_cast<unsigned>(*depth + 1)));
							m_stack.emplace_back(_value);
							return;
						}
					}
				std::visit(util::GenericVisitor{
					[&](SSACFG::UnreachableValue const&) { solAssert(false); },
					[&](SSACFG::VariableValue const&) { solAssert(false); },
					[&](SSACFG::PhiValue const&) { solAssert(false); },
					[&](SSACFG::LiteralValue const& _literal) {
						m_assembly.appendConstant(_literal.value);
						m_stack.emplace_back(_value);
					}
				}, m_cfg.valueInfo(_value));
			},
			[&](AbstractAssembly::LabelID _label) {
				m_assembly.appendLabelReference(_label);
				m_stack.emplace_back(_label);
			}
		}, slot);
}

void SSAEVMCodeTransform::bringUpSlot(StackSlot const& _slot)
{
	std::visit(util::GenericVisitor{
		[&](SSACFG::ValueId _value) {
			if (auto depth = util::findOffset(m_stack | ranges::view::reverse, _slot))
			{
				m_assembly.appendInstruction(evmasm::dupInstruction(static_cast<unsigned>(*depth + 1)));
				m_stack.emplace_back(_slot);
				return;
			}
			std::visit(util::GenericVisitor{
				[&](SSACFG::UnreachableValue const&) { solAssert(false); },
				[&](SSACFG::VariableValue const&) { solAssert(false); },
				[&](SSACFG::PhiValue const&) { solAssert(false); },
				[&](SSACFG::LiteralValue const& _literal) {
					m_assembly.appendConstant(_literal.value);
					m_stack.emplace_back(_value);
				}
			}, m_cfg.valueInfo(_value));
		},
		[&](AbstractAssembly::LabelID _label) {
			m_assembly.appendLabelReference(_label);
			m_stack.emplace_back(_label);
		}
	}, _slot);
}

void SSAEVMCodeTransform::createExactStack(std::vector<StackSlot> const& _target)
{
	// TODO: again a rather quick and dirty algorithm to be refined to an optimal one.
	std::map<StackSlot, std::set<size_t>> targetPositions;

	for (auto&& [pos, slot]: _target | ranges::view::enumerate)
		targetPositions[slot].insert(pos);

	auto swapTopToTargetOrPop = [&]() {
		auto top = m_stack.back();
		// Check if the top should be somewhere else where it not already is.
		for(auto targetPos: targetPositions[top])
			if (targetPos < m_stack.size() - 1 && m_stack[targetPos] != top)
			{
				// If so swap it there.
				swap(m_stack.size() - targetPos - 1);
				yulAssert(m_stack[targetPos] == top);
				return;
			}
		if (!targetPositions[top].empty() && *targetPositions[top].begin() >= m_stack.size())
		{
			bringUpSlot(_target.at(m_stack.size()));
		}
		else
		{
			// If not, pop it.
			pop();
		}
	};

	std::cout << "Create exact stack " + stackToString(_target) << " from " << stackToString(m_stack) << " (Current block: " << m_currentBlock.value << ")" << std::endl;
	std::cout << "Target positions: {" << std::endl;
	for (auto&& [key, values]: targetPositions)
		std::cout << "  " << stackSlotToString(key) << " => " << util::joinHumanReadable(values | ranges::view::transform([](auto x) { return std::to_string(x); })	) << std::endl;
	std::cout << "}" << std::endl;

	do
	{
		// As long as stack top isn't final
		while(m_stack.size() > _target.size() || (!m_stack.empty() && _target[m_stack.size() - 1] != m_stack.back()))
		{
			std::cout << "  ..." << stackToString(m_stack) << std::endl;

			// Try to swap it to where it is still needed - or otherwise pop it.
			swapTopToTargetOrPop();
		}

		// Stack top is final. If everything else is good, terminate - otherwise bring up one of the slots that are still needed in the target and continue.
	} while ([&](){
		 std::cout << "  ...(2)..." << stackToString(m_stack) << std::endl;
		for (auto [pos, slots]: ranges::zip_view(m_stack, _target) | ranges::view::enumerate)
		{
			auto [stack, target] = slots;
			if (stack != target)
			{
				bringUpSlot(target);
				return true;
			}
		}
		return false;
	}());
	while(m_stack.size() < _target.size())
		bringUpSlot(_target[m_stack.size()]);

	yulAssert(m_stack.size() == _target.size());
	yulAssert(m_stack == _target);
}

std::string SSAEVMCodeTransform::stackToString(std::vector<StackSlot> const& _stack)
{
	return "[" + util::joinHumanReadable(_stack | ranges::view::transform([&](StackSlot const& _slot) { return stackSlotToString(_slot); })) + "]";
}

std::string SSAEVMCodeTransform::stackSlotToString(StackSlot const& _slot)
{
	return std::visit(util::GenericVisitor{
		[&](SSACFG::ValueId _value) {
			return ssaCfgVarToString(m_cfg, _value);
		},
		[](AbstractAssembly::LabelID _label) {
			return "LABEL[" + std::to_string(_label) + "]";
		}
	}, _slot);
}
