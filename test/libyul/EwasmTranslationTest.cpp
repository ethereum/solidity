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

#include <test/libyul/EwasmTranslationTest.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/Options.h>

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


EwasmTranslationTest::EwasmTranslationTest(string const& _filename)
{
	boost::filesystem::path path(_filename);

	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test case: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSourceAndSettings(file);
	m_expectation = parseSimpleExpectations(file);
}

TestCase::TestResult EwasmTranslationTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	if (!parse(_stream, _linePrefix, _formatted))
		return TestResult::FatalError;

	*m_object = EVMToEwasmTranslator(
		EVMDialect::strictAssemblyForEVMObjects(solidity::test::Options::get().evmVersion())
	).run(*m_object);

	// Add call to "main()".
	m_object->code->statements.emplace_back(
		ExpressionStatement{{}, FunctionCall{{}, Identifier{{}, "main"_yulstring}, {}}}
	);

	m_obtainedResult = interpret();

	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		// TODO could compute a simple diff with highlighted lines
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return TestResult::Failure;
	}
	return TestResult::Success;
}

void EwasmTranslationTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void EwasmTranslationTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void EwasmTranslationTest::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		_stream << _linePrefix << line << endl;
}

bool EwasmTranslationTest::parse(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	AssemblyStack stack(
		solidity::test::Options::get().evmVersion(),
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
