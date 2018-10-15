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

#include <libyul/ASTDataForward.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/Exceptions.h>

#include <libevmasm/SourceLocation.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace dev
{
namespace julia
{

class NameCollector;


/**
 * Optimiser component that modifies an AST in place, inlining arbitrary functions.
 *
 * Code of the form
 *
 * function f(a, b) -> c { ... }
 * h(g(x(...), f(arg1(...), arg2(...)), y(...)), z(...))
 *
 * is transformed into
 *
 * function f(a, b) -> c { ... }
 *
 * let z1 := z(...) let y1 := y(...) let a2 := arg2(...) let a1 := arg1(...)
 * let c1 := 0
 * { code of f, with replacements: a -> a1, b -> a2, c -> c1, d -> d1 }
 * h(g(x(...), c1, y1), z1)
 *
 * No temporary variable is created for expressions that are "movable"
 * (i.e. they are "pure", have no side-effects and also do not depend on other code
 * that might have side-effects).
 *
 * This component can only be used on sources with unique names and with hoisted functions,
 * i.e. the root node has to be a block that itself contains a single block followed by all
 * function definitions.
 */
class FullInliner: public ASTModifier
{
public:
	explicit FullInliner(Block& _ast);

	void run();

	/// Perform inlining operations inside the given function.
	void handleFunction(FunctionDefinition& _function);

	FunctionDefinition& function(std::string _name) { return *m_functions.at(_name); }

private:
	/// The AST to be modified. The root block itself will not be modified, because
	/// we store pointers to functions.
	Block& m_ast;
	std::map<std::string, FunctionDefinition*> m_functions;
	std::set<FunctionDefinition*> m_functionsToVisit;
	NameDispenser m_nameDispenser;
};

/**
 * Class that walks the AST of a block that does not contain function definitions and perform
 * the actual code modifications.
 */
class InlineModifier: public ASTModifier
{
public:
	InlineModifier(FullInliner& _driver, NameDispenser& _nameDispenser, std::string _functionName):
		m_currentFunction(std::move(_functionName)),
		m_driver(_driver),
		m_nameDispenser(_nameDispenser)
	{ }
	~InlineModifier()
	{
		assertThrow(m_statementsToPrefix.empty(), OptimizerException, "");
	}

	virtual void operator()(FunctionalInstruction&) override;
	virtual void operator()(FunctionCall&) override;
	virtual void operator()(ForLoop&) override;
	virtual void operator()(Block& _block) override;

	using ASTModifier::visit;
	virtual void visit(Expression& _expression) override;
	virtual void visit(Statement& _st) override;

private:

	/// Visits a list of expressions (usually an argument list to a function call) and tries
	/// to inline them. If one of them is inlined, all right of it have to be moved to the front
	/// (to keep the order of evaluation). If @a _moveToFront is true, all elements are moved
	/// to the front. @a _nameHints and @_types are used for the newly created variables, but
	/// both can be empty.
	void visitArguments(
		std::vector<Expression>& _arguments,
		std::vector<std::string> const& _nameHints = {},
		std::vector<std::string> const& _types = {},
		bool _moveToFront = false
	);

	/// Visits an expression, but saves and restores the current statements to prefix and returns
	/// the statements that should be prefixed for @a _expression.
	std::vector<Statement> visitRecursively(Expression& _expression);

	std::string newName(std::string const& _prefix);

	/// @returns an expression returning nothing.
	Expression noop(SourceLocation const& _location);

	/// List of statements that should go in front of the currently visited AST element,
	/// at the statement level.
	std::vector<Statement> m_statementsToPrefix;
	std::string m_currentFunction;
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
		std::string const& _varNamePrefix,
		std::map<std::string, std::string> const& _variableReplacements
	):
		m_nameDispenser(_nameDispenser),
		m_varNamePrefix(_varNamePrefix),
		m_variableReplacements(_variableReplacements)
	{}

	using ASTCopier::operator ();

	virtual Statement operator()(VariableDeclaration const& _varDecl) override;
	virtual Statement operator()(FunctionDefinition const& _funDef) override;

	virtual std::string translateIdentifier(std::string const& _name) override;

	NameDispenser& m_nameDispenser;
	std::string const& m_varNamePrefix;
	std::map<std::string, std::string> m_variableReplacements;
};


}
}
