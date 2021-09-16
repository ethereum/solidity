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

#include <test/libyul/ObjectCompilerTest.h>

#include <test/libsolidity/util/SoltestErrors.h>

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
	m_optimisationPreset = m_reader.enumSetting<OptimisationPreset>(
		"optimizationPreset",
		{
			{"none", OptimisationPreset::None},
			{"minimal", OptimisationPreset::Minimal},
			{"standard", OptimisationPreset::Standard},
			{"full", OptimisationPreset::Full},
		},
		"minimal"
	);
	m_wasm = m_reader.boolSetting("wasm", false);
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult ObjectCompilerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	AssemblyStack stack(
		EVMVersion(),
		m_wasm ? AssemblyStack::Language::Ewasm : AssemblyStack::Language::StrictAssembly,
		OptimiserSettings::preset(m_optimisationPreset)
	);
	if (!stack.parseAndAnalyze("source", m_source))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		SourceReferenceFormatter{_stream, stack, true, false}
			.printErrorInformation(stack.errors());
		return TestResult::FatalError;
	}
	stack.optimize();

	if (m_wasm)
	{
		MachineAssemblyObject obj = stack.assemble(AssemblyStack::Machine::Ewasm);
		solAssert(obj.bytecode, "");

		m_obtainedResult = "Text:\n" + obj.assembly + "\n";
		m_obtainedResult += "Binary:\n" + util::toHex(obj.bytecode->bytecode) + "\n";
	}
	else
	{
		MachineAssemblyObject obj = stack.assemble(AssemblyStack::Machine::EVM);
		solAssert(obj.bytecode, "");
		solAssert(obj.sourceMappings, "");

		m_obtainedResult = "Assembly:\n" + obj.assembly;
		if (obj.bytecode->bytecode.empty())
			m_obtainedResult += "-- empty bytecode --\n";
		else
			m_obtainedResult +=
				"Bytecode: " +
				util::toHex(obj.bytecode->bytecode) +
				"\nOpcodes: " +
				boost::trim_copy(evmasm::disassemble(obj.bytecode->bytecode)) +
				"\nSourceMappings:" +
				(obj.sourceMappings->empty() ? "" : " " + *obj.sourceMappings) +
				"\n";
	}

	return checkResult(_stream, _linePrefix, _formatted);
}
