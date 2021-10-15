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

#include <test/libyul/EVMCodeTransformTest.h>
#include <test/libyul/Common.h>

#include <libyul/AssemblyStack.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/AnsiColorized.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;

EVMCodeTransformTest::EVMCodeTransformTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_stackOpt = m_reader.boolSetting("stackOptimization", false);
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult EVMCodeTransformTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	solidity::frontend::OptimiserSettings settings = solidity::frontend::OptimiserSettings::none();
	settings.runYulOptimiser = false;
	settings.optimizeStackAllocation = m_stackOpt;
	AssemblyStack stack(
		EVMVersion{},
		AssemblyStack::Language::StrictAssembly,
		settings,
		DebugInfoSelection::All()
	);
	if (!stack.parseAndAnalyze("", m_source))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		SourceReferenceFormatter{_stream, stack, true, false}
			.printErrorInformation(stack.errors());
		return TestResult::FatalError;
	}

	m_obtainedResult = evmasm::disassemble(stack.assemble(AssemblyStack::Machine::EVM).bytecode->bytecode, "\n");

	return checkResult(_stream, _linePrefix, _formatted);
}
