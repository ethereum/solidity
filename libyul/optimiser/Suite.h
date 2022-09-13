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
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameDispenser.h>
#include <liblangutil/EVMVersion.h>

#include <set>
#include <string>
#include <string_view>
#include <memory>

namespace solidity::yul
{

struct AsmAnalysisInfo;
struct Dialect;
class GasMeter;
struct Object;

/**
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 * Only optimizes the code of the provided object, does not descend into the sub-objects.
 */
class OptimiserSuite
{
public:
	static constexpr size_t MaxRounds = 12;

	/// Special characters that do not represent optimiser steps but are allowed in abbreviation sequences.
	/// Some of them (like whitespace) are ignored, others (like brackets) are a part of the syntax.
	static constexpr char NonStepAbbreviations[] = " \n[]:";

	enum class Debug
	{
		None,
		PrintStep,
		PrintChanges
	};
	OptimiserSuite(OptimiserStepContext& _context, Debug _debug = Debug::None): m_context(_context), m_debug(_debug) {}

	/// The value nullopt for `_expectedExecutionsPerDeployment` represents creation code.
	static void run(
		Dialect const& _dialect,
		GasMeter const* _meter,
		Object& _object,
		bool _optimizeStackAllocation,
		std::string_view _optimisationSequence,
		std::string_view _optimisationCleanupSequence,
		std::optional<size_t> _expectedExecutionsPerDeployment,
		std::set<YulString> const& _externallyUsedIdentifiers = {}
	);

	/// Ensures that specified sequence of step abbreviations is well-formed and can be executed.
	/// @throw OptimizerException if the sequence is invalid
	static void validateSequence(std::string_view _stepAbbreviations);

	void runSequence(std::vector<std::string> const& _steps, Block& _ast);
	void runSequence(std::string_view _stepAbbreviations, Block& _ast, bool _repeatUntilStable = false);

	static std::map<std::string, std::unique_ptr<OptimiserStep>> const& allSteps();
	static std::map<std::string, char> const& stepNameToAbbreviationMap();
	static std::map<char, std::string> const& stepAbbreviationToNameMap();

private:
	OptimiserStepContext& m_context;
	Debug m_debug;
};

}
