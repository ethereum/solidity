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
 * Optimisation stage that replaces the literal 0 by returndatasize() in case there could not have been a call yet.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>
#include <libsolutil/Common.h>

namespace solidity::yul
{

/**
 * Optimisation stage that replaces the literal 0 by returndatasize() in case there could not have been a call yet.
 *
 * Only works for an EVMDialect that supports returndata and only works for a dialect with object access, since
 * the analysis requires the entire contract to be available as Yul AST.
 *
 * Prerequisite: Disambiguator
 */
class ZeroByReturndatasizeReplacer: public ASTModifier
{
public:
	static constexpr char const* name{"ZeroByReturndatasizeReplacer"};
	static void run(OptimiserStepContext& _context, Block& _ast);

protected:
	using ASTModifier::operator();
	using ASTModifier::visit;

	void operator()(FunctionCall& _funCall) override;
	void operator()(FunctionDefinition& _funDef) override;
	void operator()(ForLoop& _loop) override;

	void visit(Expression& _e) override;

private:
	explicit ZeroByReturndatasizeReplacer(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> const& _functionSideEffects,
		std::set<YulString> const& _badFunctions,
		std::set<ForLoop const*> const& _badLoops
	):
	m_dialect(_dialect),
	m_functionSideEffects(_functionSideEffects),
	m_badFunctions(_badFunctions),
	m_badLoops(_badLoops)
	{}
	bool m_hasZeroReturndata = true;
	Dialect const& m_dialect;
	std::map<YulString, SideEffects>  const& m_functionSideEffects;
	std::set<YulString> const& m_badFunctions;
	std::set<ForLoop const*> const& m_badLoops;
};

}
