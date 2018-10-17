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
 * Prerequisites: Disambiguator, Function Hoister
 * More efficient if run after: Expression Splitter
 */
class FullInliner: public ASTModifier
{
public:
	explicit FullInliner(Block& _ast);

	void run();

	/// Inlining heuristic.
	/// @param _callSite the name of the function in which the function call is located.
	bool shallInline(FunctionCall const& _funCall, std::string const& _callSite);

	FunctionDefinition& function(std::string _name) { return *m_functions.at(_name); }

private:
	void updateCodeSize(FunctionDefinition& fun);
	void handleBlock(std::string const& _currentFunctionName, Block& _block);

	/// The AST to be modified. The root block itself will not be modified, because
	/// we store pointers to functions.
	Block& m_ast;
	std::map<std::string, FunctionDefinition*> m_functions;
	/// Names of functions to always inline.
	std::set<std::string> m_alwaysInline;
	/// Variables that are constants (used for inlining heuristic)
	std::set<std::string> m_constants;
	std::map<std::string, size_t> m_functionSizes;
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

	virtual void operator()(Block& _block) override;

private:
	boost::optional<std::vector<Statement>> tryInlineStatement(Statement& _statement);
	std::vector<Statement> performInline(Statement& _statement, FunctionCall& _funCall);

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
