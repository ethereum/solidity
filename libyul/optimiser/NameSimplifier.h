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
#include <libyul/YulString.h>
#include <libyul/optimiser/OptimiserStep.h>

#include <map>
#include <set>
#include <string>

namespace solidity::yul
{

struct Dialect;

/**
 * Pass to "simplify" all identifier names.
 *
 * The purpose of this is to make generated code more readable, but also
 * to remove AST identifiers that could lead to a different sorting order
 * and thus influence e.g. the order of function inlining.
 *
 * Prerequisites: Disambiguator, FunctionHoister, FunctionGrouper
 */
class NameSimplifier: public ASTModifier
{
public:
	static constexpr char const* name{"NameSimplifier"};
	static void run(OptimiserStepContext& _context, Block& _ast)
	{
		NameSimplifier{_context, _ast}(_ast);
	}

	using ASTModifier::operator();
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(Identifier& _identifier) override;
	void operator()(FunctionCall& _funCall) override;
	void operator()(FunctionDefinition& _funDef) override;

private:
	NameSimplifier(OptimiserStepContext& _context, Block const& _ast);

	/// Tries to rename a list of variables.
	void renameVariables(std::vector<TypedName>& _variables);

	void findSimplification(YulString const& _name);
	void translate(YulString& _name);

	OptimiserStepContext& m_context;
	std::map<YulString, YulString> m_translations;
};

}
