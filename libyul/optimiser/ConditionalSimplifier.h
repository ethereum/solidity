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
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Dialect.h>
#include <libdevcore/Common.h>

namespace yul
{

/**
 * Conditional simplifier.
 *
 * Inserts assignments to condition variables if the value can be determined
 * from the control-flow.
 *
 * Destroys SSA form.
 *
 * Currently, this tool is very limited, mostly because we do not yet have support
 * for boolean types. Since conditions only check for expressions being nonzero,
 * we cannot assign a specific value.
 *
 * Current features:
 *  - switch cases: insert "<condition> := <caseLabel>"
 *  - after if statement with terminating control-flow, insert "<condition> := 0"
 *
 * Future features:
 *  - allow replacements by "1"
 *  - take termination of user-defined functions into account
 *
 * Works best with SSA form and if dead code removal has run before.
 *
 * Prerequisite: Disambiguator.
 */
class ConditionalSimplifier: public ASTModifier
{
public:
	static constexpr char const* name{"ConditionalSimplifier"};
	static void run(OptimiserStepContext& _context, Block& _ast)
	{
		ConditionalSimplifier{_context.dialect}(_ast);
	}

	using ASTModifier::operator();
	void operator()(Switch& _switch) override;
	void operator()(Block& _block) override;

private:
	explicit ConditionalSimplifier(Dialect const& _dialect):
		m_dialect(_dialect)
	{}
	Dialect const& m_dialect;
};

}
