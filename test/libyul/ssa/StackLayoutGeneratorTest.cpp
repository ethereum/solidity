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

#include <test/libyul/ssa/StackLayoutGeneratorTest.h>

#include <libyul/backends/evm/ssa/io/DotExporterBase.h>
#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/SSACFGBuilder.h>
#include <libyul/backends/evm/ssa/Stack.h>
#include <libyul/backends/evm/ssa/StackLayout.h>
#include <libyul/backends/evm/ssa/StackLayoutGenerator.h>
#include <libyul/backends/evm/ssa/StackUtils.h>

#include <libyul/backends/evm/ssa/transform/OptimizationPipeline.h>

#include <libyul/Common.h>
#include <libyul/YulStack.h>

#include <fmt/ranges.h>

#include <range/v3/view/split.hpp>
#include <range/v3/view/zip.hpp>

#ifdef ISOLTEST
#include <boost/version.hpp>
#if (BOOST_VERSION < 108800)
#include <boost/process.hpp>
#else
#define BOOST_PROCESS_VERSION 1
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#endif
#endif

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul::ssa;
using namespace solidity::yul::test::ssa;

namespace
{

class StackLayoutDotExporter: public io::DotExporterBase
{
public:
	StackLayoutDotExporter(
		SSACFG const& _cfg,
		std::size_t _functionIndex,
		SSACFGStackLayout const& _layout,
		spill::SpillSet const& _spillSet,
		ControlFlowGraphs const& _controlFlow
	):
		DotExporterBase(_cfg, _functionIndex),
		m_layout(_layout),
		m_spillSet(_spillSet),
		m_controlFlow(_controlFlow)
	{
	}

protected:
	std::string extraEntryLabel() override
	{
		return fmt::format("spilled: {{{}}}", fmt::join(m_spillSet.spilledValues(), ", "));
	}

	void writeBlockLabel(std::ostream& _out, SSACFG::BlockId _blockId) override
	{
		auto const& block = m_cfg.block(_blockId);
		auto const& blockLayout = m_layout[_blockId];
		yulAssert(blockLayout.has_value());

		_out << "\\\n";
		_out << "IN: " << stackToString(blockLayout->stackIn) << "\\l\\\n";

		// Reconstruct the per-operation input layouts and the exit state by replaying the recorded
		// shuffle traces and operation effects from the block's stackIn.
		StackData operationStack = blockLayout->stackIn;
		yulAssert(blockLayout->operationShuffles.size() == block.instructions.size());
		for (auto const& [instId, trace]: ranges::views::zip(block.instructions, blockLayout->operationShuffles))
		{
			SSACFG::Inst const& inst = m_cfg.inst(instId);
			if (!inst.isOperation())
				continue;
			replay(operationStack, trace);

			_out << "\\l\\\n";
			_out << stackToString(operationStack) << "\\l\\\n";

			if (inst.opcode == InstOpcode::Call)
				_out << escapeLabel(m_controlFlow.functionGraph(m_cfg.callPayload(instId).graphID)->name);
			else
				_out << escapeLabel(m_cfg.evmDialect.builtin(m_cfg.builtinPayload(instId).builtin).name);
			_out << "\\l\\\n";

			yulAssert(inst.inputs.size() <= operationStack.size());
			for (std::size_t j = 0; j < inst.inputs.size(); ++j)
				operationStack.pop_back();
			// A continuing function call's return label is not an SSA input but is consumed by the
			// callee's return jump, so drop it to reflect the actual post-call stack.
			if (inst.opcode == InstOpcode::Call && m_cfg.callPayload(instId).canContinue)
			{
				yulAssert(!operationStack.empty() && operationStack.back().isFunctionCallReturnLabel());
				operationStack.pop_back();
			}
			m_cfg.forEachOutput(instId, [&](InstId const output) {
				operationStack.push_back(StackSlot::makeValue(m_cfg, output));
			});
			_out << stackToString(operationStack) << "\\l\\\n";
		}

		replay(operationStack, blockLayout->exitShuffle);
		_out << "\\l\\\n";
		_out << "OUT: " << stackToString(operationStack) << "\\l\\\n";
	}

private:
	SSACFGStackLayout const& m_layout;
	spill::SpillSet const& m_spillSet;
	ControlFlowGraphs const& m_controlFlow;
};

}

std::unique_ptr<frontend::test::TestCase> StackLayoutGeneratorTest::create(Config const& _config)
{
	return std::make_unique<StackLayoutGeneratorTest>(_config.filename);
}

StackLayoutGeneratorTest::StackLayoutGeneratorTest(std::string const& _filename): TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	soltestAssert(dialectName == "evm");
	m_expectation = m_reader.simpleExpectations();
}

frontend::test::TestCase::TestResult StackLayoutGeneratorTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	static std::string_view constexpr SUBOBJECT_GRAPH_SEPARATOR = "\n>>>>> GRAPH SEPARATOR\n";
	YulStack const yulStack = yul::test::parseYul(m_source);
	if (yulStack.hasErrors())
	{
		yul::test::printYulErrors(yulStack, _stream, _linePrefix, _formatted);
		return TestResult::FatalError;
	}

	std::set<Object const*> visited;
	visited.insert(yulStack.parserResult().get());

	std::vector<Object const*> toVisit{yulStack.parserResult().get()};
	while (!toVisit.empty())
	{
		auto const& object = *toVisit.back();
		toVisit.pop_back();

		auto const* evmDialect = dynamic_cast<EVMDialect const*>(object.dialect());
		yulAssert(evmDialect);

		std::unique_ptr<ControlFlowGraphs> const controlFlowGraphs = SSACFGBuilder::build(
			*object.analysisInfo,
			*evmDialect,
			object.code()->root(),
			false
		);
		yul::ssa::transform::optimize(*controlFlowGraphs);
		// insert separator
		if (!m_obtainedResult.empty())
			m_obtainedResult += SUBOBJECT_GRAPH_SEPARATOR;

		m_obtainedResult += "digraph SSACFG {\nnodesep=0.7;\ngraph[fontname=\"DejaVu Sans\", rankdir=LR]\nnode[shape=box,fontname=\"DejaVu Sans\"];\n\n";

		for (std::size_t index = 0; index < controlFlowGraphs->functionGraphs.size(); ++index)
		{
			auto const& cfg = *controlFlowGraphs->functionGraphs[index];
			auto result = StackLayoutGenerator::generate(
				LivenessAnalysis(cfg),
				gatherCallSites(cfg),
				static_cast<ControlFlowGraphs::FunctionGraphID>(index),
				true
			);
			StackLayoutDotExporter exporter(cfg, index, result.layout, result.spillSet, *controlFlowGraphs);
			if (!cfg.isMainGraph())
				m_obtainedResult += exporter.exportFunction(cfg, false);
			else
				m_obtainedResult += exporter.exportBlocks(cfg.entry, false);
		}

		m_obtainedResult += "}\n";

		for (auto const& subNode: object.subObjects)
			if (auto subObject = std::dynamic_pointer_cast<Object>(subNode))
				if (!visited.contains(subObject.get()))
				{
					visited.insert(subObject.get());
					toVisit.push_back(subObject.get());
				}
	}

	auto const result = checkResult(_stream, _linePrefix, _formatted);

#ifdef ISOLTEST
	char* graphDisplayer = nullptr;
	if (result == TestResult::Failure)
		graphDisplayer = getenv("ISOLTEST_DISPLAY_GRAPHS_FAILURE");
	else if (result == TestResult::Success)
		graphDisplayer = getenv("ISOLTEST_DISPLAY_GRAPHS_SUCCESS");

	if (graphDisplayer)
	{
		if (result == TestResult::Success)
			std::cout << std::endl << m_source << std::endl;
		for (auto const dotRange: ranges::views::split(m_obtainedResult, SUBOBJECT_GRAPH_SEPARATOR))
		{
			std::string_view dot(&*dotRange.begin(), static_cast<std::size_t>(ranges::distance(dotRange)));
			boost::process::opstream pipe;
			boost::process::child child(graphDisplayer, boost::process::std_in < pipe);

			pipe << dot;
			pipe.flush();
			pipe.pipe().close();
			if (result == TestResult::Success)
				child.wait();
			else
				child.detach();
		}
	}
#endif

	return result;
}
