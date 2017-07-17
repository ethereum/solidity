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
 * @author Alex Beregszaszi
 * @date 2017
 * Helper class for optimiser settings.
 */

#pragma once

namespace dev
{
namespace solidity
{

struct OptimiserSettings
{
	/// Reset to defaults.
	void reset()
	{
		runOrderLiterals = false;
		runPeephole = false;
		runDeduplicate = false;
		runCSE = false;
		runConstantOptimiser = false;
		expectedExecutionsPerDeployment = 200;
	}

	bool runOrderLiterals = false;
	bool runPeephole = false;
	bool runDeduplicate = false;
	bool runCSE = false;
	bool runConstantOptimiser = false;
	/// This specifies an estimate on how often each opcode in this assembly will be executed,
	/// i.e. use a small value to optimise for size and a large value to optimise for runtime gas usage.
	size_t expectedExecutionsPerDeployment = 200;
};

}
}
