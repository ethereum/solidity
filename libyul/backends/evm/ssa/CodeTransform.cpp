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

#include <libyul/backends/evm/ssa/CodeTransform.h>

#include <libyul/backends/evm/ssa/CallGraph.h>
#include <libyul/backends/evm/ssa/PhiInverse.h>
#include <libyul/backends/evm/ssa/StackLayoutGenerator.h>
#include <libyul/backends/evm/ssa/StackUtils.h>

#include <libyul/backends/evm/EVMBuiltins.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/Visitor.h>

#include <range/v3/view/take_last.hpp>
#include <range/v3/view/zip.hpp>

using namespace solidity::yul;
using namespace solidity::yul::ssa;

namespace
{
void assertLayoutCompatibility(StackData const& _layout1, StackData const& _layout2)
{
	auto const compatibility = checkLayoutCompatibility(_layout1, _layout2);
	yulAssert(compatibility.ok(), compatibility.formatErrors());
}
}

void CodeTransform::run
(
	AbstractAssembly& _assembly,
	ControlFlowGraphs& _controlFlowGraphs,
	ControlFlowGraphsLiveness const& _controlFlowLiveness,
	BuiltinContext& _builtinContext
)
{
	yulAssert(&_controlFlowLiveness.controlFlowGraphs.get() == &_controlFlowGraphs);
	yulAssert(!_controlFlowLiveness.cfgLiveness.empty());
	yulAssert(_controlFlowGraphs.functionGraphs.size() == _controlFlowLiveness.cfgLiveness.size());
	FunctionLabels const functionLabels = registerFunctionLabels(_assembly, _controlFlowGraphs);
	CallGraph const callGraph(_controlFlowGraphs);

	std::size_t const numCFGs = _controlFlowGraphs.functionGraphs.size();
	std::vector<CallSites> callSitesPerCFG;
	std::vector<SSACFGStackLayout> layouts;
	std::vector<spill::SpillSet> spillSetsPerCFG;
	std::vector<spill::SpillStoreTraces> spillStoreTracesPerCFG;
	callSitesPerCFG.reserve(numCFGs);
	layouts.reserve(numCFGs);
	spillSetsPerCFG.reserve(numCFGs);
	spillStoreTracesPerCFG.reserve(numCFGs);

	for (std::size_t functionIndex = 0; functionIndex < numCFGs; ++functionIndex)
	{
		std::unique_ptr<SSACFG> const& functionGraphPtr = _controlFlowGraphs.functionGraphs[functionIndex];
		yulAssert(functionGraphPtr);
		SSACFG const& cfg = *functionGraphPtr;
		auto const& liveness = _controlFlowLiveness.cfgLiveness[functionIndex];
		yulAssert(liveness);
		auto const graphID = static_cast<ControlFlowGraphs::FunctionGraphID>(functionIndex);
		callSitesPerCFG.push_back(gatherCallSites(cfg));
		bool const spillingAllowed = !callGraph.isRecursive(graphID);
		auto [layout, spillSet, spillStoreTraces] = StackLayoutGenerator::generate(*liveness, callSitesPerCFG.back(), graphID, spillingAllowed);
		layouts.push_back(std::move(layout));
		spillSetsPerCFG.push_back(std::move(spillSet));
		spillStoreTracesPerCFG.push_back(std::move(spillStoreTraces));
	}

	// build up global addressing based on the spill sets
	spill::MemoryAddressing const addressing(_controlFlowGraphs, spillSetsPerCFG);

	for (std::size_t functionIndex = 0; functionIndex < _controlFlowGraphs.functionGraphs.size(); ++functionIndex)
	{
		SSACFG const& cfg = *_controlFlowGraphs.functionGraphs[functionIndex];
		auto const graphID = static_cast<ControlFlowGraphs::FunctionGraphID>(functionIndex);
		auto const& liveness = _controlFlowLiveness.cfgLiveness[functionIndex];
		yulAssert(liveness);
		CodeTransform transform(
			_assembly,
			_builtinContext,
			_controlFlowGraphs,
			functionLabels,
			callSitesPerCFG[functionIndex],
			cfg,
			layouts[functionIndex],
			spillSetsPerCFG[functionIndex],
			spillStoreTracesPerCFG[functionIndex],
			graphID,
			addressing
		);
		transform(cfg.entry);
	}
}

CodeTransform::FunctionLabels CodeTransform::registerFunctionLabels(
	AbstractAssembly& _assembly, ControlFlowGraphs const& _controlFlow)
{
	FunctionLabels functionLabels;
	std::set<std::string> assignedFunctionNames;

	for (std::size_t index = 0; index < _controlFlow.functionGraphs.size(); ++index)
	{
		std::unique_ptr<SSACFG> const& functionGraphPtr = _controlFlow.functionGraphs[index];
		yulAssert(functionGraphPtr);
		SSACFG const& functionGraph = *functionGraphPtr;
		if (functionGraph.isMainGraph())
			continue;
		auto const graphID = static_cast<ControlFlowGraphs::FunctionGraphID>(index);
		bool const nameAlreadySeen = !assignedFunctionNames.insert(functionGraph.name).second;
		auto const sourceID = [&]() -> std::optional<std::size_t> {
			if (functionGraph.debugInfo && functionGraph.debugInfo->graphDebugData)
				return functionGraph.debugInfo->graphDebugData->astID;
			return std::nullopt;
		}();
		functionLabels[graphID] = !nameAlreadySeen ?
			_assembly.namedLabel(
				functionGraph.name,
				functionGraph.arguments.size(),
				functionGraph.numReturns,
				sourceID
			) :
			_assembly.newLabelId();
	}
	return functionLabels;
}

CodeTransform::CodeTransform(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	ControlFlowGraphs const& _controlFlow,
	FunctionLabels const& _functionLabels,
	CallSites const& _callSites,
	SSACFG const& _cfg,
	SSACFGStackLayout const& _stackLayout,
	spill::SpillSet const& _spillSet,
	spill::SpillStoreTraces const& _spillStoreTraces,
	ControlFlowGraphs::FunctionGraphID _graphID,
	spill::MemoryAddressing const& _addressing
):
	m_assembly(_assembly),
	m_builtinContext(_builtinContext),
	m_controlFlow(_controlFlow),
	m_functionLabels(_functionLabels),
	m_callSites(_callSites),
	m_cfg(_cfg),
	m_stackLayout(_stackLayout),
	m_spillSet(_spillSet),
	m_spillStoreTraces(_spillStoreTraces),
	m_graphID(_graphID),
	m_blockIsTransformed(_cfg.numBlocks(), false),
	m_blockLabels([this] {
		std::vector<AbstractAssembly::LabelID> blockLabels;
		blockLabels.reserve(m_cfg.numBlocks());
		for (std::size_t i = 0; i < m_cfg.numBlocks(); ++i)
			blockLabels.push_back(m_assembly.newLabelId());
		return blockLabels;
	}()),
	m_spillEmitter([&]() -> std::optional<spill::Emitter> {
		if (_spillSet.numSpilled() > 0)
			return spill::Emitter(_addressing, _graphID, _assembly);
		return std::nullopt;
	}()),
	m_stackData([&]
	{
		auto const& entryLayout = m_stackLayout[m_cfg.entry];
		yulAssert(entryLayout);
		return entryLayout->stackIn;
	}()),
	m_stack(m_stackData)
{
	bool const isFunctionGraph = !m_cfg.isMainGraph();
	if (isFunctionGraph)
	{
		auto const findIt = m_functionLabels.find(_graphID);
		yulAssert(findIt != m_functionLabels.end());
		m_assembly.appendLabel(findIt->second);
		m_assembly.setStackHeight(static_cast<int>(m_cfg.arguments.size()) + (m_cfg.canContinue ? 1 : 0));
	}
	StackData expectedStackTop;
	expectedStackTop.reserve(m_cfg.arguments.size() + (isFunctionGraph && m_cfg.canContinue ? 1 : 0));
	if (isFunctionGraph && m_cfg.canContinue)
		expectedStackTop.push_back(StackSlot::makeFunctionReturnLabel(m_graphID));
	for (auto const& arg: m_cfg.arguments | ranges::views::reverse)
		expectedStackTop.push_back(StackSlot::makeValue(_cfg, arg));
	assertLayoutCompatibility(m_stack.data(), expectedStackTop);

	// Spilled function args need an `mstore` at function entry so later `mload`s see a populated slot
	if (m_spillEmitter && isFunctionGraph)
		for (InstId const argId: m_cfg.arguments)
			spillStore(argId);
}

void CodeTransform::operator()(SSACFG::BlockId const _blockId)
{
	yulAssert(!m_blockIsTransformed[_blockId.value], "Each block is transformed exactly once.");
	m_blockIsTransformed[_blockId.value] = true;

	m_assembly.appendLabel(m_blockLabels[_blockId.value]);

	auto const& blockLayout = m_stackLayout[_blockId];
	yulAssert(blockLayout);
	assertLayoutCompatibility(m_stack.data(), blockLayout->stackIn);
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());

	auto const& block = m_cfg.block(_blockId);
	// Phis can appear after operations in the instruction vector,
	// but phi spills must happen first.
	m_cfg.forEachPhi(block, [this](InstId const _instId, SSACFG::Inst const&) {
		spillStore(_instId);
	});

	std::size_t operationIndex = 0;

	// Only operations advance codegen. Phis are already materialized on the block's stackIn.
	for (InstId const instId: block.instructions)
	{
		SSACFG::Inst const& inst = m_cfg.inst(instId);
		if (inst.isOperation())
		{
			yulAssert(operationIndex < blockLayout->operationShuffles.size());
			(*this)(instId, blockLayout->operationShuffles[operationIndex]);
			++operationIndex;
		}
	}
	yulAssert(operationIndex == blockLayout->operationShuffles.size());

	// Play back the recorded shuffle to the block's exit state before dispatching the exit.
	// This ensures the condition is on top for ConditionalJump, phi pre-images are
	// in the right positions for jumps, and return values are accessible for FunctionReturn.
	playback(blockLayout->exitShuffle);

	// handle the block exit
	std::visit(solidity::util::GenericVisitor{ [this, &_blockId](auto const& exit) { (*this)(_blockId, exit); } }, block.exit);
}

void CodeTransform::operator()(InstId _instId, ShuffleTrace const& _operationShuffle)
{
	SSACFG::Inst const& _inst = m_cfg.inst(_instId);
	yulAssert(_inst.isOperation());
	bool const isCall = _inst.opcode == InstOpcode::Call;
	bool const hasReturnLabel = isCall && m_cfg.callPayload(_instId).canContinue;

	if (hasReturnLabel)
	{
		auto const [it, inserted] = m_returnLabels.try_emplace(_instId, 0);
		yulAssert(inserted, "Call sites should be unique.");
		it->second = m_assembly.newLabelId();
	}

	// check that the assembly stack height corresponds to the stack size before shuffling
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());

	// prepare stack for operation by playing back the recorded shuffle
	playback(_operationShuffle);

	// check that the assembly stack height corresponds to the stack size after shuffling
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());

	// Assert that we have the inputs of the operation on stack top.
	yulAssert(m_stack.size() >= _inst.inputs.size());
	for (auto const& [stackEntry, input]: ranges::views::zip(
		m_stack | ranges::views::take_last(_inst.inputs.size()),
		_inst.inputs | ranges::views::reverse
	))
		yulAssert(stackEntry.isValue() && stackEntry.value() == input);

	// if the function can continue (doesn't always abort), make sure we have the correct return label slot in place
	if (hasReturnLabel)
	{
		yulAssert(m_stack.size() > _inst.inputs.size());
		auto const returnLabelSlot = m_stack.slot(StackDepth{_inst.inputs.size()});
		yulAssert(isCall);
		yulAssert(
			returnLabelSlot.isFunctionCallReturnLabel() &&
			m_callSites.instId(returnLabelSlot.functionCallReturnLabel()) == _instId
		);
	}

	// height of the stack sans function return label and operation inputs
	std::size_t const baseHeight = m_stack.size() - _inst.inputs.size() - (hasReturnLabel ? 1 : 0);

	auto const opOriginLocation = [&]() -> langutil::SourceLocation {
		if (m_cfg.debugInfo)
			if (auto const& dbg = m_cfg.debugInfo->instDebugData(_instId))
				return dbg->originLocation;
		return {};
	}();

	// generate code for the operation
	m_assembly.setSourceLocation(opOriginLocation);
	switch (_inst.opcode)
	{
	case InstOpcode::Call:
	{
		SSACFG::Call const& call = m_cfg.callPayload(_instId);
		auto const* returnLabel = solidity::util::valueOrNullptr(m_returnLabels, _instId);
		// check that if we have a return label, the call can continue
		yulAssert(!!returnLabel == call.canContinue);
		SSACFG const* calleeCFG = m_controlFlow.functionGraph(call.graphID);
		yulAssert(calleeCFG);
		m_assembly.appendJumpTo(
			m_functionLabels.at(call.graphID),
			static_cast<int>(calleeCFG->numReturns) - static_cast<int>(calleeCFG->arguments.size()) - (call.canContinue ? 1 : 0),
			AbstractAssembly::JumpType::IntoFunction
		);
		// if we have a return label, append it to assembly and pop the label from the stack
		// it might also be one of the inputs that is popped here but then the label will be popped below with
		// the other inputs
		if (returnLabel)
		{
			m_assembly.appendLabel(*returnLabel);
			m_stack.pop();
		}
		break;
	}
	case InstOpcode::BuiltinCall:
	{
		SSACFG::BuiltinCall const& builtinCall = m_cfg.builtinPayload(_instId);
		auto const& builtin = m_cfg.evmDialect.builtin(builtinCall.builtin);
		// build up the call with transient args to handle literal arguments as needed
		std::vector<Expression> transientArgs;
		transientArgs.reserve(builtin.literalArguments.size());
		auto litIt = builtinCall.literalArguments.begin();
		for (size_t i = 0; i < builtin.literalArguments.size(); ++i)
			if (builtin.literalArgument(i).has_value())
				transientArgs.emplace_back(*litIt++);
			else
				transientArgs.emplace_back(Identifier{});
		FunctionCall const transient{{}, BuiltinName{{}, builtinCall.builtin}, std::move(transientArgs)};
		builtin.generateCode(transient, m_assembly, m_builtinContext);
		break;
	}
	case InstOpcode::MemoryGuard:
	{
		yulAssert(m_controlFlow.memoryGuard.has_value());
		m_assembly.appendConstant(*m_controlFlow.memoryGuard);
		break;
	}
	default:
		yulAssert(false);
	}
	// simulate that the inputs are consumed
	for (size_t i = 0; i < _inst.inputs.size(); ++i)
		m_stack.pop();
	// simulate that the outputs are produced
	auto const numOutputs = m_cfg.numReturnsOf(_instId);
	for (InstId const id: m_cfg.outputsOf(_instId))
		m_stack.push(StackSlot::makeValue(m_cfg, id));

	// Each output the layout decided to spill gets its `mstore` here
	if (m_spillEmitter)
		for (InstId const outputId: m_cfg.outputsOf(_instId))
			spillStore(outputId);

	yulAssert(m_stack.size() == baseHeight + numOutputs);
	for (auto const& [stackEntry, output]: ranges::views::zip(
		m_stack.data() | ranges::views::take_last(numOutputs),
		m_cfg.outputsOf(_instId)
	))
		yulAssert(stackEntry.isValue() && stackEntry.value() == output);
	yulAssert(
		static_cast<int>(m_stack.size()) == m_assembly.stackHeight(),
		fmt::format("symbolic stack size = {} =/= {} = assembly stack height", m_stack.size(), m_assembly.stackHeight())
	);
}

void CodeTransform::spillStore(InstId const _value)
{
	if (!m_spillEmitter || !m_spillSet.isSpilled(_value))
		return;

	// Play back the recorded def-site trace: it brings `_value` to the stack top and concludes with the
	// `mstore` consuming it, leaving the rest of the stack in place.
	auto const it = m_spillStoreTraces.find(_value);
	yulAssert(it != m_spillStoreTraces.end(), fmt::format("no def-site store trace recorded for spilled value {}", _value));
	ShuffleTrace const& storeTrace = it->second;
	yulAssert(
		!storeTrace.empty() &&
		storeTrace.back().kind == ShuffleOp::Kind::Store &&
		storeTrace.back().slot == StackSlot::makeValue(m_cfg, _value),
		fmt::format("def-site trace for {} must conclude with its store", _value)
	);
	playback(storeTrace);
}

void CodeTransform::operator()(SSACFG::BlockId const&, SSACFG::BasicBlock::MainExit const&)
{
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());
	m_assembly.appendInstruction(evmasm::Instruction::STOP);
}

void CodeTransform::operator()(SSACFG::BlockId const& _currentBlock, SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
{
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());
	// condition must be at the top of the stack
	yulAssert(m_stack.top().isValue() && m_stack.top().value() == _conditionalJump.condition);
	// emit JUMPI to nonZero block
	m_assembly.appendJumpToIf(m_blockLabels[_conditionalJump.nonZero.value]);
	// update symbolic stack by popping the condition as it'll be consumed by JUMPI
	m_stack.pop();

	{
		// restore stack to previous state once zero-path is handled
		ScopedSaveAndRestore restoreStack(m_stackData, StackData(m_stackData));
		yulAssert(m_stackLayout[_conditionalJump.zero]);

		// transform stack to a state in which we can jump to the zero branch
		prepareBlockExitStack(_currentBlock, _conditionalJump.zero);
		assertLayoutCompatibility(m_stack.data(), m_stackLayout[_conditionalJump.zero]->stackIn);
		m_assembly.appendJumpTo(m_blockLabels[_conditionalJump.zero.value]);

		if (!m_blockIsTransformed[_conditionalJump.zero.value])
			(*this)(_conditionalJump.zero);
	}
	{
		yulAssert(m_stackLayout[_conditionalJump.nonZero]);
		m_assembly.setStackHeight(static_cast<int>(m_stack.size()));
		// transform stack to a state in which we can jump to the nonZero branch
		prepareBlockExitStack(_currentBlock, _conditionalJump.nonZero);
		assertLayoutCompatibility(m_stack.data(), m_stackLayout[_conditionalJump.nonZero]->stackIn);
		if (!m_blockIsTransformed[_conditionalJump.nonZero.value])
			(*this)(_conditionalJump.nonZero);
	}
}

void CodeTransform::operator()(SSACFG::BlockId const& _currentBlock, SSACFG::BasicBlock::Jump const& _jump)
{
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());
	yulAssert(m_stackLayout[_jump.target]);
	prepareBlockExitStack(_currentBlock, _jump.target);
	assertLayoutCompatibility(m_stack.data(), m_stackLayout[_jump.target]->stackIn);
	m_assembly.appendJumpTo(m_blockLabels[_jump.target.value]);
	if (!m_blockIsTransformed[_jump.target.value])
		(*this)(_jump.target);
}

void CodeTransform::operator()(SSACFG::BlockId const&, SSACFG::BasicBlock::FunctionReturn const& _functionReturn)
{
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());
	// Each CodeTransform instance handles exactly one function's CFG, so a FunctionReturn exit
	// here necessarily belongs to a function graph. No identity cross-check is needed.
	yulAssert(!m_cfg.isMainGraph());
	yulAssert(m_cfg.canContinue);
	yulAssert(m_stack.size() == _functionReturn.returnValues.size() + 1, "There must be at least the function return label element on stack");
	yulAssert(m_stack.top().isFunctionReturnLabel());
	yulAssert(m_stack.top().functionReturnLabel() == m_graphID);
	for (std::size_t i = 0; i < _functionReturn.returnValues.size(); ++i)
	{
		auto const& returnValueSlot = m_stack.slot(StackOffset{i});
		yulAssert(returnValueSlot.isValue());
		yulAssert(returnValueSlot.value() == _functionReturn.returnValues[i]);
	}
	m_assembly.appendJump(0, AbstractAssembly::JumpType::OutOfFunction);
}

void CodeTransform::operator()(SSACFG::BlockId const& _blockId, SSACFG::BasicBlock::Terminated const&)
{
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight());
	auto const& block = m_cfg.block(_blockId);
	// Find the last BuiltinCall/Call Inst in the block.
	InstId lastOpInstId{};
	for (InstId const instId: block.instructions | ranges::views::reverse)
		if (m_cfg.isOperation(instId))
		{
			lastOpInstId = instId;
			break;
		}
	yulAssert(lastOpInstId.hasValue(), "Terminated block must have at least one operation.");
	auto const lastOpcode = m_cfg.inst(lastOpInstId).opcode;
	if (lastOpcode == InstOpcode::BuiltinCall)
		yulAssert(
			m_cfg.evmDialect.builtin(m_cfg.builtinPayload(lastOpInstId).builtin).controlFlowSideEffects.terminatesOrReverts(),
			"Last operation of Terminated block must terminate or revert."
		);
	else if (lastOpcode == InstOpcode::Call)
		yulAssert(
			!m_cfg.callPayload(lastOpInstId).canContinue,
			"Last operation of Terminated block must be a non-continuable call."
		);
	else
		yulAssert(false, "Last operation of Terminated block must be a non-continuable Call or terminating BuiltinCall.");
	// To be sure just emit another INVALID - should be removed by optimizer.
	m_assembly.appendInstruction(evmasm::Instruction::INVALID);
}

void CodeTransform::playback(ShuffleTrace const& _trace)
{
	for (ShuffleOp const& op: _trace)
	{
		apply(m_stackData, op);
		emit(op);
	}
}

void CodeTransform::emit(ShuffleOp const& _op)
{
	switch (_op.kind)
	{
	case ShuffleOp::Kind::Swap:
		m_assembly.appendInstruction(evmasm::swapInstruction(_op.depth));
		return;
	case ShuffleOp::Kind::Dup:
		m_assembly.appendInstruction(evmasm::dupInstruction(_op.depth));
		return;
	case ShuffleOp::Kind::Pop:
		m_assembly.appendInstruction(evmasm::Instruction::POP);
		return;
	case ShuffleOp::Kind::Push:
		switch (_op.slot.kind())
		{
		case StackSlot::Kind::Value:
			yulAssert(m_cfg.isLiteral(_op.slot.value()), "Non-literal value pushes must be recorded as Load.");
			m_assembly.appendConstant(m_cfg.literalPayload(_op.slot.value()));
			return;
		case StackSlot::Kind::Junk:
			if (m_assembly.evmVersion().hasPush0())
				m_assembly.appendConstant(0);
			else
				m_assembly.appendInstruction(evmasm::Instruction::CODESIZE);
			return;
		case StackSlot::Kind::FunctionCallReturnLabel:
		{
			auto const instId = m_callSites.instId(_op.slot.functionCallReturnLabel());
			yulAssert(m_returnLabels.count(instId), "FunctionCallReturnLabel not pre-registered before shuffle.");
			m_assembly.appendLabelReference(m_returnLabels.at(instId));
			return;
		}
		case StackSlot::Kind::FunctionReturnLabel:
			yulAssert(false, "Cannot produce function return label.");
		}
		solidity::util::unreachable();
	case ShuffleOp::Kind::Load:
	{
		InstId const id = _op.slot.value();
		yulAssert(
			m_spillEmitter && m_spillEmitter->hasAddress(id),
			fmt::format("Tried bringing up non-spilled non-const {}", id)
		);
		m_spillEmitter->emitLoad(id);
		return;
	}
	case ShuffleOp::Kind::Store:
	{
		InstId const id = _op.slot.value();
		yulAssert(
			m_spillEmitter && m_spillEmitter->hasAddress(id),
			fmt::format("Tried storing value {} without a spill slot", id)
		);
		m_spillEmitter->emitStore(id);
		return;
	}
	}
	solidity::util::unreachable();
}

void CodeTransform::prepareBlockExitStack(SSACFG::BlockId const& _currentBlock, SSACFG::BlockId const& _target)
{
	auto const& targetLayout = m_stackLayout[_target];
	yulAssert(targetLayout);
	// pull back target to live in current variable space
	auto const pulledBackTarget = stackPreImage(m_cfg, targetLayout->stackIn, PhiInverse(m_cfg, _currentBlock, _target));
	// play back the recorded shuffle for this edge
	playback(targetLayout->traceForStackIn(_currentBlock));
	// check that the playback reproduced the edge target
	assertLayoutCompatibility(m_stack.data(), pulledBackTarget);
	// now we can simply set the target to the actual one which will take care of the application of phi functions
	m_stackData = targetLayout->stackIn;
}
