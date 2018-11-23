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
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <libyul/optimiser/ASTWalker.h>

namespace yul
{

/**
 * Applies simplification rules to all expressions.
 * The component will work best if the code is in SSA form, but
 * this is not required for correctness.
 *
 * Prerequisite: Disambiguator.
 */
class ExpressionSimplifier: public ASTModifier
{
public:
	using ASTModifier::operator();
	virtual void visit(Expression& _expression);

	static void run(Block& _ast);
private:
	explicit ExpressionSimplifier(std::map<YulString, Expression const*> _ssaValues):
		m_ssaValues(std::move(_ssaValues))
	{}

	std::map<YulString, Expression const*> m_ssaValues;
};

}
