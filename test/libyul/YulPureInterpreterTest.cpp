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

#include <test/libyul/YulPureInterpreterTest.h>

#include <libyul/interpreter/PureInterpreter.h>

#include <test/Common.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/YulStack.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/Visitor.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

using solidity::util::h256;

using namespace solidity::yul::interpreter;

YulPureInterpreterTest::YulPureInterpreterTest(std::string const& _filename):
	EVMVersionRestrictedTestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();

	m_config.maxTraceSize = m_reader.sizetSetting("maxTraceSize", 128);
	m_config.maxExprNesting = m_reader.sizetSetting("maxExprNesting", 64);
	m_config.maxSteps = m_reader.sizetSetting("maxSteps", 512);
	m_config.maxRecursionDepth = m_reader.sizetSetting("maxRecursionDepth", 64);

	m_printHex = m_reader.boolSetting("printHex", false);
}

TestCase::TestResult YulPureInterpreterTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	if (!parse(_stream, _linePrefix, _formatted))
		return TestResult::FatalError;

	m_obtainedResult = interpret();

	return checkResult(_stream, _linePrefix, _formatted);
}

bool YulPureInterpreterTest::parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	YulStack stack(
		solidity::test::CommonOptions::get().evmVersion(),
		solidity::test::CommonOptions::get().eofVersion(),
		YulStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::none(),
		DebugInfoSelection::All()
	);
	if (stack.parseAndAnalyze("", m_source))
	{
		m_ast = stack.parserResult()->code();
		m_analysisInfo = stack.parserResult()->analysisInfo;
		return true;
	}
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << std::endl;
		SourceReferenceFormatter{_stream, stack, true, false}
			.printErrorInformation(stack.errors());
		return false;
	}
}

std::string YulPureInterpreterTest::interpret() const
{
	std::stringstream resultStream;

	Block const& block = m_ast->root();

	PureInterpreterState state{m_config};
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(
		solidity::test::CommonOptions::get().evmVersion(),
		solidity::test::CommonOptions::get().eofVersion()
	);
	interpreter::Scope rootScope(nullptr, block);

	PureInterpreter interpreter(state, dialect, rootScope, 0);
	ExecutionResult result = interpreter.execute(block.statements);
	VariableValuesMap const& outerMostVariables = interpreter.allVariables();

	dumpExecutionData(
		resultStream,
		result,
		state,
		outerMostVariables
	);

	return resultStream.str();
}

void YulPureInterpreterTest::dumpExecutionData(
	std::ostream& _stream,
	interpreter::ExecutionResult _result,
	interpreter::PureInterpreterState const& _state,
	VariableValuesMap const& _outerMostVariables
) const
{
	_stream << "Execution result: ";
	dumpExecutionResult(_stream, _result);
	_stream << std::endl;

	_stream << "Outer most variable values:" << std::endl;
	dumpVariables(_stream, _outerMostVariables);
	_stream << std::endl;

	_state.dumpTraces(_stream);
}

void YulPureInterpreterTest::dumpExecutionResult(std::ostream& _stream, interpreter::ExecutionResult _result) const
{
	if (_result) {
		_stream << "ExecutionOk";
	} else {
		_stream << std::visit(GenericVisitor {
			[&](ExplicitlyTerminated) { return "ExplicitlyTerminated"; },
			[&](StepLimitReached) { return "StepLimitReached"; },
			[&](RecursionDepthLimitReached) { return "RecursionDepthLimitReached"; },
			[&](ExpressionNestingLimitReached) { return "ExpressionNestingLimitReached"; },
			[&](ImpureBuiltinEncountered) { return "ImpureBuiltinEncountered"; },
			[&](UnlimitedLiteralEncountered) { return "UnlimitedLiteralEncountered"; },
			[&](TraceLimitReached) { return "TraceLimitReached"; }
		}, _result.error());
	}
}

void YulPureInterpreterTest::dumpVariables(
	std::ostream& _stream,
	interpreter::VariableValuesMap const& _variables
) const
{
	static std::string_view const INDENT = "  ";

	// _variables are sorted by id. We should sort them by name
	std::map<std::string, u256> sortedVariables;
	for (auto const& [name, value]: _variables)
		sortedVariables[name.str()] = value;

	for (auto const& [name, value]: sortedVariables)
	{
		_stream << INDENT << name << " = ";
		dumpValue(_stream, value);
		_stream << std::endl;
	}
}

void YulPureInterpreterTest::dumpValue(
	std::ostream& _stream,
	u256 _value
) const
{
	if (m_printHex) _stream << "0x" << h256(_value).hex();
	else _stream << _value;
}
