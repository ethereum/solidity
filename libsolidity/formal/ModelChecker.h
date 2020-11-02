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

#include <libsolidity/interface/ReadFile.h>

#include <libsmtutil/SolverInterface.h>
#include <liblangutil/ErrorReporter.h>

#include <optional>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::frontend
{

struct ModelCheckerEngine
{
	bool bmc = false;
	bool chc = false;

	static constexpr ModelCheckerEngine All() { return {true, true}; }
	static constexpr ModelCheckerEngine BMC() { return {true, false}; }
	static constexpr ModelCheckerEngine CHC() { return {false, true}; }
	static constexpr ModelCheckerEngine None() { return {false, false}; }

	bool none() const { return !any(); }
	bool any() const { return bmc || chc; }
	bool all() const { return bmc && chc; }

	static std::optional<ModelCheckerEngine> fromString(std::string const& _engine)
	{
		static std::map<std::string, ModelCheckerEngine> engineMap{
			{"all", All()},
			{"bmc", BMC()},
			{"chc", CHC()},
			{"none", None()}
		};
		if (engineMap.count(_engine))
			return engineMap.at(_engine);
		return {};
	}
};

struct ModelCheckerSettings
{
	ModelCheckerEngine engine = ModelCheckerEngine::All();
	std::optional<unsigned> timeout;
};

class ModelChecker
{
public:
	/// @param _enabledSolvers represents a runtime choice of which SMT solvers
	/// should be used, even if all are available. The default choice is to use all.
	ModelChecker(
		langutil::ErrorReporter& _errorReporter,
		std::map<solidity::util::h256, std::string> const& _smtlib2Responses,
		ModelCheckerSettings _settings = ModelCheckerSettings{},
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
	ModelCheckerSettings m_settings;

	/// Stores the context of the encoding.
	smt::EncodingContext m_context;

	/// Bounded Model Checker engine.
	BMC m_bmc;

	/// Constrained Horn Clauses engine.
	CHC m_chc;
};

}
