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
 * Optimiser component that performs function inlining.
 */

#include <libjulia/optimiser/FunctionalInliner.h>

#include <libjulia/optimiser/InlinableFunctionFilter.h>
#include <libjulia/optimiser/Substitution.h>
#include <libjulia/optimiser/Semantics.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <boost/algorithm/cxx11/all_of.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;

void FunctionalInliner::run()
{
	InlinableFunctionFilter filter;
	filter(m_block);
	m_inlinableFunctions = filter.inlinableFunctions();

	(*this)(m_block);
}


void FunctionalInliner::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);
	if (_expression.type() == typeid(FunctionCall))
	{
		FunctionCall& funCall = boost::get<FunctionCall>(_expression);

		bool movable = boost::algorithm::all_of(
			funCall.arguments,
			[=](Expression const& _arg) { return MovableChecker(_arg).movable(); }
		);
		if (m_inlinableFunctions.count(funCall.functionName.name) && movable)
		{
			FunctionDefinition const& fun = *m_inlinableFunctions.at(funCall.functionName.name);
			map<string, Expression const*> substitutions;
			for (size_t i = 0; i < fun.parameters.size(); ++i)
				substitutions[fun.parameters[i].name] = &funCall.arguments[i];
			_expression = Substitution(substitutions).translate(*boost::get<Assignment>(fun.body.statements.front()).value);

			// TODO actually in the process of inlining, we could also make a function non-inlinable
			// because it could now call itself

			// If two functions call each other, we have to exit after some iterations.
		}
	}
}
