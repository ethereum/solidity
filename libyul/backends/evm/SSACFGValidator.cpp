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
#include <range/v3/to_container.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/filter.hpp>
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
		if (std::holds_alternative<FunctionDefinition>(stmt) && !consumeStatement(stmt))
			return false;
	for (auto const& stmt: _block.statements)
		if (!std::holds_alternative<FunctionDefinition>(stmt) && !consumeStatement(stmt))
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
		[&](FunctionDefinition const& _function) {
			auto* function = resolveFunction(_function.name);
			auto const* functionGraph = m_context.controlFlow.functionGraph(function);
			yulAssert(functionGraph);
			Context nestedContext{m_context.analysisInfo, m_context.dialect, m_context.controlFlow, *functionGraph};
			SSACFGValidator nestedValidator(nestedContext);
			nestedValidator.m_currentBlock = functionGraph->entry;
			nestedValidator.m_currentOperation = 0;
			yulAssert(_function.parameters.size() == functionGraph->arguments.size());
			yulAssert(_function.returnVariables.size() == functionGraph->returns.size());
			nestedValidator.m_currentVariableValues = functionGraph->arguments
				| ranges::views::transform([](auto const& tup) { return std::make_pair(&std::get<0>(tup).get(), std::get<1>(tup)); })
				| ranges::to<std::map>;
			for (auto const& returnVar: functionGraph->returns)
				nestedValidator.m_currentVariableValues[&returnVar.get()] = {0};
			nestedValidator.consumeBlock(_function.body);
			for (auto const& returnVar: functionGraph->returns)
				yulAssert(nestedValidator.m_currentVariableValues.at(&returnVar.get()).hasValue());
			return true;
		},
		[&](If const& _if) {
			if (auto cond = consumeUnaryExpression(*_if.condition))
			{
				auto const& exit = expectConditionalJump();
				yulAssert(exit.condition == *cond);
				auto zeroBranchVariableValues = applyPhis(m_currentBlock, exit.zero);

				m_currentBlock = exit.nonZero;
				m_currentOperation = 0;
				if (consumeBlock(_if.body))
				{
					auto const& jumpBack = expectUnconditionalJump();
					yulAssert(exit.zero == jumpBack.target);
					auto nonZeroBranchVariableValues = applyPhis(m_currentBlock, jumpBack.target);
					// consolidate the values in the zero branch with the values in the non-zero branch
					for (auto&& [var, value]: zeroBranchVariableValues)
						yulAssert(nonZeroBranchVariableValues.at(var) == value);
				}

				m_currentBlock = exit.zero;
				m_currentOperation = 0;
				m_currentVariableValues = zeroBranchVariableValues;
				return true;
			}
			return false;
		},
		[&](Switch const& _switch) {
			// todo this will be different for EOF and can probably be simplified
			if(auto cond = consumeUnaryExpression(*_switch.expression))
			{
				auto const validateGhostEq = [this, cond](SSACFG::Operation const& _operation) {
					yulAssert(std::holds_alternative<SSACFG::BuiltinCall>(_operation.kind));
					yulAssert(&std::get<SSACFG::BuiltinCall>(_operation.kind).builtin.get() == m_context.dialect.equalityFunction());
					yulAssert(_operation.inputs.size() == 2);
					yulAssert(_operation.inputs.back() == *cond);
					yulAssert(_operation.outputs.size() == 1);
				};
				yulAssert(currentBlock().operations.size() >= 2, "switch at least has the expression and ghost eq");
				yulAssert(m_currentOperation == currentBlock().operations.size() - 1);
				auto const& ghostEq = currentBlock().operations.back();
				validateGhostEq(ghostEq);
				m_currentOperation = currentBlock().operations.size();
				std::reference_wrapper<const SSACFG::BasicBlock::ConditionalJump> exit = expectConditionalJump();
				yulAssert(exit.get().condition == ghostEq.outputs[0]);
				// auto zeroBranchVariableValues = applyPhis(m_currentBlock, exit.zero);
				std::optional<SSACFG::BlockId> afterSwitch{std::nullopt};
				m_currentBlock = exit.get().nonZero;
				m_currentOperation = 0;
				for (auto const& switchCase: _switch.cases | ranges::views::drop_last(2))
				{
					if (consumeBlock(switchCase.body))
					{
						auto const& jumpBack = expectUnconditionalJump();
						yulAssert(!afterSwitch || *afterSwitch == jumpBack.target);
						afterSwitch = jumpBack.target;
						auto nonZeroBranchVariableValues = applyPhis(m_currentBlock, *afterSwitch);
						for (auto&& [var, value]: nonZeroBranchVariableValues)
							yulAssert(nonZeroBranchVariableValues.at(var) == value);
					}
					m_currentBlock = exit.get().zero;
					m_currentOperation = 0;
					yulAssert(currentBlock().operations.size() == 1);
					validateGhostEq(currentBlock().operations.front());
					m_currentOperation = 1;
					auto const& jumpDeeper = expectConditionalJump();
					yulAssert(jumpDeeper.condition == currentBlock().operations.front().outputs[0]);
					exit = jumpDeeper;
					m_currentBlock = jumpDeeper.nonZero;
					m_currentOperation = 0;
				}

				m_currentBlock = exit.get().nonZero;
				m_currentOperation = 0;
				if (_switch.cases.size() >= 2 && consumeBlock(_switch.cases.at(_switch.cases.size() - 2).body))
				{
					auto const& jumpBack = expectUnconditionalJump();
					yulAssert(!afterSwitch || *afterSwitch == jumpBack.target);
					afterSwitch = jumpBack.target;
					auto nonZeroBranchVariableValues = applyPhis(m_currentBlock, *afterSwitch);
					for (auto&& [var, value]: nonZeroBranchVariableValues)
						yulAssert(nonZeroBranchVariableValues.at(var) == value);
				}

				m_currentBlock = exit.get().zero;
				m_currentOperation = 0;
				if (!_switch.cases.empty() && consumeBlock(_switch.cases.at(_switch.cases.size() - 1).body))
				{
					auto const& jumpBack = expectUnconditionalJump();
					yulAssert(!afterSwitch || *afterSwitch == jumpBack.target);
					afterSwitch = jumpBack.target;
					auto zeroBranchVariableValues = applyPhis(m_currentBlock, *afterSwitch);
					m_currentVariableValues = zeroBranchVariableValues;
					// consolidate the values in the zero branch with the values in the non-zero branch
					for (auto&& [var, value]: zeroBranchVariableValues)
						yulAssert(zeroBranchVariableValues.at(var) == value);
				}
				else
					yulAssert(!_switch.cases.empty(), "empty switch is forbidden, we need at least the default case");

				// todo double check that this is actually sane and we always have an
				//  after switch (consuming body of a case returning false could be a weird case)
				yulAssert(afterSwitch);
				m_currentBlock = *afterSwitch;
				m_currentOperation = 0;
				return true;
			}
			return false;
		},
		[&](ForLoop const& _loop)
		{
			// TODO: try to clean up and simplify especially the "continue" validation
        	ScopedSaveAndRestore scopeRestore(m_scope, m_context.analysisInfo.scopes.at(&_loop.pre).get());
			consumeBlock(_loop.pre);
			auto const& entryJump = expectUnconditionalJump();

			auto entryVariableValues = applyPhis(m_currentBlock, entryJump.target);
			m_currentBlock = entryJump.target;
			m_currentOperation = 0;
			m_currentVariableValues = entryVariableValues;

			if (auto cond = consumeUnaryExpression(*_loop.condition))
			{
				auto const& exit = expectConditionalJump();
				yulAssert(exit.condition == cond);
				auto loopExitVariableValues = applyPhis(m_currentBlock, exit.zero);

				std::unique_ptr<LoopInfo> loopInfo = std::make_unique<LoopInfo>(LoopInfo{
					entryVariableValues | ranges::view::keys | ranges::to<std::set<Scope::Variable const*>>,
						loopExitVariableValues,
						exit.zero,
						std::nullopt,
						std::nullopt
				});
				std::swap(m_currentLoopInfo, loopInfo);
				m_currentBlock = exit.nonZero;
				m_currentOperation = 0;
				if (consumeBlock(_loop.body))
				{
					std::swap(m_currentLoopInfo, loopInfo);

					auto const& jumpToPost = expectUnconditionalJump();
					m_currentVariableValues = applyPhis(m_currentBlock, jumpToPost.target);
					if (loopInfo->postBlock)
						yulAssert(loopInfo->postBlock == jumpToPost.target);
					if (loopInfo->loopPostVariableValues)
						for (auto var: entryVariableValues | ranges::view::keys)
							yulAssert(loopInfo->loopPostVariableValues->at(var) == m_currentVariableValues.at(var));
					m_currentBlock = jumpToPost.target;
					m_currentOperation = 0;
					if (consumeBlock(_loop.post))
					{
						auto const& jumpBackToCondition = expectUnconditionalJump();
						m_currentVariableValues = applyPhis(m_currentBlock, jumpBackToCondition.target);
						m_currentBlock = jumpBackToCondition.target;
						m_currentOperation = 0;
						yulAssert(m_currentBlock == entryJump.target);
						for (auto&& [var, value]: entryVariableValues)
							yulAssert(m_currentVariableValues.at(var) == value);

					}
				}
				else
					std::swap(m_currentLoopInfo, loopInfo);
				m_currentBlock = exit.zero;
				m_currentOperation = 0;
				m_currentVariableValues = loopExitVariableValues;
				return true;
			}
			return false;
		},
		[&](Break const&) {
			yulAssert(m_currentLoopInfo);
			auto const& breakJump = expectUnconditionalJump();
			yulAssert(breakJump.target == m_currentLoopInfo->exitBlock);
			m_currentVariableValues = applyPhis(m_currentBlock, breakJump.target);
			for (auto&& [var, value]: m_currentLoopInfo->loopExitVariableValues)
				yulAssert(m_currentVariableValues.at(var) == value);
			return false;
		},
		[&](Continue const&) {
			// TODO: check if this can be simplified
			yulAssert(m_currentLoopInfo);
			auto const& continueJump = expectUnconditionalJump();
			if (m_currentLoopInfo->postBlock)
				yulAssert(continueJump.target == *m_currentLoopInfo->postBlock);
			else
				m_currentLoopInfo->postBlock = continueJump.target;
			m_currentVariableValues = applyPhis(m_currentBlock, continueJump.target);
			if (m_currentLoopInfo->loopPostVariableValues)
				for (auto const* var: m_currentLoopInfo->loopVariables)
					yulAssert(m_currentVariableValues.at(var) == m_currentLoopInfo->loopPostVariableValues->at(var));
			else
				m_currentLoopInfo->loopPostVariableValues = m_currentVariableValues;
			return false;
		},
		[&](Leave const&) {
			auto const& functionReturn = expectFunctionReturn();
			yulAssert(functionReturn.returnValues.size() == m_context.cfg.returns.size());
			for(auto&& [variable, value]: ranges::zip_view(m_context.cfg.returns, functionReturn.returnValues))
			{
				yulAssert(m_currentVariableValues.count(&variable.get()));
				m_currentVariableValues[&variable.get()] = value;
			}
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
			yulAssert(currentOp.inputs == arguments);
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

SSACFG::BasicBlock::FunctionReturn const& SSACFGValidator::expectFunctionReturn() const
{
	yulAssert(m_currentOperation == currentBlock().operations.size());
	auto const* functionReturn = std::get_if<SSACFG::BasicBlock::FunctionReturn>(&currentBlock().exit);
	yulAssert(functionReturn);
	return *functionReturn;
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
	auto valueId = m_context.cfg.lookupLiteral(_literal.value);
	yulAssert(valueId.hasValue());
	return valueId;
}

std::map<Scope::Variable const*, SSACFG::ValueId> SSACFGValidator::applyPhis(SSACFG::BlockId _source, SSACFG::BlockId _target)
{
	auto const& targetBlock = m_context.cfg.block(_target);
	auto entryOffset = util::findOffset(targetBlock.entries, _source);
	yulAssert(entryOffset);
	std::map<SSACFG::ValueId, SSACFG::ValueId> phiMap;
	for (auto phi: targetBlock.phis)
	{
		auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&m_context.cfg.valueInfo(phi));
		yulAssert(phiInfo);
		phiMap[phiInfo->arguments.at(*entryOffset)] = phi;
	}
	std::map<Scope::Variable const*, SSACFG::ValueId> result;
	for (auto& [var, value]: m_currentVariableValues)
		result[var] = util::valueOrDefault(phiMap, value, value);
	return result;
}
