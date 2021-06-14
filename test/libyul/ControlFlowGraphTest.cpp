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

#include <test/libyul/ControlFlowGraphTest.h>
#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/Object.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/Visitor.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;

ControlFlowGraphTest::ControlFlowGraphTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	m_dialect = &dialect(dialectName, solidity::test::CommonOptions::get().evmVersion());
	m_expectation = m_reader.simpleExpectations();
}

namespace
{
static std::string stackSlotToString(StackSlot const& _slot)
{
	return std::visit(util::GenericVisitor{
		[](FunctionCallReturnLabelSlot const& _ret) -> std::string { return "RET[" + _ret.call.get().functionName.name.str() + "]"; },
		[](FunctionReturnLabelSlot const&) -> std::string { return "RET"; },
		[](VariableSlot const& _var) { return _var.variable.str(); },
		[](LiteralSlot const& _lit) { return util::toCompactHexWithPrefix(_lit.value); },
		[](JunkSlot const&) -> std::string { return "JUNK"; }
	}, _slot);
}

static std::string stackToString(Stack const& _stack)
{
	std::string result("[ ");
	for (auto const& slot: _stack)
		result += stackSlotToString(slot) + ' ';
	result += ']';
	return result;
}
}

class ControlFlowGraphPrinter
{
public:
	ControlFlowGraphPrinter(std::ostream& _stream):
	m_stream(_stream)
	{
	}
	void operator()(CFG::BasicBlock const& _block)
	{
		getBlockId(_block);
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
		m_stream << m_indent << "function " << _info.function->name.str() << "(";
		// TODO m_stream << joinHumanReadable(_info.parameters | ranges::views::transform(variableSlotToString));
		m_stream << ")";
		/* TODO if (!_info.returnVariables.empty())
		{
			m_stream << " -> ";
			m_stream << joinHumanReadable(_info.returnVariables | ranges::views::transform(variableSlotToString));
		}*/
		m_stream << ":\n";
		ScopedSaveAndRestore linePrefixRestore(m_indent, m_indent + "  ");
		(*this)(*_info.entry);
	}

private:
	void printBlock(CFG::BasicBlock const& _block)
	{
		m_stream << m_indent << "Block " << getBlockId(_block) << ":\n";
		ScopedSaveAndRestore linePrefixRestore(m_indent, m_indent + "  ");

		m_stream << m_indent << "Entries: ";
		if (_block.entries.empty())
			m_stream << "None\n";
		else
			m_stream << joinHumanReadable(_block.entries | ranges::views::transform([&](auto const* _entry) {
				return to_string(getBlockId(*_entry));
			})) << "\n";

		for (auto const& operation: _block.operations)
		{
			m_stream << m_indent;
			std::visit(util::GenericVisitor{
				[&](CFG::FunctionCall const& _call) {
					m_stream << _call.functionCall.get().functionName.name.str() << ": ";
				},
				[&](CFG::BuiltinCall const& _call) {
					m_stream << _call.functionCall.get().functionName.name.str() << ": ";

				},
				[&](CFG::Assignment const&) {
					m_stream << "Assignment: ";
				}
			}, operation.operation);
			m_stream << stackToString(operation.input) << " => " << stackToString(operation.output) << "\n";
		}
		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&)
			{
				m_stream << m_indent << "MainExit\n";
			},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				m_stream << m_indent << "Jump" << (_jump.backwards ? " (backwards): " : ": ") << getBlockId(*_jump.target) << "\n";
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				m_stream << m_indent << "ConditionalJump " << stackSlotToString(_conditionalJump.condition) << ":\n";
				m_stream << m_indent << "  NonZero: " << getBlockId(*_conditionalJump.nonZero) << "\n";
				m_stream << m_indent << "  Zero: " << getBlockId(*_conditionalJump.zero) << "\n";
			},
			[&](CFG::BasicBlock::FunctionReturn const& _return)
			{
				m_stream << m_indent << "FunctionReturn of " << _return.info->function->name.str() << "\n";
			},
			[&](CFG::BasicBlock::Terminated const&)
			{
				m_stream << m_indent << "Terminated\n";
			}
		}, _block.exit);
	}
	size_t getBlockId(CFG::BasicBlock const& _block)
	{
		if (size_t *id = util::valueOrNullptr(m_blockIds, &_block))
			return *id;
		size_t id = m_blockIds[&_block] = m_blockCount++;
		m_blocksToPrint.emplace_back(&_block);
		return id;
	}
	std::ostream& m_stream;
	std::string m_indent;
	std::map<CFG::BasicBlock const*, size_t> m_blockIds;
	size_t m_blockCount = 0;
	std::list<CFG::BasicBlock const*> m_blocksToPrint;
};

TestCase::TestResult ControlFlowGraphTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	ErrorList errors;
	auto [object, analysisInfo] = parse(m_source, *m_dialect, errors);
	if (!object || !analysisInfo || !Error::containsOnlyWarnings(errors))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(errors);
		return TestResult::FatalError;
	}

	std::ostringstream output;

	std::unique_ptr<CFG> cfg = ControlFlowGraphBuilder::build(*analysisInfo, *m_dialect, *object->code);

	ControlFlowGraphPrinter{output}(*cfg->entry);
	for (auto function: cfg->functions)
		ControlFlowGraphPrinter{output}(cfg->functionInfo.at(function));

	m_obtainedResult = output.str();

	return checkResult(_stream, _linePrefix, _formatted);
}
