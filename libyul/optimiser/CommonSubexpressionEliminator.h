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
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 */

#pragma once

#include <libyul/optimiser/DataFlowAnalyzer.h>

namespace yul
{

struct Dialect;
struct SideEffects;

/**
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class CommonSubexpressionEliminator: public DataFlowAnalyzer
{
public:
	/// Runs the CSE pass. @a _ast needs to be the complete AST of the program!
	static void run(Dialect const& _dialect, Block& _ast);

private:
	CommonSubexpressionEliminator(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> _functionSideEffects
	);

protected:
	using ASTModifier::visit;
	void visit(Expression& _e) override;
};

}
