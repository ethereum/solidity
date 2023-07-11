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
 * Base class for both UnusedAssignEliminator and UnusedStoreEliminator.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AST.h>

#include <range/v3/action/remove_if.hpp>

#include <variant>


namespace solidity::yul
{
struct Dialect;

/**
 * Base class for both UnusedAssignEliminator and UnusedStoreEliminator.
 *
 * The class tracks the state of abstract "stores" (assignments or mstore/sstore
 * statements) across the control-flow. It is the job of the derived class to create
 * the stores and track references, but the base class manages control-flow splits and joins.
 *
 * In general, active stores are those where it has not yet been determined if they are used
 * or not. Those are split and joined at control-flow forks. Once a store has been deemed
 * used, it is removed from the active set and marked as used and this will never change.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class UnusedStoreBase: public ASTWalker
{
public:
	explicit UnusedStoreBase(Dialect const& _dialect): m_dialect(_dialect) {}

	using ASTWalker::operator();
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(ForLoop const&) override;
	void operator()(Break const&) override;
	void operator()(Continue const&) override;

protected:
	using ActiveStores = std::map<YulString, std::set<Statement const*>>;

	/// This function is called for a loop that is nested too deep to avoid
	/// horrible runtime and should just resolve the situation in a pragmatic
	/// and correct manner.
	virtual void shortcutNestedLoop(ActiveStores const& _beforeLoop) = 0;

	/// This function is called right before the scoped restore of the function definition.
	virtual void finalizeFunctionDefinition(FunctionDefinition const& /*_functionDefinition*/) {}

	/// Joins the assignment mapping of @a _source into @a _target according to the rules laid out
	/// above.
	/// Will destroy @a _source.
	static void merge(ActiveStores& _target, ActiveStores&& _source);
	static void merge(ActiveStores& _target, std::vector<ActiveStores>&& _source);

	Dialect const& m_dialect;
	/// Set of all stores encountered during the traversal (in the current function).
	std::set<Statement const*> m_allStores;
	/// Set of stores that are marked as being used (in the current function).
	std::set<Statement const*> m_usedStores;
	/// List of stores that can be removed (globally).
	std::vector<Statement const*> m_storesToRemove;
	/// Active (undecided) stores in the current branch.
	ActiveStores m_activeStores;

	/// Working data for traversing for-loops.
	struct ForLoopInfo
	{
		/// Tracked assignment states for each break statement.
		std::vector<ActiveStores> pendingBreakStmts;
		/// Tracked assignment states for each continue statement.
		std::vector<ActiveStores> pendingContinueStmts;
	};
	ForLoopInfo m_forLoopInfo;
	size_t m_forLoopNestingDepth = 0;
};

}
