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
#include <libyul/optimiser/RedundantStoreBase.h>

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
 * actual removal step. During information gathering, we maintain a
 * mapping from assignment statements to the three states
 * "unused", "undecided" and "used".
 * When an assignment is visited, it is added to the mapping in the "undecided" state
 * (see remark about for loops below) and every other assignment to the same variable
 * that is still in the "undecided" state is changed to "unused".
 * When a variable is referenced, the state of any assignment to that variable still
 * in the "undecided" state is changed to "used".
 * At points where control flow splits, a copy
 * of the mapping is handed over to each branch. At points where control flow
 * joins, the two mappings coming from the two branches are combined in the following way:
 * Statements that are only in one mapping or have the same state are used unchanged.
 * Conflicting values are resolved in the following way:
 * "unused", "undecided" -> "undecided"
 * "unused", "used" -> "used"
 * "undecided, "used" -> "used".
 *
 * For for-loops, the condition, body and post-part are visited twice, taking
 * the joining control-flow at the condition into account.
 * In other words, we create three control flow paths: Zero runs of the loop,
 * one run and two runs and then combine them at the end.
 * Running at most twice is enough because there are only three different states.
 *
 * Since this algorithm has exponential runtime in the nesting depth of for loops,
 * a shortcut is taken at a certain nesting level: We only use the zero- and
 * once-run of the for loop and change any assignment that was newly introduced
 * in the for loop from to "used".
 *
 * For switch statements that have a "default"-case, there is no control-flow
 * part that skips the switch.
 *
 * At ``leave`` statements, all return variables are set to "used".
 *
 * When a variable goes out of scope, all statements still in the "undecided"
 * state are changed to "unused", unless the variable is the return
 * parameter of a function - there, the state changes to "used".
 *
 * In the second traversal, all assignments that are in the "unused" state are removed.
 *
 *
 * This step is usually run right after the SSA transform to complete
 * the generation of the pseudo-SSA.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class RedundantAssignEliminator: public RedundantStoreBase
{
public:
	static constexpr char const* name{"RedundantAssignEliminator"};
	static void run(OptimiserStepContext&, Block& _ast);

	explicit RedundantAssignEliminator(Dialect const& _dialect): RedundantStoreBase(_dialect) {}

	void operator()(Identifier const& _identifier) override;
	void operator()(VariableDeclaration const& _variableDeclaration) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(Leave const&) override;
	void operator()(Block const& _block) override;

	using RedundantStoreBase::visit;
	void visit(Statement const& _statement) override;

private:
	void shortcutNestedLoop(TrackedStores const& _beforeLoop) override;
	void finalizeFunctionDefinition(FunctionDefinition const& _functionDefinition) override;

	void changeUndecidedTo(YulString _variable, State _newState);
	/// Called when a variable goes out of scope. Sets the state of all still undecided
	/// assignments to the final state. In this case, this also applies to pending
	/// break and continue TrackedStores.
	void finalize(YulString _variable, State _finalState);


	std::set<YulString> m_declaredVariables;
	std::set<YulString> m_returnVariables;
};

}
