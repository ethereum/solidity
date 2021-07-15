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
/**
 * Transformation of a Yul AST into a control flow graph.
 */

#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>
#include <libyul/AsmPrinter.h>

#include <libsolutil/cxx20.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/Algorithms.h>

#include <range/v3/action/push_back.hpp>
#include <range/v3/action/erase.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/take_last.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

std::unique_ptr<CFG> ControlFlowGraphBuilder::build(
	AsmAnalysisInfo const& _analysisInfo,
	Dialect const& _dialect,
	Block const& _block
)
{
	auto result = std::make_unique<CFG>();
	result->entry = &result->makeBlock();

	ControlFlowGraphBuilder builder(*result, _analysisInfo, _dialect);
	builder.m_currentBlock = result->entry;
	builder(_block);

	// Determine which blocks are reachable from the entry.
	util::BreadthFirstSearch<CFG::BasicBlock*> reachabilityCheck{{result->entry}};
	for (auto const& functionInfo: result->functionInfo | ranges::views::values)
		reachabilityCheck.verticesToTraverse.emplace_back(functionInfo.entry);

	reachabilityCheck.run([&](CFG::BasicBlock* _node, auto&& _addChild) {
		visit(util::GenericVisitor{
			[&](CFG::BasicBlock::Jump const& _jump) {
				_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _jump) {
				_addChild(_jump.zero);
				_addChild(_jump.nonZero);
			},
			[](CFG::BasicBlock::FunctionReturn const&) {},
			[](CFG::BasicBlock::Terminated const&) {},
			[](CFG::BasicBlock::MainExit const&) {}
		}, _node->exit);
	});

	// Remove all entries from unreachable nodes from the graph.
	for (CFG::BasicBlock* node: reachabilityCheck.visited)
		cxx20::erase_if(node->entries, [&](CFG::BasicBlock* entry) -> bool {
			return !reachabilityCheck.visited.count(entry);
		});

	// TODO: It might be worthwhile to run some further simplifications on the graph itself here.
	// E.g. if there is a jump to a node that has the jumping node as its only entry, the nodes can be fused, etc.

	return result;
}

ControlFlowGraphBuilder::ControlFlowGraphBuilder(
	CFG& _graph,
	AsmAnalysisInfo const& _analysisInfo,
	Dialect const& _dialect
):
	m_graph(_graph),
	m_info(_analysisInfo),
	m_dialect(_dialect)
{
}

StackSlot ControlFlowGraphBuilder::operator()(Literal const& _literal)
{
	return LiteralSlot{valueOfLiteral(_literal), _literal.debugData};
}

StackSlot ControlFlowGraphBuilder::operator()(Identifier const& _identifier)
{
	return VariableSlot{lookupVariable(_identifier.name), _identifier.debugData};
}

StackSlot ControlFlowGraphBuilder::operator()(Expression const& _expression)
{
	return std::visit(*this, _expression);
}

StackSlot ControlFlowGraphBuilder::operator()(FunctionCall const& _call)
{
	CFG::Operation const& operation = visitFunctionCall(_call);
	yulAssert(operation.output.size() == 1, "");
	return operation.output.front();
}

void ControlFlowGraphBuilder::operator()(VariableDeclaration const& _varDecl)
{
	yulAssert(m_currentBlock, "");
	auto declaredVariables = _varDecl.variables | ranges::views::transform([&](TypedName const& _var) {
		return VariableSlot{lookupVariable(_var.name), _var.debugData};
	}) | ranges::to<vector<VariableSlot>>;
	Stack input;
	if (_varDecl.value)
		input = visitAssignmentRightHandSide(*_varDecl.value, declaredVariables.size());
	else
		input = Stack(_varDecl.variables.size(), LiteralSlot{0, _varDecl.debugData});
	m_currentBlock->operations.emplace_back(CFG::Operation{
		std::move(input),
		declaredVariables | ranges::to<Stack>,
		CFG::Assignment{_varDecl.debugData, declaredVariables}
	});
}
void ControlFlowGraphBuilder::operator()(Assignment const& _assignment)
{
	auto assignedVariables = _assignment.variableNames | ranges::views::transform([&](Identifier const& _var) {
		return VariableSlot{lookupVariable(_var.name), _var.debugData};
	}) | ranges::to<vector<VariableSlot>>;

	yulAssert(m_currentBlock, "");
	m_currentBlock->operations.emplace_back(CFG::Operation{
		// input
		visitAssignmentRightHandSide(*_assignment.value, assignedVariables.size()),
		// output
		assignedVariables | ranges::to<Stack>,
		// operation
		CFG::Assignment{_assignment.debugData, assignedVariables}
	});
}
void ControlFlowGraphBuilder::operator()(ExpressionStatement const& _exprStmt)
{
	yulAssert(m_currentBlock, "");
	std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) {
			CFG::Operation const& operation = visitFunctionCall(_call);
			yulAssert(operation.output.empty(), "");
		},
		[&](auto const&) { yulAssert(false, ""); }
	}, _exprStmt.expression);

	// TODO: Ideally this would be done on the expression label and for all functions that always revert,
	//       not only for builtins.
	if (auto const* funCall = get_if<FunctionCall>(&_exprStmt.expression))
		if (BuiltinFunction const* builtin = m_dialect.builtin(funCall->functionName.name))
			if (builtin->controlFlowSideEffects.terminates)
			{
				m_currentBlock->exit = CFG::BasicBlock::Terminated{};
				m_currentBlock = &m_graph.makeBlock();
			}
}

void ControlFlowGraphBuilder::operator()(Block const& _block)
{
	ScopedSaveAndRestore saveScope(m_scope, m_info.scopes.at(&_block).get());
	for (auto const& statement: _block.statements)
		std::visit(*this, statement);
}

void ControlFlowGraphBuilder::operator()(If const& _if)
{
	auto&& [ifBranch, afterIf] = makeConditionalJump(std::visit(*this, *_if.condition));
	m_currentBlock = ifBranch;
	(*this)(_if.body);
	jump(*afterIf);
}

void ControlFlowGraphBuilder::operator()(Switch const& _switch)
{
	yulAssert(m_currentBlock, "");
	auto ghostVariableId = m_graph.ghostVariables.size();
	YulString ghostVariableName("GHOST[" + to_string(ghostVariableId) + "]");
	auto& ghostVar = m_graph.ghostVariables.emplace_back(Scope::Variable{""_yulstring, ghostVariableName});

	// Artificially generate:
	// let <ghostVariable> := <switchExpression>
	VariableSlot ghostVarSlot{ghostVar, debugDataOf(*_switch.expression)};
	m_currentBlock->operations.emplace_back(CFG::Operation{
		Stack{std::visit(*this, *_switch.expression)},
		Stack{ghostVarSlot},
		CFG::Assignment{_switch.debugData, {ghostVarSlot}}
	});

	BuiltinFunction const* equalityBuiltin = m_dialect.equalityFunction({});
	yulAssert(equalityBuiltin, "");

	// Artificially generate:
	// eq(<literal>, <ghostVariable>)
	auto makeValueCompare = [&](Literal const& _value) {
		yul::FunctionCall const& ghostCall = m_graph.ghostCalls.emplace_back(yul::FunctionCall{
			_value.debugData,
			yul::Identifier{{}, "eq"_yulstring},
			{_value, Identifier{{}, ghostVariableName}}
		});
		CFG::Operation& operation = m_currentBlock->operations.emplace_back(CFG::Operation{
			Stack{ghostVarSlot, LiteralSlot{valueOfLiteral(_value), _value.debugData}},
			Stack{TemporarySlot{ghostCall, 0}},
			CFG::BuiltinCall{_switch.debugData, *equalityBuiltin, ghostCall, 2},
		});
		return operation.output.front();
	};
	CFG::BasicBlock& afterSwitch = m_graph.makeBlock();
	yulAssert(!_switch.cases.empty(), "");
	for (auto const& switchCase: _switch.cases | ranges::views::drop_last(1))
	{
		yulAssert(switchCase.value, "");
		auto&& [caseBranch, elseBranch] = makeConditionalJump(makeValueCompare(*switchCase.value));
		m_currentBlock = caseBranch;
		(*this)(switchCase.body);
		jump(afterSwitch);
		m_currentBlock = elseBranch;
	}
	Case const& switchCase = _switch.cases.back();
	if (switchCase.value)
	{
		CFG::BasicBlock& caseBranch = m_graph.makeBlock();
		makeConditionalJump(makeValueCompare(*switchCase.value), caseBranch, afterSwitch);
		m_currentBlock = &caseBranch;
	}
	(*this)(switchCase.body);
	jump(afterSwitch);
}

void ControlFlowGraphBuilder::operator()(ForLoop const& _loop)
{
	ScopedSaveAndRestore scopeRestore(m_scope, m_info.scopes.at(&_loop.pre).get());
	(*this)(_loop.pre);

	std::optional<bool> constantCondition;
	if (auto const* literalCondition = get_if<yul::Literal>(_loop.condition.get()))
		constantCondition = valueOfLiteral(*literalCondition) != 0;

	CFG::BasicBlock& loopCondition = m_graph.makeBlock();
	CFG::BasicBlock& loopBody = m_graph.makeBlock();
	CFG::BasicBlock& post = m_graph.makeBlock();
	CFG::BasicBlock& afterLoop = m_graph.makeBlock();

	ScopedSaveAndRestore scopedSaveAndRestore(m_forLoopInfo, ForLoopInfo{afterLoop, post});

	if (constantCondition.has_value())
	{
		if (*constantCondition)
		{
			jump(loopBody);
			(*this)(_loop.body);
			jump(post);
			(*this)(_loop.post);
			jump(loopBody, true);
		}
		else
			jump(afterLoop);
	}
	else
	{
		jump(loopCondition);
		makeConditionalJump(std::visit(*this, *_loop.condition), loopBody, afterLoop);
		m_currentBlock = &loopBody;
		(*this)(_loop.body);
		jump(post);
		(*this)(_loop.post);
		jump(loopCondition, true);
	}

	m_currentBlock = &afterLoop;
}

void ControlFlowGraphBuilder::operator()(Break const&)
{
	yulAssert(m_forLoopInfo.has_value(), "");
	jump(m_forLoopInfo->afterLoop);
	m_currentBlock = &m_graph.makeBlock();
}

void ControlFlowGraphBuilder::operator()(Continue const&)
{
	yulAssert(m_forLoopInfo.has_value(), "");
	jump(m_forLoopInfo->post);
	m_currentBlock = &m_graph.makeBlock();
}

void ControlFlowGraphBuilder::operator()(Leave const&)
{
	yulAssert(m_currentFunctionExit.has_value(), "");
	m_currentBlock->exit = *m_currentFunctionExit;
	m_currentBlock = &m_graph.makeBlock();
}

void ControlFlowGraphBuilder::operator()(FunctionDefinition const& _function)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_function.name));
	m_graph.functions.emplace_back(&function);

	yulAssert(m_info.scopes.at(&_function.body), "");
	Scope* virtualFunctionScope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	yulAssert(virtualFunctionScope, "");

	auto&& [it, inserted] = m_graph.functionInfo.emplace(std::make_pair(&function, CFG::FunctionInfo{
		_function.debugData,
		function,
		&m_graph.makeBlock(),
		_function.parameters | ranges::views::transform([&](auto const& _param) {
			return VariableSlot{
				std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_param.name)),
				_param.debugData
			};
		}) | ranges::to<vector>,
		_function.returnVariables | ranges::views::transform([&](auto const& _retVar) {
			return VariableSlot{
				std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_retVar.name)),
				_retVar.debugData
			};
		}) | ranges::to<vector>
	}));
	yulAssert(inserted, "");
	CFG::FunctionInfo& functionInfo = it->second;

	ControlFlowGraphBuilder builder{m_graph, m_info, m_dialect};
	builder.m_currentFunctionExit = CFG::BasicBlock::FunctionReturn{&functionInfo};
	builder.m_currentBlock = functionInfo.entry;
	builder(_function.body);
	builder.m_currentBlock->exit = CFG::BasicBlock::FunctionReturn{&functionInfo};
}


CFG::Operation const& ControlFlowGraphBuilder::visitFunctionCall(FunctionCall const& _call)
{
	yulAssert(m_scope, "");
	yulAssert(m_currentBlock, "");

	if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		Stack inputs;
		for (auto&& [idx, arg]: _call.arguments | ranges::views::enumerate | ranges::views::reverse)
			if (!builtin->literalArgument(idx).has_value())
				inputs.emplace_back(std::visit(*this, arg));
		CFG::BuiltinCall builtinCall{_call.debugData, *builtin, _call, inputs.size()};
		return m_currentBlock->operations.emplace_back(CFG::Operation{
			// input
			std::move(inputs),
			// output
			ranges::views::iota(0u, builtin->returns.size()) | ranges::views::transform([&](size_t _i) {
				return TemporarySlot{_call, _i};
			}) | ranges::to<Stack>,
			// operation
			move(builtinCall)
		});
	}
	else
	{
		Scope::Function const& function = lookupFunction(_call.functionName.name);
		Stack inputs{FunctionCallReturnLabelSlot{_call}};
		for (auto const& arg: _call.arguments | ranges::views::reverse)
			inputs.emplace_back(std::visit(*this, arg));
		return m_currentBlock->operations.emplace_back(CFG::Operation{
			// input
			std::move(inputs),
			// output
			ranges::views::iota(0u, function.returns.size()) | ranges::views::transform([&](size_t _i) {
				return TemporarySlot{_call, _i};
			}) | ranges::to<Stack>,
			// operation
			CFG::FunctionCall{_call.debugData, function, _call}
		});
	}
}

Stack ControlFlowGraphBuilder::visitAssignmentRightHandSide(Expression const& _expression, size_t _expectedSlotCount)
{
	return std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) -> Stack {
			CFG::Operation const& operation = visitFunctionCall(_call);
			yulAssert(_expectedSlotCount == operation.output.size(), "");
			return operation.output;
		},
		[&](auto const& _identifierOrLiteral) -> Stack {
			yulAssert(_expectedSlotCount == 1, "");
			return {(*this)(_identifierOrLiteral)};
		}
	}, _expression);
}

Scope::Function const& ControlFlowGraphBuilder::lookupFunction(YulString _name) const
{
	Scope::Function const* function = nullptr;
	yulAssert(m_scope->lookup(_name, util::GenericVisitor{
		[](Scope::Variable&) { yulAssert(false, "Expected function name."); },
		[&](Scope::Function& _function) { function = &_function; }
	}), "Function name not found.");
	yulAssert(function, "");
	return *function;
}

Scope::Variable const& ControlFlowGraphBuilder::lookupVariable(YulString _name) const
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
		return *var;
	};
	yulAssert(false, "External identifier access unimplemented.");
}

std::pair<CFG::BasicBlock*, CFG::BasicBlock*> ControlFlowGraphBuilder::makeConditionalJump(StackSlot _condition)
{
	CFG::BasicBlock& nonZero = m_graph.makeBlock();
	CFG::BasicBlock& zero = m_graph.makeBlock();
	makeConditionalJump(move(_condition), nonZero, zero);
	return {&nonZero, &zero};
}

void ControlFlowGraphBuilder::makeConditionalJump(StackSlot _condition, CFG::BasicBlock& _nonZero, CFG::BasicBlock& _zero)
{
	yulAssert(m_currentBlock, "");
	m_currentBlock->exit = CFG::BasicBlock::ConditionalJump{
		move(_condition),
		&_nonZero,
		&_zero
	};
	_nonZero.entries.emplace_back(m_currentBlock);
	_zero.entries.emplace_back(m_currentBlock);
	m_currentBlock = nullptr;
}

void ControlFlowGraphBuilder::jump(CFG::BasicBlock& _target, bool backwards)
{
	yulAssert(m_currentBlock, "");
	m_currentBlock->exit = CFG::BasicBlock::Jump{&_target, backwards};
	_target.entries.emplace_back(m_currentBlock);
	m_currentBlock = &_target;
}
