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
/**
 * Entry point to the model checking engines.
 * The goal of this class is to make different
 * engines share knowledge to boost their proving power.
 */

#pragma once

#include <libsolidity/formal/BMC.h>
#include <libsolidity/formal/CHC.h>
#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/ModelCheckerSettings.h>

#include <libsolidity/interface/ReadFile.h>

#include <libsmtutil/SolverInterface.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/UniqueErrorReporter.h>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::frontend
{

class ModelChecker
{
public:
	/// @param _enabledSolvers represents a runtime choice of which SMT solvers
	/// should be used, even if all are available. The default choice is to use all.
	ModelChecker(
		langutil::ErrorReporter& _errorReporter,
		langutil::CharStreamProvider const& _charStreamProvider,
		std::map<solidity::util::h256, std::string> const& _smtlib2Responses,
		ModelCheckerSettings _settings = ModelCheckerSettings{},
		ReadCallback::Callback const& _smtCallback = ReadCallback::Callback()
	);

	// TODO This should be removed for 0.9.0.
	static bool isPragmaPresent(std::vector<std::shared_ptr<SourceUnit>> const& _sources);

	/// Generates error messages if the requested sources and contracts
	/// do not exist.
	void checkRequestedSourcesAndContracts(std::vector<std::shared_ptr<SourceUnit>> const& _sources);

	void analyze(SourceUnit const& _sources);

	/// This is used if the SMT solver is not directly linked into this binary.
	/// @returns a list of inputs to the SMT solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries();

	/// @returns SMT solvers that are available via the C++ API.
	static smtutil::SMTSolverChoice availableSolvers();

	/// @returns the intersection of the enabled and available solvers,
	/// reporting warnings when a solver is enabled but not available.
	static smtutil::SMTSolverChoice checkRequestedSolvers(smtutil::SMTSolverChoice _enabled, langutil::ErrorReporter& _errorReporter);

private:
	/// Error reporter from CompilerStack.
	/// We need to append m_uniqueErrorReporter
	/// to this one when the analysis is done.
	langutil::ErrorReporter& m_errorReporter;

	/// Used by ModelChecker, SMTEncoder, BMC and CHC to avoid duplicates.
	/// This is local to ModelChecker, so needs to be appended
	/// to m_errorReporter at the end of the analysis.
	langutil::UniqueErrorReporter m_uniqueErrorReporter;

	ModelCheckerSettings m_settings;

	/// Stores the context of the encoding.
	smt::EncodingContext m_context;

	/// Bounded Model Checker engine.
	BMC m_bmc;

	/// Constrained Horn Clauses engine.
	CHC m_chc;
};

}
