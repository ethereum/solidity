// SPDX-License-Identifier: GPL-3.0

#include <test/libsolidity/SMTCheckerTest.h>
#include <test/Common.h>

#include <libsolidity/formal/ModelChecker.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

SMTCheckerTest::SMTCheckerTest(string const& _filename, langutil::EVMVersion _evmVersion): SyntaxTest(_filename, _evmVersion)
{
	auto const& choice = m_reader.stringSetting("SMTSolvers", "any");
	if (choice == "any")
		m_enabledSolvers = smtutil::SMTSolverChoice::All();
	else if (choice == "z3")
		m_enabledSolvers = smtutil::SMTSolverChoice::Z3();
	else if (choice == "cvc4")
		m_enabledSolvers = smtutil::SMTSolverChoice::CVC4();
	else if (choice == "none")
		m_enabledSolvers = smtutil::SMTSolverChoice::None();
	else
		BOOST_THROW_EXCEPTION(runtime_error("Invalid SMT solver choice."));

	auto available = ModelChecker::availableSolvers();
	if (!available.z3)
		m_enabledSolvers.z3 = false;
	if (!available.cvc4)
		m_enabledSolvers.cvc4 = false;

	if (m_enabledSolvers.none())
		m_shouldRun = false;
}

TestCase::TestResult SMTCheckerTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	setupCompiler();
	compiler().setSMTSolverChoice(m_enabledSolvers);
	parseAndAnalyze();
	filterObtainedErrors();

	return conclude(_stream, _linePrefix, _formatted);
}
