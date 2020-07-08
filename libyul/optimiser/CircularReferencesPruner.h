// SPDX-License-Identifier: GPL-3.0
/**
 * Optimization stage that removes functions that call each other but are
 * otherwise unreferenced.
 *
 *  Prerequisites: Disambiguator, FunctionHoister.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * Optimization stage that removes functions that call each other but are
 * neither externally referenced nor referenced from the outermost context.
 */
class CircularReferencesPruner: public ASTModifier
{
public:
	static constexpr char const* name{"CircularReferencesPruner"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	using ASTModifier::operator();
	void operator()(Block& _block) override;
private:
	CircularReferencesPruner(std::set<YulString> const& _reservedIdentifiers):
		m_reservedIdentifiers(_reservedIdentifiers)
	{}

	/// Run a breadth-first search starting from the outermost context and
	/// externally referenced functions to find all the functions that are
	/// called from there either directly or indirectly.
	std::set<YulString> functionsCalledFromOutermostContext(CallGraph const& _callGraph);

	std::set<YulString> const& m_reservedIdentifiers;
};

}
