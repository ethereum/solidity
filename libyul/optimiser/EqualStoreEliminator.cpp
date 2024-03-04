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

#include <libyul/optimiser/EqualStoreEliminator.h>

#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>
#include <libyul/Utilities.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;
using namespace solidity::yul;

void EqualStoreEliminator::run(OptimiserStepContext const& _context, Block& _ast)
{
	EqualStoreEliminator eliminator{
		_context.dialect,
		SideEffectsPropagator::sideEffects(_context.dialect, CallGraphGenerator::callGraph(_ast))
	};
	eliminator(_ast);

	StatementRemover remover{eliminator.m_pendingRemovals};
	remover(_ast);
}

void EqualStoreEliminator::visit(Statement& _statement)
{
	// No need to consider potential changes through complex arguments since
	// isSimpleStore only returns something if the arguments are identifiers.
	if (ExpressionStatement const* expression = std::get_if<ExpressionStatement>(&_statement))
	{
		if (auto vars = isSimpleStore(StoreLoadLocation::Storage, *expression))
		{
			if (std::optional<YulString> currentValue = storageValue(vars->first))
				if (*currentValue == vars->second)
					m_pendingRemovals.insert(&_statement);
		}
		else if (auto vars = isSimpleStore(StoreLoadLocation::Memory, *expression))
		{
			if (std::optional<YulString> currentValue = memoryValue(vars->first))
				if (*currentValue == vars->second)
					m_pendingRemovals.insert(&_statement);
		}
	}

	DataFlowAnalyzer::visit(_statement);
}
