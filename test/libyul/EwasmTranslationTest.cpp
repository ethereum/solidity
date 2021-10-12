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

#include <test/libyul/EwasmTranslationTest.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/Common.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/wasm/EVMToEwasmTranslator.h>
#include <libyul/AssemblyStack.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>
#include <libyul/Object.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/AnsiColorized.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;


EwasmTranslationTest::EwasmTranslationTest(string const& _filename):
	EVMVersionRestrictedTestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult EwasmTranslationTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	if (!parse(_stream, _linePrefix, _formatted))
		return TestResult::FatalError;

	*m_object = EVMToEwasmTranslator(
		EVMDialect::strictAssemblyForEVMObjects(solidity::test::CommonOptions::get().evmVersion()),
		m_stack
	).run(*m_object);

	// Add call to "main()".
	m_object->code->statements.emplace_back(
		ExpressionStatement{{}, FunctionCall{{}, Identifier{{}, "main"_yulstring}, {}}}
	);

	m_obtainedResult = interpret();

	return checkResult(_stream, _linePrefix, _formatted);
}

bool EwasmTranslationTest::parse(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	m_stack = AssemblyStack(
		solidity::test::CommonOptions::get().evmVersion(),
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::none(),
		DebugInfoSelection::All()
	);
	if (m_stack.parseAndAnalyze("", m_source))
	{
		m_object = m_stack.parserResult();
		return true;
	}
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		SourceReferenceFormatter{_stream, m_stack, true, false}
			.printErrorInformation(m_stack.errors());
		return false;
	}
}

string EwasmTranslationTest::interpret()
{
	InterpreterState state;
	state.maxTraceSize = 10000;
	state.maxSteps = 1000000;
	state.maxExprNesting = 64;
	try
	{
		Interpreter::run(state, WasmDialect{}, *m_object->code);
	}
	catch (InterpreterTerminatedGeneric const&)
	{
	}

	stringstream result;
	state.dumpTraceAndState(result);
	return result.str();
}
