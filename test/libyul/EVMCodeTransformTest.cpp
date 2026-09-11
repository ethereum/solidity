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

#include <test/Common.h>

#include <libyul/YulStack.h>
#include <libyul/backends/evm/EthAssemblyAdapter.h>
#include <libyul/backends/evm/EVMObjectCompiler.h>

#include <libevmasm/Assembly.h>

#include <libsolutil/CommonIO.h>

using namespace solidity;
using namespace solidity::test;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

EVMCodeTransformTest::EVMCodeTransformTest(std::string const& _filename):
	EVMVersionRestrictedTestCase(_filename)
{
	m_source = m_reader.source();
	m_stackOpt = m_reader.boolSetting("stackOptimization", false);
	m_viaSSACFG = m_reader.boolSetting("viaSSACFG", false);
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult EVMCodeTransformTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	solidity::frontend::OptimiserSettings settings = solidity::frontend::OptimiserSettings::none();
	settings.runYulOptimiser = false;
	settings.optimizeStackAllocation = m_stackOpt;
	// Restrict to a single EVM version (the default one) as code generation
	// can be different from version to version.
	YulStack yulStack(
		CommonOptions::get().evmVersion(),
		settings,
		DebugInfoSelection::AllExceptExperimental()
	);
	yulStack.parseAndAnalyze("", m_source);
	if (yulStack.hasErrors())
	{
		printYulErrors(yulStack, _stream, _linePrefix, _formatted);
		return TestResult::FatalError;
	}

	evmasm::Assembly assembly{CommonOptions::get().evmVersion(), false, {}};
	EthAssemblyAdapter adapter(assembly);
	EVMObjectCompiler::compile(
		*yulStack.parserResult(),
		adapter,
		m_stackOpt,
		m_viaSSACFG
	);

	m_obtainedResult = toString(assembly);

	return checkResult(_stream, _linePrefix, _formatted);
}
