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
#include <libyul/ControlFlowSideEffects.h>

namespace solidity::yul
{

class ControlFlowGraphBuilder
{
public:
	ControlFlowGraphBuilder(ControlFlowGraphBuilder const&) = delete;
	ControlFlowGraphBuilder& operator=(ControlFlowGraphBuilder const&) = delete;
	static std::unique_ptr<CFG> build(AsmAnalysisInfo const& _analysisInfo, Dialect const& _dialect, Block const& _block);

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
		AsmAnalysisInfo const& _analysisInfo,
		std::map<FunctionDefinition const*, ControlFlowSideEffects> const& _functionSideEffects,
		Dialect const& _dialect
	);
	void registerFunction(FunctionDefinition const& _function);
	Stack const& visitFunctionCall(FunctionCall const&);
	Stack visitAssignmentRightHandSide(Expression const& _expression, size_t _expectedSlotCount);

	Scope::Function const& lookupFunction(YulString _name) const;
	Scope::Variable const& lookupVariable(YulString _name) const;
	/// Resets m_currentBlock to enforce a subsequent explicit reassignment.
	void makeConditionalJump(
		std::shared_ptr<DebugData const> _debugData,
		StackSlot _condition,
		CFG::BasicBlock& _nonZero,
		CFG::BasicBlock& _zero
	);
	void jump(
		std::shared_ptr<DebugData const> _debugData,
		CFG::BasicBlock& _target,
		bool _backwards = false
	);
	CFG& m_graph;
	AsmAnalysisInfo const& m_info;
	std::map<FunctionDefinition const*, ControlFlowSideEffects> const& m_functionSideEffects;
	Dialect const& m_dialect;
	CFG::BasicBlock* m_currentBlock = nullptr;
	Scope* m_scope = nullptr;
	struct ForLoopInfo
	{
		std::reference_wrapper<CFG::BasicBlock> afterLoop;
		std::reference_wrapper<CFG::BasicBlock> post;
	};
	std::optional<ForLoopInfo> m_forLoopInfo;
	std::optional<CFG::FunctionInfo*> m_currentFunction;
};

}
