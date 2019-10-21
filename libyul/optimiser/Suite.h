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

namespace yul
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
		std::set<YulString> const& _externallyUsedIdentifiers = {}
	);

	void runSequence(std::vector<std::string> const& _steps, Block& _ast);

	static std::map<std::string, std::unique_ptr<OptimiserStep>> const& allSteps();

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
