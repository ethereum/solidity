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
 * Optimisation stage that removes unused variables and functions.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/YulString.h>

#include <map>
#include <set>

namespace yul
{
struct Dialect;
struct SideEffects;

/**
 * Optimisation stage that removes unused variables and functions and also
 * removes side-effect-free expression statements.
 *
 * If msize is used, we cannot remove any statements that access memory.
 * Because of that, the Unused Pruner should only be invoked on full ASTs,
 * such that it can check for the presence of msize itself, or
 * the `_allowMSizeOptimization` needs to be passed.
 *
 * Note that this does not remove circular references.
 *
 * Prerequisite: Disambiguator
 */
class UnusedPruner: public ASTModifier
{
public:
	static constexpr char const* name{"UnusedPruner"};
	static void run(OptimiserStepContext& _context, Block& _ast) {
		UnusedPruner::runUntilStabilisedOnFullAST(_context.dialect, _ast, _context.reservedIdentifiers);
	}


	using ASTModifier::operator();
	void operator()(Block& _block) override;

	// @returns true iff the code changed in the previous run.
	bool shouldRunAgain() const { return m_shouldRunAgain; }

	// Run the pruner until the code does not change anymore.
	static void runUntilStabilised(
		Dialect const& _dialect,
		Block& _ast,
		bool _allowMSizeOptimization,
		std::map<YulString, SideEffects> const* _functionSideEffects = nullptr,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);

	static void run(
		Dialect const& _dialect,
		Block& _ast,
		std::set<YulString> const& _externallyUsedFunctions = {}
	)
	{
		runUntilStabilisedOnFullAST(_dialect, _ast, _externallyUsedFunctions);
	}

	/// Run the pruner until the code does not change anymore.
	/// The provided block has to be a full AST.
	/// The pruner itself determines if msize is used and which user-defined functions
	/// are side-effect free.
	static void runUntilStabilisedOnFullAST(
		Dialect const& _dialect,
		Block& _ast,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);

	// Run the pruner until the code does not change anymore.
	// Only run on the given function.
	// @param _allowMSizeOptimization if true, allows to remove instructions
	//        whose only side-effect is a potential change of the return value of
	//        the msize instruction.
	static void runUntilStabilised(
		Dialect const& _dialect,
		FunctionDefinition& _functionDefinition,
		bool _allowMSizeOptimization,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);

private:
	UnusedPruner(
		Dialect const& _dialect,
		Block& _ast,
		bool _allowMSizeOptimization,
		std::map<YulString, SideEffects> const* _functionSideEffects = nullptr,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);
	UnusedPruner(
		Dialect const& _dialect,
		FunctionDefinition& _function,
		bool _allowMSizeOptimization,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);

	bool used(YulString _name) const;
	void subtractReferences(std::map<YulString, size_t> const& _subtrahend);

	Dialect const& m_dialect;
	bool m_allowMSizeOptimization = false;
	std::map<YulString, SideEffects> const* m_functionSideEffects = nullptr;
	bool m_shouldRunAgain = false;
	std::map<YulString, size_t> m_references;
};

}
