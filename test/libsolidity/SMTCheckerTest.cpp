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

#include <test/libsolidity/SMTCheckerTest.h>
#include <test/Options.h>

#include <libsolidity/formal/ModelChecker.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

SMTCheckerTest::SMTCheckerTest(string const& _filename, langutil::EVMVersion _evmVersion): SyntaxTest(_filename, _evmVersion)
{
	if (m_settings.count("SMTSolvers"))
	{
		auto const& choice = m_settings.at("SMTSolvers");
		if (choice == "any")
			m_enabledSolvers = smt::SMTSolverChoice::All();
		else if (choice == "z3")
			m_enabledSolvers = smt::SMTSolverChoice::Z3();
		else if (choice == "cvc4")
			m_enabledSolvers = smt::SMTSolverChoice::CVC4();
		else if (choice == "none")
			m_enabledSolvers = smt::SMTSolverChoice::None();
		else
			BOOST_THROW_EXCEPTION(runtime_error("Invalid SMT solver choice."));
	}
	else
		m_enabledSolvers = smt::SMTSolverChoice::All();
}

TestCase::TestResult SMTCheckerTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	setupCompiler();
	compiler().setSMTSolverChoice(m_enabledSolvers);
	parseAndAnalyze();
	filterObtainedErrors();

	return printExpectationAndError(_stream, _linePrefix, _formatted) ? TestResult::Success : TestResult::Failure;
}

bool SMTCheckerTest::validateSettings(langutil::EVMVersion _evmVersion)
{
	auto available = ModelChecker::availableSolvers();
	if (!available.z3)
		m_enabledSolvers.z3 = false;
	if (!available.cvc4)
		m_enabledSolvers.cvc4 = false;

	if (m_enabledSolvers.none())
		return false;

	return SyntaxTest::validateSettings(_evmVersion);
}
