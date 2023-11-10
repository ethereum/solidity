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

#include <algorithm>
#include <cstddef>
#include <random>
#include <stack>
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
		"["
			"xa[r]EscLM"               // Turn into SSA and simplify
			"cCTUtTOntnfDIul"          // Perform structural simplification
			"Lcul"                     // Simplify again
			"Vcul [j]"                 // Reverse SSA

			// should have good "compilability" property here.

			"Tpeul"                    // Run functional expression inliner
			"xa[rul]"                  // Prune a bit more in SSA
			"xa[r]cL"                  // Turn into SSA again and simplify
			"gvif"                     // Run full inliner
			"CTUca[r]LSsTFOtfDnca[r]Iulc" // SSA plus simplify
		"]"
		"jmul[jul] VcTOcul jmul";      // Make source short and pretty

	static char constexpr DefaultYulOptimiserCleanupSteps[] = "fDnTOcmu";

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

	/// Create valid sequence
	static std::string createValidSequence(std::string& _result)
	{
		std::stack<char> stk;
		// Remove unmatched brackets
		for (auto it = _result.begin(); it != _result.end(); /* no increment here */)
		{
			if (*it == '[')
		        {
				stk.push(*it);
				++it;
		        }
		        else if (*it == ']')
		        {
			        if (stk.empty())
				        it = _result.erase(it); // Erase returns the new iterator position
				else
			        {
					stk.pop();
					++it;
			        }
			}
		        else
			        ++it;
		}
		// Add matching brackets
		while (!stk.empty())
		{
			_result += ']';
			stk.pop();
		}
		// Remove colon inside brackets
		std::stack<bool> insideBrackets;

		// Remove colon within brackets
		for (auto it = _result.begin(); it != _result.end(); /* no increment here */)
		{
			if (*it == '[') {
			        insideBrackets.push(true);
			        ++it; // Move to the next character
		        }
			else if (*it == ']')
			{
				insideBrackets.pop();
			        ++it; // Move to the next character
			}
			else if (*it == ':' && !insideBrackets.empty())
			{
				// Erase the colon if we're inside brackets
				it = _result.erase(it); // Erase returns the new iterator position
			}
			else
				++it; // Move to the next character
		}
		assert(insideBrackets.empty());
		std::string output;
		bool wasColon = false;

		// Keep at most one colon
		for (auto& c: _result)
		{
			if (c == ':')
			{
			        if (!wasColon)
				{
			                output += c; // Append the first colon
			                wasColon = true;
			        }
			        // Skip if the last character was also a colon
			}
		        else
				output += c; // Append non-colon characters
		}
		return output;
	}

	/// Generate random Yul optimiser sequence
	static std::string generateRandomPermutation(std::string const& input, size_t _seed)
	{
		std::string result = input;
		std::mt19937 g(_seed);
		std::shuffle(result.begin(), result.end(), g);

		// Generate a random length for the substring
		std::uniform_int_distribution<size_t> lengthDistribution(1, result.length());
		size_t substringLength = lengthDistribution(g);

		// Extract a substring from the shuffled result
		std::string shuffledSubstring = result.substr(0, substringLength);
		return createValidSequence(shuffledSubstring);
	}

	/// Get random Yul optimiser sequence
	static std::string randomYulOptimiserSequence(size_t _seed)
	{
		return generateRandomPermutation(DefaultYulOptimiserSteps, _seed);
	}

	/// Fuzz Yul optimiser sequence
	static OptimiserSettings fuzz(std::string const& _optSequence)
	{
		OptimiserSettings s = standard();
		s.yulOptimiserSteps = _optSequence;
		return s;
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
