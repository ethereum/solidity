// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that combines syntactically equivalent functions.
 */

#include <libyul/optimiser/EquivalentFunctionDetector.h>
#include <libyul/optimiser/SyntacticalEquality.h>

#include <libyul/AsmData.h>
#include <libyul/optimiser/Metrics.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void EquivalentFunctionDetector::operator()(FunctionDefinition const& _fun)
{
	uint64_t bodyHash = m_blockHashes[&_fun.body];
	auto& candidates = m_candidates[bodyHash];
	for (auto const& candidate: candidates)
		if (SyntacticallyEqual{}.statementEqual(_fun, *candidate))
		{
			m_duplicates[_fun.name] = candidate;
			return;
		}
	candidates.push_back(&_fun);
}
