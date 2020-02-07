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
