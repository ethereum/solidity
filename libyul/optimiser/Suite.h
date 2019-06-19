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
#include <liblangutil/EVMVersion.h>

#include <set>

namespace yul
{

struct AsmAnalysisInfo;
struct Dialect;
class GasMeter;

/**
 * Optimiser suite that combines all steps and also provides the settings for the heuristics
 */
class OptimiserSuite
{
public:
	static void run(
		Dialect const& _dialect,
		GasMeter const* _meter,
		Block& _ast,
		AsmAnalysisInfo const& _analysisInfo,
		bool _optimizeStackAllocation,
		std::set<YulString> const& _externallyUsedIdentifiers = {}
	);
};

}
