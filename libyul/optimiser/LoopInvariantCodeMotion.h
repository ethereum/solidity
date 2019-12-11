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
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * Loop-invariant code motion.
 *
 * This optimization moves movable SSA variable declarations outside the loop.
 *
 * Only statements at the top level in a loop's body or post block are considered, i.e variable
 * declarations inside conditional branches will not be moved out of the loop.
 *
 * Requirements:
 * - The Disambiguator, ForLoopInitRewriter and FunctionHoister must be run upfront.
 * - Expression splitter and SSA transform should be run upfront to obtain better result.
 */

class LoopInvariantCodeMotion: public ASTModifier
{
public:
	static constexpr char const* name{"LoopInvariantCodeMotion"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	void operator()(Block& _block) override;

private:
	explicit LoopInvariantCodeMotion(
		Dialect const& _dialect,
		std::set<YulString> const& _ssaVariables,
		std::map<YulString, SideEffects> const& _functionSideEffects
	):
		m_dialect(_dialect),
		m_ssaVariables(_ssaVariables),
		m_functionSideEffects(_functionSideEffects)
	{ }

	/// @returns true if the given variable declaration can be moved to in front of the loop.
	bool canBePromoted(VariableDeclaration const& _varDecl, std::set<YulString> const& _varsDefinedInCurrentScope) const;
	std::optional<std::vector<Statement>> rewriteLoop(ForLoop& _for);

	Dialect const& m_dialect;
	std::set<YulString> const& m_ssaVariables;
	std::map<YulString, SideEffects> const& m_functionSideEffects;
};

}
