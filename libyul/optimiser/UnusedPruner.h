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
#include <libyul/YulString.h>

#include <map>
#include <set>

namespace yul
{
struct Dialect;

/**
 * Optimisation stage that removes unused variables and functions and also
 * removes movable expression statements.
 *
 * Note that this does not remove circular references.
 *
 * Prerequisite: Disambiguator
 */
class UnusedPruner: public ASTModifier
{
public:
	UnusedPruner(
		Dialect const& _dialect,
		Block& _ast,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);

	using ASTModifier::operator();
	void operator()(Block& _block) override;

	// @returns true iff the code changed in the previous run.
	bool shouldRunAgain() const { return m_shouldRunAgain; }

	// Run the pruner until the code does not change anymore.
	static void runUntilStabilised(
		Dialect const& _dialect,
		Block& _ast,
		std::set<YulString> const& _externallyUsedFunctions = {}
	);

private:
	bool used(YulString _name) const;
	void subtractReferences(std::map<YulString, size_t> const& _subtrahend);

	Dialect const& m_dialect;
	bool m_shouldRunAgain = false;
	std::map<YulString, size_t> m_references;
};

}
