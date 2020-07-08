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

#include <libsolidity/interface/ReadFile.h>

#include <libsmtutil/SolverInterface.h>
#include <liblangutil/ErrorReporter.h>

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
		std::map<solidity::util::h256, std::string> const& _smtlib2Responses,
		ReadCallback::Callback const& _smtCallback = ReadCallback::Callback(),
		smtutil::SMTSolverChoice _enabledSolvers = smtutil::SMTSolverChoice::All()
	);

	void analyze(SourceUnit const& _sources);

	/// This is used if the SMT solver is not directly linked into this binary.
	/// @returns a list of inputs to the SMT solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries();

	/// @returns SMT solvers that are available via the C++ API.
	static smtutil::SMTSolverChoice availableSolvers();

private:
	/// Stores the context of the encoding.
	smt::EncodingContext m_context;

	/// Bounded Model Checker engine.
	BMC m_bmc;

	/// Constrained Horn Clauses engine.
	CHC m_chc;
};

}
