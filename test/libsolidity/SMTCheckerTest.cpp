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

#include <test/libsolidity/SMTCheckerTest.h>
#include <test/Common.h>

#include <range/v3/action/remove_if.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

SMTCheckerTest::SMTCheckerTest(std::string const& _filename): SyntaxTest(_filename, EVMVersion{})
{
	auto contract = m_reader.stringSetting("SMTContract", "");
	if (!contract.empty())
		m_modelCheckerSettings.contracts.contracts[""] = {contract};

	auto extCallsMode = ModelCheckerExtCalls::fromString(m_reader.stringSetting("SMTExtCalls", "untrusted"));
	if (extCallsMode)
		m_modelCheckerSettings.externalCalls = *extCallsMode;
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT external calls mode."));

	auto const& showUnproved = m_reader.stringSetting("SMTShowUnproved", "yes");
	if (showUnproved == "no")
		m_modelCheckerSettings.showUnproved = false;
	else if (showUnproved == "yes")
		m_modelCheckerSettings.showUnproved = true;
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT \"show unproved\" choice."));

	auto const& showUnsupported = m_reader.stringSetting("SMTShowUnsupported", "yes");
	if (showUnsupported == "no")
		m_modelCheckerSettings.showUnsupported = false;
	else if (showUnsupported == "yes")
		m_modelCheckerSettings.showUnsupported = true;
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT \"show unsupported\" choice."));

	m_modelCheckerSettings.solvers = smtutil::SMTSolverChoice::None();
	auto const& choice = m_reader.stringSetting("SMTSolvers", "z3");
	if (choice == "none")
		m_modelCheckerSettings.solvers = smtutil::SMTSolverChoice::None();
	else if (!m_modelCheckerSettings.solvers.setSolver(choice))
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT solver choice."));

	m_modelCheckerSettings.solvers &= ModelChecker::availableSolvers();

	/// Underflow and Overflow are not enabled by default for Solidity >=0.8.7,
	/// so we explicitly enable all targets for the tests,
	/// if the targets were not explicitly set by the test.
	auto targets = ModelCheckerTargets::fromString(m_reader.stringSetting("SMTTargets", "all"));
	if (targets)
		m_modelCheckerSettings.targets = *targets;
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT targets."));

	auto engine = ModelCheckerEngine::fromString(m_reader.stringSetting("SMTEngine", "all"));
	if (engine)
		m_modelCheckerSettings.engine = *engine;
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT engine choice."));

	if (m_modelCheckerSettings.solvers.none() || m_modelCheckerSettings.engine.none())
		m_shouldRun = false;

	auto const& ignoreCex = m_reader.stringSetting("SMTIgnoreCex", "yes");
	if (ignoreCex == "no")
		m_ignoreCex = false;
	else if (ignoreCex == "yes")
		m_ignoreCex = true;
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT counterexample choice."));

	static auto removeInv = [](std::vector<SyntaxTestError>&& errors) {
		std::vector<SyntaxTestError> filtered;
		for (auto&& e: errors)
			if (e.errorId != 1180_error)
				filtered.emplace_back(e);
		return filtered;
	};
	if (m_modelCheckerSettings.invariants.invariants.empty())
		m_expectations = removeInv(std::move(m_expectations));

	auto const& ignoreInv = m_reader.stringSetting("SMTIgnoreInv", "yes");
	if (ignoreInv == "no")
		m_modelCheckerSettings.invariants = ModelCheckerInvariants::All();
	else if (ignoreInv == "yes")
		m_modelCheckerSettings.invariants = ModelCheckerInvariants::None();
	else
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid SMT invariant choice."));

	auto const& ignoreOSSetting = m_reader.stringSetting("SMTIgnoreOS", "none");
	for (std::string const& os: ignoreOSSetting | ranges::views::split(',') | ranges::to<std::vector<std::string>>())
	{
#ifdef __APPLE__
		if (os == "macos")
			m_shouldRun = false;
#elif _WIN32
		if (os == "windows")
			m_shouldRun = false;
#elif __linux__
		if (os == "linux")
			m_shouldRun = false;
#endif
	}

	auto const& bmcLoopIterations = m_reader.sizetSetting("BMCLoopIterations", 1);
	m_modelCheckerSettings.bmcLoopIterations = std::optional<unsigned>{bmcLoopIterations};
}

void SMTCheckerTest::setupCompiler(CompilerStack& _compiler)
{
	SyntaxTest::setupCompiler(_compiler);

	_compiler.setModelCheckerSettings(m_modelCheckerSettings);
}

void SMTCheckerTest::filterObtainedErrors()
{
	SyntaxTest::filterObtainedErrors();
	m_unfilteredErrorList = m_errorList;

	static auto removeCex = [](std::vector<SyntaxTestError>& errors) {
		for (auto& e: errors)
			if (
				auto cexPos = e.message.find("\\nCounterexample");
				cexPos != std::string::npos
			)
				e.message = e.message.substr(0, cexPos);
	};

	if (m_ignoreCex)
	{
		removeCex(m_expectations);
		removeCex(m_errorList);
	}
}

void SMTCheckerTest::printUpdatedExpectations(std::ostream &_stream, const std::string &_linePrefix) const {
	if (!m_unfilteredErrorList.empty())
		printErrorList(_stream, m_unfilteredErrorList, _linePrefix, false);
	else
		CommonSyntaxTest::printUpdatedExpectations(_stream, _linePrefix);
}
