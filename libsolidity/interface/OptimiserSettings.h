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

#include <cstddef>

namespace dev
{
namespace solidity
{

struct OptimiserSettings
{
	/// No optimisations at all - not recommended.
	static OptimiserSettings none()
	{
		return {};
	}
	/// Minimal optimisations: Peephole and jumpdest remover
	static OptimiserSettings minimal()
	{
		OptimiserSettings s = none();
		s.runJumpdestRemover = true;
		s.runPeephole = true;
		return s;
	}
	/// Standard optimisations.
	static OptimiserSettings enabled()
	{
		OptimiserSettings s;
		s.runOrderLiterals = true;
		s.runJumpdestRemover = true;
		s.runPeephole = true;
		s.runDeduplicate = true;
		s.runCSE = true;
		s.runConstantOptimiser = true;
		// The only disabled one
		s.runYulOptimiser = false;
		s.expectedExecutionsPerDeployment = 200;
		return s;
	}
	/// Standard optimisations plus yul optimiser.
	static OptimiserSettings full()
	{
		OptimiserSettings s = enabled();
		s.runYulOptimiser = true;
		return s;
	}

	bool operator==(OptimiserSettings const& _other) const
	{
		return
			runOrderLiterals == _other.runOrderLiterals &&
			runJumpdestRemover == _other.runJumpdestRemover &&
			runPeephole == _other.runPeephole &&
			runDeduplicate == _other.runDeduplicate &&
			runCSE == _other.runCSE &&
			runConstantOptimiser == _other.runConstantOptimiser &&
			runYulOptimiser == _other.runYulOptimiser &&
			expectedExecutionsPerDeployment == _other.expectedExecutionsPerDeployment;
	}

	/// Move literals to the right of commutative binary operators during code generation.
	/// This helps exploiting associativity.
	bool runOrderLiterals = false;
	/// Non-referenced jump destination remover.
	bool runJumpdestRemover = false;
	/// Peephole optimizer
	bool runPeephole = false;
	/// Assembly block deduplicator
	bool runDeduplicate = false;
	/// Common subexpression eliminator based on assembly items.
	bool runCSE = false;
	/// Constant optimizer, which tries to find better representations that satisfy the given
	/// size/cost-trade-off.
	bool runConstantOptimiser = false;
	/// Yul optimiser with default settings. Will only run on certain parts of the code for now.
	bool runYulOptimiser = false;
	/// This specifies an estimate on how often each opcode in this assembly will be executed,
	/// i.e. use a small value to optimise for size and a large value to optimise for runtime gas usage.
	size_t expectedExecutionsPerDeployment = 200;
};

}
}
