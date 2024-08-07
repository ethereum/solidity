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

#include <libyul/backends/evm/SSAControlFlowGraphBuilder.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/ControlFlowSideEffectsCollector.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/cxx20.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include <range/v3/view/drop_last.hpp>

using namespace solidity;
using namespace solidity::yul;

namespace
{
SSACFG::ValueId tryRemoveTrivialPhi(SSACFG& _cfg, SSACFG::ValueId _phi)
{
	// TODO: double-check if this is sane
	auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_cfg.valueInfo(_phi));
	yulAssert(phiInfo);

	SSACFG::ValueId same;
	for (SSACFG::ValueId arg: phiInfo->arguments)
	{
		if (arg == same || arg == _phi)
			continue;
		if (same)
			return _phi;
		same = arg;
	}

	struct Use {
		std::reference_wrapper<SSACFG::Operation> operation;
		size_t inputIndex = std::numeric_limits<size_t>::max();
	};
	std::vector<Use> uses;
	std::vector<bool> visited;
	std::vector<SSACFG::ValueId> phiUses;
	auto isVisited = [&](SSACFG::BlockId _block) -> bool {
		if (_block.value < visited.size())
			return visited.at(_block.value);
		else
			return false;
	};
	auto markVisited = [&](SSACFG::BlockId _block) {
		if (visited.size() <= _block.value)
			visited.resize(_block.value + 1);
		visited[_block.value] = true;
	};
	// does this even need to be recursive? or can uses only be in the same or neighbouring blocks?
	auto findUses = [&](SSACFG::BlockId _blockId, auto _recurse) -> void {
		if (isVisited(_blockId))
			return;
		markVisited(_blockId);
		auto& block = _cfg.block(_blockId);
		for (auto blockPhi: block.phis)
		{
			if (blockPhi == _phi)
				continue;
			auto const* blockPhiInfo = std::get_if<SSACFG::PhiValue>(&_cfg.valueInfo(blockPhi));
			yulAssert(blockPhiInfo);
			bool usedInPhi = false;
			for (auto input: blockPhiInfo->arguments)
			{
				if (input == _phi)
					usedInPhi = true;
			}
			if (usedInPhi)
				phiUses.emplace_back(blockPhi);
		}
		for (auto& op: block.operations)
		{
			// TODO: double check when phiVar occurs in outputs here and what to do about it.
			if (op.outputs.size() == 1 && op.outputs.front() == _phi)
				continue;
			// TODO: check if this always hold - and if so if the assertion is really valuable.
			for (auto& output: op.outputs)
				yulAssert(output != _phi);

			for (auto&& [n, input]: op.inputs | ranges::views::enumerate)
				if (input == _phi)
					uses.emplace_back(Use{std::ref(op), n});
		}
		block.forEachExit([&](SSACFG::BlockId _block) {
			_recurse(_block, _recurse);
		});
	};
	auto phiBlock = phiInfo->block;
	_cfg.block(phiBlock).phis.erase(_phi);
	findUses(phiBlock, findUses);

	if (!same)
	{
		// This will happen for unreachable paths.
		// TODO: check how best to deal with this
		same = _cfg.unreachableValue();
	}

	for (auto& use: uses)
		use.operation.get().inputs.at(use.inputIndex) = same;
	for (auto phiUse: phiUses)
	{
		auto* blockPhiInfo = std::get_if<SSACFG::PhiValue>(&_cfg.valueInfo(phiUse));
		yulAssert(blockPhiInfo);
		for (auto& arg: blockPhiInfo->arguments)
			if (arg == _phi)
				arg = same;
	}

	for (auto phiUse: phiUses)
		tryRemoveTrivialPhi(_cfg, phiUse);

	return same;
}

/// Removes edges to blocks that are not reachable.
void cleanUnreachable(SSACFG& _cfg)
{
	// Determine which blocks are reachable from the entry.
	util::BreadthFirstSearch<SSACFG::BlockId> reachabilityCheck{{SSACFG::BlockId{0}}};
	for (auto const& functionInfo: _cfg.functionInfos | ranges::views::values)
		reachabilityCheck.verticesToTraverse.emplace_back(functionInfo.entry);

	reachabilityCheck.run([&](SSACFG::BlockId _blockId, auto&& _addChild) {
		auto const& block = _cfg.block(_blockId);
		visit(util::GenericVisitor{
			[&](SSACFG::BasicBlock::Jump const& _jump) {
				_addChild(_jump.target);
			},
			[&](SSACFG::BasicBlock::ConditionalJump const& _jump) {
				_addChild(_jump.zero);
				_addChild(_jump.nonZero);
			},
			[](SSACFG::BasicBlock::FunctionReturn const&) {},
			[](SSACFG::BasicBlock::Terminated const&) {},
			[](SSACFG::BasicBlock::MainExit const&) {}
		}, block.exit);
	});

	auto isUnreachableValue = [&](SSACFG::ValueId const& _value) -> bool {
		auto* valueInfo = std::get_if<SSACFG::UnreachableValue>(&_cfg.valueInfo(_value));
		return (valueInfo) ? true : false;
	};

	// Remove all entries from unreachable nodes from the graph.
	for (SSACFG::BlockId blockId: reachabilityCheck.visited)
	{
		auto& block = _cfg.block(blockId);
		std::set<SSACFG::ValueId> maybeTrivialPhi;
		for (auto it = block.entries.begin(); it != block.entries.end();)
			if (reachabilityCheck.visited.count(*it))
				it++;
			else
				it = block.entries.erase(it);
		for (auto phi: block.phis)
			if (auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_cfg.valueInfo(phi)))
				cxx20::erase_if(phiInfo->arguments, [&](SSACFG::ValueId _arg) {
					if (isUnreachableValue(_arg))
					{
						maybeTrivialPhi.insert(phi);
						return true;
					}
					return false;
				});

		// After removing a phi argument, we might end up with a trivial phi that can be removed.
		for (auto phi: maybeTrivialPhi)
			tryRemoveTrivialPhi(_cfg, phi);
	}
}

}

namespace solidity::yul
{

// https://c9x.me/compile/bib/braun13cc.pdf

SSAControlFlowGraphBuilder::SSAControlFlowGraphBuilder(
	SSACFG& _graph,
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

std::unique_ptr<SSACFG> SSAControlFlowGraphBuilder::build(
	AsmAnalysisInfo const& _analysisInfo,
	Dialect const& _dialect,
	Block const& _block
)
{
	auto result = std::make_unique<SSACFG>();

	ControlFlowSideEffectsCollector sideEffects(_dialect, _block);
	SSAControlFlowGraphBuilder builder(*result, _analysisInfo, sideEffects.functionSideEffects(), _dialect);
	builder.m_currentBlock = result->makeBlock(debugDataOf(_block));
	builder.sealBlock(builder.m_currentBlock);
	builder(_block);
	if (!builder.blockInfo(builder.m_currentBlock).sealed)
		builder.sealBlock(builder.m_currentBlock);
	result->block(builder.m_currentBlock).exit = SSACFG::BasicBlock::MainExit{};

	cleanUnreachable(*result);

	return result;
}

void SSAControlFlowGraphBuilder::operator()(ExpressionStatement const& _expressionStatement)
{
	auto const* functionCall = std::get_if<FunctionCall>(&_expressionStatement.expression);
	yulAssert(functionCall);
	auto results = visitFunctionCall(*functionCall);
	yulAssert(results.empty());
}

void SSAControlFlowGraphBuilder::operator()(Assignment const& _assignment)
{
	assign(
		_assignment.variableNames | ranges::views::transform([&](auto& _var) { return std::ref(lookupVariable(_var.name)); }) | ranges::to<std::vector>,
		_assignment.value.get()
	);
}

void SSAControlFlowGraphBuilder::operator()(VariableDeclaration const& _variableDeclaration)
{
	assign(
		_variableDeclaration.variables | ranges::views::transform([&](auto& _var) { return std::ref(lookupVariable(_var.name)); }) | ranges::to<std::vector>,
		_variableDeclaration.value.get()
	);
}

void SSAControlFlowGraphBuilder::operator()(FunctionDefinition const& _functionDefinition)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_functionDefinition.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_functionDefinition.name));
	m_graph.functions.emplace_back(function);

	SSACFG::FunctionInfo& functionInfo = m_graph.functionInfos.at(&function);

	SSAControlFlowGraphBuilder builder{m_graph, m_info, m_functionSideEffects, m_dialect};
	builder.m_currentFunction = &functionInfo;
	builder.m_currentBlock = functionInfo.entry;
	for (auto&& [var, varId]: functionInfo.arguments)
		builder.currentDef(var, functionInfo.entry) = varId;
	builder.sealBlock(functionInfo.entry);
	builder(_functionDefinition.body);
	functionInfo.exits.insert(builder.m_currentBlock);
	builder(Leave{debugDataOf(_functionDefinition)});
}

void SSAControlFlowGraphBuilder::operator()(If const& _if)
{
	auto condition = std::visit(*this, *_if.condition);
	auto ifBranch = m_graph.makeBlock(debugDataOf(_if.body));
	auto afterIf = m_graph.makeBlock(debugDataOf(currentBlock()));
	conditionalJump(
		debugDataOf(_if),
		condition,
		ifBranch,
		afterIf
	);
	sealBlock(ifBranch);
	m_currentBlock = ifBranch;
	(*this)(_if.body);
	jump(debugDataOf(_if.body), afterIf);
	sealBlock(afterIf);
}

void SSAControlFlowGraphBuilder::operator()(Switch const& _switch)
{
	SSACFG::ValueId expression = std::visit(*this, *_switch.expression);

	BuiltinFunction const* equalityBuiltin = m_dialect.equalityFunction({});
	yulAssert(equalityBuiltin, "");
	// Artificially generate:
	// eq(<literal>, <caseVariable>)
	auto makeValueCompare = [&](Case const& _case) {
		SSACFG::ValueId outputValueId = m_graph.newVariable(m_currentBlock);
		YulName caseVariableName("GHOST[" + std::to_string(outputValueId.value) + "]");

		yul::FunctionCall const& ghostCall = m_graph.ghostCalls.emplace_back(yul::FunctionCall{
			debugDataOf(_case),
			yul::Identifier{{}, "eq"_yulname},
			{*_case.value, Identifier{{}, caseVariableName}},
		});
		yulAssert(_case.value);
		currentBlock().operations.emplace_back(SSACFG::Operation{
			{outputValueId},
			SSACFG::BuiltinCall{debugDataOf(_case), *equalityBuiltin, ghostCall},
			{m_graph.newLiteral(_case.value->value.value()), expression},
		});
		return outputValueId;
	};

	auto afterSwitch = m_graph.makeBlock(debugDataOf(currentBlock()));
	yulAssert(!_switch.cases.empty(), "");
	for (auto const& switchCase: _switch.cases | ranges::views::drop_last(1))
	{
		auto caseBranch = m_graph.makeBlock(debugDataOf(switchCase.body));
		auto elseBranch = m_graph.makeBlock(debugDataOf(_switch));
		conditionalJump(debugDataOf(switchCase), makeValueCompare(switchCase), caseBranch, elseBranch);
		sealBlock(caseBranch);
		m_currentBlock = caseBranch;
		(*this)(switchCase.body);
		jump(debugDataOf(switchCase.body), afterSwitch);
		sealBlock(elseBranch);
		m_currentBlock = elseBranch;
	}
	Case const& switchCase = _switch.cases.back();
	if (switchCase.value)
	{
		auto caseBranch = m_graph.makeBlock(debugDataOf(switchCase.body));
		conditionalJump(debugDataOf(switchCase), makeValueCompare(switchCase), caseBranch, afterSwitch);
		sealBlock(caseBranch);
		m_currentBlock = caseBranch;
	}
	(*this)(switchCase.body);
	jump(debugDataOf(switchCase.body), afterSwitch);
	sealBlock(afterSwitch);
}
void SSAControlFlowGraphBuilder::operator()(ForLoop const& _loop)
{
	ScopedSaveAndRestore scopeRestore(m_scope, m_info.scopes.at(&_loop.pre).get());
	(*this)(_loop.pre);
	auto preLoopDebugData = debugDataOf(currentBlock());

	std::optional<bool> constantCondition;
	if (auto const* literalCondition = std::get_if<yul::Literal>(_loop.condition.get()))
		constantCondition = literalCondition->value.value() != 0;

	SSACFG::BlockId loopCondition = m_graph.makeBlock(debugDataOf(*_loop.condition));
	SSACFG::BlockId loopBody = m_graph.makeBlock(debugDataOf(_loop.body));
	SSACFG::BlockId post = m_graph.makeBlock(debugDataOf(_loop.post));
	SSACFG::BlockId afterLoop = m_graph.makeBlock(preLoopDebugData);

	class ForLoopInfoScope {
	public:
		ForLoopInfoScope(std::stack<ForLoopInfo>& _info, SSACFG::BlockId _breakBlock, SSACFG::BlockId _continueBlock): m_info(_info)
		{
			m_info.push(ForLoopInfo{_breakBlock, _continueBlock});
		}
		~ForLoopInfoScope() {
			m_info.pop();
		}
	private:
		std::stack<ForLoopInfo>& m_info;
	} forLoopInfoScope(m_forLoopInfo, afterLoop, post);

	if (constantCondition.has_value())
	{
		if (*constantCondition)
		{
			jump(debugDataOf(*_loop.condition), loopBody);
			(*this)(_loop.body);
			jump(debugDataOf(_loop.body), post);
			sealBlock(post);
			(*this)(_loop.post);
			jump(debugDataOf(_loop.post), loopBody);
			sealBlock(loopBody);
		}
		else
			jump(debugDataOf(*_loop.condition), afterLoop);
	}
	else
	{
		jump(debugDataOf(_loop.pre), loopCondition);
		auto condition = std::visit(*this, *_loop.condition);
		conditionalJump(debugDataOf(*_loop.condition), condition, loopBody, afterLoop);
		sealBlock(loopBody);
		m_currentBlock = loopBody;
		(*this)(_loop.body);
		jump(debugDataOf(_loop.body), post);
		sealBlock(post);
		(*this)(_loop.post);
		jump(debugDataOf(_loop.post), loopCondition);
		sealBlock(loopCondition);
	}

	sealBlock(afterLoop);
	m_currentBlock = afterLoop;
}

void SSAControlFlowGraphBuilder::operator()(Break const& _break)
{
	yulAssert(!m_forLoopInfo.empty());
	auto currentBlockDebugData = debugDataOf(currentBlock());
	jump(debugDataOf(_break), m_forLoopInfo.top().breakBlock);
	m_currentBlock = m_graph.makeBlock(currentBlockDebugData);
	sealBlock(m_currentBlock);
}

void SSAControlFlowGraphBuilder::operator()(Continue const& _continue)
{
	yulAssert(!m_forLoopInfo.empty());
	auto currentBlockDebugData = debugDataOf(currentBlock());
	jump(debugDataOf(_continue), m_forLoopInfo.top().continueBlock);
	m_currentBlock = m_graph.makeBlock(currentBlockDebugData);
	sealBlock(m_currentBlock);
}

void SSAControlFlowGraphBuilder::operator()(Leave const& _leaveStatement)
{
	yulAssert(m_currentFunction);
	auto currentBlockDebugData = debugDataOf(currentBlock());
	currentBlock().exit = SSACFG::BasicBlock::FunctionReturn{
		debugDataOf(_leaveStatement),
		m_currentFunction->returns | ranges::views::transform([&](auto _var) {
			return readVariable(_var, m_currentBlock);
		}) | ranges::to<std::vector>
	};
	m_currentBlock = m_graph.makeBlock(currentBlockDebugData);
	sealBlock(m_currentBlock);
}

void SSAControlFlowGraphBuilder::operator()(Block const& _block)
{
	ScopedSaveAndRestore saveScope(m_scope, m_info.scopes.at(&_block).get());
	for (auto const& statement: _block.statements)
		if (auto const* function = std::get_if<FunctionDefinition>(&statement))
			registerFunction(*function);
	for (auto const& statement: _block.statements)
		std::visit(*this, statement);
}

SSACFG::ValueId SSAControlFlowGraphBuilder::operator()(FunctionCall const& _call)
{
	auto results = visitFunctionCall(_call);
	yulAssert(results.size() == 1);
	return results.front();
}
SSACFG::ValueId SSAControlFlowGraphBuilder::operator()(Identifier const& _identifier)
{
	auto const& var = lookupVariable(_identifier.name);
	return readVariable(var, m_currentBlock);
}
SSACFG::ValueId SSAControlFlowGraphBuilder::operator()(Literal const& _literal)
{
	return m_graph.newLiteral(_literal.value.value());
}

void SSAControlFlowGraphBuilder::assign(std::vector<std::reference_wrapper<Scope::Variable const>> _variables, Expression const* _expression)
{
	auto rhs = [&]() -> std::vector<SSACFG::ValueId> {
		if (auto const* functionCall = std::get_if<FunctionCall>(_expression))
			return visitFunctionCall(*functionCall);
		else if (_expression)
			return {std::visit(*this, *_expression)};
		else
			return {_variables.size(), zero()};
	}();
	yulAssert(rhs.size() == _variables.size());

	for (auto const& [var, value]: ranges::zip_view(_variables, rhs))
		writeVariable(var, m_currentBlock, value);

}

void SSAControlFlowGraphBuilder::registerFunction(FunctionDefinition const& _functionDefinition)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_functionDefinition.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_functionDefinition.name));

	yulAssert(m_info.scopes.at(&_functionDefinition.body), "");
	Scope* virtualFunctionScope = m_info.scopes.at(m_info.virtualBlocks.at(&_functionDefinition).get()).get();
	yulAssert(virtualFunctionScope, "");

	// TODO: think about using another root node before the function body here (in particular as defining block for the
	// arguments below)
	SSACFG::BlockId entryBlock = m_graph.makeBlock(debugDataOf(_functionDefinition.body));

	auto arguments = _functionDefinition.parameters | ranges::views::transform([&](auto const& _param) {
		auto const& var = std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_param.name));
		// Note: cannot use std::make_tuple since it unwraps reference wrappers.
		return std::tuple{std::cref(var), m_graph.newVariable(entryBlock)};
	}) | ranges::to<std::vector>;
	auto returns = _functionDefinition.returnVariables | ranges::views::transform([&](auto const& _param) {
		return std::cref(std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_param.name)));
	}) | ranges::to<std::vector>;
	auto [it, inserted] = m_graph.functionInfos.emplace(std::make_pair(&function, SSACFG::FunctionInfo{
		_functionDefinition.debugData,
		entryBlock,
		{},
		m_functionSideEffects.at(&_functionDefinition).canContinue,
		arguments,
		returns
	}));
	yulAssert(inserted);
}

std::vector<SSACFG::ValueId> SSAControlFlowGraphBuilder::visitFunctionCall(FunctionCall const& _call)
{
	bool canContinue = true;
	SSACFG::Operation operation = [&](){
		if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
		{
			SSACFG::Operation result{{}, SSACFG::BuiltinCall{_call.debugData, *builtin, _call}, {}};
			for (auto&& [idx, arg]: _call.arguments | ranges::views::enumerate | ranges::views::reverse)
				if (!builtin->literalArgument(idx).has_value())
					result.inputs.emplace_back(std::visit(*this, arg));
			for (size_t i = 0; i < builtin->returns.size(); ++i)
				result.outputs.emplace_back(m_graph.newVariable(m_currentBlock));
			canContinue = builtin->controlFlowSideEffects.canContinue;
			return result;
		}
		else
		{
			Scope::Function const& function = lookupFunction(_call.functionName.name);
			SSACFG::Operation result{{}, SSACFG::Call{debugDataOf(_call), function, _call}, {}};
			canContinue = m_graph.functionInfos.at(&function).canContinue;
			for (auto const& arg: _call.arguments | ranges::views::reverse)
				result.inputs.emplace_back(std::visit(*this, arg));
			for (size_t i = 0; i < function.returns.size(); ++i)
				result.outputs.emplace_back(m_graph.newVariable(m_currentBlock));
			return result;
		}
	}();
	auto results = operation.outputs;
	currentBlock().operations.emplace_back(std::move(operation));
	if (!canContinue)
	{
		currentBlock().exit = SSACFG::BasicBlock::Terminated{};
		m_currentBlock = m_graph.makeBlock(debugDataOf(currentBlock()));
		sealBlock(m_currentBlock);
	}
	return results;
}

SSACFG::ValueId SSAControlFlowGraphBuilder::zero()
{
	return m_graph.newLiteral(0u);
}

SSACFG::ValueId SSAControlFlowGraphBuilder::readVariable(Scope::Variable const& _variable, SSACFG::BlockId _block)
{
	auto& def = currentDef(_variable, _block);
	if (def.has_value())
		return *def;
	return readVariableRecursive(_variable, _block);
}

SSACFG::ValueId SSAControlFlowGraphBuilder::readVariableRecursive(Scope::Variable const& _variable, SSACFG::BlockId _block)
{
	auto& block = m_graph.block(_block);
	auto& info = blockInfo(_block);

	if (info.sealed && block.entries.size() == 1)
		return readVariable(_variable, *block.entries.begin());

	SSACFG::ValueId phi = m_graph.newPhi(_block);
	block.phis.insert(phi);
	if (info.sealed)
	{
		writeVariable(_variable, _block, phi);
		phi = addPhiOperands(_variable, phi);
	}
	else
	{
		info.incompletePhis.emplace_back(std::make_tuple(phi, std::ref(_variable)));
	}
	writeVariable(_variable, _block, phi);
	return phi;
}

SSACFG::ValueId SSAControlFlowGraphBuilder::addPhiOperands(Scope::Variable const& _variable, SSACFG::ValueId _phi)
{
	auto* phi = std::get_if<SSACFG::PhiValue>(&m_graph.valueInfo(_phi));
	yulAssert(phi);
	for (auto pred: m_graph.block(phi->block).entries)
		phi->arguments.emplace_back(readVariable(_variable, pred));
	return tryRemoveTrivialPhi(m_graph, _phi);
}

void SSAControlFlowGraphBuilder::writeVariable(Scope::Variable const& _variable, SSACFG::BlockId _block, SSACFG::ValueId _value)
{
	currentDef(_variable, _block) = _value;
}

Scope::Function const& SSAControlFlowGraphBuilder::lookupFunction(YulName _name) const
{
	Scope::Function const* function = nullptr;
	yulAssert(m_scope->lookup(_name, util::GenericVisitor{
		[](Scope::Variable&) { yulAssert(false, "Expected function name."); },
		[&](Scope::Function& _function) { function = &_function; }
	}), "Function name not found.");
	yulAssert(function, "");
	return *function;
}

Scope::Variable const& SSAControlFlowGraphBuilder::lookupVariable(YulName _name) const
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

void SSAControlFlowGraphBuilder::sealBlock(SSACFG::BlockId _block)
{
	auto& info = blockInfo(_block);
	yulAssert(!info.sealed, "Trying to seal already sealed block.");
	for (auto&& [phi, variable] : info.incompletePhis)
		addPhiOperands(variable, phi);
	info.sealed = true;
}


void SSAControlFlowGraphBuilder::conditionalJump(
	langutil::DebugData::ConstPtr _debugData,
	SSACFG::ValueId _condition,
	SSACFG::BlockId _nonZero,
	SSACFG::BlockId _zero
)
{
	currentBlock().exit = SSACFG::BasicBlock::ConditionalJump{
		std::move(_debugData),
		_condition,
		_nonZero,
		_zero
	};
	m_graph.block(_nonZero).entries.insert(m_currentBlock);
	m_graph.block(_zero).entries.insert(m_currentBlock);
	m_currentBlock = {};
}

void SSAControlFlowGraphBuilder::jump(
	langutil::DebugData::ConstPtr _debugData,
	SSACFG::BlockId _target
)
{
	currentBlock().exit = SSACFG::BasicBlock::Jump{std::move(_debugData), _target};
	yulAssert(!blockInfo(_target).sealed);
	m_graph.block(_target).entries.insert(m_currentBlock);
	m_currentBlock = _target;
}

}
