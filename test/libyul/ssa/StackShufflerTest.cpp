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

#include <test/libyul/ssa/StackShufflerTest.h>

#include <libyul/backends/evm/ssa/spill/SpillSet.h>
#include <libyul/backends/evm/ssa/stack/Shuffler.h>

#include <libyul/backends/evm/ssa/InstructionStore.h>
#include <libyul/backends/evm/ssa/SSACFG.h>
#include <libyul/backends/evm/ssa/ShuffleTrace.h>
#include <libyul/backends/evm/ssa/Stack.h>
#include <libyul/backends/evm/ssa/StackUtils.h>

#include <range/v3/view/split.hpp>

#include <fmt/ranges.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::ssa;
using namespace solidity::yul::test;
using namespace solidity::yul::test::ssa;

namespace
{
std::string_view constexpr parserKeyInitialStack {"initial"};
std::string_view constexpr parserKeyStackTop {"targetStackTop"};
std::string_view constexpr parserKeyAllowSpilling {"allowSpilling"};
std::string_view constexpr parserKeyInitialSpilled {"initialSpilledSet"};
std::string_view constexpr parserKeyReachableDepth {"reachableStackDepth"};

using Slot = StackSlot;

struct ParsedIdentifierTable
{
	InstructionStore store;
	std::map<std::string, InstId> tokenToId;
	std::map<InstId, std::string> idToToken;

	std::string render(StackSlot const& _slot) const
	{
		if (_slot.isValue())
			if (auto const it = idToToken.find(_slot.value()); it != idToToken.end())
				return it->second;
		return slotToString(_slot);
	}
};

/// Renders a recorded operation like the corresponding EVM instruction, with slots rendered through the
/// identifier table. A spill reload renders as a plain PUSH, matching the assembly's single push-like materialization of a slot.
std::string render(ParsedIdentifierTable const& _table, ShuffleOp const& _op)
{
	switch (_op.kind)
	{
	case ShuffleOp::Kind::Swap:
		return fmt::format("SWAP{}", _op.depth);
	case ShuffleOp::Kind::Dup:
		return fmt::format("DUP{}", _op.depth);
	case ShuffleOp::Kind::Pop:
		return "POP";
	case ShuffleOp::Kind::Push:
	case ShuffleOp::Kind::Load:
		return fmt::format("PUSH {}", _table.render(_op.slot));
	case ShuffleOp::Kind::Store:
		return fmt::format("STORE {}", _table.render(_op.slot));
	}
	solidity::util::unreachable();
}

/// removes leading and trailing whitespace from a string view
std::string_view trim(std::string_view s)
{
	s.remove_prefix(std::min(s.find_first_not_of(" \t\r\v\n"), s.size()));
	s.remove_suffix(std::min(s.size() - s.find_last_not_of(" \t\r\v\n") - 1, s.size()));
	return s;
}

/// Parse a value ID token like "v172", "phi109", "lit7", or "JUNK".
Slot parseSlot(ParsedIdentifierTable& _table, std::string_view _token)
{
	if (_token == "JUNK")
		return Slot::makeJunk();

	static constexpr std::string_view returnLabelPrefix = "ReturnLabel[";
	if (_token.starts_with(returnLabelPrefix) && _token.ends_with(']'))
	{
		auto const inner = _token.substr(returnLabelPrefix.size(), _token.size() - returnLabelPrefix.size() - 1);
		if (auto const num = solidity::util::parseArithmetic<ControlFlowGraphs::FunctionGraphID>(inner))
			return Slot::makeFunctionReturnLabel(*num);
		throw std::runtime_error(fmt::format("Couldn't parse ReturnLabel token: {}", _token));
	}

	static constexpr std::string_view callReturnLabelPrefix = "FunctionCallReturnLabel[";
	if (_token.starts_with(callReturnLabelPrefix) && _token.ends_with(']'))
	{
		auto const inner = _token.substr(callReturnLabelPrefix.size(), _token.size() - callReturnLabelPrefix.size() - 1);
		if (auto const num = solidity::util::parseArithmetic<CallSites::CallSiteID>(inner))
			return Slot::makeFunctionCallReturnLabel(*num);
		throw std::runtime_error(fmt::format("Couldn't parse FunctionCallReturnLabel token: {}", _token));
	}

	auto const allocateInst = [&]() -> InstId
	{
		if (_token.starts_with("phi"))
		{
			if (!solidity::util::parseArithmetic<InstId::ValueType>(_token.substr(3)))
				throw std::runtime_error(fmt::format("Couldn't parse phi token: {}", _token));
			return _table.store.appendPhi({0});
		}
		if (_token.starts_with("lit"))
		{
			if (auto const num = solidity::util::parseArithmetic<InstId::ValueType>(_token.substr(3)))
				return _table.store.appendLiteral({0}, u256(*num)).first;
			throw std::runtime_error(fmt::format("Couldn't parse literal token: {}", _token));
		}
		if (_token.starts_with('v'))
		{
			if (!solidity::util::parseArithmetic<InstId::ValueType>(_token.substr(1)))
				throw std::runtime_error(fmt::format("Couldn't parse variable token: {}", _token));
			return _table.store.appendBuiltinCall({0}, {}, {});
		}
		throw std::runtime_error(fmt::format("Unknown token: {}", _token));
	};

	std::string const tokenStr{_token};
	auto const it = _table.tokenToId.find(tokenStr);
	InstId const id = it != _table.tokenToId.end() ? it->second : allocateInst();
	if (it == _table.tokenToId.end())
	{
		_table.tokenToId.emplace(tokenStr, id);
		_table.idToToken.emplace(id, tokenStr);
	}
	return Slot::makeValue(_table.store, id);
}

/// Parse a string like "[v172, phi109, lit7, JUNK]" into Stack::Data
StackData parseSlots(ParsedIdentifierTable& _table, std::string_view _input, char const brackBegin = '[', char const brackEnd = ']')
{
	StackData result;

	// trim and remove brackets
	{
		_input = trim(_input);
		yulAssert(_input.starts_with(brackBegin));
		_input.remove_prefix(1);
		yulAssert(_input.ends_with(brackEnd));
		_input.remove_suffix(1);
	}

	for (auto&& slotToken: ranges::views::split(_input, ','))
	{
		auto const slotTokenBegin = ranges::begin(slotToken);
		auto const slotTokenEnd  = ranges::end(slotToken);

		std::string_view token;
		if(slotTokenBegin != slotTokenEnd)
			token = {&*slotTokenBegin, static_cast<std::size_t>(ranges::distance(slotTokenBegin, slotTokenEnd))};
		token = trim(token);
		yulAssert(!token.empty(), "Empty token.");
		result.push_back(parseSlot(_table, token));
	}
	return result;
}

struct ShuffleTestInput
{
	std::optional<StackData> initial;
	std::optional<StackData> targetStackTop;
	bool allowSpilling = false;
	std::size_t reachableDepth = reachableStackDepth;
	spill::SpillSet initialSpilledSet{};
	StackData initialSpilledSetSlots{};

	bool valid() const
	{
		return initial.has_value() && targetStackTop.has_value();
	}

	static ShuffleTestInput parse(ParsedIdentifierTable& _table, std::string_view _source)
	{
		ShuffleTestInput result;

		auto const stripComment = [](std::string_view sv) -> std::string_view
		{
			auto const pos = sv.find("//");
			if (pos != std::string_view::npos)
				return sv.substr(0, pos);
			return sv;
		};

		for (auto&& lineRange: ranges::views::split(_source, '\n'))
		{
			auto lineBegin = ranges::begin(lineRange);
			auto lineEnd  = ranges::end(lineRange);
			if (lineBegin == lineEnd)
				continue;

			std::string_view line{&*lineBegin, static_cast<std::size_t>(ranges::distance(lineBegin, lineEnd))};
			line = trim(stripComment(line));
			if (line.empty())
				continue;

			auto const colonPos = line.find(':');
			if (colonPos == std::string_view::npos)
				continue;

			auto const key = trim(line.substr(0, colonPos));
			auto const value = trim(line.substr(colonPos + 1));

			if (key == parserKeyInitialStack)
				result.initial = parseSlots(_table, value, '[', ']');
			else if (key == parserKeyStackTop)
				result.targetStackTop = parseSlots(_table, value, '[', ']');
			else if (key == parserKeyAllowSpilling)
			{
				if (value == "true")
					result.allowSpilling = true;
				else if (value == "false")
					result.allowSpilling = false;
				else
					throw std::runtime_error(fmt::format("Couldn't parse allowSpilling: {}", value));
			}
			else if (key == parserKeyReachableDepth)
			{
				auto const depth = solidity::util::parseArithmetic<std::size_t>(value);
				if (!depth || *depth < 2 || *depth > reachableStackDepth)
					throw std::runtime_error(fmt::format("Couldn't parse reachableStackDepth: {}", value));
				result.reachableDepth = *depth;
			}
			else if (key == parserKeyInitialSpilled)
			{
				result.initialSpilledSetSlots = parseSlots(_table, value, '{', '}');
				for (auto const& slot: result.initialSpilledSetSlots)
				{
					yulAssert(slot.isValue(), "Only value IDs can be spilled.");
					result.initialSpilledSet.add(slot.value());
				}
			}
		}
		return result;
	}
};

/// Records a shuffling trace and produces formatted output into some ostream when going out of scope
class TraceRecorder
{
	static size_t constexpr operationColumnWidth = 12;
	static size_t constexpr slotColumnWidth = 7;
	static char constexpr junkSymbol = '*';

public:
	TraceRecorder(std::ostream& _out, ParsedIdentifierTable const& _table, StackData const& _target):
		m_out(_out),
		m_table(_table),
		m_target(_target)
	{}

	void record(std::string const& _operation, StackData const& _stack)
	{
		m_entries.push_back(TraceEntry{_operation, _stack});
	}

	~TraceRecorder()
	{
		if (m_entries.empty())
			return;

		size_t maxStackDepth = 0;
		for (const auto& [operation, stackAfter]: m_entries)
			maxStackDepth = std::max(maxStackDepth, stackAfter.size());

		if (maxStackDepth == 0)
			return;

		std::size_t const numColumns = std::max(maxStackDepth, m_target.size());
		std::vector columnWidths(numColumns, slotColumnWidth);
		for (const auto& [operation, stackAfter]: m_entries)
			for (std::size_t i = 0; i < stackAfter.size(); ++i)
				columnWidths[i] = std::max(columnWidths[i], render(stackAfter[i]).size() + 1);
		for (std::size_t i = 0; i < m_target.size(); ++i)
			columnWidths[i] = std::max(columnWidths[i], render(m_target[i]).size() + 1);

		// a separator marks the end of the target when the stack grew beyond it
		bool const hasExcess = maxStackDepth > m_target.size();

		emitHeader(hasExcess, columnWidths);
		emitSeparatorLine(hasExcess, columnWidths);
		for (auto const& entry: m_entries)
			emitDataRow(entry, hasExcess, columnWidths);
		emitSeparatorLine(hasExcess, columnWidths);
		emitTargetRow(hasExcess, columnWidths);
	}

private:
	struct TraceEntry {
		std::string operation;
		StackData stackAfter;
	};

	std::ostream& m_out;
	ParsedIdentifierTable const& m_table;
	StackData const& m_target;
	std::vector<TraceEntry> m_entries;

	std::string render(StackSlot const& _slot) const
	{
		return _slot.isJunk() ? std::string(1, junkSymbol) : m_table.render(_slot);
	}

	void emitSeparator(size_t const _index, bool const _hasExcess, char const _junction) const
	{
		if (_hasExcess && _index == m_target.size())
			m_out << ' ' << _junction;
	}

	void emitHeader(bool const _hasExcess, std::vector<std::size_t> const& _columnWidths) const
	{
		m_out << fmt::format("{:>{}}", "", operationColumnWidth) << "|";
		for (std::size_t i = 0; i < _columnWidths.size(); ++i)
		{
			emitSeparator(i, _hasExcess, '|');
			m_out << fmt::format("{:>{}}", i, _columnWidths[i]);
		}
		m_out << "\n";
	}

	void emitSeparatorLine(bool const _hasExcess, std::vector<std::size_t> const& _columnWidths) const
	{
		m_out << fmt::format("{:>{}}", "", operationColumnWidth) << '+';
		for (std::size_t i = 0; i < _columnWidths.size(); ++i)
		{
			emitSeparator(i, _hasExcess, '+');
			m_out << std::string(_columnWidths[i], '-');
		}
		m_out << '\n';
	}

	void emitDataRow(TraceEntry const& _entry, bool const _hasExcess, std::vector<std::size_t> const& _columnWidths) const
	{
		m_out << fmt::format("{:>{}}", _entry.operation, operationColumnWidth) << "|";
		for (size_t i = 0; i < _entry.stackAfter.size(); ++i)
		{
			emitSeparator(i, _hasExcess, '|');
			m_out << fmt::format("{:>{}}", render(_entry.stackAfter[i]), _columnWidths[i]);
		}
		m_out << '\n';
	}

	void emitTargetRow(bool const _hasExcess, std::vector<size_t> const& _columnWidths) const
	{
		m_out << fmt::format("{:>{}}", "(target)", operationColumnWidth) << "|";
		for (std::size_t i = 0; i < m_target.size(); ++i)
			m_out << fmt::format("{:>{}}", render(m_target[i]), _columnWidths[i]);
		if (_hasExcess)
			m_out << " |";
		m_out << '\n';
	}
};
}

std::unique_ptr<frontend::test::TestCase> ShufflingTest::create(Config const& _config)
{
	return std::make_unique<ShufflingTest>(_config.filename);
}

ShufflingTest::ShufflingTest(std::string const& _filename): TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	soltestAssert(dialectName == "evm");
	m_expectation = m_reader.simpleExpectations();
}

ShufflingTest::TestResult ShufflingTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	ParsedIdentifierTable table;
	auto const testConfig = ShuffleTestInput::parse(table, m_source);
	if (!testConfig.valid())
	{
		  static constexpr std::string_view formatHelp = R"(initial: [<slot>, ...]
targetStackTop: [<slot>, ...]
allowSpilling: true|false
reachableStackDepth: <N>  (optional, default 16)
initialSpilledSet: {<slot>, ...}

Where <slot> is one of:
  v<N>    - variable
  phi<N>  - phi node
  lit<N>  - literal
  JUNK    - junk slot (a wildcard in the target)
  ReturnLabel[<N>], FunctionCallReturnLabel[<N>]

Lines starting with // are comments. Comments at the end of lines are supported, too.)";
		util::AnsiColorized out(_stream, _formatted, {util::formatting::BOLD, util::formatting::RED});
		out	<< _linePrefix << fmt::format("Error parsing source. Expected format:") << '\n';

		for (auto const line: ranges::views::split(formatHelp, '\n'))
		{
			auto const lineSVBegin = ranges::begin(line);
			auto const lineSVEnd = ranges::end(line);
			std::string_view lineSV;
			if (lineSVBegin != lineSVEnd)
				lineSV = {&*lineSVBegin, static_cast<std::size_t>(ranges::distance(lineSVBegin, lineSVEnd))};
			out << _linePrefix << "  " << lineSV << '\n';
		}
		return TestResult::FatalError;
	}

	StackData const& target = *testConfig.targetStackTop;
	spill::SpillSet spillSet = testConfig.initialSpilledSet;
	auto stackData = *testConfig.initial;
	std::ostringstream oss;
	// Tracks the kind of each spilled value
	std::vector<StackSlot> spilledSlotList = testConfig.initialSpilledSetSlots;

	// The planner plans its spills itself; without allowSpilling a plan that needs one is reported as too deep.
	spill::SpillSet const spillSetBefore = spillSet;
	stack::ShuffleResult shuffleResult = stack::shuffle(stackData, target, spillSet, true, testConfig.reachableDepth);
	std::vector<StackSlot> newlySpilled;
	for (InstId const value: spillSet.spilledValues())
		if (!spillSetBefore.isSpilled(value))
			newlySpilled.push_back(Slot::makeValue(table.store, value));
	bool const tooDeep =
		shuffleResult.status != stack::ShuffleResult::Status::Admissible ||
		(!testConfig.allowSpilling && !newlySpilled.empty());
	if (tooDeep)
	{
		shuffleResult.trace.clear();
		stackData = *testConfig.initial;
	}
	else
		spilledSlotList += newlySpilled;

	// Reconstruct the intermediate stack states by replaying the trace on top of the initial stack.
	{
		TraceRecorder trace(oss, table, target);
		trace.record("(initial)", *testConfig.initial);
		StackData replayData = *testConfig.initial;
		for (ShuffleOp const& op: shuffleResult.trace)
		{
			apply(replayData, op);
			trace.record(render(table, op), replayData);
		}
		yulAssert(replayData == stackData, "replayed trace must reproduce the shuffled stack");
	}

	if (tooDeep)
		oss << fmt::format(
			"Status: StackTooDeep{}\n",
			newlySpilled.empty() ? "" : fmt::format(" (would spill: {})", table.render(newlySpilled.front()))
		);
	else
	{
		// the plan: which source offset serves each target position, `gen` for generated slots
		oss << "Plan:";
		for (auto const& source: shuffleResult.sourceOf)
			if (source)
				oss << ' ' << *source;
			else
				oss << " gen";
		oss << "\nStatus: Admissible\n";
	}
	if (testConfig.allowSpilling)
		oss << fmt::format(
			"Spilled: {{{}}}\n",
			fmt::join(
				spilledSlotList | ranges::views::transform([&](StackSlot const& _slot) { return table.render(_slot); }),
				", "
			)
		);
	// check stack data
	if (!tooDeep)
		yulAssert(checkLayoutCompatibility(stackData, target).ok());
	m_obtainedResult = oss.str();


	return checkResult(_stream, _linePrefix, _formatted);
}
