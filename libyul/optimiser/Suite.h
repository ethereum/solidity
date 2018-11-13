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

#include <libyul/ASTDataForward.h>
#include <libyul/YulString.h>

#include <set>

namespace dev
{
namespace solidity
{
namespace assembly
{
struct AsmAnalysisInfo;
}
}
namespace yul
{

/**
 * Optimiser suite that combines all steps and also provides the settings for the heuristics
 */
class OptimiserSuite
{
public:
	static void run(
		Block& _ast,
		solidity::assembly::AsmAnalysisInfo const& _analysisInfo,

		std::set<YulString> const& _externallyUsedIdentifiers = {}
	);
};

}
}
