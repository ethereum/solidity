// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * Rewrites variable declarations so that all of them are initialized.
 * Declarations like ``let x, y`` are split into multiple declaration
 * statements.
 * Only supports initializing with the zero literal for now.
 */
class VarDeclInitializer: public ASTModifier
{
public:
	static constexpr char const* name{"VarDeclInitializer"};
	static void run(OptimiserStepContext& _ctx, Block& _ast) { VarDeclInitializer{_ctx.dialect}(_ast); }

	void operator()(Block& _block) override;

private:
	explicit VarDeclInitializer(Dialect const& _dialect): m_dialect(_dialect) {}

	Dialect const& m_dialect;
};

}
