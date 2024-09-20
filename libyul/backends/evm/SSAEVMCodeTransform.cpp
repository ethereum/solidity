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

#include <libyul/backends/evm/SSAControlFlowGraph.h>
#include <libyul/backends/evm/SSAControlFlowGraphBuilder.h>

#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>

#include <libevmasm/Instruction.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

using namespace solidity;
using namespace solidity::yul;

namespace
{

std::string ssaCfgVarToString(SSACFG const& _ssacfg, SSACFG::ValueId _var)
{
	if (_var.value == std::numeric_limits<size_t>::max())
		return "INVALID";
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
	std::unique_ptr<ControlFlow> controlFlow = SSAControlFlowGraphBuilder::build(_analysisInfo, _dialect, _block);
	ControlFlowLiveness liveness(*controlFlow);

	fmt::print("{}\n", controlFlow->toDot(&liveness));

	SSAEVMCodeTransform mainCodeTransform(
		_assembly,
		_builtinContext,
		_useNamedLabelsForFunctions,
		*controlFlow->mainGraph,
		*liveness.mainLiveness
	);

	// Force main entry block to start from an empty stack.
	mainCodeTransform.blockData(SSACFG::BlockId{0}).stackIn = std::make_optional(std::vector<StackSlot>{});
	mainCodeTransform(SSACFG::BlockId{0});

	std::vector<StackTooDeepError> stackErrors;
	if (!mainCodeTransform.m_stackErrors.empty())
		stackErrors = std::move(mainCodeTransform.m_stackErrors);

	for (auto const& [functionAndGraph, functionLiveness]: ranges::zip_view(controlFlow->functionGraphMapping, liveness.functionLiveness))
	{
		auto const& [function, functionGraph] = functionAndGraph;
		SSAEVMCodeTransform functionCodeTransform(
			_assembly,
			_builtinContext,
			_useNamedLabelsForFunctions,
			*functionGraph,
			*functionLiveness
		);
		functionCodeTransform(*function);
		if (!functionCodeTransform.m_stackErrors.empty())
			stackErrors.insert(stackErrors.end(), functionCodeTransform.m_stackErrors.begin(), functionCodeTransform.m_stackErrors.end());
	}

	return stackErrors;
}

SSAEVMCodeTransform::SSAEVMCodeTransform(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions,
	SSACFG const& _cfg,
	SSACFGLiveness const& _liveness
):
	m_assembly(_assembly),
	m_builtinContext(_builtinContext),
	m_cfg(_cfg),
	m_liveness(_liveness),
	m_functionLabels([&](){
		std::map<Scope::Function const*, AbstractAssembly::LabelID> functionLabels;

		if (_cfg.function)
		{
			std::set<YulString> assignedFunctionNames;
			bool nameAlreadySeen = !assignedFunctionNames.insert(_cfg.function->name).second;
			if (_useNamedLabelsForFunctions == UseNamedLabels::YesAndForceUnique)
				yulAssert(!nameAlreadySeen);
			bool useNamedLabel = _useNamedLabelsForFunctions != UseNamedLabels::Never && !nameAlreadySeen;
			functionLabels[_cfg.function] = useNamedLabel ?
				m_assembly.namedLabel(
					_cfg.function->name.str(),
					_cfg.arguments.size(),
					_cfg.returns.size(),
					_cfg.debugData ? _cfg.debugData->astID : std::nullopt
				) :
				m_assembly.newLabelId();
		}
		return functionLabels;
	}()),
	m_blockData(_cfg.numBlocks())
{
}

AbstractAssembly::LabelID SSAEVMCodeTransform::getFunctionLabel(Scope::Function const& _function) const
{
	return m_functionLabels.at(&_function);
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

void SSAEVMCodeTransform::operator()(SSACFG::BlockId const _block)
{
	{
		// TODO: remove - only used for debugging
		m_currentBlock = _block;
	}

	ScopedSaveAndRestore stackSave{m_stack, {}};

	if (_block != m_cfg.entry)
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

	yulAssert(m_cfg.block(_block).operations.size() == m_liveness.operationsLiveOut(_block).size());
	for (auto&& [op, liveOut]: ranges::zip_view(m_cfg.block(_block).operations, m_liveness.operationsLiveOut(_block)))
		(*this)(op, liveOut);

	auto restrictStackToSet = [&](std::set<SSACFG::ValueId> liveOut) {
		auto originalLiveOut = liveOut;
		std::vector<size_t> freeSlots;
		for (auto&& [pos, slot]: m_stack | ranges::views::enumerate)
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
		auto liveOut = m_liveness.liveOut(_block);
		if (_additional)
			liveOut.emplace(*_additional);
		restrictStackToSet(liveOut);
	};
	auto addPhiValuesToStack = [&](SSACFG::BlockId _target)
	{
		m_stack += m_cfg.block(_target).phis;
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
		return _stack | ranges::views::transform([&](StackSlot const& _slot) {
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
		std::set<SSACFG::ValueId> result = m_liveness.liveIn(_target);
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
			if (!m_liveness.liveIn(_target).count(*value))
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
				createStackTop({_conditionalJump.condition}, m_liveness.liveOut(_block));
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
				addPhiValuesToStack(_conditionalJump.zero);
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
				auto targetStack = _return.returnValues | ranges::views::drop_exactly(1) | ranges::to<std::vector<StackSlot>>;
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
	yulAssert(std::none_of(_liveOut.begin(), _liveOut.end(), [this](SSACFG::ValueId _valueId){ return std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(_valueId)); }));
	createStackTop(requiredStackTop, _liveOut - _operation.outputs);
	std::visit(util::GenericVisitor {
		[&](SSACFG::BuiltinCall const& _builtin) {
			m_assembly.setSourceLocation(originLocationOf(_builtin));
			// todo builtinfunctionforevm should be polymorphic
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
				static_cast<int>(_call.function.get().numReturns - _call.function.get().numArguments) - (_call.canContinue ? 1 : 0),
				AbstractAssembly::JumpType::IntoFunction
			);
			if (returnLabel)
				m_assembly.appendLabel(*returnLabel);
	   },
	}, _operation.kind);
	for (size_t i = 0; i < _operation.inputs.size() + (returnLabel ? 1 : 0); ++i)
		m_stack.pop_back();
	for (auto value: _operation.outputs)
		m_stack.emplace_back(value);
}

void SSAEVMCodeTransform::operator()(SSACFG::Call const&){}
void SSAEVMCodeTransform::operator()(SSACFG::BuiltinCall const&){}

void SSAEVMCodeTransform::operator()(Scope::Function const& _function)
{
	// Force function entry block to start from initial function layout.
	m_assembly.appendLabel(getFunctionLabel(_function));
	blockData(m_cfg.entry).stackIn = m_cfg.arguments | ranges::views::transform([](auto&& _tuple) { return std::get<1>(_tuple); }) | ranges::to<std::vector<StackSlot>>;
	(*this)(m_cfg.entry);
}

void SSAEVMCodeTransform::createStackTop(
	std::vector<StackSlot> const& _targetTop,
	std::set<SSACFG::ValueId> const& _liveOut
)
{
	std::cout << "Create Stack Top " << stackToString(_targetTop) << " from " << stackToString(m_stack) << " (live out: " << stackToString(_liveOut|ranges::to<std::vector<StackSlot>>) << ")" << " (Current block: " << m_currentBlock.value << ")" << std::endl;
	createExactStack((_liveOut | ranges::to<std::vector<StackSlot>>) + _targetTop);
	return; // todo
	// Hack to keep stacks small despite being otherwise wasteful.
	auto cleanUpStack = [&]() {
		auto liveOut = _liveOut;
		for (auto const& slot: _targetTop)
		{
			auto* value = std::get_if<SSACFG::ValueId>(&slot);
			if (value)
				liveOut.emplace(*value);
		}
		std::vector<size_t> freeSlots;
		for (auto&& [pos, slot]: m_stack | ranges::views::enumerate)
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
					if (auto depth = util::findOffset(m_stack | ranges::views::reverse, _value))
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
			if (auto depth = util::findOffset(m_stack | ranges::views::reverse, _slot))
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
	for (auto&& [pos, slot]: _target | ranges::views::enumerate)
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
		std::cout << "  " << stackSlotToString(key) << " => " << util::joinHumanReadable(values | ranges::views::transform([](auto x) { return std::to_string(x); })	) << std::endl;
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
		for (auto [pos, slots]: ranges::zip_view(m_stack, _target) | ranges::views::enumerate)
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
	return "[" + util::joinHumanReadable(_stack | ranges::views::transform([&](StackSlot const& _slot) { return stackSlotToString(_slot); })) + "]";
}

std::string SSAEVMCodeTransform::stackSlotToString(StackSlot const& _slot) const
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
