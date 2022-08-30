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
 * Optimisation stage that removes mstore and sstore operations if they store the same
 * value that is already known to be in that slot.
 */

#pragma once

#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * Optimisation stage that removes mstore and sstore operations if they store the same
 * value that is already known to be in that slot.
 *
 * Works best if the code is in SSA form - without literal arguments.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class EqualStoreEliminator: public DataFlowAnalyzer
{
public:
	static constexpr char const* name{"EqualStoreEliminator"};
	static void run(OptimiserStepContext const&, Block& _ast);

private:
	EqualStoreEliminator(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> _functionSideEffects
	):
		DataFlowAnalyzer(_dialect, MemoryAndStorage::Analyze, std::move(_functionSideEffects))
	{}

protected:
	using ASTModifier::visit;
	void visit(Statement& _statement) override;

	std::set<Statement const*> m_pendingRemovals;
};

}
