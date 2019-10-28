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
 * Optimiser component that performs function inlining for arbitrary functions.
 */
#pragma once

#include <libyul/AsmDataForward.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Exceptions.h>

#include <liblangutil/SourceLocation.h>

#include <boost/variant.hpp>

#include <optional>
#include <set>

namespace yul
{

class NameCollector;


/**
 * Optimiser component that modifies an AST in place, inlining functions.
 * Expressions are expected to be split, i.e. the component will only inline
 * function calls that are at the root of the expression and that only contains
 * variables as arguments. More specifically, it will inline
 *  - let x1, ..., xn := f(a1, ..., am)
 *  - x1, ..., xn := f(a1, ..., am)
 * f(a1, ..., am)
 *
 * The transform changes code of the form
 *
 * function f(a, b) -> c { ... }
 * let z := f(x, y)
 *
 * into
 *
 * function f(a, b) -> c { ... }
 *
 * let f_a := x
 * let f_b := y
 * let f_c
 * code of f, with replacements: a -> f_a, b -> f_b, c -> f_c
 * let z := f_c
 *
 * Prerequisites: Disambiguator
 * More efficient if run after: Function Hoister, Expression Splitter
 */
class FullInliner: public ASTModifier
{
public:
	static constexpr char const* name{"FullInliner"};
	static void run(OptimiserStepContext&, Block& _ast);

	/// Inlining heuristic.
	/// @param _callSite the name of the function in which the function call is located.
	bool shallInline(FunctionCall const& _funCall, YulString _callSite);

	FunctionDefinition* function(YulString _name)
	{
		auto it = m_functions.find(_name);
		if (it != m_functions.end())
			return it->second;
		return nullptr;
	}

	/// Adds the size of _funCall to the size of _callSite. This is just
	/// a rough estimate that is done during inlining. The proper size
	/// should be determined after inlining is completed.
	void tentativelyUpdateCodeSize(YulString _function, YulString _callSite);

private:
	FullInliner(Block& _ast, NameDispenser& _dispenser);
	void run();

	void updateCodeSize(FunctionDefinition const& _fun);
	void handleBlock(YulString _currentFunctionName, Block& _block);
	bool recursive(FunctionDefinition const& _fun) const;

	/// The AST to be modified. The root block itself will not be modified, because
	/// we store pointers to functions.
	Block& m_ast;
	std::map<YulString, FunctionDefinition*> m_functions;
	/// Functions not to be inlined (because they contain the ``leave`` statement).
	std::set<YulString> m_noInlineFunctions;
	/// Names of functions to always inline.
	std::set<YulString> m_singleUse;
	/// Variables that are constants (used for inlining heuristic)
	std::set<YulString> m_constants;
	std::map<YulString, size_t> m_functionSizes;
	NameDispenser& m_nameDispenser;
};

/**
 * Class that walks the AST of a block that does not contain function definitions and perform
 * the actual code modifications.
 */
class InlineModifier: public ASTModifier
{
public:
	InlineModifier(FullInliner& _driver, NameDispenser& _nameDispenser, YulString _functionName):
		m_currentFunction(std::move(_functionName)),
		m_driver(_driver),
		m_nameDispenser(_nameDispenser)
	{ }

	void operator()(Block& _block) override;

private:
	std::optional<std::vector<Statement>> tryInlineStatement(Statement& _statement);
	std::vector<Statement> performInline(Statement& _statement, FunctionCall& _funCall);

	YulString m_currentFunction;
	FullInliner& m_driver;
	NameDispenser& m_nameDispenser;
};

/**
 * Creates a copy of a block that is supposed to be the body of a function.
 * Applies replacements to referenced variables and creates new names for
 * variable declarations.
 */
class BodyCopier: public ASTCopier
{
public:
	BodyCopier(
		NameDispenser& _nameDispenser,
		std::map<YulString, YulString> const& _variableReplacements
	):
		m_nameDispenser(_nameDispenser),
		m_variableReplacements(_variableReplacements)
	{}

	using ASTCopier::operator ();

	Statement operator()(VariableDeclaration const& _varDecl) override;
	Statement operator()(FunctionDefinition const& _funDef) override;

	YulString translateIdentifier(YulString _name) override;

	NameDispenser& m_nameDispenser;
	std::map<YulString, YulString> m_variableReplacements;
};


}
