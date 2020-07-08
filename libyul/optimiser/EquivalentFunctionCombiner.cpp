// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that combines syntactically equivalent functions.
 */

#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/AsmData.h>
#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void EquivalentFunctionCombiner::run(OptimiserStepContext&, Block& _ast)
{
	EquivalentFunctionCombiner{EquivalentFunctionDetector::run(_ast)}(_ast);
}

void EquivalentFunctionCombiner::operator()(FunctionCall& _funCall)
{
	auto it = m_duplicates.find(_funCall.functionName.name);
	if (it != m_duplicates.end())
		_funCall.functionName.name = it->second->name;
	ASTModifier::operator()(_funCall);
}
