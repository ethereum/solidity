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
	yulAssert(_currentStack.size() == _desiredStack.size(), "");
	for (auto&& [currentSlot, desiredSlot]: ranges::zip_view(_currentStack, _desiredStack))
		yulAssert(holds_alternative<JunkSlot>(desiredSlot) || currentSlot == desiredSlot, "");
}

AbstractAssembly::LabelID OptimizedEVMCodeTransform::getFunctionLabel(Scope::Function const& _function)
{
	CFG::FunctionInfo const& functionInfo = m_dfg.functionInfo.at(&_function);
	if (!m_functionLabels.count(&functionInfo))
		m_functionLabels[&functionInfo] = m_useNamedLabelsForFunctions ?
			m_assembly.namedLabel(
				functionInfo.function.name.str(),
				functionInfo.function.arguments.size(),
				functionInfo.function.returns.size(),
				{}
			) : m_assembly.newLabelId();
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
			yulAssert(temporarySlot && &temporarySlot->call.get() == &_call && temporarySlot->index == 0, "");
		}
	}, _expression);
}

void OptimizedEVMCodeTransform::operator()(CFG::FunctionInfo const& _functionInfo)
{
	yulAssert(!m_currentFunctionInfo, "");
	ScopedSaveAndRestore currentFunctionSetter(m_currentFunctionInfo, &_functionInfo);

	yulAssert(m_stack.empty() && m_assembly.stackHeight() == 0, "");

	m_stack.emplace_back(FunctionReturnLabelSlot{_functionInfo.function});
	for (auto const& param: _functionInfo.parameters | ranges::views::reverse)
		m_stack.emplace_back(param);
	m_assembly.setStackHeight(static_cast<int>(m_stack.size()));
	m_assembly.setSourceLocation(locationOf(_functionInfo));

	m_assembly.appendLabel(getFunctionLabel(_functionInfo.function));

	Stack const& entryLayout = m_stackLayout.blockInfos.at(_functionInfo.entry).entryLayout;
	createStackLayout(entryLayout);

	(*this)(*_functionInfo.entry);

	m_stack.clear();
	m_assembly.setStackHeight(0);
}

void OptimizedEVMCodeTransform::operator()(CFG::FunctionCall const& _call)
{
	yulAssert(m_stack.size() >= _call.function.get().arguments.size() + 1, "");
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");

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
	m_assembly.appendLabel(m_returnLabels.at(&_call.functionCall.get()));
	// Remove arguments and return label from m_stack.
	for (size_t i = 0; i < _call.function.get().arguments.size() + 1; ++i)
		m_stack.pop_back();
	// Push return values to m_stack.
	for (size_t index: ranges::views::iota(0u, _call.function.get().returns.size()))
		m_stack.emplace_back(TemporarySlot{_call.functionCall, index});
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
}

void OptimizedEVMCodeTransform::operator()(CFG::BuiltinCall const& _call)
{
	yulAssert(m_stack.size() >= _call.arguments, "");
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
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
	for (size_t index: ranges::views::iota(0u, _call.builtin.get().returns.size()))
		m_stack.emplace_back(TemporarySlot{_call.functionCall, index});

	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
}

void OptimizedEVMCodeTransform::operator()(CFG::Assignment const& _assignment)
{
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
	for (auto& currentSlot: m_stack)
		if (VariableSlot const* varSlot = get_if<VariableSlot>(&currentSlot))
			if (util::findOffset(_assignment.variables, *varSlot))
				currentSlot = JunkSlot{};

	yulAssert(m_stack.size() >= _assignment.variables.size(), "");
	for (auto&& [currentSlot, varSlot]: ranges::zip_view(m_stack | ranges::views::take_last(_assignment.variables.size()), _assignment.variables))
		currentSlot = varSlot;
}

void OptimizedEVMCodeTransform::operator()(CFG::BasicBlock const& _block)
{
	// Assert that this is the first visit of the block.
	yulAssert(m_generated.insert(&_block).second, "");

	auto const& blockInfo = m_stackLayout.blockInfos.at(&_block);

	if (auto label = util::valueOrNullptr(m_blockLabels, &_block))
		m_assembly.appendLabel(*label);

	assertLayoutCompatibility(m_stack, blockInfo.entryLayout);
	m_stack = blockInfo.entryLayout;
	yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");

	for (auto const& operation: _block.operations)
	{
		createStackLayout(m_stackLayout.operationEntryLayout.at(&operation));
		yulAssert(m_stack.size() >= operation.input.size(), "");
		size_t baseHeight = m_stack.size() - operation.input.size();
		assertLayoutCompatibility(
			m_stack | ranges::views::take_last(operation.input.size()) | ranges::to<Stack>,
			operation.input
		);
		std::visit(*this, operation.operation);
		yulAssert(m_stack.size() == baseHeight + operation.output.size(), "");
		yulAssert(m_stack.size() >= operation.output.size(), "");
		assertLayoutCompatibility(
			m_stack | ranges::views::take_last(operation.output.size()) | ranges::to<Stack>,
			operation.output
		);
	}

	std::visit(util::GenericVisitor{
		[&](CFG::BasicBlock::MainExit const&)
		{
			m_assembly.appendInstruction(evmasm::Instruction::STOP);
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

				if (m_generated.count(_jump.target))
					m_assembly.appendJumpTo(m_blockLabels[_jump.target]);
				else
					(*this)(*_jump.target);
			}
		},
		[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
		{
			createStackLayout(blockInfo.exitLayout);
			if (!m_blockLabels.count(_conditionalJump.nonZero))
				m_blockLabels[_conditionalJump.nonZero] = m_assembly.newLabelId();
			yulAssert(!m_stack.empty(), "");
			yulAssert(m_stack.back() == _conditionalJump.condition, "");
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
			exitStack.emplace_back(FunctionReturnLabelSlot{_functionReturn.info->function});

			createStackLayout(exitStack);
			m_assembly.setSourceLocation(locationOf(*m_currentFunctionInfo));
			m_assembly.appendJump(0, AbstractAssembly::JumpType::OutOfFunction);
		},
		[&](CFG::BasicBlock::Terminated const&)
		{
			yulAssert(!_block.operations.empty(), "");
			CFG::BuiltinCall const* builtinCall = get_if<CFG::BuiltinCall>(&_block.operations.back().operation);
			yulAssert(builtinCall && builtinCall->builtin.get().controlFlowSideEffects.terminates, "");
		}
	}, _block.exit);
	// TODO: we could assert that the last emitted assembly item terminated or was an (unconditional) jump.
	m_stack.clear();
	m_assembly.setStackHeight(0);
}

void OptimizedEVMCodeTransform::createStackLayout(Stack _targetStack)
{
	static constexpr auto slotVariableName = [](StackSlot const& _slot) {
		return std::visit(util::GenericVisitor{
			[](VariableSlot const& _var) { return _var.variable.get().name; },
			[](auto const&) { return YulString{}; }
		}, _slot);
	};

	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
	// ::createStack asserts that it has achieved the target layout.
	::createStackLayout(m_stack, _targetStack | ranges::to<Stack>, [&](unsigned _i) {
		yulAssert(_i > 0, "");
		if (_i > 16)
		{
			int deficit = static_cast<int>(_i - 16);
			StackSlot const& deepSlot = m_stack.at(m_stack.size() - _i - 1);
			YulString varNameDeep = slotVariableName(deepSlot);
			YulString varNameTop = slotVariableName(m_stack.back());
			string msg = "Cannot swap " + (varNameDeep.empty() ? "Slot " + stackSlotToString(deepSlot) : "Variable " + varNameDeep.str()) +
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
		else
			m_assembly.appendInstruction(evmasm::swapInstruction(_i));
	}, [&](StackSlot const& _slot) {
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
				string msg = (varName.empty() ? "Slot " + stackSlotToString(_slot) : "Variable " + varName.str())
					+ " is " + to_string(*depth - 15) + " too deep in the stack " + stackToString(m_stack);
				m_stackErrors.emplace_back(StackTooDeepError(
					m_currentFunctionInfo ? m_currentFunctionInfo->function.name : YulString{},
					varName,
					deficit,
					msg
				));
				m_assembly.markAsInvalid();
				m_assembly.appendConstant(u256(0xDEADBEEF));
				return;
			}
		}
		yulAssert(static_cast<int>(m_stack.size()) == m_assembly.stackHeight(), "");
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
	yulAssert(m_assembly.stackHeight() == static_cast<int>(m_stack.size()), "");
}

vector<StackTooDeepError> OptimizedEVMCodeTransform::run(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	EVMDialect const& _dialect,
	BuiltinContext& _builtinContext,
	bool _useNamedLabelsForFunctions
)
{
	std::unique_ptr<CFG> dfg = ControlFlowGraphBuilder::build(_analysisInfo, _dialect, _block);
	StackLayout stackLayout = StackLayoutGenerator::run(*dfg);
	OptimizedEVMCodeTransform optimizedCodeTransform(_assembly, _builtinContext, _useNamedLabelsForFunctions,  *dfg, stackLayout);
	// Create initial entry layout.
	optimizedCodeTransform.createStackLayout(stackLayout.blockInfos.at(dfg->entry).entryLayout);
	optimizedCodeTransform(*dfg->entry);
	for (Scope::Function const* function: dfg->functions)
		optimizedCodeTransform(dfg->functionInfo.at(function));
	return optimizedCodeTransform.stackErrors();
}
