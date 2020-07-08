// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Dialect.h>
#include <libsolutil/Common.h>

namespace solidity::yul
{

/**
 * Reverse of conditional simplifier.
 *
 */
class ConditionalUnsimplifier: public ASTModifier
{
public:
	static constexpr char const* name{"ConditionalUnsimplifier"};
	static void run(OptimiserStepContext& _context, Block& _ast)
	{
		ConditionalUnsimplifier{_context.dialect}(_ast);
	}

	using ASTModifier::operator();
	void operator()(Switch& _switch) override;
	void operator()(Block& _block) override;

private:
	explicit ConditionalUnsimplifier(Dialect const& _dialect):
		m_dialect(_dialect)
	{}
	Dialect const& m_dialect;
};

}
