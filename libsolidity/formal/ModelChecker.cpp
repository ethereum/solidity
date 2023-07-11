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
#ifdef HAVE_Z3_DLOPEN
#include <z3_version.h>
#endif

#if defined(__linux) || defined(__APPLE__)
#include <boost/process.hpp>
#endif

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::smtutil;

ModelChecker::ModelChecker(
	ErrorReporter& _errorReporter,
	langutil::CharStreamProvider const& _charStreamProvider,
	map<h256, string> const& _smtlib2Responses,
	ModelCheckerSettings _settings,
	ReadCallback::Callback const& _smtCallback
):
	m_errorReporter(_errorReporter),
	m_settings(std::move(_settings)),
	m_context(),
	m_bmc(m_context, m_uniqueErrorReporter, m_unsupportedErrorReporter, _smtlib2Responses, _smtCallback, m_settings, _charStreamProvider),
	m_chc(m_context, m_uniqueErrorReporter, m_unsupportedErrorReporter, _smtlib2Responses, _smtCallback, m_settings, _charStreamProvider)
{
}

// TODO This should be removed for 0.9.0.
bool ModelChecker::isPragmaPresent(vector<shared_ptr<SourceUnit>> const& _sources)
{
	return ranges::any_of(_sources, [](auto _source) {
		return _source && _source->annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker);
	});
}

void ModelChecker::checkRequestedSourcesAndContracts(vector<shared_ptr<SourceUnit>> const& _sources)
{
	map<string, set<string>> exist;
	for (auto const& source: _sources)
		for (auto node: source->nodes())
			if (auto contract = dynamic_pointer_cast<ContractDefinition>(node))
				exist[contract->sourceUnitName()].insert(contract->name());

	// Requested sources
	for (auto const& sourceName: m_settings.contracts.contracts | ranges::views::keys)
	{
		if (!exist.count(sourceName))
		{
			m_uniqueErrorReporter.warning(
				9134_error,
				SourceLocation(),
				"Requested source \"" + sourceName + "\" does not exist."
			);
			continue;
		}
		auto const& source = exist.at(sourceName);
		// Requested contracts in source `s`.
		for (auto const& contract: m_settings.contracts.contracts.at(sourceName))
			if (!source.count(contract))
				m_uniqueErrorReporter.warning(
					7400_error,
					SourceLocation(),
					"Requested contract \"" + contract + "\" does not exist in source \"" + sourceName + "\"."
				);
	}
}

void ModelChecker::analyze(SourceUnit const& _source)
{
	// TODO This should be removed for 0.9.0.
	if (_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
	{
		PragmaDirective const* smtPragma = nullptr;
		for (auto node: _source.nodes())
			if (auto pragma = dynamic_pointer_cast<PragmaDirective>(node))
				if (
					pragma->literals().size() >= 2 &&
					pragma->literals().at(1) == "SMTChecker"
				)
				{
					smtPragma = pragma.get();
					break;
				}
		solAssert(smtPragma, "");
		m_uniqueErrorReporter.warning(
			5523_error,
			smtPragma->location(),
			"The SMTChecker pragma has been deprecated and will be removed in the future. "
			"Please use the \"model checker engine\" compiler setting to activate the SMTChecker instead. "
			"If the pragma is enabled, all engines will be used."
		);
	}

	if (m_settings.engine.none())
		return;

	if (m_settings.engine.chc)
		m_chc.analyze(_source);

	map<ASTNode const*, set<VerificationTargetType>, smt::EncodingContext::IdCompare> solvedTargets;

	for (auto const& [node, targets]: m_chc.safeTargets())
		for (auto const& target: targets)
			solvedTargets[node].insert(target.type);

	for (auto const& [node, targets]: m_chc.unsafeTargets())
		solvedTargets[node] += targets | ranges::views::keys;

	if (m_settings.engine.bmc)
		m_bmc.analyze(_source, solvedTargets);

	if (m_settings.showUnsupported)
	{
		m_errorReporter.append(m_unsupportedErrorReporter.errors());
		m_unsupportedErrorReporter.clear();
	}
	else if (!m_unsupportedErrorReporter.errors().empty())
		m_errorReporter.warning(
			5724_error,
			{},
			"SMTChecker: " +
			to_string(m_unsupportedErrorReporter.errors().size()) +
			" unsupported language feature(s)."
			" Enable the model checker option \"show unsupported\" to see all of them."
		);

	m_errorReporter.append(m_uniqueErrorReporter.errors());
	m_uniqueErrorReporter.clear();
}

vector<string> ModelChecker::unhandledQueries()
{
	return m_bmc.unhandledQueries() + m_chc.unhandledQueries();
}

SMTSolverChoice ModelChecker::availableSolvers()
{
	smtutil::SMTSolverChoice available = smtutil::SMTSolverChoice::SMTLIB2();
#if defined(__linux) || defined(__APPLE__)
	available.eld = !boost::process::search_path("eld").empty();
#endif
#ifdef HAVE_Z3
	available.z3 = solidity::smtutil::Z3Interface::available();
#endif
#ifdef HAVE_CVC4
	available.cvc4 = true;
#endif
	return available;
}

SMTSolverChoice ModelChecker::checkRequestedSolvers(SMTSolverChoice _enabled, ErrorReporter& _errorReporter)
{
	SMTSolverChoice availableSolvers{ModelChecker::availableSolvers()};

	if (_enabled.cvc4 && !availableSolvers.cvc4)
	{
		_enabled.cvc4 = false;
		_errorReporter.warning(
			4902_error,
			SourceLocation(),
			"Solver CVC4 was selected for SMTChecker but it is not available."
		);
	}

	if (_enabled.eld && !availableSolvers.eld)
	{
		_enabled.eld = false;
		_errorReporter.warning(
			4458_error,
			SourceLocation(),
#if defined(__linux) || defined(__APPLE__)
			"Solver Eldarica was selected for SMTChecker but it was not found in the system."
#else
			"Solver Eldarica was selected for SMTChecker but it is only supported on Linux and MacOS."
#endif
		);
	}

	if (_enabled.z3 && !availableSolvers.z3)
	{
		_enabled.z3 = false;
		_errorReporter.warning(
			8158_error,
			SourceLocation(),
			"Solver z3 was selected for SMTChecker but it is not available."
#ifdef HAVE_Z3_DLOPEN
			" libz3.so." + to_string(Z3_MAJOR_VERSION) + "." + to_string(Z3_MINOR_VERSION) + " was not found."
#endif
		);
	}

	return _enabled;
}
