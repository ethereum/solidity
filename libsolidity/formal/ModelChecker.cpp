// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/formal/ModelChecker.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

ModelChecker::ModelChecker(
	ErrorReporter& _errorReporter,
	map<h256, string> const& _smtlib2Responses,
	ReadCallback::Callback const& _smtCallback,
	smtutil::SMTSolverChoice _enabledSolvers
):
	m_context(),
	m_bmc(m_context, _errorReporter, _smtlib2Responses, _smtCallback, _enabledSolvers),
	m_chc(m_context, _errorReporter, _smtlib2Responses, _smtCallback, _enabledSolvers)
{
}

void ModelChecker::analyze(SourceUnit const& _source)
{
	if (!_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
		return;

	m_chc.analyze(_source);
	m_bmc.analyze(_source, m_chc.safeAssertions());
}

vector<string> ModelChecker::unhandledQueries()
{
	return m_bmc.unhandledQueries() + m_chc.unhandledQueries();
}

solidity::smtutil::SMTSolverChoice ModelChecker::availableSolvers()
{
	smtutil::SMTSolverChoice available = smtutil::SMTSolverChoice::None();
#ifdef HAVE_Z3
	available.z3 = true;
#endif
#ifdef HAVE_CVC4
	available.cvc4 = true;
#endif
	return available;
}
