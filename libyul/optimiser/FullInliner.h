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
 * Optimiser component that performs function inlining for arbitrary functions.
 */
#pragma once

#include <libyul/ASTForward.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Exceptions.h>

#include <liblangutil/SourceLocation.h>

#include <optional>
#include <set>
#include <utility>

namespace solidity::yul
{

class NameCollector;


/**
 * Optimiser component that modifies an AST in place, inlining functions.
 * Expressions are expected to be split, i.e. the component will only inline
 * function calls that are at the root of the expression and that only contains
 * variables or literals as arguments. More specifically, it will inline
 *  - let x1, ..., xn := f(a1, ..., am)
 *  - x1, ..., xn := f(a1, ..., am)
 *  - f(a1, ..., am)
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
 * let f_b := y
 * let f_a := x
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
	static void run(OptimiserStepContext& _context, Block& _ast);

	/// Inlining heuristic.
	/// @param _callSite the name of the function in which the function call is located.
	bool shallInline(FunctionCall const& _funCall, YulName _callSite);

	FunctionDefinition* function(YulName _name)
	{
		auto it = m_functions.find(_name);
		if (it != m_functions.end())
			return it->second;
		return nullptr;
	}

	/// Adds the size of _funCall to the size of _callSite. This is just
	/// a rough estimate that is done during inlining. The proper size
	/// should be determined after inlining is completed.
	void tentativelyUpdateCodeSize(YulName _function, YulName _callSite);

private:
	enum Pass { InlineTiny, InlineRest };

	FullInliner(Block& _ast, NameDispenser& _dispenser, Dialect const& _dialect);
	void run(Pass _pass);

	/// @returns a map containing the maximum depths of a call chain starting at each
	/// function. For recursive functions, the value is one larger than for all others.
	std::map<FunctionHandle, size_t> callDepths() const;

	void updateCodeSize(FunctionDefinition const& _fun);
	void handleBlock(YulName _currentFunctionName, Block& _block);
	bool recursive(FunctionDefinition const& _fun) const;

	Pass m_pass;
	/// The AST to be modified. The root block itself will not be modified, because
	/// we store pointers to functions.
	Block& m_ast;
	std::map<YulName, FunctionDefinition*> m_functions;
	/// Functions not to be inlined (because they contain the ``leave`` statement).
	std::set<YulName> m_noInlineFunctions;
	/// True, if the code contains a ``memoryguard`` and we can expect to be able to move variables to memory later.
	bool m_hasMemoryGuard = false;
	/// Set of recursive functions.
	std::set<FunctionHandle> m_recursiveFunctions;
	/// Names of functions to always inline.
	std::set<YulName> m_singleUse;
	/// Variables that are constants (used for inlining heuristic)
	std::set<YulName> m_constants;
	std::map<YulName, size_t> m_functionSizes;
	NameDispenser& m_nameDispenser;
	Dialect const& m_dialect;
};

/**
 * Class that walks the AST of a block that does not contain function definitions and perform
 * the actual code modifications.
 */
class InlineModifier: public ASTModifier
{
public:
	InlineModifier(FullInliner& _driver, NameDispenser& _nameDispenser, YulName _functionName, Dialect const& _dialect):
		m_currentFunction(std::move(_functionName)),
		m_driver(_driver),
		m_nameDispenser(_nameDispenser),
		m_dialect(_dialect)
	{ }

	void operator()(Block& _block) override;

private:
	std::optional<std::vector<Statement>> tryInlineStatement(Statement& _statement);
	std::vector<Statement> performInline(Statement& _statement, FunctionCall& _funCall);

	YulName m_currentFunction;
	FullInliner& m_driver;
	NameDispenser& m_nameDispenser;
	Dialect const& m_dialect;
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
		std::map<YulName, YulName> _variableReplacements
	):
		m_nameDispenser(_nameDispenser),
		m_variableReplacements(std::move(_variableReplacements))
	{}

	using ASTCopier::operator ();

	Statement operator()(VariableDeclaration const& _varDecl) override;
	Statement operator()(FunctionDefinition const& _funDef) override;

	YulName translateIdentifier(YulName _name) override;

	NameDispenser& m_nameDispenser;
	std::map<YulName, YulName> m_variableReplacements;
};


}
