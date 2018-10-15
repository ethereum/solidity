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
 * Optimisation stage that removes unused variables and functions.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <string>
#include <map>
#include <set>

namespace dev
{
namespace yul
{

/**
 * Optimisation stage that removes unused variables and functions.
 *
 * TODO: Also remove intermediate variable assignments from movable expressions
 * which are not referenced until after the next assignment to the same variable.
 *
 * Note that this does not remove circular references.
 *
 * Prerequisite: Disambiguator
 */
class UnusedPruner: public ASTModifier
{
public:
	explicit UnusedPruner(Block& _ast);

	using ASTModifier::operator();
	virtual void operator()(Block& _block) override;

	// @returns true iff the code changed in the previous run.
	bool shouldRunAgain() const { return m_shouldRunAgain; }

	// Run the pruner until the code does not change anymore.
	static void runUntilStabilised(Block& _ast);

private:
	bool used(std::string const& _name) const;
	void subtractReferences(std::map<std::string, size_t> const& _subtrahend);

	bool m_shouldRunAgain = false;
	std::map<std::string, size_t> m_references;
};

}
}
