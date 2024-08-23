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
 * @author Alex Beregszaszi
 * @date 2017
 * Helper class for optimiser settings.
 */

#pragma once

#include <liblangutil/Exceptions.h>

#include <cstddef>
#include <string>

namespace solidity::frontend
{

enum class OptimisationPreset
{
	None,
	Minimal,
	Standard,
	Full,
};

struct OptimiserSettings
{
	static char constexpr DefaultYulOptimiserSteps[] =
		"dhfoDgvulfnTUtnIf"            // None of these can make stack problems worse

		"xa[r]EscLM"                   // Turn into SSA and simplify
		"Vcul [j]"                     // Reverse SSA

		"k"                            // preprocessing after some simplifications

		// should have good "compilability" property here.

		"Trpeul"                       // Run functional expression inliner
		"xa[r]cL"                      // Turn into SSA again and simplify
		"gvifM"                        // Run full inliner
		"CTUca[r]LSsTFOtfDnca[r]Iulc"  // SSA plus simplify

		"scCTUt"
		"gvifM"                        // Run full inliner
		"x[scCTUt] TOntnfDIul"         // Perform structural simplification
		"gvifM"                        // Run full inliner

		"jmul[jul] VcTOcul jmul";      // Make source short and pretty

	static char constexpr DefaultYulOptimiserCleanupSteps[] = "fDnTOcmuO";

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
		s.simpleCounterForLoopUncheckedIncrement = true;
		return s;
	}
	/// Standard optimisations.
	static OptimiserSettings standard()
	{
		OptimiserSettings s;
		s.runOrderLiterals = true;
		s.runInliner = true;
		s.runJumpdestRemover = true;
		s.runPeephole = true;
		s.runDeduplicate = true;
		s.runCSE = true;
		s.runConstantOptimiser = true;
		s.simpleCounterForLoopUncheckedIncrement = true;
		s.runYulOptimiser = true;
		s.optimizeStackAllocation = true;
		return s;
	}
	/// Full optimisations. Currently an alias for standard optimisations.
	static OptimiserSettings full()
	{
		return standard();
	}

	static OptimiserSettings preset(OptimisationPreset _preset)
	{
		switch (_preset)
		{
			case OptimisationPreset::None: return none();
			case OptimisationPreset::Minimal: return minimal();
			case OptimisationPreset::Standard: return standard();
			case OptimisationPreset::Full: return full();
		}
		util::unreachable();
	}

	bool operator==(OptimiserSettings const& _other) const
	{
		return
			runOrderLiterals == _other.runOrderLiterals &&
			runInliner == _other.runInliner &&
			runJumpdestRemover == _other.runJumpdestRemover &&
			runPeephole == _other.runPeephole &&
			runDeduplicate == _other.runDeduplicate &&
			runCSE == _other.runCSE &&
			runConstantOptimiser == _other.runConstantOptimiser &&
			simpleCounterForLoopUncheckedIncrement == _other.simpleCounterForLoopUncheckedIncrement &&
			optimizeStackAllocation == _other.optimizeStackAllocation &&
			runYulOptimiser == _other.runYulOptimiser &&
			yulOptimiserSteps == _other.yulOptimiserSteps &&
			expectedExecutionsPerDeployment == _other.expectedExecutionsPerDeployment;
	}

	bool operator!=(OptimiserSettings const& _other) const
	{
		return !(*this == _other);
	}

	/// Move literals to the right of commutative binary operators during code generation.
	/// This helps exploiting associativity.
	bool runOrderLiterals = false;
	/// Inliner
	bool runInliner = false;
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
	/// Perform more efficient stack allocation for variables during code generation from Yul to bytecode.
	bool simpleCounterForLoopUncheckedIncrement = false;
	/// Yul optimiser with default settings. Will only run on certain parts of the code for now.
	bool optimizeStackAllocation = false;
	/// Allow unchecked arithmetic when incrementing the counter of certain kinds of 'for' loop
	bool runYulOptimiser = false;
	/// Sequence of optimisation steps to be performed by Yul optimiser.
	/// Note that there are some hard-coded steps in the optimiser and you cannot disable
	/// them just by setting this to an empty string. Set @a runYulOptimiser to false if you want
	/// no optimisations.
	std::string yulOptimiserSteps = DefaultYulOptimiserSteps;
	/// Sequence of clean-up optimisation steps after yulOptimiserSteps is run. Note that if the string
	/// is left empty, there will still be hard-coded optimisation steps that will run regardless.
	/// Set @a runYulOptimiser to false if you want no optimisations.
	std::string yulOptimiserCleanupSteps = DefaultYulOptimiserCleanupSteps;
	/// This specifies an estimate on how often each opcode in this assembly will be executed,
	/// i.e. use a small value to optimise for size and a large value to optimise for runtime gas usage.
	size_t expectedExecutionsPerDeployment = 200;
};

}
