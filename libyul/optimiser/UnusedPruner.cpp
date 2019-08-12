/*(
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

#include <libyul/optimiser/UnusedPruner.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <boost/algorithm/cxx11/none_of.hpp>

using namespace std;
using namespace dev;
using namespace yul;

UnusedPruner::UnusedPruner(
	Dialect const& _dialect,
	Block& _ast,
	bool _allowMSizeOptimization,
	set<YulString> const& _externallyUsedFunctions
):
	m_dialect(_dialect),
	m_allowMSizeOptimization(_allowMSizeOptimization)
{
	m_references = ReferencesCounter::countReferences(_ast);
	for (auto const& f: _externallyUsedFunctions)
		++m_references[f];
}

UnusedPruner::UnusedPruner(
	Dialect const& _dialect,
	FunctionDefinition& _function,
	bool _allowMSizeOptimization,
	set<YulString> const& _externallyUsedFunctions
):
	m_dialect(_dialect),
	m_allowMSizeOptimization(_allowMSizeOptimization)
{
	m_references = ReferencesCounter::countReferences(_function);
	for (auto const& f: _externallyUsedFunctions)
		++m_references[f];
}

void UnusedPruner::operator()(Block& _block)
{
	for (auto&& statement: _block.statements)
		if (statement.type() == typeid(FunctionDefinition))
		{
			FunctionDefinition& funDef = boost::get<FunctionDefinition>(statement);
			if (!used(funDef.name))
			{
				subtractReferences(ReferencesCounter::countReferences(funDef.body));
				statement = Block{std::move(funDef.location), {}};
			}
		}
		else if (statement.type() == typeid(VariableDeclaration))
		{
			VariableDeclaration& varDecl = boost::get<VariableDeclaration>(statement);
			// Multi-variable declarations are special. We can only remove it
			// if all variables are unused and the right-hand-side is either
			// movable or it returns a single value. In the latter case, we
			// replace `let a := f()` by `pop(f())` (in pure Yul, this will be
			// `drop(f())`).
			if (boost::algorithm::none_of(
				varDecl.variables,
				[=](TypedName const& _typedName) { return used(_typedName.name); }
			))
			{
				if (!varDecl.value)
					statement = Block{std::move(varDecl.location), {}};
				else if (SideEffectsCollector(m_dialect, *varDecl.value).sideEffectFree(m_allowMSizeOptimization))
				{
					subtractReferences(ReferencesCounter::countReferences(*varDecl.value));
					statement = Block{std::move(varDecl.location), {}};
				}
				else if (varDecl.variables.size() == 1 && m_dialect.discardFunction())
					statement = ExpressionStatement{varDecl.location, FunctionCall{
						varDecl.location,
						{varDecl.location, m_dialect.discardFunction()->name},
						{*std::move(varDecl.value)}
					}};
			}
		}
		else if (statement.type() == typeid(ExpressionStatement))
		{
			ExpressionStatement& exprStmt = boost::get<ExpressionStatement>(statement);
			if (SideEffectsCollector(m_dialect, exprStmt.expression).sideEffectFree(m_allowMSizeOptimization))
			{
				subtractReferences(ReferencesCounter::countReferences(exprStmt.expression));
				statement = Block{std::move(exprStmt.location), {}};
			}
		}

	removeEmptyBlocks(_block);

	ASTModifier::operator()(_block);
}

void UnusedPruner::runUntilStabilised(
	Dialect const& _dialect,
	Block& _ast,
	bool _allowMSizeOptimization,
	set<YulString> const& _externallyUsedFunctions
)
{
	while (true)
	{
		UnusedPruner pruner(_dialect, _ast, _allowMSizeOptimization, _externallyUsedFunctions);
		pruner(_ast);
		if (!pruner.shouldRunAgain())
			return;
	}
}

void UnusedPruner::runUntilStabilised(
	Dialect const& _dialect,
	Block& _ast,
	set<YulString> const& _externallyUsedFunctions
)
{
	bool allowMSizeOptimization = !SideEffectsCollector(_dialect, _ast).containsMSize();
	runUntilStabilised(_dialect, _ast, allowMSizeOptimization, _externallyUsedFunctions);
}

void UnusedPruner::runUntilStabilised(
	Dialect const& _dialect,
	FunctionDefinition& _function,
	bool _allowMSizeOptimization,
	set<YulString> const& _externallyUsedFunctions
)
{
	while (true)
	{
		UnusedPruner pruner(_dialect, _function, _allowMSizeOptimization, _externallyUsedFunctions);
		pruner(_function);
		if (!pruner.shouldRunAgain())
			return;
	}
}

bool UnusedPruner::used(YulString _name) const
{
	return m_references.count(_name) && m_references.at(_name) > 0;
}

void UnusedPruner::subtractReferences(map<YulString, size_t> const& _subtrahend)
{
	for (auto const& ref: _subtrahend)
	{
		assertThrow(m_references.count(ref.first), OptimizerException, "");
		assertThrow(m_references.at(ref.first) >= ref.second, OptimizerException, "");
		m_references[ref.first] -= ref.second;
		m_shouldRunAgain = true;
	}
}
