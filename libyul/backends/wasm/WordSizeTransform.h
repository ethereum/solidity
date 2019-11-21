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
 * Replace every u256 variable with four u64 variables.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>

#include <liblangutil/SourceLocation.h>

#include <array>
#include <vector>

namespace yul
{

/**
 * A stage that replace every u256 variable with four u64 variables.
 * This transformation stage is required because values in EVM are 256 bits,
 * but wasm only supports values up to 64 bits, so we use four u64 values to simulate
 * one u256 value.
 *
 * For FunctionCall that accepts or returns u256 values, they accepts or returns
 * four times the number of values after this transformation, with the order of significance,
 * from the most significant to the least significant.
 *
 * For example, the FunctionCall MUL supplied by code generator
 * should take 8 arguments and return 4 values (instead of 2 and 1) after this transformation.
 *
 * mul(a1, a2, a3, a4, b1, b2, b3, b4) -> c1, c2, c3, c4
 *
 * the value of c4 should be
 *	((a1*(2^192) + a2*(2^128) + a3(2^64) + a4) * (b1*(2^192) + b2*(2^128) + b3(2^64) + b4)) & ((1<<64)-1)
 *
 * The resulting code still uses the EVM builtin functions but assumes that they
 * take four times the parameters and each of type u64.
 * In addition, it uses a single other builtin function called `or_bool` that
 * takes four u64 parameters and is supposed to return the logical disjunction
 * of them as a u64 value. If this name is already used somewhere, it is renamed.
 *
 * Prerequisite: Disambiguator, ForLoopConditionIntoBody, ExpressionSplitter
 */
class WordSizeTransform: public ASTModifier
{
public:
	void operator()(FunctionDefinition&) override;
	void operator()(FunctionCall&) override;
	void operator()(If&) override;
	void operator()(Switch&) override;
	void operator()(ForLoop&) override;
	void operator()(Block& _block) override;

	static void run(Dialect const& _inputDialect, Block& _ast, NameDispenser& _nameDispenser);

private:
	explicit WordSizeTransform(Dialect const& _inputDialect, NameDispenser& _nameDispenser):
		m_inputDialect(_inputDialect),
		m_nameDispenser(_nameDispenser)
	{ }

	void rewriteVarDeclList(std::vector<TypedName>&);
	void rewriteIdentifierList(std::vector<Identifier>&);
	void rewriteFunctionCallArguments(std::vector<Expression>&);

	std::vector<Statement> handleSwitch(Switch& _switch);
	std::vector<Statement> handleSwitchInternal(
		langutil::SourceLocation const& _location,
		std::vector<YulString> const& _splitExpressions,
		std::vector<Case> _cases,
		YulString _runDefaultFlag,
		size_t _depth
	);

	std::array<YulString, 4> generateU64IdentifierNames(YulString const& _s);
	std::array<std::unique_ptr<Expression>, 4> expandValue(Expression const& _e);
	std::vector<Expression> expandValueToVector(Expression const& _e);

	Dialect const& m_inputDialect;
	NameDispenser& m_nameDispenser;
	/// maps original u256 variable's name to corresponding u64 variables' names
	std::map<YulString, std::array<YulString, 4>> m_variableMapping;
};

}
