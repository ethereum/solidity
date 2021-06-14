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
#pragma once

#include <libyul/backends/evm/ControlFlowGraph.h>

namespace solidity::yul
{

class ControlFlowGraphBuilder
{
public:
	ControlFlowGraphBuilder(ControlFlowGraphBuilder const&) = delete;
	ControlFlowGraphBuilder& operator=(ControlFlowGraphBuilder const&) = delete;
	static std::unique_ptr<CFG> build(AsmAnalysisInfo& _analysisInfo, Dialect const& _dialect, Block const& _block);

	StackSlot operator()(Expression const& _literal);
	StackSlot operator()(Literal const& _literal);
	StackSlot operator()(Identifier const& _identifier);
	StackSlot operator()(FunctionCall const&);

	void operator()(VariableDeclaration const& _varDecl);
	void operator()(Assignment const& _assignment);
	void operator()(ExpressionStatement const& _statement);

	void operator()(Block const& _block);

	void operator()(If const& _if);
	void operator()(Switch const& _switch);
	void operator()(ForLoop const&);
	void operator()(Break const&);
	void operator()(Continue const&);
	void operator()(Leave const&);
	void operator()(FunctionDefinition const&);

private:
	ControlFlowGraphBuilder(
		CFG& _graph,
		AsmAnalysisInfo& _analysisInfo,
		Dialect const& _dialect,
		std::map<YulString, FunctionDefinition const*> const& _functionDefinitions
	);
	CFG::Operation& visitFunctionCall(FunctionCall const& _call, std::vector<VariableSlot> _outputs);

	std::pair<CFG::BasicBlock*, CFG::BasicBlock*> makeConditionalJump(StackSlot _condition);
	void makeConditionalJump(StackSlot _condition, CFG::BasicBlock& _nonZero, CFG::BasicBlock& _zero);
	void jump(CFG::BasicBlock& _target, bool _backwards = false);
	CFG& m_graph;
	AsmAnalysisInfo& m_info;
	Dialect const& m_dialect;
	std::map<YulString, FunctionDefinition const*> const& m_functionDefinitions;
	CFG::BasicBlock* m_currentBlock = nullptr;
	struct ForLoopInfo
	{
		std::reference_wrapper<CFG::BasicBlock> afterLoop;
		std::reference_wrapper<CFG::BasicBlock> post;
	};
	std::optional<ForLoopInfo> m_forLoopInfo;
	std::optional<CFG::BasicBlock::FunctionReturn> m_currentFunctionExit;
};

}
