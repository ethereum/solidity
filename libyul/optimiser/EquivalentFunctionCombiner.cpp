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
// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that combines syntactically equivalent functions.
 */

#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/AST.h>
#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;

void EquivalentFunctionCombiner::run(OptimiserStepContext&, Block& _ast)
{
	EquivalentFunctionCombiner{EquivalentFunctionDetector::run(_ast)}(_ast);
}

void EquivalentFunctionCombiner::operator()(FunctionCall& _funCall)
{
	if (!isBuiltinFunctionCall(_funCall))
	{
		auto* identifier = std::get_if<Identifier>(&_funCall.functionName);
		yulAssert(identifier);
		auto it = m_duplicates.find(identifier->name);
		if (it != m_duplicates.end())
			identifier->name = it->second->name;
	}
	ASTModifier::operator()(_funCall);
}
