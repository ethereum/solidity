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

#include <test/libyul/ObjectCompilerTest.h>

#include <libdevcore/AnsiColorized.h>

#include <libyul/AssemblyStack.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace dev;
using namespace langutil;
using namespace yul;
using namespace yul::test;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace std;

ObjectCompilerTest::ObjectCompilerTest(string const& _filename)
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
		if (m_source.empty() && boost::algorithm::starts_with(line, "// optimize"))
			m_optimize = true;
		m_source += line + "\n";
	}
	while (getline(file, line))
		if (boost::algorithm::starts_with(line, "//"))
			m_expectation += line.substr((line.size() >= 3 && line[2] == ' ') ? 3 : 2) + "\n";
		else
			m_expectation += line + "\n";
}

bool ObjectCompilerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	AssemblyStack stack(
		EVMVersion(),
		AssemblyStack::Language::StrictAssembly,
		m_optimize ? OptimiserSettings::full() : OptimiserSettings::minimal()
	);
	if (!stack.parseAndAnalyze("source", m_source))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, stack.errors());
		return false;
	}
	stack.optimize();

	MachineAssemblyObject obj = stack.assemble(AssemblyStack::Machine::EVM);
	solAssert(obj.bytecode, "");

	m_obtainedResult = "Assembly:\n" + obj.assembly;
	if (obj.bytecode->bytecode.empty())
		m_obtainedResult += "-- empty bytecode --\n";
	else
		m_obtainedResult +=
			"Bytecode: " +
			toHex(obj.bytecode->bytecode) +
			"\nOpcodes: " +
			boost::trim_copy(dev::eth::disassemble(obj.bytecode->bytecode)) +
			"\n";

	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return false;
	}
	return true;
}

void ObjectCompilerTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void ObjectCompilerTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void ObjectCompilerTest::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		if (line.empty())
			// Avoid trailing spaces.
			_stream << boost::trim_right_copy(_linePrefix) << endl;
		else
			_stream << _linePrefix << line << endl;
}

void ObjectCompilerTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printErrorInformation(*error);
}
