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
#include <libyul/backends/evm/DirectEVMCodeTransform.h>

#include <libyul/backends/evm/StackHelpers.h>

#include <libsolutil/Visitor.h>

#include <range/v3/range/conversion.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

void DirectEVMCodeTransform::run(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions,
	AsmAnalysisInfo const& _analysisInfo,
	Dialect const& _dialect,
	Block const& _ast
)
{
	DirectStackLayoutGenerator::Context context = DirectStackLayoutGenerator::run(_analysisInfo, _dialect, _ast);
	DirectEVMCodeTransform codeTransform{
		_assembly,
		_builtinContext,
		_useNamedLabelsForFunctions,
		context,
		_analysisInfo,
		_dialect
	};
	codeTransform(_ast);
}

void DirectEVMCodeTransform::operator()(Block const& _block)
{
	auto const& blockInfo = m_context.layout.blockInfos.at(&_block);
	createStackLayout(debugDataOf(_block), blockInfo.entry);
	for(auto const& _statement: _block.statements)
	{
		auto const& statementInfo = m_context.layout.statementInfos.at(&_statement);
		createStackLayout(debugDataOf(_statement), statementInfo);
		visit(_statement);
	}
}

void DirectEVMCodeTransform::operator()(FunctionCall const&)
{

}
void DirectEVMCodeTransform::operator()(ExpressionStatement const&)
{

}
void DirectEVMCodeTransform::operator()(Assignment const&)
{

}
void DirectEVMCodeTransform::operator()(VariableDeclaration const&)
{

}
void DirectEVMCodeTransform::operator()(If const&)
{
	yulAssert(false, "");
}
void DirectEVMCodeTransform::operator()(Switch const&)
{
	yulAssert(false, "");
}
void DirectEVMCodeTransform::operator()(ForLoop const&)
{
	yulAssert(false, "");
}
void DirectEVMCodeTransform::operator()(FunctionDefinition const&)
{

}
void DirectEVMCodeTransform::operator()(Break const&)
{
	yulAssert(false, "");
}
void DirectEVMCodeTransform::operator()(Continue const&)
{
	yulAssert(false, "");
}
void DirectEVMCodeTransform::operator()(Leave const&)
{
	yulAssert(false, "");
}


void DirectEVMCodeTransform::createStackLayout(std::shared_ptr<DebugData const> _debugData, Stack _targetStack)
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
