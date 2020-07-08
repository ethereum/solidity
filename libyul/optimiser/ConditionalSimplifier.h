// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Dialect.h>
#include <libsolutil/Common.h>

namespace solidity::yul
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
