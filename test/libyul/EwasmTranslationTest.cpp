// SPDX-License-Identifier: GPL-3.0

#include <test/libyul/EwasmTranslationTest.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/Common.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/wasm/EVMToEwasmTranslator.h>
#include <libyul/AsmParser.h>
#include <libyul/AssemblyStack.h>
#include <libyul/AsmAnalysisInfo.h>

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
		EVMDialect::strictAssemblyForEVMObjects(solidity::test::CommonOptions::get().evmVersion())
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
	AssemblyStack stack(
		solidity::test::CommonOptions::get().evmVersion(),
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::none()
	);
	if (stack.parseAndAnalyze("", m_source))
	{
		m_object = stack.parserResult();
		return true;
	}
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, stack.errors());
		return false;
	}
}

string EwasmTranslationTest::interpret()
{
	InterpreterState state;
	state.maxTraceSize = 10000;
	state.maxSteps = 100000;
	WasmDialect dialect;
	Interpreter interpreter(state, dialect);
	try
	{
		interpreter(*m_object->code);
	}
	catch (InterpreterTerminatedGeneric const&)
	{
	}

	stringstream result;
	state.dumpTraceAndState(result);
	return result.str();
}

void EwasmTranslationTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printErrorInformation(*error);
}
