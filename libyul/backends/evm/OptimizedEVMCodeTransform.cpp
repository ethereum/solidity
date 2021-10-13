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
#include <libyul/backends/evm/OptimizedEVMCodeTransform.h>

#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/StackLayoutGenerator.h>

#include <libyul/Utilities.h>

#include <libevmasm/Instruction.h>

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
using namespace std;

vector<StackTooDeepError> OptimizedEVMCodeTransform::run(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	EVMDialect const& _dialect,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions
)
{
	std::unique_ptr<CFG> dfg = ControlFlowGraphBuilder::build(_analysisInfo, _dialect, _block);
	StackLayout stackLayout = StackLayoutGenerator::run(*dfg);
	OptimizedEVMCodeTransform optimizedCodeTransform(
		_assembly,
		_builtinContext,
		_useNamedLabelsForFunctions,
		*dfg,
		stackLayout
	);
	// Create initial entry layout.
	optimizedCodeTransform.createStackLayout(debugDataOf(*dfg->entry), stackLayout.blockInfos.at(dfg->entry).entryLayout);
	optimizedCodeTransform(*dfg->entry);
	for (Scope::Function const* function: dfg->functions)
		optimizedCodeTransform(dfg->functionInfo.at(function));
	return std::move(optimizedCodeTransform.m_stackErrors);
}

void OptimizedEVMCodeTransform::operator()(CFG::FunctionCall const& _call)
{
	// Validate stack.
	{
		yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
		yulAssert(m_stack.size() >= _call.function.get().arguments.size() + (_call.canContinue ? 1 : 0), "");
		// Assert that we got the correct arguments on stack for the call.
		for (auto&& [arg, slot]: ranges::zip_view(
			_call.functionCall.get().arguments | ranges::views::reverse,
			m_stack | ranges::views::take_last(_call.functionCall.get().arguments.size())
		))
			validateSlot(slot, arg);
		// Assert that we got the correct return label on stack.
		if (_call.canContinue)
		{
			auto const* returnLabelSlot = get_if<FunctionCallReturnLabelSlot>(
				&m_stack.at(m_stack.size() - _call.functionCall.get().arguments.size() - 1)
			);
			yulAssert(returnLabelSlot && &returnLabelSlot->call.get() == &_call.functionCall.get(), "");
		}
	}

	// Emit code.
	{
		m_assembly.setSourceLocation(originLocationOf(_call));
		m_assembly.appendJumpTo(
			getFunctionLabel(_call.function),
			static_cast<int>(_call.function.get().returns.size() - _call.function.get().arguments.size()) - (_call.canContinue ? 1 : 0),
			AbstractAssembly::JumpType::IntoFunction
		);
		if (_call.canContinue)
			m_assembly.appendLabel(m_returnLabels.at(&_call.functionCall.get()));
	}

	// Update stack.
	{
		// Remove arguments and return label from m_stack.
		for (size_t i = 0; i < _call.function.get().arguments.size() + (_call.canContinue ? 1 : 0); ++i)
			m_stack.pop_back();
		// Push return values to m_stack.
		for (size_t index: ranges::views::iota(0u, _call.function.get().returns.size()))
			m_stack.emplace_back(TemporarySlot{_call.functionCall, index});
		yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
	}
}

void OptimizedEVMCodeTransform::operator()(CFG::BuiltinCall const& _call)
{
	// Validate stack.
	{
		yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
		yulAssert(m_stack.size() >= _call.arguments, "");
		// Assert that we got a correct stack for the call.
		for (auto&& [arg, slot]: ranges::zip_view(
			_call.functionCall.get().arguments |
			ranges::views::enumerate |
			ranges::views::filter(util::mapTuple([&](size_t idx, auto&) -> bool {
				return !_call.builtin.get().literalArgument(idx);
			})) |
			ranges::views::reverse |
			ranges::views::values,
			m_stack | ranges::views::take_last(_call.arguments)
		))
			validateSlot(slot, arg);
	}

	// Emit code.
	{
		m_assembly.setSourceLocation(originLocationOf(_call));
		static_cast<BuiltinFunctionForEVM const&>(_call.builtin.get()).generateCode(
			_call.functionCall,
			m_assembly,
			m_builtinContext
		);
	}

	// Update stack.
	{
		// Remove arguments from m_stack.
		for (size_t i = 0; i < _call.arguments; ++i)
			m_stack.pop_back();
		// Push return values to m_stack.
		for (size_t index: ranges::views::iota(0u, _call.builtin.get().returns.size()))
			m_stack.emplace_back(TemporarySlot{_call.functionCall, index});
		yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
	}
}

void OptimizedEVMCodeTransform::operator()(CFG::Assignment const& _assignment)
{
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");

	// Invalidate occurrences of the assigned variables.
	for (auto& currentSlot: m_stack)
		if (VariableSlot const* varSlot = get_if<VariableSlot>(&currentSlot))
			if (util::contains(_assignment.variables, *varSlot))
				currentSlot = JunkSlot{};

	// Assign variables to current stack top.
	yulAssert(m_stack.size() >= _assignment.variables.size(), "");
	for (auto&& [currentSlot, varSlot]: ranges::zip_view(
		m_stack | ranges::views::take_last(_assignment.variables.size()),
		_assignment.variables
	))
		currentSlot = varSlot;
}

OptimizedEVMCodeTransform::OptimizedEVMCodeTransform(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions,
	CFG const& _dfg,
	StackLayout const& _stackLayout
):
	m_assembly(_assembly),
	m_builtinContext(_builtinContext),
	m_dfg(_dfg),
	m_stackLayout(_stackLayout),
	m_functionLabels([&](){
		map<CFG::FunctionInfo const*, AbstractAssembly::LabelID> functionLabels;
		set<YulString> assignedFunctionNames;
		for (Scope::Function const* function: m_dfg.functions)
		{
			CFG::FunctionInfo const& functionInfo = m_dfg.functionInfo.at(function);
			bool nameAlreadySeen = !assignedFunctionNames.insert(function->name).second;
			if (_useNamedLabelsForFunctions == UseNamedLabels::YesAndForceUnique)
				yulAssert(!nameAlreadySeen);
			bool useNamedLabel = _useNamedLabelsForFunctions != UseNamedLabels::Never && !nameAlreadySeen;
			functionLabels[&functionInfo] = useNamedLabel ?
				m_assembly.namedLabel(
					function->name.str(),
					function->arguments.size(),
					function->returns.size(),
					functionInfo.debugData ? functionInfo.debugData->astID : nullopt
				) :
				m_assembly.newLabelId();
		}
		return functionLabels;
	}())
{
}

void OptimizedEVMCodeTransform::assertLayoutCompatibility(Stack const& _currentStack, Stack const& _desiredStack)
{
	yulAssert(_currentStack.size() == _desiredStack.size(), "");
	for (auto&& [currentSlot, desiredSlot]: ranges::zip_view(_currentStack, _desiredStack))
		yulAssert(holds_alternative<JunkSlot>(desiredSlot) || currentSlot == desiredSlot, "");
}

AbstractAssembly::LabelID OptimizedEVMCodeTransform::getFunctionLabel(Scope::Function const& _function)
{
	return m_functionLabels.at(&m_dfg.functionInfo.at(&_function));
}

void OptimizedEVMCodeTransform::validateSlot(StackSlot const& _slot, Expression const& _expression)
{
	std::visit(util::GenericVisitor{
		[&](yul::Literal const& _literal) {
			auto* literalSlot = get_if<LiteralSlot>(&_slot);
			yulAssert(literalSlot && valueOfLiteral(_literal) == literalSlot->value, "");
		},
		[&](yul::Identifier const& _identifier) {
			auto* variableSlot = get_if<VariableSlot>(&_slot);
			yulAssert(variableSlot && variableSlot->variable.get().name == _identifier.name, "");
		},
		[&](yul::FunctionCall const& _call) {
			auto* temporarySlot = get_if<TemporarySlot>(&_slot);
			yulAssert(temporarySlot && &temporarySlot->call.get() == &_call && temporarySlot->index == 0, "");
		}
	}, _expression);
}

void OptimizedEVMCodeTransform::createStackLayout(std::shared_ptr<DebugData const> _debugData, Stack _targetStack)
{
	static constexpr auto slotVariableName = [](StackSlot const& _slot) {
		return std::visit(util::GenericVisitor{
			[](VariableSlot const& _var) { return _var.variable.get().name; },
			[](auto const&) { return YulString{}; }
		}, _slot);
	};

	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
	// ::createStackLayout asserts that it has successfully achieved the target layout.
	langutil::SourceLocation sourceLocation = _debugData ? _debugData->originLocation : langutil::SourceLocation{};
	m_assembly.setSourceLocation(sourceLocation);
	::createStackLayout(
		m_stack,
		_targetStack | ranges::to<Stack>,
		// Swap callback.
		[&](unsigned _i)
		{
			yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");
			yulAssert(_i > 0 && _i < m_stack.size(), "");
			if (_i <= 16)
				m_assembly.appendInstruction(evmasm::swapInstruction(_i));
			else
			{
				int deficit = static_cast<int>(_i) - 16;
				StackSlot const& deepSlot = m_stack.at(m_stack.size() - _i - 1);
				YulString varNameDeep = slotVariableName(deepSlot);
				YulString varNameTop = slotVariableName(m_stack.back());
				string msg =
					"Cannot swap " + (varNameDeep.empty() ? "Slot " + stackSlotToString(deepSlot) : "Variable " + varNameDeep.str()) +
					" with " + (varNameTop.empty() ? "Slot " + stackSlotToString(m_stack.back()) : "Variable " + varNameTop.str()) +
					": too deep in the stack by " + to_string(deficit) + " slots in " + stackToString(m_stack);
				m_stackErrors.emplace_back(StackTooDeepError(
					m_currentFunctionInfo ? m_currentFunctionInfo->function.name : YulString{},
					varNameDeep.empty() ? varNameTop : varNameDeep,
					deficit,
					msg
				));
				m_assembly.markAsInvalid();
			}
		},
		// Push or dup callback.
		[&](StackSlot const& _slot)
		{
			yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");

			// Dup the slot, if already on stack and reachable.
			if (auto depth = util::findOffset(m_stack | ranges::views::reverse, _slot))
			{
				if (*depth < 16)
				{
					m_assembly.appendInstruction(evmasm::dupInstruction(static_cast<unsigned>(*depth + 1)));
					return;
				}
				else if (!canBeFreelyGenerated(_slot))
				{
					int deficit = static_cast<int>(*depth - 15);
					YulString varName = slotVariableName(_slot);
					string msg =
						(varName.empty() ? "Slot " + stackSlotToString(_slot) : "Variable " + varName.str())
						+ " is " + to_string(*depth - 15) + " too deep in the stack " + stackToString(m_stack);
					m_stackErrors.emplace_back(StackTooDeepError(
						m_currentFunctionInfo ? m_currentFunctionInfo->function.name : YulString{},
						varName,
						deficit,
						msg
					));
					m_assembly.markAsInvalid();
					m_assembly.appendConstant(u256(0xCAFFEE));
					return;
				}
				// else: the slot is too deep in stack, but can be freely generated, we fall through to push it again.
			}

			// The slot can be freely generated or is an unassigned return variable. Push it.
			std::visit(util::GenericVisitor{
				[&](LiteralSlot const& _literal)
				{
					m_assembly.setSourceLocation(originLocationOf(_literal));
					m_assembly.appendConstant(_literal.value);
					m_assembly.setSourceLocation(sourceLocation);
				},
				[&](FunctionReturnLabelSlot const&)
				{
					yulAssert(false, "Cannot produce function return label.");
				},
				[&](FunctionCallReturnLabelSlot const& _returnLabel)
				{
					if (!m_returnLabels.count(&_returnLabel.call.get()))
						m_returnLabels[&_returnLabel.call.get()] = m_assembly.newLabelId();
					m_assembly.setSourceLocation(originLocationOf(_returnLabel.call.get()));
					m_assembly.appendLabelReference(m_returnLabels.at(&_returnLabel.call.get()));
					m_assembly.setSourceLocation(sourceLocation);
				},
				[&](VariableSlot const& _variable)
				{
					if (m_currentFunctionInfo && util::contains(m_currentFunctionInfo->returnVariables, _variable))
					{
						m_assembly.setSourceLocation(originLocationOf(_variable));
						m_assembly.appendConstant(0);
						m_assembly.setSourceLocation(sourceLocation);
						return;
					}
					yulAssert(false, "Variable not found on stack.");
				},
				[&](TemporarySlot const&)
				{
					yulAssert(false, "Function call result requested, but not found on stack.");
				},
				[&](JunkSlot const&)
				{
					// Note: this will always be popped, so we can push anything.
					m_assembly.appendInstruction(evmasm::Instruction::CODESIZE);
				}
			}, _slot);
		},
		// Pop callback.
		[&]()
		{
			m_assembly.appendInstruction(evmasm::Instruction::POP);
		}
	);
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
}

void OptimizedEVMCodeTransform::operator()(CFG::BasicBlock const& _block)
{
	// Assert that this is the first visit of the block and mark as generated.
	yulAssert(m_generated.insert(&_block).second, "");

	m_assembly.setSourceLocation(originLocationOf(_block));
	auto const& blockInfo = m_stackLayout.blockInfos.at(&_block);

	// Assert that the stack is valid for entering the block.
	assertLayoutCompatibility(m_stack, blockInfo.entryLayout);
	m_stack = blockInfo.entryLayout; // Might set some slots to junk, if not required by the block.
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");

	// Emit jump label, if required.
	if (auto label = util::valueOrNullptr(m_blockLabels, &_block))
		m_assembly.appendLabel(*label);

	for (auto const& operation: _block.operations)
	{
		// Create required layout for entering the operation.
		createStackLayout(debugDataOf(operation.operation), m_stackLayout.operationEntryLayout.at(&operation));

		// Assert that we have the inputs of the operation on stack top.
		yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");
		yulAssert(m_stack.size() >= operation.input.size(), "");
		size_t baseHeight = m_stack.size() - operation.input.size();
		assertLayoutCompatibility(
			m_stack | ranges::views::take_last(operation.input.size()) | ranges::to<Stack>,
			operation.input
		);

		// Perform the operation.
		std::visit(*this, operation.operation);

		// Assert that the operation produced its proclaimed output.
		yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");
		yulAssert(m_stack.size() == baseHeight + operation.output.size(), "");
		yulAssert(m_stack.size() >= operation.output.size(), "");
		assertLayoutCompatibility(
			m_stack | ranges::views::take_last(operation.output.size()) | ranges::to<Stack>,
			operation.output
		);
	}

	// Exit the block.
	m_assembly.setSourceLocation(originLocationOf(_block));
	std::visit(util::GenericVisitor{
		[&](CFG::BasicBlock::MainExit const&)
		{
			m_assembly.appendInstruction(evmasm::Instruction::STOP);
		},
		[&](CFG::BasicBlock::Jump const& _jump)
		{
			// Create the stack expected at the jump target.
			createStackLayout(debugDataOf(_jump), m_stackLayout.blockInfos.at(_jump.target).entryLayout);

			// If this is the only jump to the block, we do not need a label and can directly continue with the target block.
			if (!m_blockLabels.count(_jump.target) && _jump.target->entries.size() == 1)
			{
				yulAssert(!_jump.backwards, "");
				(*this)(*_jump.target);
			}
			else
			{
				// Generate a jump label for the target, if not already present.
				if (!m_blockLabels.count(_jump.target))
					m_blockLabels[_jump.target] = m_assembly.newLabelId();

				// If we already have generated the target block, jump to it, otherwise generate it in place.
				if (m_generated.count(_jump.target))
					m_assembly.appendJumpTo(m_blockLabels[_jump.target]);
				else
					(*this)(*_jump.target);
			}
		},
		[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
		{
			// Create the shared entry layout of the jump targets, which is stored as exit layout of the current block.
			createStackLayout(debugDataOf(_conditionalJump), blockInfo.exitLayout);

			// Create labels for the targets, if not already present.
			if (!m_blockLabels.count(_conditionalJump.nonZero))
				m_blockLabels[_conditionalJump.nonZero] = m_assembly.newLabelId();
			if (!m_blockLabels.count(_conditionalJump.zero))
				m_blockLabels[_conditionalJump.zero] = m_assembly.newLabelId();

			// Assert that we have the correct condition on stack.
			yulAssert(!m_stack.empty(), "");
			yulAssert(m_stack.back() == _conditionalJump.condition, "");

			// Emit the conditional jump to the non-zero label and update the stored stack.
			m_assembly.appendJumpToIf(m_blockLabels[_conditionalJump.nonZero]);
			m_stack.pop_back();

			// Assert that we have a valid stack for both jump targets.
			assertLayoutCompatibility(m_stack, m_stackLayout.blockInfos.at(_conditionalJump.nonZero).entryLayout);
			assertLayoutCompatibility(m_stack, m_stackLayout.blockInfos.at(_conditionalJump.zero).entryLayout);

			{
				// Restore the stack afterwards for the non-zero case below.
				ScopeGuard stackRestore([storedStack = m_stack, this]() {
					m_stack = std::move(storedStack);
					m_assembly.setStackHeight(static_cast<int>(m_stack.size()));
				});

				// If we have already generated the zero case, jump to it, otherwise generate it in place.
				if (m_generated.count(_conditionalJump.zero))
					m_assembly.appendJumpTo(m_blockLabels[_conditionalJump.zero]);
				else
					(*this)(*_conditionalJump.zero);
			}
			// Note that each block visit terminates control flow, so we cannot fall through from the zero case.

			// Generate the non-zero block, if not done already.
			if (!m_generated.count(_conditionalJump.nonZero))
				(*this)(*_conditionalJump.nonZero);
		},
		[&](CFG::BasicBlock::FunctionReturn const& _functionReturn)
		{
			yulAssert(m_currentFunctionInfo);
			yulAssert(m_currentFunctionInfo == _functionReturn.info);
			yulAssert(m_currentFunctionInfo->canContinue);

			// Construct the function return layout, which is fully determined by the function signature.
			Stack exitStack = m_currentFunctionInfo->returnVariables | ranges::views::transform([](auto const& _varSlot){
				return StackSlot{_varSlot};
			}) | ranges::to<Stack>;
			exitStack.emplace_back(FunctionReturnLabelSlot{_functionReturn.info->function});

			// Create the function return layout and jump.
			createStackLayout(debugDataOf(_functionReturn), exitStack);
			m_assembly.appendJump(0, AbstractAssembly::JumpType::OutOfFunction);
		},
		[&](CFG::BasicBlock::Terminated const&)
		{
			yulAssert(!_block.operations.empty());
			if (CFG::BuiltinCall const* builtinCall = get_if<CFG::BuiltinCall>(&_block.operations.back().operation))
				yulAssert(builtinCall->builtin.get().controlFlowSideEffects.terminatesOrReverts(), "");
			else if (CFG::FunctionCall const* functionCall = get_if<CFG::FunctionCall>(&_block.operations.back().operation))
				yulAssert(!functionCall->canContinue);
			else
				yulAssert(false);
		}
	}, _block.exit);
	// TODO: We could assert that the last emitted assembly item terminated or was an (unconditional) jump.
	//       But currently AbstractAssembly does not allow peeking at the last emitted assembly item.
	m_stack.clear();
	m_assembly.setStackHeight(0);
}

void OptimizedEVMCodeTransform::operator()(CFG::FunctionInfo const& _functionInfo)
{
	yulAssert(!m_currentFunctionInfo, "");
	ScopedSaveAndRestore currentFunctionInfoRestore(m_currentFunctionInfo, &_functionInfo);

	yulAssert(m_stack.empty() && m_assembly.stackHeight() == 0, "");

	// Create function entry layout in m_stack.
	if (_functionInfo.canContinue)
		m_stack.emplace_back(FunctionReturnLabelSlot{_functionInfo.function});
	for (auto const& param: _functionInfo.parameters | ranges::views::reverse)
		m_stack.emplace_back(param);
	m_assembly.setStackHeight(static_cast<int>(m_stack.size()));

	m_assembly.setSourceLocation(originLocationOf(_functionInfo));
	m_assembly.appendLabel(getFunctionLabel(_functionInfo.function));

	// Create the entry layout of the function body block and visit.
	createStackLayout(debugDataOf(_functionInfo), m_stackLayout.blockInfos.at(_functionInfo.entry).entryLayout);
	(*this)(*_functionInfo.entry);

	m_stack.clear();
	m_assembly.setStackHeight(0);
}
