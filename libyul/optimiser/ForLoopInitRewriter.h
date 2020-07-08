// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
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
