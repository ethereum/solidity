// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that removes unreachable code.
 */

#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AsmData.h>

#include <libevmasm/SemanticInformation.h>
#include <libevmasm/AssemblyItem.h>

#include <algorithm>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

void DeadCodeEliminator::run(OptimiserStepContext& _context, Block& _ast)
{
	DeadCodeEliminator{_context.dialect}(_ast);
}

void DeadCodeEliminator::operator()(ForLoop& _for)
{
	yulAssert(_for.pre.statements.empty(), "DeadCodeEliminator needs ForLoopInitRewriter as a prerequisite.");
	ASTModifier::operator()(_for);
}

void DeadCodeEliminator::operator()(Block& _block)
{
	TerminationFinder::ControlFlow controlFlowChange;
	size_t index;
	tie(controlFlowChange, index) = TerminationFinder{m_dialect}.firstUnconditionalControlFlowChange(_block.statements);

	// Erase everything after the terminating statement that is not a function definition.
	if (controlFlowChange != TerminationFinder::ControlFlow::FlowOut && index != std::numeric_limits<size_t>::max())
		_block.statements.erase(
			remove_if(
				_block.statements.begin() + static_cast<ptrdiff_t>(index) + 1,
				_block.statements.end(),
				[] (Statement const& _s) { return !holds_alternative<yul::FunctionDefinition>(_s); }
			),
			_block.statements.end()
		);

	ASTModifier::operator()(_block);
}
