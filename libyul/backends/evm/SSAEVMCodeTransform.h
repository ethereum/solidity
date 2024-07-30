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
 * Code generator for translating Yul / inline assembly to EVM.
 */

#pragma once

#include <libyul/AST.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>
#include <libyul/Exceptions.h>
#include <libyul/Scope.h>

#include <optional>
#include <stack>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::yul
{
struct AsmAnalysisInfo;
struct StackLayout;

struct BlockLivenessData {
	std::set<SSACFG::ValueId> liveIn;
	std::set<SSACFG::ValueId> liveOut;
};

template<typename T, typename IdType>
class IdContainer
{
public:
	template<typename... Args>
	IdContainer(size_t _size, Args&&... _args): m_data(_size, T{std::forward<Args>(_args)...}) {}
	IdContainer(IdContainer const&) = default;
	IdContainer(IdContainer&&) = default;
	IdContainer& operator=(IdContainer const&) = default;
	IdContainer& operator=(IdContainer&&) = default;
	T& operator[](IdType _id)
	{
		return m_data.at(_id.value);
	}
	T const& operator[](IdType _id) const
	{
		return m_data.at(_id.value);
	}
	std::vector<T> release()
	{
		return std::move(m_data);
	}
private:
	std::vector<T> m_data;
};

using LivenessData = IdContainer<BlockLivenessData, SSACFG::BlockId>;

class SSAEVMCodeTransform
{
public:
	/// Use named labels for functions 1) Yes and check that the names are unique
	/// 2) For none of the functions 3) for the first function of each name.
	enum class UseNamedLabels { YesAndForceUnique, Never, ForFirstFunctionOfEachName };

	[[nodiscard]] static std::vector<StackTooDeepError> run(
		AbstractAssembly& _assembly,
		AsmAnalysisInfo& _analysisInfo,
		Block const& _block,
		EVMDialect const& _dialect,
		BuiltinContext& _builtinContext,
		UseNamedLabels _useNamedLabelsForFunctions
	);

	/// Generate code for the function call @a _call. Only public for using with std::visit.
	void operator()(CFG::FunctionCall const& _call);
	/// Generate code for the builtin call @a _call. Only public for using with std::visit.
	void operator()(CFG::BuiltinCall const& _call);
	/// Generate code for the assignment @a _assignment. Only public for using with std::visit.
	void operator()(CFG::Assignment const& _assignment);
private:
	SSAEVMCodeTransform(
		AbstractAssembly& _assembly,
		BuiltinContext& _builtinContext,
		UseNamedLabels _useNamedLabelsForFunctions,
		SSACFG const& _ssacfg,
		LivenessData const& _livenessData
	);

	AbstractAssembly::LabelID getFunctionLabel(Scope::Function const& _function);

	void operator()(SSACFG::BlockId _block);

	void operator()(SSACFG::FunctionInfo const& _functionInfo);

	BlockLivenessData const& liveness(SSACFG::BlockId _block) const {
		return m_livenessData[_block];
	}

	struct BlockData {
		bool generated = false;
		std::optional<AbstractAssembly::LabelID> label;
		std::vector<SSACFG::ValueId> stackIn;
		std::vector<SSACFG::ValueId> stackOut;
	};
	BlockData& blockData(SSACFG::BlockId _block) { return m_blockData[_block]; }
	BlockData const& blockData(SSACFG::BlockId _block) const { return m_blockData[_block]; }

	AbstractAssembly& m_assembly;
	BuiltinContext& m_builtinContext;
	SSACFG const& m_cfg;
	LivenessData const& m_livenessData;
	std::vector<StackTooDeepError> m_stackErrors;
	std::map<SSACFG::FunctionInfo const*, AbstractAssembly::LabelID> const m_functionLabels;
	std::vector<SSACFG::ValueId> m_stack;
	IdContainer<BlockData, SSACFG::BlockId> m_blockData;
};

}
