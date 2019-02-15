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
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>

namespace yul
{

/**
 * Structural simplifier. Performs the following simplification steps:
 * - replace if with empty body with pop(condition)
 * - replace if with true condition with its body
 * - remove if with false condition
 * - turn switch with single case into if
 * - replace switch with only default case with pop(expression) and body
 * - remove for with false condition
 *
 * Prerequisites: Disambiguator
 *
 * Important: Can only be used on EVM code.
 */
class StructuralSimplifier: public DataFlowAnalyzer
{
public:
	explicit StructuralSimplifier(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}

	using DataFlowAnalyzer::operator();
	void operator()(Block& _block) override;
private:
	void simplify(std::vector<Statement>& _statements);
	bool expressionAlwaysTrue(Expression const& _expression);
	bool expressionAlwaysFalse(Expression const& _expression);
};

}
