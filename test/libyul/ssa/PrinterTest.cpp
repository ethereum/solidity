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

#include <test/libyul/ssa/PrinterTest.h>

#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/backends/evm/ssa/io/Parser.h>
#include <libyul/backends/evm/ssa/SSACFGBuilder.h>
#include <libyul/backends/evm/ssa/transform/OptimizationPipeline.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/Object.h>
#include <libyul/YulStack.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/Visitor.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/zip.hpp>

#include <functional>
#include <map>
#include <optional>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::ssa;
using namespace solidity::yul::test::ssa;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

namespace
{

/// Compares two graphs up to renumbering
class GraphComparison
{
public:
	GraphComparison(SSACFG const& _expected, SSACFG const& _actual):
		m_expected(_expected),
		m_actual(_actual)
	{}

	bool run()
	{
		if (ranges::distance(m_expected.liveBlocks()) != ranges::distance(m_actual.liveBlocks()))
			return false;
		for (auto const& [expected, actual]: ranges::views::zip(m_expected.liveBlocks(), m_actual.liveBlocks()))
			m_blocks.emplace(expected, actual);
		return
			m_expected.name == m_actual.name &&
			m_expected.numReturns == m_actual.numReturns &&
			m_expected.canContinue == m_actual.canContinue &&
			sameBlock(m_expected.entry, m_actual.entry) &&
			sameValues(m_expected.arguments, m_actual.arguments) &&
			ranges::all_of(m_expected.liveBlocks(), [&](BlockId const& _block) { return sameBasicBlock(_block); });
	}

private:
	bool sameBasicBlock(BlockId const _block)
	{
		SSACFG::BasicBlock const& expected = m_expected.block(_block);
		SSACFG::BasicBlock const& actual = m_actual.block(m_blocks.at(_block));
		auto const sameBlocksPredicate = std::bind_front(&GraphComparison::sameBlock, this);
		auto const sameInstructionsPredicate = std::bind_front(&GraphComparison::sameInstruction, this);
		return
			ranges::equal(expected.entries, actual.entries, sameBlocksPredicate) &&
			ranges::equal(expected.instructions, actual.instructions, sameInstructionsPredicate) &&
			expected.exit.index() == actual.exit.index() &&
			std::visit(GenericVisitor{
				[&](SSACFG::BasicBlock::Jump const& _jump) {
					auto const& other = std::get<SSACFG::BasicBlock::Jump>(actual.exit);
					return sameBlock(_jump.target, other.target);
				},
				[&](SSACFG::BasicBlock::ConditionalJump const& _jump) {
					auto const& other = std::get<SSACFG::BasicBlock::ConditionalJump>(actual.exit);
					return
						sameValue(_jump.condition, other.condition) &&
						sameBlock(_jump.nonZero, other.nonZero) &&
						sameBlock(_jump.zero, other.zero);
				},
				[&](SSACFG::BasicBlock::FunctionReturn const& _return) {
					auto const& other = std::get<SSACFG::BasicBlock::FunctionReturn>(actual.exit);
					return sameValues(_return.returnValues, other.returnValues);
				},
				[](auto const&) { return true; }
			}, expected.exit);
	}

	bool sameInstruction(InstId const _expected, InstId const _actual)
	{
		SSACFG::Inst const& expected = m_expected.inst(_expected);
		SSACFG::Inst const& actual = m_actual.inst(_actual);
		if (
			!sameValue(_expected, _actual) ||
			expected.opcode != actual.opcode ||
			!sameBlock(expected.block, actual.block) ||
			!sameValues(expected.inputs, actual.inputs)
		)
			return false;
		switch (expected.opcode)
		{
		case InstOpcode::Const:
			return m_expected.literalPayload(_expected) == m_actual.literalPayload(_actual);
		case InstOpcode::Upsilon:
			return sameValue(m_expected.upsilonPhi(_expected), m_actual.upsilonPhi(_actual));
		case InstOpcode::Projection:
			return m_expected.projectionIndex(_expected) == m_actual.projectionIndex(_actual);
		case InstOpcode::BuiltinCall:
		{
			auto const& expectedPayload = m_expected.builtinPayload(_expected);
			auto const& actualPayload = m_actual.builtinPayload(_actual);
			static auto constexpr sameLiteral = [](Literal const& _e, Literal const& _a) {
				return _e.kind == _a.kind && _e.value == _a.value;
			};
			return
				expectedPayload.builtin == actualPayload.builtin &&
				ranges::equal(
					expectedPayload.literalArguments,
					actualPayload.literalArguments,
					sameLiteral
				);
		}
		case InstOpcode::Call:
			return m_expected.callPayload(_expected) == m_actual.callPayload(_actual);
		case InstOpcode::Phi:
		case InstOpcode::Unreachable:
		case InstOpcode::FunctionArg:
		case InstOpcode::Identity:
		case InstOpcode::Nop:
		case InstOpcode::MemoryGuard:
		case InstOpcode::Tombstone:
			return true;
		}
		std::unreachable();
	}

	bool sameBlock(BlockId const _expected, BlockId const _actual) const
	{
		return m_blocks.at(_expected) == _actual;
	}

	/// Records `_expected <-> _actual` in the bijection.
	/// @returns false if either side is already mapped differently.
	bool sameValue(InstId const _expected, InstId const _actual)
	{
		auto const forward = m_values.emplace(_expected, _actual).first;
		auto const backward = m_valuesInverse.emplace(_actual, _expected).first;
		return forward->second == _actual && backward->second == _expected;
	}

	bool sameValues(std::vector<InstId> const& _expected, std::vector<InstId> const& _actual)
	{
		auto const sameValuePair = std::bind_front(&GraphComparison::sameValue, this);
		return ranges::equal(_expected, _actual, sameValuePair);
	}

	SSACFG const& m_expected;
	SSACFG const& m_actual;
	std::map<BlockId, BlockId> m_blocks;
	std::map<InstId, InstId> m_values;
	std::map<InstId, InstId> m_valuesInverse;
};

/// Checks that printing `_cfgs` and parsing the result gives the same graphs up to renumbering.
std::optional<std::string> roundTripError
(
	ControlFlowGraphs const& _cfgs,
	EVMDialect const& _dialect
)
{
	std::string const printed = _cfgs.print();
	auto const parsed = io::parse(printed, _dialect);
	if (!parsed)
		return fmt::format("Printed graphs do not parse: {}", parsed.error().message);
	ControlFlowGraphs const& reparsed = **parsed;
	auto const graphs = ranges::views::zip(_cfgs.functionGraphs, reparsed.functionGraphs);
	if (
		_cfgs.memoryGuard != reparsed.memoryGuard ||
		_cfgs.functionGraphs.size() != reparsed.functionGraphs.size() ||
		!ranges::all_of(graphs, [](auto const& _pair) {
			return GraphComparison(*std::get<0>(_pair), *std::get<1>(_pair)).run();
		})
	)
		return "Reparsed graphs differ from the printed ones.";
	return std::nullopt;
}

}

std::unique_ptr<TestCase> PrinterTest::create(Config const& _config)
{
	return std::make_unique<PrinterTest>(_config.filename);
}

PrinterTest::PrinterTest(std::string const& _filename): TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	soltestAssert(dialectName == "evm");
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult PrinterTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	YulStack yulStack = parseYul(m_source);
	solUnimplementedAssert(yulStack.parserResult()->subObjects.empty(), "Tests with subobjects not supported.");

	if (yulStack.hasErrors())
	{
		printYulErrors(yulStack, _stream, _linePrefix, _formatted);
		return TestResult::FatalError;
	}

	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&yulStack.dialect());
	yulAssert(evmDialect);

	std::unique_ptr<ControlFlowGraphs> controlFlowGraphs = SSACFGBuilder::build(
		*yulStack.parserResult()->analysisInfo,
		*evmDialect,
		yulStack.parserResult()->code()->root(),
		true
	);
	transform::optimize(*controlFlowGraphs);
	m_obtainedResult = controlFlowGraphs->print();
	if (std::optional<std::string> const roundTripFailure = roundTripError(*controlFlowGraphs, *evmDialect))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix
			<< "Round trip through the SSA CFG parser failed. " << *roundTripFailure << std::endl;
		return TestResult::FatalError;
	}

	return checkResult(_stream, _linePrefix, _formatted);
}
