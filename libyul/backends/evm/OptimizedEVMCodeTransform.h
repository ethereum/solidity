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

class OptimizedEVMCodeTransform
{
public:
	[[nodiscard]] static std::vector<StackTooDeepError> run(
		AbstractAssembly& _assembly,
		AsmAnalysisInfo& _analysisInfo,
		Block const& _block,
		EVMDialect const& _dialect,
		BuiltinContext& _builtinContext,
		bool _useNamedLabelsForFunctions = false
	);
private:
	OptimizedEVMCodeTransform(
		AbstractAssembly& _assembly,
		BuiltinContext& _builtinContext,
		bool _useNamedLabelsForFunctions,
		CFG const& _dfg,
		StackLayout const& _stackLayout
	);

	AbstractAssembly::LabelID getFunctionLabel(Scope::Function const& _function);

	/// Shuffles m_stack to the desired @a _targetStack while emitting the shuffling code to m_assembly.
	void createStackLayout(Stack _targetStack);

	/// Generate code for the given block @a _block.
	/// Expects the current stack layout m_stack to be a stack layout that is compatible with the
	/// entry layout expected by the block.
	/// Recursively generates code for blocks that are jumped to.
	/// The last emitted assembly instruction is always an unconditional jump or terminating.
	/// Always exits with an empty stack layout.
	void operator()(CFG::BasicBlock const& _block);

	/// Generate code for the given function.
	/// Resets m_stack.
	void operator()(CFG::FunctionInfo const& _functionInfo);
public:
	/// Generate code for the function call @a _call.
	void operator()(CFG::FunctionCall const& _call);
	/// Generate code for the builtin call @a _call.
	void operator()(CFG::BuiltinCall const& _call);
	/// Generate code for the assignment @a _assignment.
	void operator()(CFG::Assignment const& _assignment);

	std::vector<StackTooDeepError> const& stackErrors() const { return m_stackErrors; }

private:
	/// Assert that @a _slot contains the value of @a _expression.
	static void validateSlot(StackSlot const& _slot, Expression const& _expression);
	/// Assert that it is valid to transition from @a _currentStack to @a _desiredStack.
	/// That is @a _currentStack matches each slot in @a _desiredStack that is not a JunkSlot exactly.
	static void assertLayoutCompatibility(Stack const& _currentStack, Stack const& _desiredStack);

	AbstractAssembly& m_assembly;
	BuiltinContext& m_builtinContext;
	bool m_useNamedLabelsForFunctions = true;
	CFG const& m_dfg;
	StackLayout const& m_stackLayout;
	Stack m_stack;
	std::map<yul::FunctionCall const*, AbstractAssembly::LabelID> m_returnLabels;
	std::map<CFG::BasicBlock const*, AbstractAssembly::LabelID> m_blockLabels;
	std::map<CFG::FunctionInfo const*, AbstractAssembly::LabelID> m_functionLabels;
	/// Set of blocks already generated. If any of the contained blocks is ever jumped to, m_blockLabels should
	/// contain a jump label for it.
	std::set<CFG::BasicBlock const*> m_generated;
	CFG::FunctionInfo const* m_currentFunctionInfo = nullptr;
	std::vector<StackTooDeepError> m_stackErrors;
};

}
