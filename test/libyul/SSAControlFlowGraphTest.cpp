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

#include <test/libyul/SSAControlFlowGraphTest.h>
#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/backends/evm/SSAControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/Object.h>

#include <libsolutil/AnsiColorized.h>

#ifdef ISOLTEST
#include <boost/process.hpp>
#endif

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

std::unique_ptr<TestCase> SSAControlFlowGraphTest::create(TestCase::Config const& _config) {
	return std::make_unique<SSAControlFlowGraphTest>(_config.filename);
}

SSAControlFlowGraphTest::SSAControlFlowGraphTest(std::string const& _filename): TestCase(_filename)
{
	m_source = m_reader.source();
	auto dialectName = m_reader.stringSetting("dialect", "evm");
	m_dialect = &dialect(
		dialectName,
		solidity::test::CommonOptions::get().evmVersion(),
		solidity::test::CommonOptions::get().eofVersion()
	);
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult SSAControlFlowGraphTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	ErrorList errors;
	auto [object, analysisInfo] = parse(m_source, *m_dialect, errors);
	if (!object || !analysisInfo || Error::containsErrors(errors))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << std::endl;
		return TestResult::FatalError;
	}

	auto info = AsmAnalyzer::analyzeStrictAssertCorrect(*m_dialect, *object);
	std::unique_ptr<ControlFlow> controlFlow = SSAControlFlowGraphBuilder::build(
		info,
		*m_dialect,
		object->code()->root()
	);
	ControlFlowLiveness liveness(*controlFlow);
	m_obtainedResult = controlFlow->toDot(&liveness);

	auto result = checkResult(_stream, _linePrefix, _formatted);

#ifdef ISOLTEST
	char* graphDisplayer = nullptr;
	// The environment variables specify an optional command that will receive the graph encoded in DOT through stdin.
	// Examples for suitable commands are ``dot -Tx11:cairo`` or ``xdot -``.
	if (result == TestResult::Failure)
		// ISOLTEST_DISPLAY_GRAPHS_ON_FAILURE_COMMAND will run on all failing tests (intended for use during modifications).
		graphDisplayer = getenv("ISOLTEST_DISPLAY_GRAPHS_ON_FAILURE_COMMAND");
	else if (result == TestResult::Success)
		// ISOLTEST_DISPLAY_GRAPHS_ON_FAILURE_COMMAND will run on all succeeding tests (intended for use during reviews).
		graphDisplayer = getenv("ISOLTEST_DISPLAY_GRAPHS_ON_SUCCESS_COMMAND");

	if (graphDisplayer)
	{
		if (result == TestResult::Success)
			std::cout << std::endl << m_source << std::endl;
		boost::process::opstream pipe;
		boost::process::child child(graphDisplayer, boost::process::std_in < pipe);

		pipe << m_obtainedResult;
		pipe.flush();
		pipe.pipe().close();
		if (result == TestResult::Success)
			child.wait();
		else
			child.detach();
	}
#endif

	return result;
}
