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

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/OptimiserStep.h>

#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>

#include <map>
#include <optional>
#include <vector>

namespace solidity::yul
{

/**
 * FunctionSpecializer: Optimiser step that specializes the function with its literal arguments.
 *
 * If a function, say, `function f(a, b) { sstore (a, b)}`, is called with literal arguments, for
 * example, `f(x, 5)`, where `x` is an identifier, it could be specialized by creating a new
 * function `f_1` that takes only one argument, i.e.,
 *
 *   function f_1(a_1) {
 *     let b_1 := 5
 *     sstore(a_1, b_1)
 *   }
 *
 * Other optimization steps will be able to make more simplifications to the function. The
 * optimization step is mainly useful for functions that would not be inlined.
 *
 * Prerequisites: Disambiguator, FunctionHoister
 *
 * LiteralRematerialiser is recommended as a prerequisite, even though it's not required for
 * correctness.
 */
class FunctionSpecializer: public ASTModifier
{
public:
	/// A vector of function-call arguments. An element 'has value' if it's a literal, and the
	/// corresponding Expression would be the literal.
	using LiteralArguments = std::vector<std::optional<Expression>>;

	static constexpr char const* name{"FunctionSpecializer"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	using ASTModifier::operator();
	void operator()(FunctionCall& _f) override;

private:
	explicit FunctionSpecializer(
		std::set<YulString> _recursiveFunctions,
		NameDispenser& _nameDispenser,
		Dialect const& _dialect
	):
		m_recursiveFunctions(std::move(_recursiveFunctions)),
		m_nameDispenser(_nameDispenser),
		m_dialect(_dialect)
	{}
	/// Returns a vector of Expressions, where the index `i` is an expression if the function's
	/// `i`-th argument can be specialized, nullopt otherwise.
	LiteralArguments specializableArguments(FunctionCall const& _f);
	/// Given a function definition `_f` and its arguments `_arguments`, of which, at least one is a
	/// literal, this function returns a new function with the literal arguments specialized.
	///
	/// Note that the returned function definition will have new (and unique) names, for both the
	/// function and variable declarations to retain the properties enforced by the Disambiguator.
	///
	/// For example, if `_f` is the function `function f(a, b, c) -> d { sstore(a, b) }`,
	/// `_arguments` is the vector of literals `{1, 2, nullopt}` and the @param, `_newName` has
	/// value `f_1`, the returned function could be:
	///
	///   function f_1(c_2) -> d_3 {
	///     let a_4 := 1
	///     let b_5 := 2
	///     sstore(a_4, b_5)
	///   }
	///
	FunctionDefinition specialize(
		FunctionDefinition const& _f,
		YulString _newName,
		FunctionSpecializer::LiteralArguments _arguments
	);

	/// A mapping between the old function name and a pair of new function name and its arguments.
	/// Note that at least one of the argument will have a literal value.
	std::map<YulString, std::vector<std::pair<YulString, LiteralArguments>>> m_oldToNewMap;
	/// We skip specializing recursive functions. Need backtracking to properly deal with them.
	std::set<YulString> const m_recursiveFunctions;

	NameDispenser& m_nameDispenser;
	Dialect const& m_dialect;
};

}
