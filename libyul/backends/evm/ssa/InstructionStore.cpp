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

#include <libyul/backends/evm/ssa/InstructionStore.h>

#ifdef SLOW_DEBUG

#include <libyul/Exceptions.h>

#include <fmt/format.h>

#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

using namespace solidity::yul::ssa;

void InstructionStore::checkValidOperand(InstId const _id, std::string const& _ctx) const
{
	yulAssert(_id.hasValue(), fmt::format("Empty InstId referenced by {}", _ctx));
	yulAssert(_id.value < numInsts(), fmt::format("InstId {} out of bounds (numInsts={}) in {}", _id, numInsts(), _ctx));
	yulAssert(!inst(_id).isTombstone(), fmt::format("Tombstone {} referenced by {}", _id, _ctx));
}

void InstructionStore::checkInvariants() const
{
	checkOpcodeShapes();
	checkLiteralDedup();
	checkValueDependencyAcyclic();
}

void InstructionStore::checkOpcodeShapes() const
{
	auto const nInsts = boost::numeric_cast<InstId::ValueType>(numInsts());
	for (InstId::ValueType v = 0; v < nInsts; ++v)
	{
		InstId const instId{v};
		checkOpcodeShape(instId);
	}
}

void InstructionStore::checkOpcodeShape(InstId const _id) const
{
	Inst const& i = inst(_id);
	if (i.isTombstone())
		return;

	for (InstId const in: i.inputs)
		checkValidOperand(in, fmt::format("inputs of {}", _id));

	switch (i.opcode)
	{
	case InstOpcode::Const:
		yulAssert(i.inputs.empty(), fmt::format("Const {} has inputs", _id));
		yulAssert(i.payload && std::holds_alternative<LiteralPayload>(*i.payload), fmt::format("Const {} lacks a literal payload", _id));
		break;
	case InstOpcode::Phi:
		yulAssert(i.inputs.empty(), fmt::format("Phi {} has inputs", _id));
		yulAssert(!i.payload, fmt::format("Phi {} has a payload", _id));
		break;
	case InstOpcode::Upsilon:
	{
		yulAssert(i.inputs.size() == 1, fmt::format("Upsilon {} needs exactly one input", _id));
		yulAssert(i.payload && std::holds_alternative<UpsilonPayload>(*i.payload), fmt::format("Upsilon {} lacks an upsilon payload", _id));
		InstId const phi = upsilonPhi(_id);
		checkValidOperand(phi, fmt::format("target phi of upsilon {}", _id));
		yulAssert(inst(phi).isPhi(), fmt::format("Upsilon {} targets non-phi {}", _id, phi));
		break;
	}
	case InstOpcode::BuiltinCall:
		yulAssert(i.payload && std::holds_alternative<BuiltinCall>(*i.payload), fmt::format("BuiltinCall {} lacks a builtin payload", _id));
		break;
	case InstOpcode::Call:
		yulAssert(i.payload && std::holds_alternative<Call>(*i.payload), fmt::format("Call {} lacks a call payload", _id));
		break;
	case InstOpcode::Unreachable:
		yulAssert(i.inputs.empty(), fmt::format("Unreachable {} has inputs", _id));
		yulAssert(!i.payload, fmt::format("Unreachable {} has a payload", _id));
		break;
	case InstOpcode::FunctionArg:
		yulAssert(i.inputs.empty(), fmt::format("FunctionArg {} has inputs", _id));
		yulAssert(!i.payload, fmt::format("FunctionArg {} has a payload", _id));
		break;
	case InstOpcode::Projection:
	{
		yulAssert(i.inputs.size() == 1, fmt::format("Projection {} needs exactly one input", _id));
		yulAssert(!i.payload, fmt::format("Projection {} has a payload", _id));
		InstId const producer = i.inputs.front();
		checkValidOperand(producer, fmt::format("producer of projection {}", _id));
		yulAssert(producer.value < _id.value, fmt::format("Projection {} precedes its producer {}", _id, producer));
		yulAssert(inst(producer).canHaveProjections(), fmt::format("Producer {} of projection {} is not an operation", producer, _id));
		NumReturnsSizeType const trailing = numTrailingProjections(producer);
		yulAssert(trailing >= 2, fmt::format("Projection {} on single-return producer {}", _id, producer));
		yulAssert(projectionIndex(_id) < trailing, fmt::format("Projection index of {} out of range", _id));
		break;
	}
	case InstOpcode::Identity:
		yulAssert(i.inputs.size() == 1, fmt::format("Identity {} needs exactly one input", _id));
		yulAssert(i.inputs.front() != _id, fmt::format("Identity {} is a self-loop", _id));
		yulAssert(!i.payload, fmt::format("Identity {} has a payload", _id));
		break;
	case InstOpcode::Nop:
		yulAssert(i.inputs.empty(), fmt::format("Nop {} has inputs", _id));
		yulAssert(!i.payload, fmt::format("Nop {} has a payload", _id));
		break;
	case InstOpcode::MemoryGuard:
		yulAssert(i.inputs.empty(), fmt::format("MemoryGuard {} has inputs", _id));
		yulAssert(!i.payload, fmt::format("MemoryGuard {} has a payload", _id));
		break;
	case InstOpcode::Tombstone:
		yulAssert(false, fmt::format("Tombstone {} reached opcode shape check", _id));
		break;
	}
}

void InstructionStore::checkLiteralDedup() const
{
	// Every map entry must be valid
	for (auto const& [value, id]: m_literalDedup)
	{
		checkValidOperand(id, "literal dedup table");
		yulAssert(inst(id).isLiteral(), fmt::format("Dedup entry {} is not a Const", id));
		yulAssert(literalPayload(id) == value, fmt::format("Dedup entry {} carries a value differing from its key", id));
	}

	// Every const is registered & canonical
	auto const nInsts = boost::numeric_cast<InstId::ValueType>(numInsts());
	for (InstId::ValueType v = 0; v < nInsts; ++v)
	{
		InstId const instId{v};
		if (inst(instId).opcode != InstOpcode::Const)
			continue;
		auto const it = m_literalDedup.find(literalPayload(instId));
		yulAssert(it != m_literalDedup.end(), fmt::format("Const {} missing from dedup table", instId));
		yulAssert(it->second == instId, fmt::format("Const {} is not the canonical dedup slot for its value", instId));
	}
}

void InstructionStore::checkValueDependencyAcyclic() const
{
	auto const nInsts = boost::numeric_cast<InstId::ValueType>(numInsts());
	enum Color : std::uint8_t { White, Gray, Black };
	// White    Not yet visited.
	// Gray     On the current DFS path
	// Black    Fully finished
	//
	std::vector<Color> color(nInsts, White);
	std::vector<std::pair<InstId::ValueType, std::size_t>> stack;
	for (InstId::ValueType v = 0; v < nInsts; ++v)
	{
		InstId const rootId{v};
		if (inst(rootId).isTombstone() || color[rootId.value] != White)
			continue;
		color[rootId.value] = Gray;
		stack.emplace_back(rootId.value, 0);
		while (!stack.empty())
		{
			auto const [node, idx] = stack.back();
			Inst const& ni = inst(InstId{node});
			if (idx < ni.inputs.size())
			{
				stack.back().second = idx + 1;
				InstId const child = ni.inputs[idx];
				yulAssert(color[child.value] != Gray, fmt::format("Value-dependency cycle at {} -> {}", InstId{node}, child));
				if (color[child.value] == White)
				{
					color[child.value] = Gray;
					stack.emplace_back(child.value, 0);
				}
			}
			else
			{
				color[node] = Black;
				stack.pop_back();
			}
		}
	}
}

#endif
