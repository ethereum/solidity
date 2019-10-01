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
#include <libyul/optimiser/OptimiserStep.h>

namespace yul
{

class AssignmentCounter;

/**
 * Reverses the SSA transformation.
 *
 * In particular, the SSA transform will rewrite
 *
 * 		a := E
 *
 * to
 *
 * 		let a_1 := E
 * 		a := a_1
 *
 * To undo this kind of transformation, the SSAReverser changes this back to
 *
 * 		a := E
 * 		let a_1 := a
 *
 * 	In the special case
 * 		let a := E
 * 		a := a
 *
 * 	the redundant assignment "a := a" is removed.
 *
 *
 * Secondly, the SSA transform will rewrite
 *
 * 		let a := E
 * to
 *
 * 		let a_1 := E
 * 		let a := a_1
 *
 * To undo this kind of transformation, the SSAReverser changes this back to
 *
 * 		let a := E
 * 		let a_1 := a
 *
 * 	After that the CSE can replace references of a_1 by references to a,
 * 	after which the unused pruner can remove the declaration of a_1.
 *
 * 	Prerequisites: Disambiguator
 *
 */
class SSAReverser: public ASTModifier
{
public:
	static constexpr char const* name{"SSAReverser"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	using ASTModifier::operator();
	void operator()(Block& _block) override;

private:
	explicit SSAReverser(AssignmentCounter const& _assignmentCounter): m_assignmentCounter(_assignmentCounter) {}

	AssignmentCounter const& m_assignmentCounter;
};

}
