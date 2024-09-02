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
#include <libyul/ControlFlowSideEffectsCollector.h>

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

namespace
{
/// Removes edges to blocks that are not reachable.
void cleanUnreachable(CFG& _cfg)
{
	// Determine which blocks are reachable from the entry.
	util::BreadthFirstSearch<CFG::BasicBlock*> reachabilityCheck{{_cfg.entry}};
	for (auto const& functionInfo: _cfg.functionInfo | ranges::views::values)
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
}

/// Sets the ``recursive`` member to ``true`` for all recursive function calls.
void markRecursiveCalls(CFG& _cfg)
{
	std::map<CFG::BasicBlock*, std::vector<CFG::FunctionCall*>> callsPerBlock;
	auto const& findCalls = [&](CFG::BasicBlock* _block)
	{
		if (auto* calls = util::valueOrNullptr(callsPerBlock, _block))
			return *calls;
		std::vector<CFG::FunctionCall*>& calls = callsPerBlock[_block];
		util::BreadthFirstSearch<CFG::BasicBlock*>{{_block}}.run([&](CFG::BasicBlock* _block, auto _addChild) {
			for (auto& operation: _block->operations)
				if (auto* functionCall = std::get_if<CFG::FunctionCall>(&operation.operation))
					calls.emplace_back(functionCall);
			std::visit(util::GenericVisitor{
				[&](CFG::BasicBlock::MainExit const&) {},
				[&](CFG::BasicBlock::Jump const& _jump)
				{
					_addChild(_jump.target);
				},
				[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
				{
					_addChild(_conditionalJump.zero);
					_addChild(_conditionalJump.nonZero);
				},
				[&](CFG::BasicBlock::FunctionReturn const&) {},
				[&](CFG::BasicBlock::Terminated const&) {},
			}, _block->exit);
		});
		return calls;
	};
	for (auto& functionInfo: _cfg.functionInfo | ranges::views::values)
		for (CFG::FunctionCall* call: findCalls(functionInfo.entry))
		{
			util::BreadthFirstSearch<CFG::FunctionCall*> breadthFirstSearch{{call}};
			breadthFirstSearch.run([&](CFG::FunctionCall* _call, auto _addChild) {
				auto& calledFunctionInfo = _cfg.functionInfo.at(&_call->function.get());
				if (&calledFunctionInfo == &functionInfo)
				{
					call->recursive = true;
					breadthFirstSearch.abort();
					return;
				}
				for (CFG::FunctionCall* nestedCall: findCalls(_cfg.functionInfo.at(&_call->function.get()).entry))
					_addChild(nestedCall);
			});
		}
}

/// Marks each cut-vertex in the CFG, i.e. each block that begins a disconnected sub-graph of the CFG.
/// Entering such a block means that control flow will never return to a previously visited block.
void markStartsOfSubGraphs(CFG& _cfg)
{
	std::vector<CFG::BasicBlock*> entries;
	entries.emplace_back(_cfg.entry);
	for (auto&& functionInfo: _cfg.functionInfo | ranges::views::values)
		entries.emplace_back(functionInfo.entry);
	for (auto& entry: entries)
	{
		/**
		 * Detect bridges following Algorithm 1 in https://arxiv.org/pdf/2108.07346.pdf
		 * and mark the bridge targets as starts of sub-graphs.
		 */
		std::set<CFG::BasicBlock*> visited;
		std::map<CFG::BasicBlock*, size_t> disc;
		std::map<CFG::BasicBlock*, size_t> low;
		std::map<CFG::BasicBlock*, CFG::BasicBlock*> parent;
		size_t time = 0;
		auto dfs = [&](CFG::BasicBlock* _u, auto _recurse) -> void {
			visited.insert(_u);
			disc[_u] = low[_u] = time;
			time++;

			std::vector<CFG::BasicBlock*> children = _u->entries;
			visit(util::GenericVisitor{
				[&](CFG::BasicBlock::Jump const& _jump) {
					children.emplace_back(_jump.target);
				},
				[&](CFG::BasicBlock::ConditionalJump const& _jump) {
					children.emplace_back(_jump.zero);
					children.emplace_back(_jump.nonZero);
				},
				[&](CFG::BasicBlock::FunctionReturn const&) {},
				[&](CFG::BasicBlock::Terminated const&) { _u->isStartOfSubGraph = true; },
				[&](CFG::BasicBlock::MainExit const&) { _u->isStartOfSubGraph = true; }
			}, _u->exit);
			yulAssert(!util::contains(children, _u));

			for (CFG::BasicBlock* v: children)
				if (!visited.count(v))
				{
					parent[v] = _u;
					_recurse(v, _recurse);
					low[_u] = std::min(low[_u], low[v]);
					if (low[v] > disc[_u])
					{
						// _u <-> v is a cut edge in the undirected graph
						bool edgeVtoU = util::contains(_u->entries, v);
						bool edgeUtoV = util::contains(v->entries, _u);
						if (edgeVtoU && !edgeUtoV)
							// Cut edge v -> _u
							_u->isStartOfSubGraph = true;
						else if (edgeUtoV && !edgeVtoU)
							// Cut edge _u -> v
							v->isStartOfSubGraph = true;
					}
				}
				else if (v != parent[_u])
					low[_u] = std::min(low[_u], disc[v]);
		};
		dfs(entry, dfs);
	}
}

/// Marks each block that needs to maintain a clean stack. That is each block that has an outgoing
/// path to a function return.
void markNeedsCleanStack(CFG& _cfg)
{
	for (auto& functionInfo: _cfg.functionInfo | ranges::views::values)
		for (CFG::BasicBlock* exit: functionInfo.exits)
			util::BreadthFirstSearch<CFG::BasicBlock*>{{exit}}.run([&](CFG::BasicBlock* _block, auto _addChild) {
				_block->needsCleanStack = true;
				for (CFG::BasicBlock* entry: _block->entries)
					_addChild(entry);
			});
}
}

std::unique_ptr<CFG> ControlFlowGraphBuilder::build(
	AsmAnalysisInfo const& _analysisInfo,
	Dialect const& _dialect,
	Block const& _block
)
{
	auto result = std::make_unique<CFG>();
	result->entry = &result->makeBlock(debugDataOf(_block));

	ControlFlowSideEffectsCollector sideEffects(_dialect, _block);
	ControlFlowGraphBuilder builder(*result, _analysisInfo, sideEffects.functionSideEffects(), _dialect);
	builder.m_currentBlock = result->entry;
	builder(_block);

	cleanUnreachable(*result);
	markRecursiveCalls(*result);
	markStartsOfSubGraphs(*result);
	markNeedsCleanStack(*result);

	// TODO: It might be worthwhile to run some further simplifications on the graph itself here.
	// E.g. if there is a jump to a node that has the jumping node as its only entry, the nodes can be fused, etc.

	return result;
}

ControlFlowGraphBuilder::ControlFlowGraphBuilder(
	CFG& _graph,
	AsmAnalysisInfo const& _analysisInfo,
	std::map<FunctionDefinition const*, ControlFlowSideEffects> const& _functionSideEffects,
	Dialect const& _dialect
):
	m_graph(_graph),
	m_info(_analysisInfo),
	m_functionSideEffects(_functionSideEffects),
	m_dialect(_dialect)
{
}

StackSlot ControlFlowGraphBuilder::operator()(Literal const& _literal)
{
	return LiteralSlot{_literal.value.value(), _literal.debugData};
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
	Stack const& output = visitFunctionCall(_call);
	yulAssert(output.size() == 1, "");
	return output.front();
}

void ControlFlowGraphBuilder::operator()(VariableDeclaration const& _varDecl)
{
	yulAssert(m_currentBlock, "");
	auto declaredVariables = _varDecl.variables | ranges::views::transform([&](NameWithDebugData const& _var) {
		return VariableSlot{lookupVariable(_var.name), _var.debugData};
	}) | ranges::to<std::vector<VariableSlot>>;
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
	}) | ranges::to<std::vector<VariableSlot>>;

	Stack input = visitAssignmentRightHandSide(*_assignment.value, assignedVariables.size());
	yulAssert(m_currentBlock);
	m_currentBlock->operations.emplace_back(CFG::Operation{
		std::move(input),
		// output
		assignedVariables | ranges::to<Stack>,
		// operation
		CFG::Assignment{_assignment.debugData, assignedVariables}
	});
}
void ControlFlowGraphBuilder::operator()(ExpressionStatement const& _exprStmt)
{
	std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) {
			Stack const& output = visitFunctionCall(_call);
			yulAssert(output.empty(), "");
		},
		[&](auto const&) { yulAssert(false, ""); }
	}, _exprStmt.expression);
}

void ControlFlowGraphBuilder::operator()(Block const& _block)
{
	ScopedSaveAndRestore saveScope(m_scope, m_info.scopes.at(&_block).get());
	for (auto const& statement: _block.statements)
		if (auto const* function = std::get_if<FunctionDefinition>(&statement))
			registerFunction(*function);
	for (auto const& statement: _block.statements)
		std::visit(*this, statement);
}

void ControlFlowGraphBuilder::operator()(If const& _if)
{
	auto& ifBranch = m_graph.makeBlock(debugDataOf(_if.body));
	auto& afterIf = m_graph.makeBlock(debugDataOf(*m_currentBlock));
	StackSlot condition = std::visit(*this, *_if.condition);
	makeConditionalJump(debugDataOf(_if), std::move(condition), ifBranch, afterIf);
	m_currentBlock = &ifBranch;
	(*this)(_if.body);
	jump(debugDataOf(_if.body), afterIf);
}

void ControlFlowGraphBuilder::operator()(Switch const& _switch)
{
	yulAssert(m_currentBlock, "");
	langutil::DebugData::ConstPtr preSwitchDebugData = debugDataOf(_switch);

	auto ghostVariableId = m_graph.ghostVariables.size();
	YulName ghostVariableName("GHOST[" + std::to_string(ghostVariableId) + "]");
	auto& ghostVar = m_graph.ghostVariables.emplace_back(Scope::Variable{ghostVariableName});

	// Artificially generate:
	// let <ghostVariable> := <switchExpression>
	VariableSlot ghostVarSlot{ghostVar, debugDataOf(*_switch.expression)};
	StackSlot expression = std::visit(*this, *_switch.expression);
	m_currentBlock->operations.emplace_back(CFG::Operation{
		Stack{std::move(expression)},
		Stack{ghostVarSlot},
		CFG::Assignment{_switch.debugData, {ghostVarSlot}}
	});

	BuiltinFunction const* equalityBuiltin = m_dialect.equalityFunction();
	yulAssert(equalityBuiltin, "");

	// Artificially generate:
	// eq(<literal>, <ghostVariable>)
	auto makeValueCompare = [&](Case const& _case) {
		yul::FunctionCall const& ghostCall = m_graph.ghostCalls.emplace_back(yul::FunctionCall{
			debugDataOf(_case),
			yul::Identifier{{}, "eq"_yulname},
			{*_case.value, Identifier{{}, ghostVariableName}}
		});
		CFG::Operation& operation = m_currentBlock->operations.emplace_back(CFG::Operation{
			Stack{ghostVarSlot, LiteralSlot{_case.value->value.value(), debugDataOf(*_case.value)}},
			Stack{TemporarySlot{ghostCall, 0}},
			CFG::BuiltinCall{debugDataOf(_case), *equalityBuiltin, ghostCall, 2},
		});
		return operation.output.front();
	};
	CFG::BasicBlock& afterSwitch = m_graph.makeBlock(preSwitchDebugData);
	yulAssert(!_switch.cases.empty(), "");
	for (auto const& switchCase: _switch.cases | ranges::views::drop_last(1))
	{
		yulAssert(switchCase.value, "");
		auto& caseBranch = m_graph.makeBlock(debugDataOf(switchCase.body));
		auto& elseBranch = m_graph.makeBlock(debugDataOf(_switch));
		makeConditionalJump(debugDataOf(switchCase), makeValueCompare(switchCase), caseBranch, elseBranch);
		m_currentBlock = &caseBranch;
		(*this)(switchCase.body);
		jump(debugDataOf(switchCase.body), afterSwitch);
		m_currentBlock = &elseBranch;
	}
	Case const& switchCase = _switch.cases.back();
	if (switchCase.value)
	{
		CFG::BasicBlock& caseBranch = m_graph.makeBlock(debugDataOf(switchCase.body));
		makeConditionalJump(debugDataOf(switchCase), makeValueCompare(switchCase), caseBranch, afterSwitch);
		m_currentBlock = &caseBranch;
	}
	(*this)(switchCase.body);
	jump(debugDataOf(switchCase.body), afterSwitch);
}

void ControlFlowGraphBuilder::operator()(ForLoop const& _loop)
{
	langutil::DebugData::ConstPtr preLoopDebugData = debugDataOf(_loop);
	ScopedSaveAndRestore scopeRestore(m_scope, m_info.scopes.at(&_loop.pre).get());
	(*this)(_loop.pre);

	std::optional<bool> constantCondition;
	if (auto const* literalCondition = std::get_if<yul::Literal>(_loop.condition.get()))
		constantCondition = literalCondition->value.value() != 0;

	CFG::BasicBlock& loopCondition = m_graph.makeBlock(debugDataOf(*_loop.condition));
	CFG::BasicBlock& loopBody = m_graph.makeBlock(debugDataOf(_loop.body));
	CFG::BasicBlock& post = m_graph.makeBlock(debugDataOf(_loop.post));
	CFG::BasicBlock& afterLoop = m_graph.makeBlock(preLoopDebugData);

	ScopedSaveAndRestore scopedSaveAndRestore(m_forLoopInfo, ForLoopInfo{afterLoop, post});

	if (constantCondition.has_value())
	{
		if (*constantCondition)
		{
			jump(debugDataOf(_loop.pre), loopBody);
			(*this)(_loop.body);
			jump(debugDataOf(_loop.body), post);
			(*this)(_loop.post);
			jump(debugDataOf(_loop.post), loopBody, true);
		}
		else
			jump(debugDataOf(_loop.pre), afterLoop);
	}
	else
	{
		jump(debugDataOf(_loop.pre), loopCondition);
		StackSlot condition = std::visit(*this, *_loop.condition);
		makeConditionalJump(debugDataOf(*_loop.condition), std::move(condition), loopBody, afterLoop);
		m_currentBlock = &loopBody;
		(*this)(_loop.body);
		jump(debugDataOf(_loop.body), post);
		(*this)(_loop.post);
		jump(debugDataOf(_loop.post), loopCondition, true);
	}

	m_currentBlock = &afterLoop;
}

void ControlFlowGraphBuilder::operator()(Break const& _break)
{
	yulAssert(m_forLoopInfo.has_value(), "");
	jump(debugDataOf(_break), m_forLoopInfo->afterLoop);
	m_currentBlock = &m_graph.makeBlock(debugDataOf(*m_currentBlock));
}

void ControlFlowGraphBuilder::operator()(Continue const& _continue)
{
	yulAssert(m_forLoopInfo.has_value(), "");
	jump(debugDataOf(_continue), m_forLoopInfo->post);
	m_currentBlock = &m_graph.makeBlock(debugDataOf(*m_currentBlock));
}

// '_leave' and '__leave' are reserved in VisualStudio
void ControlFlowGraphBuilder::operator()(Leave const& leave_)
{
	yulAssert(m_currentFunction.has_value(), "");
	m_currentBlock->exit = CFG::BasicBlock::FunctionReturn{debugDataOf(leave_), *m_currentFunction};
	(*m_currentFunction)->exits.emplace_back(m_currentBlock);
	m_currentBlock = &m_graph.makeBlock(debugDataOf(*m_currentBlock));
}

void ControlFlowGraphBuilder::operator()(FunctionDefinition const& _function)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_function.name));
	m_graph.functions.emplace_back(&function);

	CFG::FunctionInfo& functionInfo = m_graph.functionInfo.at(&function);

	ControlFlowGraphBuilder builder{m_graph, m_info, m_functionSideEffects, m_dialect};
	builder.m_currentFunction = &functionInfo;
	builder.m_currentBlock = functionInfo.entry;
	builder(_function.body);
	functionInfo.exits.emplace_back(builder.m_currentBlock);
	builder.m_currentBlock->exit = CFG::BasicBlock::FunctionReturn{debugDataOf(_function), &functionInfo};
}

void ControlFlowGraphBuilder::registerFunction(FunctionDefinition const& _functionDefinition)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_functionDefinition.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_functionDefinition.name));

	yulAssert(m_info.scopes.at(&_functionDefinition.body), "");
	Scope* virtualFunctionScope = m_info.scopes.at(m_info.virtualBlocks.at(&_functionDefinition).get()).get();
	yulAssert(virtualFunctionScope, "");

	bool inserted = m_graph.functionInfo.emplace(std::make_pair(&function, CFG::FunctionInfo{
		_functionDefinition.debugData,
		function,
		_functionDefinition,
		&m_graph.makeBlock(debugDataOf(_functionDefinition.body)),
		_functionDefinition.parameters | ranges::views::transform([&](auto const& _param) {
			return VariableSlot{
				std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_param.name)),
				_param.debugData
			};
		}) | ranges::to<std::vector>,
		_functionDefinition.returnVariables | ranges::views::transform([&](auto const& _retVar) {
			return VariableSlot{
				std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_retVar.name)),
				_retVar.debugData
			};
		}) | ranges::to<std::vector>,
		{},
		m_functionSideEffects.at(&_functionDefinition).canContinue
	})).second;
	yulAssert(inserted);
}

Stack const& ControlFlowGraphBuilder::visitFunctionCall(FunctionCall const& _call)
{
	yulAssert(m_scope, "");
	yulAssert(m_currentBlock, "");

	Stack const* output = nullptr;
	bool canContinue = true;
	if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		Stack inputs;
		for (auto&& [idx, arg]: _call.arguments | ranges::views::enumerate | ranges::views::reverse)
			if (!builtin->literalArgument(idx).has_value())
				inputs.emplace_back(std::visit(*this, arg));
		CFG::BuiltinCall builtinCall{_call.debugData, *builtin, _call, inputs.size()};
		output = &m_currentBlock->operations.emplace_back(CFG::Operation{
			// input
			std::move(inputs),
			// output
			ranges::views::iota(0u, builtin->numReturns) | ranges::views::transform([&](size_t _i) {
				return TemporarySlot{_call, _i};
			}) | ranges::to<Stack>,
			// operation
			std::move(builtinCall)
		}).output;
		canContinue = builtin->controlFlowSideEffects.canContinue;
	}
	else
	{
		Scope::Function const& function = lookupFunction(_call.functionName.name);
		canContinue = m_graph.functionInfo.at(&function).canContinue;
		Stack inputs;
		if (canContinue)
			inputs.emplace_back(FunctionCallReturnLabelSlot{_call});
		for (auto const& arg: _call.arguments | ranges::views::reverse)
			inputs.emplace_back(std::visit(*this, arg));
		output = &m_currentBlock->operations.emplace_back(CFG::Operation{
			// input
			std::move(inputs),
			// output
			ranges::views::iota(0u, function.numReturns) | ranges::views::transform([&](size_t _i) {
				return TemporarySlot{_call, _i};
			}) | ranges::to<Stack>,
			// operation
			CFG::FunctionCall{_call.debugData, function, _call, /* recursive */ false, canContinue}
		}).output;
	}
	if (!canContinue)
	{
		m_currentBlock->exit = CFG::BasicBlock::Terminated{};
		m_currentBlock = &m_graph.makeBlock(debugDataOf(*m_currentBlock));
	}
	return *output;
}

Stack ControlFlowGraphBuilder::visitAssignmentRightHandSide(Expression const& _expression, size_t _expectedSlotCount)
{
	return std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) -> Stack {
			Stack const& output = visitFunctionCall(_call);
			yulAssert(_expectedSlotCount == output.size(), "");
			return output;
		},
		[&](auto const& _identifierOrLiteral) -> Stack {
			yulAssert(_expectedSlotCount == 1, "");
			return {(*this)(_identifierOrLiteral)};
		}
	}, _expression);
}

Scope::Function const& ControlFlowGraphBuilder::lookupFunction(YulName _name) const
{
	Scope::Function const* function = nullptr;
	yulAssert(m_scope->lookup(_name, util::GenericVisitor{
		[](Scope::Variable&) { yulAssert(false, "Expected function name."); },
		[&](Scope::Function& _function) { function = &_function; }
	}), "Function name not found.");
	yulAssert(function, "");
	return *function;
}

Scope::Variable const& ControlFlowGraphBuilder::lookupVariable(YulName _name) const
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

void ControlFlowGraphBuilder::makeConditionalJump(
	langutil::DebugData::ConstPtr _debugData,
	StackSlot _condition,
	CFG::BasicBlock& _nonZero,
	CFG::BasicBlock& _zero
)
{
	yulAssert(m_currentBlock, "");
	m_currentBlock->exit = CFG::BasicBlock::ConditionalJump{
		std::move(_debugData),
		std::move(_condition),
		&_nonZero,
		&_zero
	};
	_nonZero.entries.emplace_back(m_currentBlock);
	_zero.entries.emplace_back(m_currentBlock);
	m_currentBlock = nullptr;
}

void ControlFlowGraphBuilder::jump(
	langutil::DebugData::ConstPtr _debugData,
	CFG::BasicBlock& _target,
	bool backwards
)
{
	yulAssert(m_currentBlock, "");
	m_currentBlock->exit = CFG::BasicBlock::Jump{std::move(_debugData), &_target, backwards};
	_target.entries.emplace_back(m_currentBlock);
	m_currentBlock = &_target;
}
