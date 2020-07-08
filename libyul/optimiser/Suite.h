// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameDispenser.h>
#include <liblangutil/EVMVersion.h>

#include <set>
#include <string>
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
	static constexpr char NonStepAbbreviations[] = " \n[]";

	enum class Debug
	{
		None,
		PrintStep,
		PrintChanges
	};
	static void run(
		Dialect const& _dialect,
		GasMeter const* _meter,
		Object& _object,
		bool _optimizeStackAllocation,
		std::string const& _optimisationSequence,
		std::set<YulString> const& _externallyUsedIdentifiers = {}
	);

	/// Ensures that specified sequence of step abbreviations is well-formed and can be executed.
	/// @throw OptimizerException if the sequence is invalid
	static void validateSequence(std::string const& _stepAbbreviations);

	void runSequence(std::vector<std::string> const& _steps, Block& _ast);
	void runSequence(std::string const& _stepAbbreviations, Block& _ast);
	void runSequenceUntilStable(
		std::vector<std::string> const& _steps,
		Block& _ast,
		size_t maxRounds = MaxRounds
	);

	static std::map<std::string, std::unique_ptr<OptimiserStep>> const& allSteps();
	static std::map<std::string, char> const& stepNameToAbbreviationMap();
	static std::map<char, std::string> const& stepAbbreviationToNameMap();

private:
	OptimiserSuite(
		Dialect const& _dialect,
		std::set<YulString> const& _externallyUsedIdentifiers,
		Debug _debug,
		Block& _ast
	):
		m_dispenser{_dialect, _ast, _externallyUsedIdentifiers},
		m_context{_dialect, m_dispenser, _externallyUsedIdentifiers},
		m_debug(_debug)
	{}

	NameDispenser m_dispenser;
	OptimiserStepContext m_context;
	Debug m_debug;
};

}
