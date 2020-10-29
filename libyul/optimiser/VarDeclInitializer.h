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

#pragma once

#include <libyul/ASTForward.h>
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
