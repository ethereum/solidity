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
 * Control flow graph and stack layout structures used during code generation.
 */

#pragma once

#include <libyul/AST.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Dialect.h>

#include <libsolutil/Common.h>

#include <functional>
#include <list>
#include <vector>

namespace solidity::yul
{

/// The label pushed as return label before a function call, i.e. the label the call is supposed to return to.
struct FunctionCallReturnLabelSlot
{
	std::reference_wrapper<yul::FunctionCall const> call;
	bool operator==(FunctionCallReturnLabelSlot const& _rhs) const { return &call.get() == &_rhs.call.get(); }
	bool operator<(FunctionCallReturnLabelSlot const& _rhs) const { return &call.get() < &_rhs.call.get(); }
	static constexpr bool canBeFreelyGenerated = true;
};
/// The return label of a function while generating the code of the function body.
struct FunctionReturnLabelSlot
{
	bool operator==(FunctionReturnLabelSlot const&) const { return true; }
	bool operator<(FunctionReturnLabelSlot const&) const { return false; }
	static constexpr bool canBeFreelyGenerated = false;
};
/// A slot containing the current value of a particular variable.
struct VariableSlot
{
	YulString variable;
	std::shared_ptr<DebugData const> debugData{};
	bool operator==(VariableSlot const& _rhs) const { return variable == _rhs.variable; }
	bool operator<(VariableSlot const& _rhs) const { return variable < _rhs.variable; }
	static constexpr bool canBeFreelyGenerated = false;
};
/// A slot containing a literal value.
struct LiteralSlot
{
	u256 value;
	std::shared_ptr<DebugData const> debugData{};
	bool operator==(LiteralSlot const& _rhs) const { return value == _rhs.value; }
	bool operator<(LiteralSlot const& _rhs) const { return value < _rhs.value; }
	static constexpr bool canBeFreelyGenerated = true;
};
/// A slot containing an arbitrary value that is always eventually popped and never used.
/// Used to maintain stack balance on control flow joins.
struct JunkSlot
{
	bool operator==(JunkSlot const&) const { return true; }
	bool operator<(JunkSlot const&) const { return false; }
	static constexpr bool canBeFreelyGenerated = true;
};
using StackSlot = std::variant<FunctionCallReturnLabelSlot, FunctionReturnLabelSlot, VariableSlot, LiteralSlot, JunkSlot>;
/// The stack top is usually the last element of the vector.
using Stack = std::vector<StackSlot>;

/// @returns true if @a _slot can be generated on the stack at any time.
inline bool canBeFreelyGenerated(StackSlot const& _slot)
{
	return std::visit([](auto const& _typedSlot) { return std::decay_t<decltype(_typedSlot)>::canBeFreelyGenerated; }, _slot);
}

/// Control flow graph consisting of ``DFG::BasicBlock``s connected by control flow.
struct CFG
{
	explicit CFG() {}
	CFG(CFG const&) = delete;
	CFG(CFG&&) = delete;
	CFG& operator=(CFG const&) = delete;
	CFG& operator=(CFG&&) = delete;

	struct BuiltinCall
	{
		std::shared_ptr<DebugData const> debugData;
		std::reference_wrapper<BuiltinFunction const> builtin;
		std::reference_wrapper<yul::FunctionCall const> functionCall;
		/// Number of proper arguments with a position on the stack, excluding literal arguments.
		size_t arguments = 0;
	};
	struct FunctionCall
	{
		std::shared_ptr<DebugData const> debugData;
		std::reference_wrapper<yul::FunctionCall const> functionCall;
		std::reference_wrapper<FunctionDefinition const> referencedFunction;
	};
	struct Assignment
	{
		std::shared_ptr<DebugData const> debugData;
	};

	struct Operation
	{
		/// Stack slots this operation expects at the top of the stack and consumes.
		Stack input;
		/// Stack slots this operation leaves on the stack as output.
		Stack output;
		std::variant<FunctionCall, BuiltinCall, Assignment> operation;
	};

	struct FunctionInfo;
	/// A basic control flow block containing ``Operation``s acting on the stack.
	/// Maintains a list of entry blocks and a typed exit.
	struct BasicBlock
	{
		std::vector<BasicBlock*> entries;
		std::vector<Operation> operations;
		struct MainExit {};
		struct ConditionalJump
		{
			StackSlot condition;
			BasicBlock* nonZero = nullptr;
			BasicBlock* zero = nullptr;
		};
		struct Jump
		{
			BasicBlock* target = nullptr;
			/// The only backwards jumps are jumps from loop post to loop condition.
			bool backwards = false;
		};
		struct FunctionReturn { CFG::FunctionInfo* info = nullptr; };
		struct Terminated {};
		std::variant<MainExit, Jump, ConditionalJump, FunctionReturn, Terminated> exit = MainExit{};
	};

	struct FunctionInfo
	{
		std::shared_ptr<DebugData const> debugData;
		yul::FunctionDefinition const* function = nullptr;
		BasicBlock* entry = nullptr;
	};

	/// The main entry point, i.e. the start of the outermost Yul block.
	BasicBlock* entry = nullptr;
	/// Subgraphs for functions.
	std::map<YulString, FunctionInfo> functionInfo;
	/// List of functions in order of declaration.
	std::list<YulString> functions;

	/// Container for blocks for explicit ownership.
	std::list<BasicBlock> blocks;
	/// Container for created variables for explicit ownership.
	std::list<yul::VariableDeclaration> ghostVariable;

	BasicBlock& makeBlock()
	{
		return blocks.emplace_back(BasicBlock{});
	}
};

}
