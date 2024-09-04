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

#include <libyul/backends/evm/SSACFGValidator.h>

#include <libsolutil/Visitor.h>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

using namespace solidity::yul;

void SSACFGValidator::validate(ControlFlow const& _controlFlow, Block const& _ast, AsmAnalysisInfo const& _analysisInfo, Dialect const& _dialect)
{
    Context context{_analysisInfo, _dialect, _controlFlow, *_controlFlow.mainGraph};
    SSACFGValidator validator(context);
    validator.m_currentBlock = context.cfg.entry;
	validator.m_currentOperation = 0;
    validator.consumeBlock(_ast);
}

bool SSACFGValidator::consumeBlock(Block const& _block)
{
	ScopedSaveAndRestore saveScope(m_scope, m_context.analysisInfo.scopes.at(&_block).get());
	for (auto const& stmt: _block.statements)
		if (!consumeStatement(stmt))
			return false;
	return true;
}

bool SSACFGValidator::consumeStatement(Statement const& _statement)
{
	return std::visit(util::GenericVisitor{
		[&](ExpressionStatement const& _stmt) {
			if (auto results = consumeExpression(_stmt.expression))
			{
				yulAssert(results->empty());
				return true;
			}
			return false;
		},
		[&](Assignment const& _assignment) {
			if (auto results = consumeExpression(*_assignment.value))
			{
				yulAssert(results->size() == _assignment.variableNames.size());
				for(auto&& [variable, value]: ranges::zip_view(_assignment.variableNames, *results))
					m_currentVariableValues.at(resolveVariable(variable.name)) = value;
				return true;
			}
			return false;
		},
		[&](VariableDeclaration const& _variableDeclaration) {
			if (auto results = consumeExpression(*_variableDeclaration.value))
			{
				yulAssert(results->size() == _variableDeclaration.variables.size());
				for(auto&& [variable, value]: ranges::zip_view(_variableDeclaration.variables, *results))
				{
					yulAssert(!m_currentVariableValues.count(resolveVariable(variable.name)));
					m_currentVariableValues[resolveVariable(variable.name)] = value;
				}
				return true;
			}
			return false;
		},
		[&](FunctionDefinition const&) {
			// TODO
			return true;
		},
		[&](If const& _if) {
			if (auto cond = consumeUnaryExpression(*_if.condition))
			{
				auto const& exit = expectConditionalJump();
				yulAssert(exit.condition == *cond);
				// transform via the zero-exit phis already
				applyPhis(m_currentBlock, exit.zero);
				auto zeroValues = m_currentVariableValues;

				m_currentBlock = exit.nonZero;
				m_currentOperation = 0;
				// TODO: process phi functions
				if (consumeBlock(_if.body))
				{
					auto const& jumpBack = expectUnconditionalJump();
					yulAssert(exit.zero == jumpBack.target);
					// TODO: consolidate m_currentVariableValues and previousValues - resp. consolidate current scope
				}

				m_currentBlock = exit.zero;
				m_currentOperation = 0;
				m_currentVariableValues = zeroValues;
				// TODO: process phi functions
				return true;
			}
			return false;
		},
		[&](Switch const&) {
			// TODO
			return true;
		},
		[&](ForLoop const& _loop)
		{
        	ScopedSaveAndRestore scopeRestore(m_scope, m_context.analysisInfo.scopes.at(&_loop.pre).get());
			consumeBlock(_loop.pre);
			// TODO: store/restore continue/break targets
			if (auto cond = consumeUnaryExpression(*_loop.condition))
			{
				auto const& exit = expectConditionalJump();
				yulAssert(exit.condition == cond);
				// TODO
				return true;
			}
			return false;
		},
		[&](Break const&) {
			// TODO: expect unconditional jump and consolidate state
			return false;
		},
		[&](Continue const&) {
			// TODO: expect unconditional jump and consolidate state
			return false;
		},
		[&](Leave const&) {
			// TODO: expect function exit and consolidate state
			return false;
		},
		[&](Block const& _nestedBlock) {
			return consumeBlock(_nestedBlock);
		}
	}, _statement);
}

std::optional<std::vector<SSACFG::ValueId>> SSACFGValidator::consumeExpression(Expression const& _expression)
{
	return std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) -> std::optional<std::vector<SSACFG::ValueId>>
		{
			std::vector<SSACFG::ValueId> arguments;
			for(auto& _arg: _call.arguments | ranges::view::reverse)
			{
				if (auto arg = consumeExpression(_arg))

					arguments += *arg;
				else
					return std::nullopt;
			}
			auto const& currentOp = currentBlock().operations.at(m_currentOperation++);
			yulAssert(currentOp.inputs.size() == arguments.size());
			for (auto&& [input, arg]: ranges::zip_view(currentOp.inputs, arguments | ranges::view::reverse))
				yulAssert(input == arg);
			if (validateCall(currentOp.kind, _call.functionName, currentOp.outputs.size()))
				return currentOp.outputs;
			else
				return std::nullopt;
		},
		[&](Identifier const& _identifier) -> std::optional<std::vector<SSACFG::ValueId>>
		{
			return {{lookupIdentifier(_identifier)}};
		},
		[&](Literal const& _literal) -> std::optional<std::vector<SSACFG::ValueId>>
		{
			return {{lookupLiteral(_literal)}};
		}
	}, _expression);
}

SSACFG::BasicBlock::ConditionalJump const& SSACFGValidator::expectConditionalJump() const
{
	yulAssert(m_currentOperation == currentBlock().operations.size());
	auto const* exit = std::get_if<SSACFG::BasicBlock::ConditionalJump>(&currentBlock().exit);
	yulAssert(exit);
	return *exit;
}

SSACFG::BasicBlock::Jump const& SSACFGValidator::expectUnconditionalJump() const
{
	yulAssert(m_currentOperation == currentBlock().operations.size());
	auto const* exit = std::get_if<SSACFG::BasicBlock::Jump>(&currentBlock().exit);
	yulAssert(exit);
	return *exit;
}

bool SSACFGValidator::validateCall(std::variant<SSACFG::BuiltinCall, SSACFG::Call> const& _kind, Identifier const& _functionName, size_t _numOutputs) const
{
	return std::visit(util::GenericVisitor{
		[&](SSACFG::BuiltinCall const& _call)
		{
			auto const* builtin = m_context.dialect.builtin(_functionName.name);
			yulAssert(&_call.builtin.get() == builtin);
			yulAssert(builtin->numReturns == _numOutputs);
			return builtin->controlFlowSideEffects.canContinue;
		},
		[&](SSACFG::Call const& _call)
		{
			auto const* function = resolveFunction(_functionName.name);
			yulAssert(function);
			yulAssert(m_context.controlFlow.functionGraph(function)->returns.size() == _numOutputs);
			yulAssert(&_call.function.get() == function);
			return _call.canContinue;
		}
	}, _kind);
}


Scope::Variable const* SSACFGValidator::resolveVariable(YulName _name) const
{
	yulAssert(m_scope, "");
	Scope::Variable const* var = nullptr;
	if (m_scope->lookup(_name, util::GenericVisitor{
		[&](Scope::Variable& _var) { var = &_var; },
		[](Scope::Function&)
		{
			yulAssert(false, "Function not removed during desugaring.");
		}
	}))
	{
		yulAssert(var, "");
		return var;
	};
	yulAssert(false, "External identifier access unimplemented.");
}

Scope::Function const* SSACFGValidator::resolveFunction(YulName _name) const
{
	Scope::Function const* function = nullptr;
	yulAssert(m_scope->lookup(_name, util::GenericVisitor{
		[](Scope::Variable&) { yulAssert(false, "Expected function name."); },
		[&](Scope::Function& _function) { function = &_function; }
	}), "Function name not found.");
	yulAssert(function, "");
	return function;
}


SSACFG::ValueId SSACFGValidator::lookupIdentifier(Identifier const& _identifier) const
{
	auto const* var = resolveVariable(_identifier.name);
	return m_currentVariableValues.at(var);
}

SSACFG::ValueId SSACFGValidator::lookupLiteral(Literal const& _literal) const
{
	return m_context.cfg.getLiteral(_literal.value.value());
}

void SSACFGValidator::applyPhis(SSACFG::BlockId _source, SSACFG::BlockId _target)
{
	auto const& targetBlock = m_context.cfg.block(_target);
	auto entryOffset = util::findOffset(targetBlock.entries, _source);
	yulAssert(entryOffset);
	for (auto phi: targetBlock.phis)
	{
		auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&m_context.cfg.valueInfo(phi));
		yulAssert(phiInfo);
		// TODO: horrible complexity
		for (auto&& [var, value]: m_currentVariableValues)
		{
			if (value == phiInfo->arguments.at(*entryOffset))
			{
				value = phi;
			}
		}
	}
}