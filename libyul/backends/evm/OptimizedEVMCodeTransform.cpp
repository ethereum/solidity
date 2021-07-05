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

#include <libsolutil/Permutations.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/cxx20.h>

#include <range/v3/action/push_back.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take_last.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

OptimizedEVMCodeTransform::OptimizedEVMCodeTransform(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	bool _useNamedLabelsForFunctions,
	CFG const& _dfg,
	StackLayout const& _stackLayout
):
m_assembly(_assembly),
m_builtinContext(_builtinContext),
m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
m_dfg(_dfg),
m_stackLayout(_stackLayout)
{
}

void OptimizedEVMCodeTransform::assertLayoutCompatibility(Stack const& _currentStack, Stack const& _desiredStack)
{
	for (auto&& [currentSlot, desiredSlot]: ranges::zip_view(_currentStack, _desiredStack))
		yulAssert(holds_alternative<JunkSlot>(desiredSlot) || currentSlot == desiredSlot, "");
}

AbstractAssembly::LabelID OptimizedEVMCodeTransform::getFunctionLabel(Scope::Function const& _function)
{
	CFG::FunctionInfo const& functionInfo = m_dfg.functionInfo.at(&_function);
	if (!m_functionLabels.count(&functionInfo))
	{
		m_functionLabels[&functionInfo] = m_useNamedLabelsForFunctions ?
			m_assembly.namedLabel(
				functionInfo.function.name.str(),
				functionInfo.function.arguments.size(),
				functionInfo.function.returns.size(),
				{}
			) : m_assembly.newLabelId();
	}
	return m_functionLabels[&functionInfo];
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
			yulAssert(temporarySlot && &temporarySlot->call.get() == &_call, "");
		}
	}, _expression);
}

void OptimizedEVMCodeTransform::operator()(CFG::FunctionInfo const& _functionInfo)
{
	yulAssert(!m_currentFunctionInfo, "");
	m_currentFunctionInfo = &_functionInfo;

	Stack const& entryLayout = m_stackLayout.blockInfos.at(_functionInfo.entry).entryLayout;

	m_stack.clear();
	m_stack.emplace_back(FunctionReturnLabelSlot{});
	for (auto const& param: _functionInfo.parameters | ranges::views::reverse)
		m_stack.emplace_back(param);
	m_assembly.setStackHeight(static_cast<int>(m_stack.size()));
	m_assembly.setSourceLocation(locationOf(_functionInfo));
	if (!m_functionLabels.count(&_functionInfo))
		m_functionLabels[&_functionInfo] = m_assembly.newLabelId();

	m_assembly.appendLabel(getFunctionLabel(_functionInfo.function));
	createStackLayout(entryLayout);

	(*this)(*_functionInfo.entry);

	m_currentFunctionInfo = nullptr;
	m_stack.clear();
	m_assembly.setStackHeight(0);
}

void OptimizedEVMCodeTransform::operator()(CFG::FunctionCall const& _call)
{
	yulAssert(m_stack.size() >= _call.function.get().arguments.size() + 1, "");
	auto returnLabel = m_returnLabels.at(&_call.functionCall.get());

	// Assert that we got a correct arguments on stack for the call.
	for (auto&& [arg, slot]: ranges::zip_view(
		_call.functionCall.get().arguments | ranges::views::reverse,
		m_stack | ranges::views::take_last(_call.functionCall.get().arguments.size())
	))
		validateSlot(slot, arg);
	// Assert that we got the correct return label on stack.
	auto* returnLabelSlot = get_if<FunctionCallReturnLabelSlot>(&m_stack.at(m_stack.size() - _call.functionCall.get().arguments.size() - 1));
	yulAssert(returnLabelSlot && &returnLabelSlot->call.get() == &_call.functionCall.get(), "");

	m_assembly.setSourceLocation(locationOf(_call));
	m_assembly.appendJumpTo(
		getFunctionLabel(_call.function),
		static_cast<int>(_call.function.get().returns.size() - _call.function.get().arguments.size()) - 1,
		AbstractAssembly::JumpType::IntoFunction
	);
	m_assembly.appendLabel(returnLabel);
	// Remove arguments and return label from m_stack.
	for (size_t i = 0; i < _call.function.get().arguments.size() + 1; ++i)
		m_stack.pop_back();
	// Push return values to m_stack.
	ranges::actions::push_back(
		m_stack,
		ranges::views::iota(0u, _call.function.get().returns.size()) |
		ranges::views::transform([&](size_t _i) -> StackSlot { return TemporarySlot{_call.functionCall, _i}; })
	);
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
}

void OptimizedEVMCodeTransform::operator()(CFG::BuiltinCall const& _call)
{
	yulAssert(m_stack.size() >= _call.arguments, "");
	// Assert that we got a correct stack for the call.
	for (auto&& [arg, slot]: ranges::zip_view(
		_call.functionCall.get().arguments | ranges::views::enumerate |
		ranges::views::filter(util::mapTuple([&](size_t idx, auto&) -> bool { return !_call.builtin.get().literalArgument(idx); })) |
		ranges::views::reverse | ranges::views::values,
		m_stack | ranges::views::take_last(_call.arguments)
	))
		validateSlot(slot, arg);

	m_assembly.setSourceLocation(locationOf(_call));
	static_cast<BuiltinFunctionForEVM const&>(_call.builtin.get()).generateCode(_call.functionCall, m_assembly, m_builtinContext, [](auto&&){});
	// Remove arguments from m_stack.
	for (size_t i = 0; i < _call.arguments; ++i)
		m_stack.pop_back();
	// Push return values to m_stack.
	ranges::actions::push_back(
		m_stack,
		ranges::views::iota(0u, _call.builtin.get().returns.size()) |
		ranges::views::transform([&](size_t _i) -> StackSlot { return TemporarySlot{_call.functionCall, _i};})
	);
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
}

void OptimizedEVMCodeTransform::operator()(CFG::Assignment const& _assignment)
{
	for (auto& currentSlot: m_stack)
		if (VariableSlot const* varSlot = get_if<VariableSlot>(&currentSlot))
			if (util::findOffset(_assignment.variables, *varSlot))
				currentSlot = JunkSlot{};

	for (auto&& [currentSlot, varSlot]: ranges::zip_view(m_stack | ranges::views::take_last(_assignment.variables.size()), _assignment.variables))
		currentSlot = varSlot;
}

void OptimizedEVMCodeTransform::operator()(CFG::BasicBlock const& _block)
{
	if (!m_generated.insert(&_block).second)
		return;

	auto&& [entryLayout, exitLayout] = m_stackLayout.blockInfos.at(&_block);

	if (auto label = util::valueOrNullptr(m_blockLabels, &_block))
		m_assembly.appendLabel(*label);

	assertLayoutCompatibility(m_stack, entryLayout);
	m_stack = entryLayout;
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");

	for (auto const& operation: _block.operations)
	{
		createStackLayout(m_stackLayout.operationEntryLayout.at(&operation));
		std::visit(*this, operation.operation);
	}
	createStackLayout(exitLayout);

	std::visit(util::GenericVisitor{
		[&](CFG::BasicBlock::MainExit const&)
		{
			m_assembly.appendInstruction(evmasm::Instruction::STOP);
			m_assembly.setStackHeight(0);
			m_stack.clear();
		},
		[&](CFG::BasicBlock::Jump const& _jump)
		{
			Stack const& entryLayout = m_stackLayout.blockInfos.at(_jump.target).entryLayout;
			createStackLayout(entryLayout);

			if (!m_blockLabels.count(_jump.target) && _jump.target->entries.size() == 1)
			{
				yulAssert(!_jump.backwards, "");
				(*this)(*_jump.target);
			}
			else
			{
				if (!m_blockLabels.count(_jump.target))
					m_blockLabels[_jump.target] = m_assembly.newLabelId();

				m_assembly.appendJumpTo(m_blockLabels[_jump.target]);
				if (!m_generated.count(_jump.target))
					(*this)(*_jump.target);
			}
		},
		[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
		{
			if (!m_blockLabels.count(_conditionalJump.nonZero))
				m_blockLabels[_conditionalJump.nonZero] = m_assembly.newLabelId();
			m_assembly.appendJumpToIf(m_blockLabels[_conditionalJump.nonZero]);
			m_stack.pop_back();

			assertLayoutCompatibility(m_stack, m_stackLayout.blockInfos.at(_conditionalJump.nonZero).entryLayout);
			assertLayoutCompatibility(m_stack, m_stackLayout.blockInfos.at(_conditionalJump.zero).entryLayout);

			{
				ScopedSaveAndRestore stackRestore(m_stack, Stack{m_stack});

				if (!m_blockLabels.count(_conditionalJump.zero))
					m_blockLabels[_conditionalJump.zero] = m_assembly.newLabelId();
				if (m_generated.count(_conditionalJump.zero))
					m_assembly.appendJumpTo(m_blockLabels[_conditionalJump.zero]);
				else
					(*this)(*_conditionalJump.zero);
			}

			if (!m_generated.count(_conditionalJump.nonZero))
			{
				m_assembly.setStackHeight(static_cast<int>(m_stack.size()));
				(*this)(*_conditionalJump.nonZero);
			}
		},
		[&](CFG::BasicBlock::FunctionReturn const& _functionReturn)
		{
			yulAssert(m_currentFunctionInfo, "");
			yulAssert(m_currentFunctionInfo == _functionReturn.info, "");

			Stack exitStack = m_currentFunctionInfo->returnVariables | ranges::views::transform([](auto const& _varSlot){
				return StackSlot{_varSlot};
			}) | ranges::to<Stack>;
			exitStack.emplace_back(FunctionReturnLabelSlot{});

			createStackLayout(exitStack);
			m_assembly.setSourceLocation(locationOf(*m_currentFunctionInfo));
			m_assembly.appendJump(0, AbstractAssembly::JumpType::OutOfFunction); // TODO: stack height diff.
			m_assembly.setStackHeight(0);
			m_stack.clear();

		},
		[&](CFG::BasicBlock::Terminated const&)
		{
			m_assembly.setStackHeight(0);
			m_stack.clear();
		}
	}, _block.exit);
}

// TODO: It may or may not be nicer to merge this with createStackLayout (e.g. by adding an option to it that
//       disables actually emitting assembly output).
Stack OptimizedEVMCodeTransform::tryCreateStackLayout(Stack const& _currentStack, Stack _targetStack)
{
	Stack unreachable;
	Stack commonPrefix;
	for (auto&& [slot1, slot2]: ranges::zip_view(_currentStack, _targetStack))
	{
		if (!(slot1 == slot2))
			break;
		commonPrefix.emplace_back(slot1);
	}
	Stack temporaryStack = _currentStack | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>;

	::createStackLayout(temporaryStack, _targetStack  | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>, [&](unsigned _i) {
		if (_i > 16)
		{
			if (!util::findOffset(unreachable, temporaryStack.at(temporaryStack.size() - _i - 1)))
				unreachable.emplace_back(temporaryStack.at(temporaryStack.size() - _i - 1));
		}
	}, [&](unsigned _i) {
		if (_i > 16)
		{
			if (!util::findOffset(unreachable, temporaryStack.at(temporaryStack.size() - _i - 1)))
				unreachable.emplace_back(temporaryStack.at(temporaryStack.size() - _i - 1));
		}
	}, [&](StackSlot const& _slot) {
		Stack currentFullStack = commonPrefix;
		for (auto slot: temporaryStack)
			currentFullStack.emplace_back(slot);
		if (auto depth = util::findOffset(currentFullStack | ranges::views::reverse, _slot))
		{
			if (*depth + 1 > 16)
			{
				if (!util::findOffset(unreachable, _slot))
					unreachable.emplace_back(_slot);
			}
			return;
		}
	}, [&]() {});
	return unreachable;
}

void OptimizedEVMCodeTransform::createStackLayout(Stack _targetStack)
{
	Stack commonPrefix;
	for (auto&& [currentSlot, targetSlot]: ranges::zip_view(m_stack, _targetStack))
	{
		if (!(currentSlot == targetSlot) || holds_alternative<JunkSlot>(targetSlot))
			break;
		commonPrefix.emplace_back(targetSlot);
	}

	Stack temporaryStack = m_stack | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>;

	if (!tryCreateStackLayout(m_stack, _targetStack).empty())
	{
		yulAssert(false, "Stack too deep."); // Make this a hard failure now to focus on avoiding it earlier.
		// TODO: check if we can do better.
		// Maybe switching to a general "fix everything deep first" algorithm.
		// Or we just let these cases, that weren't fixed in the StackLayoutGenerator go through and report the
		// need for stack limit evasion for these cases instead.
		std::map<unsigned, StackSlot> slotsByDepth;
		for (auto slot: _targetStack | ranges::views::take_last(_targetStack.size() - commonPrefix.size()))
			if (auto offset = util::findOffset(m_stack | ranges::views::reverse | ranges::to<Stack>, slot))
				slotsByDepth.insert(std::make_pair(*offset, slot));
		for (auto slot: slotsByDepth | ranges::views::reverse | ranges::views::values)
			if (!util::findOffset(temporaryStack, slot))
			{
				auto offset = util::findOffset(m_stack | ranges::views::reverse | ranges::to<Stack>, slot);
				m_stack.emplace_back(slot);
				m_assembly.appendInstruction(evmasm::dupInstruction(static_cast<unsigned>(*offset + 1)));
			}

		temporaryStack = m_stack | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>;
	}


	::createStackLayout(temporaryStack, _targetStack  | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>, [&](unsigned _i) {
		m_assembly.appendInstruction(evmasm::swapInstruction(_i));
	}, [&](unsigned _i) {
		m_assembly.appendInstruction(evmasm::dupInstruction(_i));
	}, [&](StackSlot const& _slot) {
		Stack currentFullStack = commonPrefix;
		for (auto slot: temporaryStack)
			currentFullStack.emplace_back(slot);
		if (auto depth = util::findOffset(currentFullStack | ranges::views::reverse, _slot))
		{
			m_assembly.appendInstruction(evmasm::dupInstruction(static_cast<unsigned>(*depth + 1)));
			return;
		}
		std::visit(util::GenericVisitor{
			[&](LiteralSlot const& _literal)
			{
				m_assembly.setSourceLocation(locationOf(_literal));
				m_assembly.appendConstant(_literal.value);
			},
			[&](FunctionReturnLabelSlot const&)
			{
				yulAssert(false, "Cannot produce function return label.");
			},
			[&](FunctionCallReturnLabelSlot const& _returnLabel)
			{
				if (!m_returnLabels.count(&_returnLabel.call.get()))
					m_returnLabels[&_returnLabel.call.get()] = m_assembly.newLabelId();
				m_assembly.appendLabelReference(m_returnLabels.at(&_returnLabel.call.get()));
			},
			[&](VariableSlot const& _variable)
			{
				if (m_currentFunctionInfo)
					if (util::contains(m_currentFunctionInfo->returnVariables, _variable))
					{
						// TODO: maybe track uninitialized return variables.
						m_assembly.appendConstant(0);
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
				// TODO: discuss if PC is in fact a good choice here.
				// Advantages:
				// - costs only 2 gas
				// - deterministic value (in case it is in fact used due to some bug)
				// - hard to exploit in case of a bug
				// - distinctive, since it is not generated elsewhere
				// Disadvantages:
				// - static analysis might get confused until it realizes that these are always popped
				// Alternatives:
				// - any other opcode with cost 2
				// - unless the stack is empty: DUP1
				// - the constant 0
				// Note: it might even make sense to introduce a specific assembly item for this, s.t.
				//       the peephole optimizer can deal with this (e.g. POP PUSHJUNK can be removed).
				m_assembly.appendInstruction(evmasm::Instruction::PC);
			}
		}, _slot);
	}, [&]() { m_assembly.appendInstruction(evmasm::Instruction::POP); });
	m_stack = commonPrefix;
	for (auto slot: temporaryStack)
		m_stack.emplace_back(slot);
}

void OptimizedEVMCodeTransform::run(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	EVMDialect const& _dialect,
	BuiltinContext& _builtinContext,
	ExternalIdentifierAccess const&,
	bool _useNamedLabelsForFunctions
)
{
	std::unique_ptr<CFG> dfg = ControlFlowGraphBuilder::build(_analysisInfo, _dialect, _block);
	StackLayout stackLayout = StackLayoutGenerator::run(*dfg);
	OptimizedEVMCodeTransform optimizedCodeTransform(_assembly, _builtinContext, _useNamedLabelsForFunctions,  *dfg, stackLayout);
	optimizedCodeTransform(*dfg->entry);
	for (Scope::Function const* function: dfg->functions)
		optimizedCodeTransform(dfg->functionInfo.at(function));
}
