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

#include <libsolidity/formal/ModelChecker.h>
#ifdef HAVE_Z3
#include <libsmtutil/Z3Interface.h>
#endif

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

ModelChecker::ModelChecker(
	ErrorReporter& _errorReporter,
	map<h256, string> const& _smtlib2Responses,
	ModelCheckerSettings _settings,
	ReadCallback::Callback const& _smtCallback,
	smtutil::SMTSolverChoice _enabledSolvers
):
	m_settings(_settings),
	m_context(),
	m_bmc(m_context, _errorReporter, _smtlib2Responses, _smtCallback, _enabledSolvers, _settings.timeout),
	m_chc(m_context, _errorReporter, _smtlib2Responses, _smtCallback, _enabledSolvers, _settings.timeout)
{
}

void ModelChecker::analyze(SourceUnit const& _source)
{
	if (!_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
		return;

	if (m_settings.engine.chc)
		m_chc.analyze(_source);

	auto solvedTargets = m_chc.safeTargets();
	for (auto const& target: m_chc.unsafeTargets())
		solvedTargets[target.first] += target.second;

	if (m_settings.engine.bmc)
		m_bmc.analyze(_source, solvedTargets);
}

vector<string> ModelChecker::unhandledQueries()
{
	return m_bmc.unhandledQueries() + m_chc.unhandledQueries();
}

solidity::smtutil::SMTSolverChoice ModelChecker::availableSolvers()
{
	smtutil::SMTSolverChoice available = smtutil::SMTSolverChoice::None();
#ifdef HAVE_Z3
	available.z3 = solidity::smtutil::Z3Interface::available();
#endif
#ifdef HAVE_CVC4
	available.cvc4 = true;
#endif
	return available;
}
