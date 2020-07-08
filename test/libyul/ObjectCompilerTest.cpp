// SPDX-License-Identifier: GPL-3.0

#include <test/libyul/ObjectCompilerTest.h>

#include <libsolutil/AnsiColorized.h>

#include <libyul/AssemblyStack.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/SourceReferenceFormatter.h>

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

ObjectCompilerTest::ObjectCompilerTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_optimize = m_reader.boolSetting("optimize", false);
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult ObjectCompilerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
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
		return TestResult::FatalError;
	}
	stack.optimize();

	MachineAssemblyObject obj = stack.assemble(AssemblyStack::Machine::EVM);
	solAssert(obj.bytecode, "");
	solAssert(obj.sourceMappings, "");

	m_obtainedResult = "Assembly:\n" + obj.assembly;
	if (obj.bytecode->bytecode.empty())
		m_obtainedResult += "-- empty bytecode --\n";
	else
		m_obtainedResult +=
			"Bytecode: " +
			toHex(obj.bytecode->bytecode) +
			"\nOpcodes: " +
			boost::trim_copy(evmasm::disassemble(obj.bytecode->bytecode)) +
			"\nSourceMappings:" +
			(obj.sourceMappings->empty() ? "" : " " + *obj.sourceMappings) +
			"\n";

	return checkResult(_stream, _linePrefix, _formatted);
}

void ObjectCompilerTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printErrorInformation(*error);
}
