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
 * Optimiser component that removes assignments to variables that are not used
 * until they go out of scope or are re-assigned.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/UnusedStoreBase.h>
#include <libyul/optimiser/Semantics.h>

#include <map>
#include <vector>

namespace solidity::yul
{
struct Dialect;

/**
 * Optimiser component that removes assignments to variables that are not used
 * until they go out of scope or are re-assigned. This component
 * respects the control-flow and takes it into account for removal.
 *
 * Example:
 *
 * {
 *   let a
 *   a := 1
 *   a := 2
 *   b := 2
 *   if calldataload(0)
 *   {
 *     b := mload(a)
 *   }
 *   a := b
 * }
 *
 * In the example, "a := 1" can be removed because the value from this assignment
 * is not used in any control-flow branch (it is replaced right away).
 * The assignment "a := 2" is also overwritten by "a := b" at the end,
 * but there is a control-flow path (through the condition body) which uses
 * the value from "a := 2" and thus, this assignment cannot be removed.
 *
 * Detailed rules:
 *
 * The AST is traversed twice: in an information gathering step and in the
 * actual removal step. During information gathering, assignment statements
 * can be marked as "potentially unused" or as "used".
 *
 * When an assignment is visited, it is stored in the "set of all stores" and
 * added to the branch-dependent "active" sets for the assigned variables. This active
 * set for a variable contains all statements where that variable was last assigned to, i.e.
 * where a read from that variable could read from.
 * Furthermore, all other active sets for the assigned variables are cleared.
 *
 * When a reference to a variable is visited, the active assignments to that variable
 * in the current branch are marked as "used". This mark is permanent.
 * Also, the active set for this variable in the current branch is cleared.
 *
 * At points where control-flow splits, we maintain a copy of the active set
 * (all other data structures are shared across branches).
 *
 * At control-flow joins, we combine the sets of active stores for each variable.
 *
 * In the example above, the active set right after the assignment "b := mload(a)" (but before
 * the control-flow join) is "b := mload(a)"; the assignment "b := 2" was removed.
 * After the control-flow join it will contain both "b := mload(a)" and "b := 2", coming from
 * the two branches.
 *
 * For for-loops, the condition, body and post-part are visited twice, taking
 * the joining control-flow at the condition into account.
 * In other words, we create three control flow paths: Zero runs of the loop,
 * one run and two runs and then combine them at the end.
 * Running at most twice is enough because this takes into account all possible control-flow connections.
 *
 * Since this algorithm has exponential runtime in the nesting depth of for loops,
 * a shortcut is taken at a certain nesting level: We only use the zero- and
 * once-run of the for loop and change any assignment that was newly introduced
 * in the for loop from to "used".
 *
 * For switch statements that have a "default"-case, there is no control-flow
 * part that skips the switch.
 *
 * At ``leave`` statements, all return variables are set to "used" and the set of active statements
 * is cleared.
 *
 * If a function or builtin is called that does not continue, the set of active statements is
 * cleared for all variables.
 *
 * In the second traversal, all assignments that are not marked as "used" are removed.
 *
 * This step is usually run right after the SSA transform to complete
 * the generation of the pseudo-SSA.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class UnusedAssignEliminator: public UnusedStoreBase
{
public:
	static constexpr char const* name{"UnusedAssignEliminator"};
	static void run(OptimiserStepContext&, Block& _ast);

	explicit UnusedAssignEliminator(
		Dialect const& _dialect,
		std::map<YulString, ControlFlowSideEffects> _controlFlowSideEffects
	):
		UnusedStoreBase(_dialect),
		m_controlFlowSideEffects(_controlFlowSideEffects)
	{}

	void operator()(Identifier const& _identifier) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(Leave const&) override;
	void operator()(Block const& _block) override;

	using UnusedStoreBase::visit;
	void visit(Statement const& _statement) override;

private:
	void shortcutNestedLoop(ActiveStores const& _beforeLoop) override;
	void finalizeFunctionDefinition(FunctionDefinition const& _functionDefinition) override;

	void markUsed(YulString _variable);

	std::set<YulString> m_returnVariables;
	std::map<YulString, ControlFlowSideEffects> m_controlFlowSideEffects;
};

}
