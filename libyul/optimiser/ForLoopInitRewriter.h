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

/**
 * Rewrites ForLoop by moving the pre statement block in front of the ForLoop.
 * Requirements:
 * - The Disambiguator must be run upfront.
 */
class ForLoopInitRewriter: public ASTModifier
{
public:
	static constexpr char const* name{"ForLoopInitRewriter"};
	static void run(OptimiserStepContext&, Block& _ast)
	{
		ForLoopInitRewriter{}(_ast);
	}

	using ASTModifier::operator();
	void operator()(Block& _block) override;

private:
	ForLoopInitRewriter() = default;
};

}
