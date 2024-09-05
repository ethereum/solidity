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

#include <test/libyul/SSAControlFlowGraphTest.h>
#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/backends/evm/SSAControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/SSACFGLiveness.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/Object.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/Visitor.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-compare"
#include <fmt/ranges.h>
#pragma GCC diagnostic pop

#ifdef ISOLTEST
#include <boost/process.hpp>
#endif

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

std::unique_ptr<TestCase> SSAControlFlowGraphTest::create(TestCase::Config const& _config) {
	return std::make_unique<SSAControlFlowGraphTest>(_config.filename);
}

SSAControlFlowGraphTest::SSAControlFlowGraphTest(std::string const& _filename): TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	m_dialect = &dialect(dialectName, solidity::test::CommonOptions::get().evmVersion());
	m_expectation = m_reader.simpleExpectations();
}

class SSACFGPrinter
{
public:
	SSACFGPrinter(SSACFG const& _cfg, SSACFG::BlockId _blockId): m_cfg(_cfg), m_functionIndex(0), m_sort(ReducedTopologicalSort::run(_cfg)), m_liveness(_cfg)
	{
		printBlock(_blockId);
	}
	SSACFGPrinter(SSACFG const& _cfg, size_t _functionIndex, Scope::Function const& _function): m_cfg(_cfg), m_functionIndex(_functionIndex), m_sort(ReducedTopologicalSort::run(_cfg)), m_liveness(_cfg)
	{
		printFunction(_function);
	}
	friend std::ostream& operator<<(std::ostream& stream, SSACFGPrinter const& printer) {
		stream << printer.m_result.str();
		return stream;
	}
private:
	std::string varToString(SSACFG::ValueId _var) {
		if (_var.value == std::numeric_limits<size_t>::max())
			return "INVALID";
		auto const& info = m_cfg.valueInfo(_var);
		return std::visit(
			util::GenericVisitor{
				[&](SSACFG::UnreachableValue const&) -> std::string {
					return "[unreachable]";
				},
				[&](SSACFG::PhiValue const&) -> std::string {
					return fmt::format("v{}", _var.value);
				},
				[&](SSACFG::VariableValue const&) -> std::string {
					return fmt::format("v{}", _var.value);
				},
				[&](SSACFG::LiteralValue const& _literal) -> std::string {
					return util::formatNumberReadable(_literal.value);
				}
			},
			info
		);
	}

	std::string formatPhi(SSACFG::PhiValue const& _phiValue)
	{
		auto const transform = [&](SSACFG::ValueId const& valueId) { return varToString(valueId); };
		std::vector<std::string> formattedArgs;
		formattedArgs.reserve(_phiValue.arguments.size());
		for(auto const& [arg, entry] : ranges::zip_view(_phiValue.arguments | ranges::views::transform(transform), m_cfg.block(_phiValue.block).entries))
		{
			formattedArgs.push_back(fmt::format("Block {} => {}", entry.value, arg));
		}
		return fmt::format("φ(\\l\\\n\t{}\\l\\\n)", fmt::join(formattedArgs, ",\\l\\\n\t"));
	}

	std::string formatBlockHandle(SSACFG::BlockId const& _id) const
	{
		return fmt::format("Block{}_{}", m_functionIndex, _id.value);
	}

	std::string formatEdge(SSACFG::BlockId const& _v, SSACFG::BlockId const& _w, std::optional<std::string> const& _vPort = std::nullopt)
	{
		std::string const style = m_sort.backEdge(_v, _w) ? "dashed" : "solid";
		if (_vPort)
			return fmt::format("{}Exit:{} -> {} [style=\"{}\"];\n", formatBlockHandle(_v), *_vPort, formatBlockHandle(_w), style);
		else
			return fmt::format("{}Exit -> {} [style=\"{}\"];\n", formatBlockHandle(_v), formatBlockHandle(_w), style);
	}

	void writeBlock(SSACFG::BlockId const& _id, SSACFG::BasicBlock const& _block)
	{
		auto const transform = [&](SSACFG::ValueId const& valueId) { return varToString(valueId); };
		bool entryBlock = _id.value == 0 && m_functionIndex == 0;
		if (entryBlock)
		{
			m_result << fmt::format("Entry{} [label=\"Entry\"];\n", m_functionIndex);
			m_result << fmt::format("Entry{} -> {};\n", m_functionIndex, formatBlockHandle(_id));
		}
		{
			m_result << fmt::format("{} [label=\"\\\nBlock {}; ({}, max {})\\n", formatBlockHandle(_id), _id.value, m_sort.preOrder()[_id.value], m_sort.maxSubtreePreOrder()[_id.value]);
			m_result << fmt::format("LiveIn: {}\\l\\\n", fmt::join(m_liveness.liveIns()[_id.value] | ranges::views::transform([](SSACFG::ValueId const& valId) { return "v" + std::to_string(valId.value); }), ","));
			m_result << fmt::format("LiveOut: {}\\l\\n", fmt::join(m_liveness.liveOuts()[_id.value] | ranges::views::transform([](SSACFG::ValueId const& valId) { return "v" + std::to_string(valId.value); }), ","));
			for (auto const& phi : _block.phis)
			{
				auto const* phiValue = std::get_if<SSACFG::PhiValue>(&m_cfg.valueInfo(phi));
				solAssert(phiValue);
				m_result << fmt::format("v{} := {}\\l\\\n", phi.value, formatPhi(*phiValue));
			}
			for (auto const& operation : _block.operations)
			{
				std::string const label = std::visit(util::GenericVisitor{
					[&](SSACFG::Call const& _call) {
					   return _call.function.get().name.str();
					},
					[&](SSACFG::BuiltinCall const& _call) {
					   return _call.builtin.get().name.str();
					},
				}, operation.kind);
				if (operation.outputs.empty())
					m_result << fmt::format(
						"{}({})\\l\\\n",
						label,
						fmt::join(operation.inputs | ranges::views::transform(transform), ", ")
					);
				else
					m_result << fmt::format(
						"{} := {}({})\\l\\\n",
						fmt::join(operation.outputs | ranges::views::transform(transform), ", "),
						label,
						fmt::join(operation.inputs | ranges::views::transform(transform), ", ")
					);
			}
			m_result << "\"];\n";
			std::visit(util::GenericVisitor{
			   [&](SSACFG::BasicBlock::MainExit const&)
			   {
				   m_result << fmt::format("{}Exit [label=\"MainExit\"];\n", formatBlockHandle(_id));
				   m_result << fmt::format("{} -> {}Exit;\n", formatBlockHandle(_id), formatBlockHandle(_id));
			   },
			   [&](SSACFG::BasicBlock::Jump const& _jump)
			   {
				   m_result << fmt::format("{} -> {}Exit [arrowhead=none];\n", formatBlockHandle(_id), formatBlockHandle(_id));
				   m_result << fmt::format("{}Exit [label=\"Jump\" shape=oval];\n", formatBlockHandle(_id));
				   m_result << formatEdge(_id, _jump.target);
			   },
			   [&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
			   {
				   m_result << fmt::format("{} -> {}Exit;\n", formatBlockHandle(_id), formatBlockHandle(_id));
				   m_result << fmt::format(
					   "{}Exit [label=\"{{ If {} | {{ <0> Zero | <1> NonZero }}}}\" shape=Mrecord];\n",
					   formatBlockHandle(_id), varToString(_conditionalJump.condition)
				   );
				   m_result << formatEdge(_id, _conditionalJump.zero, "0");
				   m_result << formatEdge(_id, _conditionalJump.nonZero, "1");
			   },
			   [&](SSACFG::BasicBlock::JumpTable const& jt)
			   {
				   m_result << fmt::format("{} -> {}Exit;\n", formatBlockHandle(_id), formatBlockHandle(_id));
				   std::string options;
				   for(const auto& jumpCase : jt.cases)
				   {
					   if (!options.empty())
						   options += " | ";
					   options += fmt::format("<{0}> {0}", formatNumber(jumpCase.first));
				   }
				   if (!options.empty())
					   options += " | ";
				   options += "<default> default";
				   m_result << fmt::format("{}Exit [label=\"{{ JT | {{ {} }} }}\" shape=Mrecord];\n", formatBlockHandle(_id), options);
				   for(const auto& jumpCase : jt.cases)
					   m_result << formatEdge(_id, jumpCase.second, formatNumber(jumpCase.first));
				   m_result << formatEdge(_id, jt.defaultCase, "default");
			   },
			   [&](SSACFG::BasicBlock::FunctionReturn const& fr)
			   {
				   m_result << formatBlockHandle(_id) << "Exit [label=\"FunctionReturn["
						  << fmt::format("{}", fmt::join(fr.returnValues | ranges::views::transform(transform), ", "))
						  << "]\"];\n";
				   m_result << formatBlockHandle(_id) << " -> " << formatBlockHandle(_id) << "Exit;\n";
			   },
			   [&](SSACFG::BasicBlock::Terminated const&)
			   {
				   m_result << formatBlockHandle(_id) << "Exit [label=\"Terminated\"];\n";
				   m_result << formatBlockHandle(_id)<< " -> " << formatBlockHandle(_id) << "Exit;\n";
			   }
		   }, _block.exit);
		}

	}

	void printBlock(SSACFG::BlockId const& _rootId)
	{
		std::set<SSACFG::BlockId> explored{};
		explored.insert(_rootId);

		std::deque<SSACFG::BlockId> toVisit{};
		toVisit.emplace_back(_rootId);

		while(!toVisit.empty())
		{
			auto const id = toVisit.front();
			toVisit.pop_front();
			auto const& block = m_cfg.block(id);
			writeBlock(id, block);
			block.forEachExit(
				[&](SSACFG::BlockId const& _exitBlock)
				{
					if (explored.count(_exitBlock) == 0)
					{
						explored.insert(_exitBlock);
						toVisit.emplace_back(_exitBlock);
					}
				}
			);
		}
	}

	void printFunction(Scope::Function const& _fun)
	{
		static auto constexpr returnsTransform = [](auto const& functionReturnValue) { return functionReturnValue.get().name.str(); };
		static auto constexpr argsTransform = [](auto const& arg) { return fmt::format("v{}", std::get<1>(arg).value); };
		m_result << "FunctionEntry_" << _fun.name.str() << "_" << m_cfg.entry.value << " [label=\"";
		if (!m_cfg.returns.empty())
			m_result << fmt::format("function {0}:\n {1} := {0}({2})", _fun.name.str(), fmt::join(m_cfg.returns | ranges::views::transform(returnsTransform), ", "), fmt::join(m_cfg.arguments | ranges::views::transform(argsTransform), ", "));
		else
			m_result << fmt::format("function {0}:\n {0}({1})", _fun.name.str(), fmt::join(m_cfg.arguments | ranges::views::transform(argsTransform), ", "));
		m_result << "\"];\n";
		m_result << "FunctionEntry_" << _fun.name.str() << "_" << m_cfg.entry.value << " -> Block" << m_functionIndex << "_" << m_cfg.entry.value << ";\n";
		printBlock(m_cfg.entry);
	}

	SSACFG const& m_cfg;
	size_t const m_functionIndex;
	ReducedTopologicalSort const m_sort;
	SSACFGLiveness const m_liveness;
	std::stringstream m_result{};
};

void SSAControlFlowGraphTest::printLiveness(SSACFG const& _cfg) const
{
	SSACFGLiveness livenessMain(_cfg);
	for (size_t blockId = 0; blockId < livenessMain.liveIns().size(); ++blockId)
	{
		fmt::print("Live ins for Block {}:\n", blockId);
		auto const& liveIns = livenessMain.liveIns()[blockId];
		for (auto const& liveIn : liveIns)
		{
			auto const& valInfo = _cfg.valueInfo(liveIn);
			std::visit(GenericVisitor{
						   [&](SSACFG::UnreachableValue const&) {},
						   [&](SSACFG::VariableValue const& var) { fmt::print("\tv{} (defining block {})\n", liveIn.value, var.definingBlock.value); },
						   [&](SSACFG::LiteralValue const&) { fmt::print("\tv{} (literal)\n", liveIn.value); },
						   [&](SSACFG::PhiValue const& phi) { fmt::print("\tv{} (phi from block {})\n", liveIn.value, phi.block.value); }
					   }, valInfo);
		}
	}

	for (size_t blockId = 0; blockId < livenessMain.liveOuts().size(); ++blockId)
	{
		fmt::print("Live outs for Block {}:\n", blockId);
		auto const& liveIns = livenessMain.liveOuts()[blockId];
		for (auto const& liveIn : liveIns)
		{
			auto const& valInfo = _cfg.valueInfo(liveIn);
			std::visit(GenericVisitor{
						   [&](SSACFG::UnreachableValue const&) {},
						   [&](SSACFG::VariableValue const& var) { fmt::print("\tv{} (defining block {})\n", liveIn.value, var.definingBlock.value); },
						   [&](SSACFG::LiteralValue const&) { fmt::print("\tv{} (literal)\n", liveIn.value); },
						   [&](SSACFG::PhiValue const&) { fmt::print("\tv{} (phi)\n", liveIn.value); }
					   }, valInfo);
		}
	}
}

TestCase::TestResult SSAControlFlowGraphTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	ErrorList errors;
	auto [object, analysisInfo] = parse(m_source, *m_dialect, errors);
	if (!object || !analysisInfo || Error::containsErrors(errors))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << std::endl;
		return TestResult::FatalError;
	}

	std::ostringstream output;

	auto info = AsmAnalyzer::analyzeStrictAssertCorrect(*m_dialect, *object);
	std::unique_ptr<ControlFlow> controlFlow = SSAControlFlowGraphBuilder::build(
		info,
		*m_dialect,
		object->code()->root()
	);

	printLiveness(*controlFlow->functionGraphs[0]);

	output << "digraph SSACFG {\nnodesep=0.7;\ngraph[fontname=\"DejaVu Sans\"]\nnode[shape=box,fontname=\"DejaVu Sans\"];\n\n";
	output << SSACFGPrinter(*controlFlow->mainGraph, SSACFG::BlockId{0});
	size_t index = 1;
	for (auto const& [function, functionGraph]: controlFlow->functionGraphMapping)
		output << SSACFGPrinter(*functionGraph, index++, *function);
	output << "}\n";

	m_obtainedResult = output.str();

	auto result = checkResult(_stream, _linePrefix, _formatted);

#ifdef ISOLTEST
	char* graphDisplayer = nullptr;
	// The environment variables specify an optional command that will receive the graph encoded in DOT through stdin.
	// Examples for suitable commands are ``dot -Tx11:cairo`` or ``xdot -``.
	if (result == TestResult::Failure)
		// ISOLTEST_DISPLAY_GRAPHS_ON_FAILURE_COMMAND will run on all failing tests (intended for use during modifications).
		graphDisplayer = getenv("ISOLTEST_DISPLAY_GRAPHS_ON_FAILURE_COMMAND");
	else if (result == TestResult::Success)
		// ISOLTEST_DISPLAY_GRAPHS_ON_FAILURE_COMMAND will run on all succeeding tests (intended for use during reviews).
		graphDisplayer = getenv("ISOLTEST_DISPLAY_GRAPHS_ON_SUCCESS_COMMAND");

	if (graphDisplayer)
	{
		if (result == TestResult::Success)
			std::cout << std::endl << m_source << std::endl;
		boost::process::opstream pipe;
		boost::process::child child(graphDisplayer, boost::process::std_in < pipe);

		pipe << output.str();
		pipe.flush();
		pipe.pipe().close();
		if (result == TestResult::Success)
			child.wait();
		else
			child.detach();
	}
#endif

	return result;

}
