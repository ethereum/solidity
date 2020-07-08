// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that changes the code so that all function definitions are at the top
 * level block.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>

namespace solidity::yul
{
struct OptimiserStepContext;

/**
 * Moves all functions to the top-level scope.
 * Applying this transformation to source code that has ambiguous identifiers might
 * lead to invalid code.
 *
 * Prerequisites: Disambiguator
 */
class FunctionHoister: public ASTModifier
{
public:
	static constexpr char const* name{"FunctionHoister"};
	static void run(OptimiserStepContext&, Block& _ast) { FunctionHoister{}(_ast); }

	using ASTModifier::operator();
	void operator()(Block& _block) override;

private:
	FunctionHoister() = default;

	bool m_isTopLevel = true;
	std::vector<Statement> m_functions;
};

}
