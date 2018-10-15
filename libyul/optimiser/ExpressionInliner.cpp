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
 * Optimiser component that performs function inlining inside expressions.
 */

#include <libyul/optimiser/ExpressionInliner.h>

#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>
#include <libyul/optimiser/Substitution.h>
#include <libyul/optimiser/Semantics.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <boost/algorithm/cxx11/all_of.hpp>

using namespace std;
using namespace dev;
using namespace dev::yul;
using namespace dev::solidity;

void ExpressionInliner::run()
{
	InlinableExpressionFunctionFinder funFinder;
	funFinder(m_block);
	m_inlinableFunctions = funFinder.inlinableFunctions();

	(*this)(m_block);
}


void ExpressionInliner::operator()(FunctionDefinition& _fun)
{
	ASTModifier::operator()(_fun);
}

void ExpressionInliner::visit(Expression& _expression)
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

			// TODO Add metric! This metric should perform well on a pair of functions who
			// call each other recursively.
		}
	}
}
