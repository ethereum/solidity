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
 */

#include <libyul/optimiser/RedundantStoreEliminator.h>

#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;


void RedundantStoreEliminator::run(OptimiserStepContext& _context, Block& _ast)
{
	RedundantStoreEliminator rse{_context.dialect};
	rse(_ast);
}

void RedundantStoreEliminator::operator()(Block& _block)
{
	// TODO This is copied from DataflowAnalyzer
	size_t numScopes = m_variableScopes.size();
	pushScope(false);

	map<YulString, size_t> latestStore;
	set<size_t> redundantStores;

	for (size_t i = 0; i < _block.statements.size(); ++i)
	{
		Statement& statement = _block.statements.at(i);
		if (holds_alternative<ExpressionStatement>(statement))
		{
			ExpressionStatement& exprStatement = get<ExpressionStatement>(statement);
			// This mechanism relies on the way the data flow analyzer
			// handles `m_storage`.
			if (auto vars = isSimpleStore(StoreLoadLocation::Storage, exprStatement))
			{
				if (m_storage.count(vars->first) && latestStore.count(vars->first))
					redundantStores.insert(latestStore.at(vars->first));
				latestStore[vars->first] = i;
			}
		}
		ASTModifier::visit(statement);
	}

	popScope();
	assertThrow(numScopes == m_variableScopes.size(), OptimizerException, "");
	DataFlowAnalyzer::operator()(_block);
	if (redundantStores.empty())
		return;

	// The arguments to sstore are identifiers and thus do not have side-effects,
	// because otherwise the statement would not have ended up in `redundantStores`.
	vector<Statement> newStatements;
	for (size_t i = 0; i < _block.statements.size(); ++i)
	{
		if (!redundantStores.count(i))
			newStatements.emplace_back(move(_block.statements.at(i)));
	}
	_block.statements = move(newStatements);
}
