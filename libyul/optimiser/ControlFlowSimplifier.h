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

namespace yul
{

/**
 * Simplifies several control-flow structures:
 * - replace if with empty body with pop(condition)
 * - remove empty default switch case
 * - remove empty switch case if no default case exists
 * - replace switch with no cases with pop(expression)
 * - turn switch with single case into if
 * - replace switch with only default case with pop(expression) and body
 * - replace switch with const expr with matching case body
 * - replace ``for`` with terminating control flow and without other break/continue by ``if``
 *
 * None of these operations depend on the data flow. The StructuralSimplifier
 * performs similar tasks that do depend on data flow.
 *
 * The ControlFlowSimplifier does record the presence or absence of ``break``
 * and ``continue`` statements during its traversal.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 *
 * Important: Introduces EVM opcodes and thus can only be used on EVM code for now.
 */
class ControlFlowSimplifier: public ASTModifier
{
public:
	using ASTModifier::operator();
	void operator()(Block& _block) override;
private:
	void simplify(std::vector<Statement>& _statements);
};

}
