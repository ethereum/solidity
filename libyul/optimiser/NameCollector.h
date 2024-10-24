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
 * Specific AST walkers that collect facts about identifiers and definitions.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/YulName.h>

#include <map>
#include <set>

namespace solidity::yul
{

/**
 * Specific AST walker that collects all defined names.
 */
class NameCollector: public ASTWalker
{
public:
	enum CollectWhat { VariablesAndFunctions, OnlyVariables, OnlyFunctions };

	explicit NameCollector(
		Block const& _block,
		CollectWhat _collectWhat = VariablesAndFunctions
	):
		m_collectWhat(_collectWhat)
	{
		(*this)(_block);
	}

	explicit NameCollector(
		FunctionDefinition const& _functionDefinition,
		CollectWhat _collectWhat = VariablesAndFunctions
	):
		m_collectWhat(_collectWhat)
	{
		(*this)(_functionDefinition);
	}

	using ASTWalker::operator ();
	void operator()(VariableDeclaration const& _varDecl) override;
	void operator()(FunctionDefinition const& _funDef) override;

	std::set<YulName> names() const { return m_names; }
private:
	std::set<YulName> m_names;
	CollectWhat m_collectWhat = VariablesAndFunctions;
};

/**
 * Specific AST walker that counts all references to all declarations.
 */
class ReferencesCounter: public ASTWalker
{
public:
	using ASTWalker::operator ();
	void operator()(Identifier const& _identifier) override;
	void operator()(FunctionCall const& _funCall) override;

	static std::map<FunctionHandle, size_t> countReferences(Block const& _block);
	static std::map<FunctionHandle, size_t> countReferences(FunctionDefinition const& _function);
	static std::map<FunctionHandle, size_t> countReferences(Expression const& _expression);

private:
	std::map<FunctionHandle, size_t> m_references;
};

/**
 * Specific AST walker that counts all references to all variable declarations.
 */
class VariableReferencesCounter: public ASTWalker
{
public:
	using ASTWalker::operator ();
	void operator()(Identifier const& _identifier) override;

	static std::map<YulName, size_t> countReferences(Block const& _block);
	static std::map<YulName, size_t> countReferences(FunctionDefinition const& _function);
	static std::map<YulName, size_t> countReferences(Expression const& _expression);
	static std::map<YulName, size_t> countReferences(Statement const& _statement);

private:
	std::map<YulName, size_t> m_references;
};

/**
 * Collects all names from a given continue statement on onwards.
 *
 * It makes only sense to be invoked from within a body of an outer for loop, that is,
 * it will only collect all names from the beginning of the first continue statement
 * of the outer-most ForLoop.
 */
class AssignmentsSinceContinue: public ASTWalker
{
public:
	using ASTWalker::operator();
	void operator()(ForLoop const& _forLoop) override;
	void operator()(Continue const&) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(FunctionDefinition const& _funDef) override;

	std::set<YulName> const& names() const { return m_names; }
	bool empty() const noexcept { return m_names.empty(); }

private:
	size_t m_forLoopDepth = 0;
	bool m_continueFound = false;
	std::set<YulName> m_names;
};

/// @returns the names of all variables that are assigned to inside @a _code.
/// (ignores variable declarations)
std::set<YulName> assignedVariableNames(Block const& _code);

/// @returns all function definitions anywhere in the AST.
/// Requires disambiguated source.
std::map<YulName, FunctionDefinition const*> allFunctionDefinitions(Block const& _block);

}
