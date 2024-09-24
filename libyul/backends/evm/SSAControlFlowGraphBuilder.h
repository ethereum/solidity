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
*
* Based on https://doi.org/10.1007/978-3-642-37051-9_6
* Braun, Matthias, et al. "Simple and efficient construction of static single assignment form."
* Compiler Construction: 22nd International Conference, CC 2013,
* ETAPS 2013, Rome, Italy, March 16-24, 2013. Proceedings 22. Springer Berlin Heidelberg, 2013.
*
* We have small deviations in Algorithms 2 and 4, as the paper's presentation leads to trivial phis being spuriously
* removed from not yet sealed blocks via a call to addPhiOperands in Algorithm 4. Instead, we perform the deletion
* of trivial phis only after a block has been sealed, i.e., all block's predecessors are present.
*/
#pragma once

#include <libyul/ControlFlowSideEffectsCollector.h>
#include <libyul/backends/evm/ControlFlow.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>
#include <stack>

namespace solidity::yul
{

class SSAControlFlowGraphBuilder
{
	SSAControlFlowGraphBuilder(
		ControlFlow& _controlFlow,
		SSACFG& _graph,
		AsmAnalysisInfo const& _analysisInfo,
		ControlFlowSideEffectsCollector const& _sideEffects,
		Dialect const& _dialect
	);
public:
	SSAControlFlowGraphBuilder(SSAControlFlowGraphBuilder const&) = delete;
	SSAControlFlowGraphBuilder& operator=(SSAControlFlowGraphBuilder const&) = delete;
	static std::unique_ptr<ControlFlow> build(
		AsmAnalysisInfo const& _analysisInfo,
		Dialect const& _dialect,
		Block const& _block
	);

	void operator()(ExpressionStatement const& _statement);
	void operator()(Assignment const& _assignment);
	void operator()(VariableDeclaration const& _varDecl);

	void operator()(FunctionDefinition const&);
	void operator()(If const& _if);
	void operator()(Switch const& _switch);
	void operator()(ForLoop const&);
	void operator()(Break const&);
	void operator()(Continue const&);
	void operator()(Leave const&);

	void operator()(Block const& _block);

	SSACFG::ValueId operator()(FunctionCall const& _call);
	SSACFG::ValueId operator()(Identifier const& _identifier);
	SSACFG::ValueId operator()(Literal const& _literal);

private:
	void cleanUnreachable();
	SSACFG::ValueId tryRemoveTrivialPhi(SSACFG::ValueId _phi);
	void assign(std::vector<std::reference_wrapper<Scope::Variable const>> _variables, Expression const* _expression);
	std::vector<SSACFG::ValueId> visitFunctionCall(FunctionCall const& _call);
	void registerFunctionDefinition(FunctionDefinition const& _functionDefinition);
	void buildFunctionGraph(Scope::Function const* _function, FunctionDefinition const* _functionDefinition);

	SSACFG::ValueId zero();
	SSACFG::ValueId readVariable(Scope::Variable const& _variable, SSACFG::BlockId _block);
	SSACFG::ValueId readVariableRecursive(Scope::Variable const& _variable, SSACFG::BlockId _block);
	SSACFG::ValueId addPhiOperands(Scope::Variable const& _variable, SSACFG::ValueId _phi);
	void writeVariable(Scope::Variable const& _variable, SSACFG::BlockId _block, SSACFG::ValueId _value);

	ControlFlow& m_controlFlow;
	SSACFG& m_graph;
	AsmAnalysisInfo const& m_info;
	ControlFlowSideEffectsCollector const& m_sideEffects;
	Dialect const& m_dialect;
	std::vector<std::tuple<Scope::Function const*, FunctionDefinition const*>> m_functionDefinitions;
	SSACFG::BlockId m_currentBlock;
	SSACFG::BasicBlock& currentBlock() { return m_graph.block(m_currentBlock); }
	Scope* m_scope = nullptr;
	Scope::Function const& lookupFunction(YulName _name) const;
	Scope::Variable const& lookupVariable(YulName _name) const;

	struct BlockInfo {
		bool sealed = false;
		std::vector<std::tuple<SSACFG::ValueId, std::reference_wrapper<Scope::Variable const>>> incompletePhis;
	};
	std::vector<BlockInfo> m_blockInfo;

	BlockInfo& blockInfo(SSACFG::BlockId _block)
	{
		if (_block.value >= m_blockInfo.size())
			m_blockInfo.resize(_block.value + 1, {});
		return m_blockInfo[_block.value];
	}
	void sealBlock(SSACFG::BlockId _block);

	std::map<
		Scope::Variable const*,
		std::vector<std::optional<SSACFG::ValueId>>
	> m_currentDef;

	struct ForLoopInfo {
		SSACFG::BlockId breakBlock;
		SSACFG::BlockId continueBlock;
	};
	std::stack<ForLoopInfo> m_forLoopInfo;

	std::optional<SSACFG::ValueId>& currentDef(Scope::Variable const& _variable, SSACFG::BlockId _block)
	{
		auto& varDefs = m_currentDef[&_variable];
		if (varDefs.size() <= _block.value)
			varDefs.resize(_block.value + 1);
		return varDefs.at(_block.value);
	}

	void conditionalJump(
		langutil::DebugData::ConstPtr _debugData,
		SSACFG::ValueId _condition,
		SSACFG::BlockId _nonZero,
		SSACFG::BlockId _zero
	);

	void jump(
		langutil::DebugData::ConstPtr _debugData,
		SSACFG::BlockId _target
	);

	void tableJump(
		langutil::DebugData::ConstPtr _debugData,
		SSACFG::ValueId _value,
		std::map<u256, SSACFG::BlockId> _cases,
		SSACFG::BlockId _defaultCase
	);

	FunctionDefinition const* findFunctionDefinition(Scope::Function const* _function) const;
};

}
