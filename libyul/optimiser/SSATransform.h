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
 * Optimiser component that turns subsequent assignments to variable declarations
 * and assignments.
 */
#pragma once

#include <libyul/ASTDataForward.h>

#include <libyul/optimiser/ASTWalker.h>

#include <vector>

namespace dev
{
namespace yul
{

class NameDispenser;

/**
 * Optimizer stage that tries to replace repeated assignments to
 * existing variables by declarations of new variables as much as
 * possible.
 * The reassignments are still there, but all references to the
 * reassigned variables are replaced by the newly declared variables.
 *
 * Example:
 * {
 *   let a := 1
 *   mstore(a, 2)
 *   a := 3
 * }
 * is transformed to
 * {
 *   let a_1 := 1
 *   let a := a_1
 *   mstore(a_1, 2)
 *   let a_3 := 3
 *   a := a_3
 * }
 *
 * Exact semantics:
 *
 * For any variable a that is assigned to somewhere in the code (assignment with
 * declaration does not count) perform the following transforms:
 *  - replace "let a := v" by "let a_1 := v   let a := a_1"
 *  - replace "a := v" by "let a_1 := v   a := a_1"
 * Furthermore, always note the current variable/value assigned to a and replace each
 * reference to a by this variable.
 * The current value mapping is cleared for a variable a at the end of each block
 * in which it was assigned and just after the for loop init block if it is assigned
 * inside the for loop.
 *
 * After this stage, redundantAssignmentRemover is recommended to remove the unnecessary
 * intermediate assignments.
 *
 * This stage provides best results if CSE is run right before it, because
 * then it does not generate excessive amounts of variables.
 *
 * TODO Which transforms are required to keep this idempotent?
 */
class SSATransform: public ASTModifier
{
public:
	void operator()(Identifier&) override;
	void operator()(ForLoop&) override;
	void operator()(Block& _block) override;

	static void run(Block& _ast, NameDispenser& _nameDispenser);

private:
	explicit SSATransform(NameDispenser& _nameDispenser, std::set<std::string> const& _variablesToReplace):
		m_nameDispenser(_nameDispenser), m_variablesToReplace(_variablesToReplace)
	{ }

	NameDispenser& m_nameDispenser;
	std::set<std::string> const& m_variablesToReplace;
	std::map<std::string, std::string> m_currentVariableValues;
};

}
}
