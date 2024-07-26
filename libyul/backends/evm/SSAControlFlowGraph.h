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
#include <libyul/Exceptions.h>
#include <libyul/Scope.h>

#include <libsolutil/Numeric.h>

#include <deque>
#include <functional>
#include <list>
#include <range/v3/view/map.hpp>
#include <vector>

namespace solidity::yul
{

struct SSACFG
{
	explicit SSACFG() {}
	SSACFG(SSACFG const&) = delete;
	SSACFG(SSACFG&&) = delete;
	SSACFG& operator=(SSACFG const&) = delete;
	SSACFG& operator=(SSACFG&&) = delete;
	~SSACFG() = default;

	struct BlockId {
		size_t value = std::numeric_limits<size_t>::max();
		operator bool() const { return value != std::numeric_limits<size_t>::max(); }
		bool operator<(BlockId const& _rhs) const { return value < _rhs.value; }
		bool operator==(BlockId const& _rhs) const { return value == _rhs.value; }
		bool operator!=(BlockId const& _rhs) const { return value != _rhs.value; }
	};
	struct VariableId {
		size_t value = std::numeric_limits<size_t>::max();
		operator bool() const { return value != std::numeric_limits<size_t>::max(); }
		bool operator<(VariableId const& _rhs) const { return value < _rhs.value; }
		bool operator==(VariableId const& _rhs) const { return value == _rhs.value; }
		bool operator!=(VariableId const& _rhs) const { return value != _rhs.value; }
	};
	struct Phi { BlockId block; };
	struct BuiltinCall
	{
		langutil::DebugData::ConstPtr debugData;
		std::reference_wrapper<BuiltinFunction const> builtin;
		std::reference_wrapper<FunctionCall const> call;
	};
	struct Call
	{
		langutil::DebugData::ConstPtr debugData;
		std::reference_wrapper<Scope::Function const> function;
		std::reference_wrapper<FunctionCall const> call;
	};

	struct Operation {
		std::vector<VariableId> outputs{};
		std::variant<Phi, BuiltinCall, Call> kind;
		std::vector<VariableId> inputs{};
	};
	struct BasicBlock
	{
		struct MainExit {};
		struct ConditionalJump
		{
			langutil::DebugData::ConstPtr debugData;
			VariableId condition;
			BlockId nonZero;
			BlockId zero;
		};
		struct Jump
		{
			langutil::DebugData::ConstPtr debugData;
			BlockId target;
		};
		struct JumpTable
		{
			langutil::DebugData::ConstPtr debugData;
			VariableId value;
			std::map<u256, BlockId> cases;
			BlockId defaultCase;

		};
		struct FunctionReturn
		{
			langutil::DebugData::ConstPtr debugData;
			std::vector<VariableId> returnValues;
		};
		struct Terminated {};
		langutil::DebugData::ConstPtr debugData;
		std::set<BlockId> entries;
		std::list<Operation> operations;
		std::variant<MainExit, Jump, ConditionalJump, JumpTable, FunctionReturn, Terminated> exit = MainExit{};
		template<typename Callable>
		void forEachExit(Callable&& _callable)
		{
			if (auto* jump = std::get_if<Jump>(&exit))
				_callable(jump->target);
			else if (auto* conditionalJump = std::get_if<ConditionalJump>(&exit))
			{
				_callable(conditionalJump->nonZero);
				_callable(conditionalJump->zero);
			}
			else if (auto* jumpTable = std::get_if<JumpTable>(&exit))
			{
				for (auto _case: jumpTable->cases | ranges::views::values)
					_callable(_case);
				_callable(jumpTable->defaultCase);
			}
		}
	};

	std::vector<BasicBlock> blocks;

	BlockId makeBlock(langutil::DebugData::ConstPtr _debugData)
	{
		BlockId blockId { blocks.size() };
		blocks.emplace_back(BasicBlock{std::move(_debugData), {}, {}, BasicBlock::Terminated{}});
		return blockId;
	}
	BasicBlock& block(BlockId _id) { return blocks.at(_id.value); }
	BasicBlock const& block(BlockId _id) const { return blocks.at(_id.value); }

	struct ValueInfo
	{
		std::optional<u256> value;
	};
	std::vector<ValueInfo> valueInfos;
	ValueInfo const& valueInfo(SSACFG::VariableId _var) const
	{
			yulAssert(_var.value < valueInfos.size());
			return valueInfos.at(_var.value);
	}
	VariableId newVariable()
	{
		SSACFG::VariableId id { valueInfos.size() };
		valueInfos.emplace_back(ValueInfo{std::nullopt});
		return id;
	}
	std::map<u256, VariableId> literals;
	VariableId newLiteral(u256 _value)
	{
		auto [it, inserted] = literals.emplace(std::make_pair(_value, SSACFG::VariableId{valueInfos.size()}));
		if (inserted)
			valueInfos.emplace_back(ValueInfo{_value});
		else
			yulAssert(_value == it->first && valueInfos.at(it->second.value).value == _value);
		yulAssert(it->second.value < valueInfos.size());
		return it->second;
	}

	std::vector<std::reference_wrapper<Scope::Function const>> functions;
	struct FunctionInfo {
		langutil::DebugData::ConstPtr debugData;
		BlockId entry;
		std::set<BlockId> exits;
		bool canContinue = true;
		std::vector<std::tuple<std::reference_wrapper<Scope::Variable const>, VariableId>> arguments;
		std::vector<std::reference_wrapper<Scope::Variable const>> returns;
	};
	std::map<Scope::Function const*, FunctionInfo> functionInfos;
};

}
