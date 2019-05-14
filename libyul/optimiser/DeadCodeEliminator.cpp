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
 * Optimisation stage that removes unreachable code.
 */

#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>

#include <libevmasm/SemanticInformation.h>
#include <libevmasm/AssemblyItem.h>

#include <algorithm>

using namespace std;
using namespace dev;
using namespace yul;


void DeadCodeEliminator::operator()(ForLoop& _for)
{
	yulAssert(_for.pre.statements.empty(), "DeadCodeEliminator needs ForLoopInitRewriter as a prerequisite.");
	ASTModifier::operator()(_for);
}

void DeadCodeEliminator::operator()(Block& _block)
{
	TerminationFinder::ControlFlow controlFlowChange;
	size_t index;
	tie(controlFlowChange, index) = TerminationFinder::firstUnconditionalControlFlowChange(_block.statements);

	// Erase everything after the terminating statement that is not a function definition.
	if (controlFlowChange != TerminationFinder::ControlFlow::FlowOut && index != size_t(-1))
		_block.statements.erase(
			remove_if(
				_block.statements.begin() + index + 1,
				_block.statements.end(),
				[] (Statement const& _s) { return _s.type() != typeid(yul::FunctionDefinition); }
			),
			_block.statements.end()
		);

	ASTModifier::operator()(_block);
}

