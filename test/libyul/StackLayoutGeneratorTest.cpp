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

#include <test/libyul/StackLayoutGeneratorTest.h>
#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/StackLayoutGenerator.h>
#include <libyul/Object.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/reverse.hpp>

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
using namespace std;

StackLayoutGeneratorTest::StackLayoutGeneratorTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	m_dialect = &dialect(dialectName, solidity::test::CommonOptions::get().evmVersion());
	m_expectation = m_reader.simpleExpectations();
}

namespace
{
static std::string variableSlotToString(VariableSlot const& _slot)
{
	return _slot.variable.get().name.str();
}
}

class StackLayoutPrinter
{
public:
	StackLayoutPrinter(std::ostream& _stream, StackLayout const& _stackLayout):
	m_stream(_stream), m_stackLayout(_stackLayout)
	{
	}
	void operator()(CFG::BasicBlock const& _block, bool _isMainEntry = true)
	{
		if (_isMainEntry)
		{
			m_stream << "Entry [label=\"Entry\"];\n";
			m_stream << "Entry -> Block" << getBlockId(_block) << ";\n";
		}
		while (!m_blocksToPrint.empty())
		{
			CFG::BasicBlock const* block = *m_blocksToPrint.begin();
			m_blocksToPrint.erase(m_blocksToPrint.begin());
			printBlock(*block);
		}

	}
	void operator()(
		CFG::FunctionInfo const& _info
	)
	{
		m_stream << "FunctionEntry_" << _info.function.name.str() << " [label=\"";
		m_stream << "function " << _info.function.name.str() << "(";
		m_stream << joinHumanReadable(_info.parameters | ranges::views::transform(variableSlotToString));
		m_stream << ")";
		if (!_info.returnVariables.empty())
		{
			m_stream << " -> ";
			m_stream << joinHumanReadable(_info.returnVariables | ranges::views::transform(variableSlotToString));
		}
		m_stream << "\\l\\\n";
		Stack functionEntryStack = {FunctionReturnLabelSlot{_info.function}};
		functionEntryStack += _info.parameters | ranges::views::reverse;
		m_stream << stackToString(functionEntryStack) << "\"];\n";
		m_stream << "FunctionEntry_" << _info.function.name.str() << " -> Block" << getBlockId(*_info.entry) << ";\n";
		(*this)(*_info.entry, false);
	}

private:
	void printBlock(CFG::BasicBlock const& _block)
	{
		m_stream << "Block" << getBlockId(_block) << " [label=\"\\\n";

		// Verify that the entries of this block exit into this block.
		for (auto const& entry: _block.entries)
			std::visit(util::GenericVisitor{
				[&](CFG::BasicBlock::Jump const& _jump)
				{
					soltestAssert(_jump.target == &_block, "Invalid control flow graph.");
				},
				[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
				{
					soltestAssert(
						_conditionalJump.zero == &_block || _conditionalJump.nonZero == &_block,
						"Invalid control flow graph."
					);
				},
				[&](auto const&)
				{
					soltestAssert(false, "Invalid control flow graph.");
				}
			}, entry->exit);

		auto const& blockInfo = m_stackLayout.blockInfos.at(&_block);
		m_stream << stackToString(blockInfo.entryLayout) << "\\l\\\n";
		for (auto const& operation: _block.operations)
		{
			auto entryLayout = m_stackLayout.operationEntryLayout.at(&operation);
			m_stream << stackToString(m_stackLayout.operationEntryLayout.at(&operation)) << "\\l\\\n";
			std::visit(util::GenericVisitor{
				[&](CFG::FunctionCall const& _call) {
					m_stream << _call.function.get().name.str();
				},
				[&](CFG::BuiltinCall const& _call) {
					m_stream << _call.functionCall.get().functionName.name.str();

				},
				[&](CFG::Assignment const& _assignment) {
					m_stream << "Assignment(";
					m_stream << joinHumanReadable(_assignment.variables | ranges::views::transform(variableSlotToString));
					m_stream << ")";
				}
			}, operation.operation);
			m_stream << "\\l\\\n";
			soltestAssert(operation.input.size() <= entryLayout.size(), "Invalid Stack Layout.");
			for (size_t i = 0; i < operation.input.size(); ++i)
				entryLayout.pop_back();
			entryLayout += operation.output;
			m_stream << stackToString(entryLayout) << "\\l\\\n";
		}
		m_stream << stackToString(blockInfo.exitLayout) << "\\l\\\n";
		m_stream << "\"];\n";
		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&)
			{
				m_stream << "Block" << getBlockId(_block) << "Exit [label=\"MainExit\"];\n";
				m_stream << "Block" << getBlockId(_block) << " -> Block" << getBlockId(_block) << "Exit;\n";
			},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				m_stream << "Block" << getBlockId(_block) << " -> Block" << getBlockId(_block) << "Exit [arrowhead=none];\n";
				m_stream << "Block" << getBlockId(_block) << "Exit [label=\"";
				if (_jump.backwards)
					m_stream << "Backwards";
				m_stream << "Jump\" shape=oval];\n";
				m_stream << "Block" << getBlockId(_block) << "Exit -> Block" << getBlockId(*_jump.target) << ";\n";
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				m_stream << "Block" << getBlockId(_block) << " -> Block" << getBlockId(_block) << "Exit;\n";
				m_stream << "Block" << getBlockId(_block) << "Exit [label=\"{ ";
				m_stream << stackSlotToString(_conditionalJump.condition);
				m_stream << "| { <0> Zero | <1> NonZero }}\" shape=Mrecord];\n";
				m_stream << "Block" << getBlockId(_block);
				m_stream << "Exit:0 -> Block" << getBlockId(*_conditionalJump.zero) << ";\n";
				m_stream << "Block" << getBlockId(_block);
				m_stream << "Exit:1 -> Block" << getBlockId(*_conditionalJump.nonZero) << ";\n";
			},
			[&](CFG::BasicBlock::FunctionReturn const& _return)
			{
				m_stream << "Block" << getBlockId(_block) << "Exit [label=\"FunctionReturn[" << _return.info->function.name.str() << "]\"];\n";
				m_stream << "Block" << getBlockId(_block) << " -> Block" << getBlockId(_block) << "Exit;\n";
			},
			[&](CFG::BasicBlock::Terminated const&)
			{
				m_stream << "Block" << getBlockId(_block) << "Exit [label=\"Terminated\"];\n";
				m_stream << "Block" << getBlockId(_block) << " -> Block" << getBlockId(_block) << "Exit;\n";
			}
		}, _block.exit);
		m_stream << "\n";
	}
	size_t getBlockId(CFG::BasicBlock const& _block)
	{
		if (size_t* id = util::valueOrNullptr(m_blockIds, &_block))
			return *id;
		size_t id = m_blockIds[&_block] = m_blockCount++;
		m_blocksToPrint.emplace_back(&_block);
		return id;
	}
	std::ostream& m_stream;
	StackLayout const& m_stackLayout;
	std::map<CFG::BasicBlock const*, size_t> m_blockIds;
	size_t m_blockCount = 0;
	std::list<CFG::BasicBlock const*> m_blocksToPrint;
};

TestCase::TestResult StackLayoutGeneratorTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	ErrorList errors;
	auto [object, analysisInfo] = parse(m_source, *m_dialect, errors);
	if (!object || !analysisInfo || Error::containsErrors(errors))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		return TestResult::FatalError;
	}

	std::ostringstream output;

	std::unique_ptr<CFG> cfg = ControlFlowGraphBuilder::build(*analysisInfo, *m_dialect, *object->code);
	StackLayout stackLayout = StackLayoutGenerator::run(*cfg);

	output << "digraph CFG {\nnodesep=0.7;\nnode[shape=box];\n\n";
	StackLayoutPrinter printer{output, stackLayout};
	printer(*cfg->entry);
	for (auto function: cfg->functions)
		printer(cfg->functionInfo.at(function));
	output << "}\n";

	m_obtainedResult = output.str();

	auto result = checkResult(_stream, _linePrefix, _formatted);

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
