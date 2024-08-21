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

#include <range/v3/view/map.hpp>
#include <deque>
#include <functional>
#include <list>
#include <vector>

namespace solidity::yul
{

class SSACFG
{
public:
	SSACFG() = default;
	SSACFG(SSACFG const&) = delete;
	SSACFG(SSACFG&&) = delete;
	SSACFG& operator=(SSACFG const&) = delete;
	SSACFG& operator=(SSACFG&&) = delete;
	~SSACFG() = default;

	struct BlockId
	{
		size_t value = std::numeric_limits<size_t>::max();
		bool operator<(BlockId const& _rhs) const { return value < _rhs.value; }
		bool operator==(BlockId const& _rhs) const { return value == _rhs.value; }
		bool operator!=(BlockId const& _rhs) const { return value != _rhs.value; }
	};
	struct ValueId
	{
		size_t value = std::numeric_limits<size_t>::max();
		bool hasValue() const { return value != std::numeric_limits<size_t>::max(); }
		bool operator<(ValueId const& _rhs) const { return value < _rhs.value; }
		bool operator==(ValueId const& _rhs) const { return value == _rhs.value; }
		bool operator!=(ValueId const& _rhs) const { return value != _rhs.value; }
	};

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
		bool const canContinue = true;
	};

	struct Operation {
		std::vector<ValueId> outputs{};
		std::variant<BuiltinCall, Call> kind;
		std::vector<ValueId> inputs{};
	};
	struct BasicBlock
	{
		struct MainExit {};
		struct ConditionalJump
		{
			langutil::DebugData::ConstPtr debugData;
			ValueId condition;
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
			ValueId value;
			std::map<u256, BlockId> cases;
			BlockId defaultCase;
		};
		struct FunctionReturn
		{
			langutil::DebugData::ConstPtr debugData;
			std::vector<ValueId> returnValues;
		};
		struct Terminated {};
		langutil::DebugData::ConstPtr debugData;
		std::set<BlockId> entries;
		std::set<ValueId> phis;
		std::vector<Operation> operations;
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
	BlockId makeBlock(langutil::DebugData::ConstPtr _debugData)
	{
		BlockId blockId { m_blocks.size() };
		m_blocks.emplace_back(BasicBlock{std::move(_debugData), {}, {}, {}, BasicBlock::Terminated{}});
		return blockId;
	}
	BasicBlock& block(BlockId _id) { return m_blocks.at(_id.value); }
	BasicBlock const& block(BlockId _id) const { return m_blocks.at(_id.value); }
	size_t numBlocks() const { return m_blocks.size(); }
private:
	std::vector<BasicBlock> m_blocks;
public:
	struct LiteralValue {
		langutil::DebugData::ConstPtr debugData;
		u256 value;
	};
	struct VariableValue {
		langutil::DebugData::ConstPtr debugData;
		BlockId definingBlock;
	};
	struct PhiValue {
		langutil::DebugData::ConstPtr debugData;
		BlockId block;
		std::vector<ValueId> arguments;
	};
	struct UnreachableValue {};
	using ValueInfo = std::variant<UnreachableValue, VariableValue, LiteralValue, PhiValue>;
	ValueInfo& valueInfo(ValueId const _var)
	{
		return m_valueInfos.at(_var.value);
	}
	ValueInfo const& valueInfo(ValueId const _var) const
	{
		return m_valueInfos.at(_var.value);
	}
	ValueId newPhi(BlockId const _definingBlock)
	{
		ValueId id { m_valueInfos.size() };
		auto block = m_blocks.at(_definingBlock.value);
		m_valueInfos.emplace_back(PhiValue{debugDataOf(block), _definingBlock, {}});
		return id;
	}
	ValueId newVariable(BlockId const _definingBlock)
	{
		ValueId id { m_valueInfos.size() };
		auto block = m_blocks.at(_definingBlock.value);
		m_valueInfos.emplace_back(VariableValue{debugDataOf(block), _definingBlock});
		return id;
	}
	ValueId unreachableValue()
	{
		if (!m_unreachableValue)
		{
			m_unreachableValue = ValueId { m_valueInfos.size() };
			m_valueInfos.emplace_back(UnreachableValue{});
		}
		return *m_unreachableValue;
	}
	ValueId newLiteral(langutil::DebugData::ConstPtr _debugData, u256 _value)
	{
		auto [it, inserted] = m_literals.emplace(_value, ValueId{m_valueInfos.size()});
		if (inserted)
			m_valueInfos.emplace_back(LiteralValue{std::move(_debugData), _value});
		else
		{
			yulAssert(_value == it->first);
			yulAssert(std::holds_alternative<LiteralValue>(m_valueInfos.at(it->second.value)));
			yulAssert(std::get<LiteralValue>(m_valueInfos.at(it->second.value)).value == _value);
		}
		yulAssert(it->second.value < m_valueInfos.size());
		return it->second;
	}
private:
	std::deque<ValueInfo> m_valueInfos;
	std::map<u256, ValueId> m_literals;
	std::optional<ValueId> m_unreachableValue;
public:
	langutil::DebugData::ConstPtr debugData;
	BlockId entry = BlockId{0};
	std::set<BlockId> exits;
	Scope::Function const* function = nullptr;
	bool canContinue = true;
	std::vector<std::tuple<std::reference_wrapper<Scope::Variable const>, ValueId>> arguments;
	std::vector<std::reference_wrapper<Scope::Variable const>> returns;
	std::vector<std::reference_wrapper<Scope::Function const>> functions;
	// Container for artificial calls generated for switch statements.
	std::list<FunctionCall> ghostCalls;
};

}
