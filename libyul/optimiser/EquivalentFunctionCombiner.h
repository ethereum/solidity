// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that combines syntactically equivalent functions.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/EquivalentFunctionDetector.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AsmDataForward.h>

namespace solidity::yul
{

struct OptimiserStepContext;

/**
 * Optimiser component that detects syntactically equivalent functions and replaces all calls to any of them by calls
 * to one particular of them.
 *
 * Prerequisite: Disambiguator, Function Hoister
 */
class EquivalentFunctionCombiner: public ASTModifier
{
public:
	static constexpr char const* name{"EquivalentFunctionCombiner"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(FunctionCall& _funCall) override;

private:
	EquivalentFunctionCombiner(std::map<YulString, FunctionDefinition const*> _duplicates): m_duplicates(std::move(_duplicates)) {}
	std::map<YulString, FunctionDefinition const*> m_duplicates;
};


}
