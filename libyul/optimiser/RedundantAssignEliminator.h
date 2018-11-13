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
 * Optimiser component that removes assignments to variables that are not used
 * until they go out of scope or are re-assigned.
 */

#pragma once

#include <libyul/ASTDataForward.h>

#include <libyul/optimiser/ASTWalker.h>

#include <map>

namespace dev
{
namespace yul
{

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
 * For switch statements that have a "default"-case, there is no control-flow
 * part that skips the switch.
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
 * Prerequisite: Disambiguator.
 */
class RedundantAssignEliminator: public ASTWalker
{
public:
	RedundantAssignEliminator(RedundantAssignEliminator const&) = default;
	RedundantAssignEliminator& operator=(RedundantAssignEliminator const&) = default;
	RedundantAssignEliminator(RedundantAssignEliminator&&) = default;
	RedundantAssignEliminator& operator=(RedundantAssignEliminator&&) = default;

	void operator()(Identifier const& _identifier) override;
	void operator()(VariableDeclaration const& _variableDeclaration) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(ForLoop const&) override;
	void operator()(Block const& _block) override;

	static void run(Block& _ast);

private:
	RedundantAssignEliminator() {}

	class State
	{
	public:
		enum Value { Unused, Undecided, Used };
		State(Value _value = Undecided): m_value(_value) {}
		inline bool operator==(State _other) const { return m_value == _other.m_value; }
		inline bool operator!=(State _other) const { return !operator==(_other); }
		static inline void join(State& _a, State const& _b)
		{
			// Using "max" works here because of the order of the values in the enum.
			_a.m_value =  Value(std::max(int(_a.m_value), int(_b.m_value)));
		}
	private:
		Value m_value = Undecided;
	};

	/**
	 * Takes care about storing the list of declared variables and
	 * sets them to "unused" when it is destroyed.
	 */
	class BlockScope
	{
	public:
		explicit BlockScope(RedundantAssignEliminator& _rae): m_rae(_rae)
		{
			swap(m_rae.m_declaredVariables, m_outerDeclaredVariables);
		}
		~BlockScope()
		{
			// This should actually store all declared variables
			// into a different mapping
			for (auto const& var: m_rae.m_declaredVariables)
				m_rae.changeUndecidedTo(var, State::Unused);
			for (auto const& var: m_rae.m_declaredVariables)
				m_rae.finalize(var);
			swap(m_rae.m_declaredVariables, m_outerDeclaredVariables);
		}

	private:
		RedundantAssignEliminator& m_rae;
		std::set<YulString> m_outerDeclaredVariables;
	};

	/// Joins the assignment mapping with @a _other according to the rules laid out
	/// above.
	/// Will destroy @a _other.
	void join(RedundantAssignEliminator& _other);
	void changeUndecidedTo(YulString _variable, State _newState);
	void finalize(YulString _variable);

	std::set<YulString> m_declaredVariables;
	// TODO check that this does not cause nondeterminism!
	// This could also be a pseudo-map from state to assignment.
	std::map<YulString, std::map<Assignment const*, State>> m_assignments;
	std::set<Assignment const*> m_assignmentsToRemove;
};

class AssignmentRemover: public ASTModifier
{
public:
	explicit AssignmentRemover(std::set<Assignment const*> const& _toRemove):
		m_toRemove(_toRemove)
	{}
	void operator()(Block& _block) override;

private:
	std::set<Assignment const*> const& m_toRemove;
};

}
}
