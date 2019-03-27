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

#include <test/libyul/YulInterpreterTest.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/Options.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmParser.h>
#include <libyul/AssemblyStack.h>
#include <libyul/AsmAnalysisInfo.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libdevcore/AnsiColorized.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace dev;
using namespace langutil;
using namespace yul;
using namespace yul::test;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace std;

YulInterpreterTest::YulInterpreterTest(string const& _filename)
{
	boost::filesystem::path path(_filename);

	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test case: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string line;
	while (getline(file, line))
	{
		if (boost::algorithm::starts_with(line, "// ----"))
			break;
		m_source += std::move(line) + "\n";
	}
	while (getline(file, line))
		if (boost::algorithm::starts_with(line, "// "))
			m_expectation += line.substr(3) + "\n";
		else
			m_expectation += line + "\n";
}

bool YulInterpreterTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	if (!parse(_stream, _linePrefix, _formatted))
		return false;

	m_obtainedResult = interpret();

	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		// TODO could compute a simple diff with highlighted lines
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return false;
	}
	return true;
}

void YulInterpreterTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void YulInterpreterTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void YulInterpreterTest::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		_stream << _linePrefix << line << endl;
}

bool YulInterpreterTest::parse(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	AssemblyStack stack(
		dev::test::Options::get().evmVersion(),
		AssemblyStack::Language::StrictAssembly,
		dev::solidity::OptimiserSettings::none()
	);
	if (stack.parseAndAnalyze("", m_source))
	{
		m_ast = stack.parserResult()->code;
		m_analysisInfo = stack.parserResult()->analysisInfo;
		return true;
	}
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, stack.errors());
		return false;
	}
}

string YulInterpreterTest::interpret()
{
	InterpreterState state;
	state.maxTraceSize = 10000;
	Interpreter interpreter(state);
	try
	{
		interpreter(*m_ast);
	}
	catch (InterpreterTerminated const&)
	{
	}

	stringstream result;
	result << "Trace:" << endl;;
	for (auto const& line: interpreter.trace())
		result << "  " << line << endl;
	result << "Memory dump:\n";
	for (size_t i = 0; i < state.memory.size(); i += 0x20)
		result << "  " << std::hex << std::setw(4) << i << ": " << toHex(bytesConstRef(state.memory.data() + i, 0x20).toBytes()) << endl;
	result << "Storage dump:" << endl;
	for (auto const& slot: state.storage)
		result << "  " << slot.first.hex() << ": " << slot.second.hex() << endl;
	return result.str();
}

void YulInterpreterTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printExceptionInformation(
			*error,
			(error->type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}
