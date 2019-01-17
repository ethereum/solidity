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
 * Optimiser component that combines syntactically equivalent functions.
 */

#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/AsmData.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::solidity;

void EquivalentFunctionCombiner::run(Block& _ast)
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
