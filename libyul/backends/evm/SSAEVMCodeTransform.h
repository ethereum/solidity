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

#pragma once

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/ControlFlow.h>
#include <libyul/backends/evm/SSACFGLiveness.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Scope.h>

#include <vector>

namespace solidity::yul
{

class SSAEVMCodeTransform
{
public:
	/// Use named labels for functions 1) Yes and check that the names are unique
	/// 2) For none of the functions 3) for the first function of each name.
	enum class UseNamedLabels { YesAndForceUnique, Never, ForFirstFunctionOfEachName };

	static std::vector<StackTooDeepError> run(
		AbstractAssembly& _assembly,
		AsmAnalysisInfo& _analysisInfo,
		Block const& _block,
		EVMDialect const& _dialect,
		BuiltinContext& _builtinContext,
		UseNamedLabels _useNamedLabelsForFunctions
	);

private:
	SSAEVMCodeTransform(
		AbstractAssembly& _assembly,
		BuiltinContext& _builtinContext,
		UseNamedLabels _useNamedLabelsForFunctions,
		SSACFG const& _cfg,
		SSACFGLiveness const& _liveness
	);

	AbstractAssembly::LabelID getFunctionLabel(Scope::Function const& _function) const;

	void operator()(SSACFG::BlockId _block);
	void operator()(SSACFG::Operation const& _operation, std::set<SSACFG::ValueId> const& _liveOut);
	void operator()(Scope::Function const& _function);
	void operator()(SSACFG::Call const& _call);
	void operator()(SSACFG::BuiltinCall const& _call);

	using StackSlot = std::variant<SSACFG::ValueId, AbstractAssembly::LabelID>;
	struct BlockData {
		bool generated = false;
		std::optional<AbstractAssembly::LabelID> label;
		std::optional<std::vector<StackSlot>> stackIn;
		//std::optional<std::vector<StackSlot>> stackOut;
	};
	BlockData& blockData(SSACFG::BlockId _block) { return m_blockData[_block.value]; }
	BlockData const& blockData(SSACFG::BlockId _block) const { return m_blockData[_block.value]; }

	void createStackTop(std::vector<StackSlot> const& _targetTop, std::set<SSACFG::ValueId> const& _liveOut);
	void createExactStack(std::vector<StackSlot> const& _target);

	void pop();
	void swap(size_t _depth);
	// Produces a copy of _slot at the current stack top (leaving the rest of the current stack untouched).
	void bringUpSlot(StackSlot const& _slot);

	std::string stackToString(std::vector<StackSlot> const& _stack);
	std::string stackSlotToString(StackSlot const& _slot) const;

	AbstractAssembly& m_assembly;
	BuiltinContext& m_builtinContext;
	SSACFG const& m_cfg;
	SSACFGLiveness const& m_liveness;
	std::vector<StackTooDeepError> m_stackErrors;
	std::map<Scope::Function const*, AbstractAssembly::LabelID> const m_functionLabels;
	std::vector<StackSlot> m_stack;
	std::vector<BlockData> m_blockData;
	SSACFG::BlockId m_currentBlock;
	std::vector<std::uint8_t> m_generatedBlocks;
};

}
