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

#include <libyul/optimiser/FunctionDefinitionCollector.h>

#include <libsolutil/cxx20.h>
#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <range/v3/action/push_back.hpp>
#include <range/v3/action/erase.hpp>
#include <range/v3/algorithm/remove.hpp>
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

namespace
{
template<typename Block, typename Callable>
void forEachExit(Block&& _block, Callable _callable)
{
	std::visit(util::GenericVisitor{
		[&](CFG::BasicBlock::Jump& _jump) {
			_callable(_jump.target);
		},
		[&](CFG::BasicBlock::ConditionalJump& _jump) {
			_callable(_jump.zero);
			_callable(_jump.nonZero);
		},
		[](CFG::BasicBlock::FunctionReturn&) {},
		[](CFG::BasicBlock::Terminated&) {},
		[](CFG::BasicBlock::MainExit&) {}
	}, _block.exit);
}
}

std::unique_ptr<CFG> ControlFlowGraphBuilder::build(
	AsmAnalysisInfo& _analysisInfo,
	Dialect const& _dialect,
	Block const& _block
)
{
	auto result = std::make_unique<CFG>();
	result->entry = &result->makeBlock();

	auto functionDefinitions = FunctionDefinitionCollector::run(_block);

	ControlFlowGraphBuilder builder(*result, _analysisInfo, _dialect, functionDefinitions);
	builder.m_currentBlock = result->entry;
	builder(_block);

	list<CFG::BasicBlock*> allEntries{result->entry};
	ranges::actions::push_back(
		allEntries,
		result->functionInfo | ranges::views::values | ranges::views::transform(
			[](auto&& _function) { return _function.entry; }
		)
	);
	{
		// Determine which blocks are reachable from the entry.
		util::BreadthFirstSearch<CFG::BasicBlock*> reachabilityCheck{allEntries};
		reachabilityCheck.run([&](CFG::BasicBlock* _node, auto&& _addChild) {
			visit(util::GenericVisitor{
				[&](CFG::BasicBlock::Jump& _jump) {
					_addChild(_jump.target);
				},
				[&](CFG::BasicBlock::ConditionalJump& _jump) {
					_addChild(_jump.zero);
					_addChild(_jump.nonZero);
				},
				[](CFG::BasicBlock::FunctionReturn&) {},
				[](CFG::BasicBlock::Terminated&) {},
				[](CFG::BasicBlock::MainExit&) {}
			}, _node->exit);
		});

		// Remove all entries from unreachable nodes from the graph.
		for (auto* node: reachabilityCheck.visited)
			cxx20::erase_if(node->entries, [&](CFG::BasicBlock* entry) -> bool {
				return !reachabilityCheck.visited.count(entry);
			});
	}
	{
		// Merge any block with only one entry forward-jumping to it with its entry.
		util::BreadthFirstSearch<CFG::BasicBlock*> reachabilityCheck{allEntries};
		reachabilityCheck.run([&](CFG::BasicBlock* _node, auto&& _addChild) {
			if (CFG::BasicBlock* _entry = (_node->entries.size() == 1) ? _node->entries.front() : nullptr)
				if (CFG::BasicBlock::Jump* jump = get_if<CFG::BasicBlock::Jump>(&_entry->exit))
					if (!jump->backwards)
					{
						_entry->operations += std::move(_node->operations);
						_node->operations.clear();
						_entry->exit = _node->exit;
						forEachExit(*_node, [&](CFG::BasicBlock* _exit) {
							ranges::remove(_exit->entries, _node);
							_exit->entries.emplace_back(_entry);
						});
					}
			visit(util::GenericVisitor{
				[&](CFG::BasicBlock::Jump& _jump) {
					_addChild(_jump.target);
				},
				[&](CFG::BasicBlock::ConditionalJump& _jump) {
					_addChild(_jump.zero);
					_addChild(_jump.nonZero);
				},
				[](CFG::BasicBlock::FunctionReturn&) {},
				[](CFG::BasicBlock::Terminated&) {},
				[](CFG::BasicBlock::MainExit&) {}
			}, _node->exit);
		});
	}


	// TODO: It might be worthwhile to run some further simplifications on the graph itself here.
	// E.g. if there is a jump to a node that has the jumping node as its only entry, the nodes can be fused, etc.

	return result;
}

ControlFlowGraphBuilder::ControlFlowGraphBuilder(
	CFG& _graph,
	AsmAnalysisInfo& _analysisInfo,
	Dialect const& _dialect,
	map<YulString, FunctionDefinition const*> const& _functionDefinitions
):
m_graph(_graph),
m_info(_analysisInfo),
m_dialect(_dialect),
m_functionDefinitions(_functionDefinitions)
{
}

StackSlot ControlFlowGraphBuilder::operator()(Literal const& _literal)
{
	return LiteralSlot{valueOfLiteral(_literal), _literal.debugData};
}

StackSlot ControlFlowGraphBuilder::operator()(Identifier const& _identifier)
{
	return VariableSlot{_identifier.name, _identifier.debugData};
}

StackSlot ControlFlowGraphBuilder::operator()(Expression const& _expression)
{
	return std::visit(*this, _expression);
}

CFG::Operation& ControlFlowGraphBuilder::visitFunctionCall(FunctionCall const& _call, vector<VariableSlot> _outputs)
{
	yulAssert(m_currentBlock, "");

	if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		CFG::Operation& operation = m_currentBlock->operations.emplace_back(CFG::Operation{
			// input
			_call.arguments |
			ranges::views::enumerate |
			ranges::views::reverse |
			ranges::views::filter(util::mapTuple([&](size_t idx, auto const&) {
				return !builtin->literalArgument(idx).has_value();
			})) |
			ranges::views::values |
			ranges::views::transform(std::ref(*this)) |
			ranges::to<Stack>,
			// output
			_outputs | ranges::to<Stack>,
			// operation
			CFG::BuiltinCall{_call.debugData, *builtin, _call}
		});
		std::get<CFG::BuiltinCall>(operation.operation).arguments = operation.input.size();
		return operation;
	}
	else
	{
		yul::FunctionDefinition const* functionDefinition = m_functionDefinitions.at(_call.functionName.name);
		yulAssert(functionDefinition, "");
		return m_currentBlock->operations.emplace_back(CFG::Operation{
			// input
			ranges::concat_view(
				ranges::views::single(StackSlot{FunctionCallReturnLabelSlot{_call}}),
				_call.arguments | ranges::views::reverse | ranges::views::transform(std::ref(*this))
			) | ranges::to<Stack>,
			// output
			_outputs | ranges::to<Stack>,
			// operation
			CFG::FunctionCall{_call.debugData, _call, *functionDefinition}
		});
	}
}

StackSlot ControlFlowGraphBuilder::operator()(FunctionCall const&)
{
	yulAssert(false, "Expected split expressions.");
	return JunkSlot{};
}

void ControlFlowGraphBuilder::operator()(VariableDeclaration const& _varDecl)
{
	yulAssert(m_currentBlock, "");
	auto declaredVariables = _varDecl.variables | ranges::views::transform([&](TypedName const& _var) {
		return VariableSlot{_var.name, _var.debugData};
	}) | ranges::to<vector>;
	Stack input;
	if (_varDecl.value)
		if (FunctionCall const* call = get_if<FunctionCall>(_varDecl.value.get()))
		{
			visitFunctionCall(*call, std::move(declaredVariables));
			return;
		}
		else
			input = {std::visit(std::ref(*this), *_varDecl.value)};
	else
		input = ranges::views::iota(0u, _varDecl.variables.size()) | ranges::views::transform([&](size_t) {
			return LiteralSlot{0, _varDecl.debugData};
		}) | ranges::to<Stack>;
	m_currentBlock->operations.emplace_back(CFG::Operation{
		std::move(input),
		declaredVariables | ranges::to<Stack>,
		CFG::Assignment{_varDecl.debugData}
	});
}
void ControlFlowGraphBuilder::operator()(Assignment const& _assignment)
{
	vector<VariableSlot> assignedVariables = _assignment.variableNames | ranges::views::transform([&](Identifier const& _var) {
		return VariableSlot{_var.name, _var.debugData};
	}) | ranges::to<vector<VariableSlot>>;

	yulAssert(m_currentBlock, "");
	if (FunctionCall const* call = get_if<FunctionCall>(_assignment.value.get()))
	{
		visitFunctionCall(*call, std::move(assignedVariables));
		return;
	}
	else

	m_currentBlock->operations.emplace_back(CFG::Operation{
		// input
		{std::visit(std::ref(*this), *_assignment.value)},
		// output
		assignedVariables | ranges::to<Stack>,
		// operation
		CFG::Assignment{_assignment.debugData}
	});
}
void ControlFlowGraphBuilder::operator()(ExpressionStatement const& _exprStmt)
{
	yulAssert(m_currentBlock, "");
	std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) {
			CFG::Operation& operation = visitFunctionCall(_call, {});
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
	for (auto const& statement: _block.statements)
		std::visit(*this, statement);
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
	StackSlot switchExpression = std::visit(std::ref(*this), *_switch.expression);

	BuiltinFunction const* equalityBuiltin = m_dialect.equalityFunction({});
	yulAssert(equalityBuiltin, "");

	// Artificially generate:
	// eq(<literal>, <ghostVariable>)
	auto makeValueCompare = [&](Literal const& _value) {
		size_t ghostVariableId = m_graph.ghostVariable.size();
		// TODO: properly use NameDispenser to generate names for these.
		YulString ghostVarName = YulString{"GHOST[" + to_string(ghostVariableId) + "]"};
		VariableDeclaration& ghostDecl = m_graph.ghostVariable.emplace_back(VariableDeclaration{
			_value.debugData,
			{TypedName{_value.debugData, ghostVarName, {}}},
			std::make_unique<Expression>(yul::FunctionCall{
				_value.debugData,
				yul::Identifier{{}, "eq"_yulstring},
				{_value, *_switch.expression}
			})
		});

		CFG::Operation& operation = m_currentBlock->operations.emplace_back(CFG::Operation{
			Stack{switchExpression, LiteralSlot{valueOfLiteral(_value), _value.debugData}},
			Stack{VariableSlot{ghostVarName, _value.debugData}},
			CFG::BuiltinCall{_switch.debugData, *equalityBuiltin, get<yul::FunctionCall>(*ghostDecl.value.get()), 2},
		});
		return operation.output.front();
	};
	CFG::BasicBlock& afterSwitch = m_graph.makeBlock();
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
		(*this)(switchCase.body);
	}
	else
		(*this)(switchCase.body);
	jump(afterSwitch);
}

void ControlFlowGraphBuilder::operator()(ForLoop const& _loop)
{
	(*this)(_loop.pre);

	std::optional<bool> constantCondition;
	if (auto const* literalCondition = get_if<yul::Literal>(_loop.condition.get()))
	{
		if (valueOfLiteral(*literalCondition) == 0)
			constantCondition = false;
		else
			constantCondition = true;
	}

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
	m_graph.functions.emplace_back(_function.name);
	auto&& [it, inserted] = m_graph.functionInfo.emplace(std::make_pair(_function.name, CFG::FunctionInfo{
		_function.debugData,
		&_function,
		&m_graph.makeBlock()
	}));
	yulAssert(inserted, "");
	CFG::FunctionInfo& info = it->second;

	ControlFlowGraphBuilder builder{m_graph, m_info, m_dialect, m_functionDefinitions};
	builder.m_currentFunctionExit = CFG::BasicBlock::FunctionReturn{&info};
	builder.m_currentBlock = info.entry;
	builder(_function.body);
	builder.m_currentBlock->exit = CFG::BasicBlock::FunctionReturn{&info};
}
