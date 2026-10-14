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

#include <test/libyul/ssa/SpillTest.h>

#include <test/libyul/Common.h>

#include <libyul/backends/evm/ssa/CallGraph.h>
#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/SSACFGBuilder.h>
#include <libyul/backends/evm/ssa/StackLayoutGenerator.h>
#include <libyul/backends/evm/ssa/StackUtils.h>
#include <libyul/backends/evm/ssa/spill/MemoryAddressing.h>
#include <libyul/backends/evm/ssa/spill/SpillSet.h>
#include <libyul/backends/evm/ssa/transform/OptimizationPipeline.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/Object.h>
#include <libyul/YulStack.h>

#include <libsolutil/Numeric.h>

#include <fmt/format.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::yul::ssa;
using namespace solidity::yul::test::ssa;

namespace
{

std::string describeCFG(
	SSACFG const& _cfg,
	ControlFlowGraphs::FunctionGraphID const _graphID,
	std::vector<std::pair<SpillKey, u256>> const& _spilled
)
{
	std::string label = _cfg.isMainGraph() ? "<main>" : _cfg.name;
	std::string out = fmt::format("CFG[{}] {}\n", _graphID, label);

	if (_spilled.empty())
	{
		out += "  spilled: none\n";
		return out;
	}

	out += "  spilled:\n";
	for (auto const& [key, address]: _spilled)
		out += fmt::format(
			"    {} -> mem {}\n",
			key,
			toCompactHexWithPrefix(address)
		);

	out += "  mstore schedule:\n";
	for (const auto& key: _spilled | std::views::keys)
	{
		SSACFG::BlockId const block = _cfg.inst(key.value()).block;
		out += fmt::format(
			"    mstore addr({}) <- {} (B#{})\n",
			key,
			key,
			block.value
		);
	}
	return out;
}

}

std::unique_ptr<frontend::test::TestCase> SpillTest::create(Config const& _config)
{
	return std::make_unique<SpillTest>(_config.filename);
}

SpillTest::SpillTest(std::string const& _filename): TestCase(_filename)
{
	m_source = m_reader.source();
	auto const dialectName = m_reader.stringSetting("dialect", "evm");
	soltestAssert(dialectName == "evm");
	m_expectation = m_reader.simpleExpectations();
}

frontend::test::TestCase::TestResult SpillTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	static std::string_view constexpr OBJECT_SEPARATOR = "\n>>>>> OBJECT SEPARATOR\n";

	YulStack const yulStack = yul::test::parseYul(m_source);
	if (yulStack.hasErrors())
	{
		printYulErrors(yulStack, _stream, _linePrefix, _formatted);
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

		std::unique_ptr<ControlFlowGraphs> controlFlowGraphs = SSACFGBuilder::build(
			*object.analysisInfo,
			*evmDialect,
			object.code()->root(),
			false
		);
		transform::optimize(*controlFlowGraphs);
		ControlFlowGraphsLiveness const liveness(*controlFlowGraphs);
		CallGraph const callGraph(*controlFlowGraphs);

		std::size_t const numCFGs = controlFlowGraphs->functionGraphs.size();
		std::vector<spill::SpillSet> spillSetsPerCFG;
		std::vector<SSACFGStackLayout> layouts;
		spillSetsPerCFG.reserve(numCFGs);
		layouts.reserve(numCFGs);

		// Recompute the spill sets and per-block stack layouts exactly as the code transform would,
		// but stop at the addressing step instead of generating any bytecode.
		for (std::size_t functionIndex = 0; functionIndex < numCFGs; ++functionIndex)
		{
			SSACFG const& cfg = *controlFlowGraphs->functionGraphs[functionIndex];
			auto const& cfgLiveness = liveness.cfgLiveness[functionIndex];
			yulAssert(cfgLiveness);
			auto const graphID = static_cast<ControlFlowGraphs::FunctionGraphID>(functionIndex);
			bool const spillingAllowed = !callGraph.isRecursive(graphID);
			auto result = StackLayoutGenerator::generate(
				*cfgLiveness,
				gatherCallSites(cfg),
				graphID,
				spillingAllowed
			);
			layouts.push_back(std::move(result.layout));
			spillSetsPerCFG.push_back(std::move(result.spillSet));
		}

		for (std::size_t functionIndex = 0; functionIndex < numCFGs; ++functionIndex)
			spillSetsPerCFG[functionIndex].closeUnderReachabilityConstraints(
				*controlFlowGraphs->functionGraphs[functionIndex],
				layouts[functionIndex]
			);

		// build up global addressing based on the spill sets
		spill::MemoryAddressing const addressing(*controlFlowGraphs, spillSetsPerCFG);

		if (!m_obtainedResult.empty())
			m_obtainedResult += OBJECT_SEPARATOR;
		m_obtainedResult += fmt::format("object \"{}\"\n", object.name);
		m_obtainedResult += "===== SSA CFG =====\n";
		m_obtainedResult += controlFlowGraphs->print();
		m_obtainedResult += "===== spill info =====\n";
		for (std::size_t functionIndex = 0; functionIndex < numCFGs; ++functionIndex)
		{
			SSACFG const& cfg = *controlFlowGraphs->functionGraphs[functionIndex];
			auto const graphID = static_cast<ControlFlowGraphs::FunctionGraphID>(functionIndex);
			std::vector<std::pair<SpillKey, u256>> spilled;
			for (SpillKey const key: spillSetsPerCFG[functionIndex].spilledValues())
				spilled.emplace_back(key, addressing.addressOf(graphID, key));
			m_obtainedResult += describeCFG(cfg, graphID, spilled);
		}

		for (auto const& subNode: object.subObjects)
			if (auto subObject = std::dynamic_pointer_cast<Object>(subNode))
				if (!visited.contains(subObject.get()))
				{
					visited.insert(subObject.get());
					toVisit.push_back(subObject.get());
				}
	}

	return checkResult(_stream, _linePrefix, _formatted);
}
