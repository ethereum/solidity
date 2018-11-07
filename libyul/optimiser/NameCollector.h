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
 * Specific AST walkers that collect facts about identifiers and definitions.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <map>
#include <set>

namespace dev
{
namespace yul
{

/**
 * Specific AST walker that collects all defined names.
 */
class NameCollector: public ASTWalker
{
public:
	explicit NameCollector(Block const& _block)
	{
		(*this)(_block);
	}

	using ASTWalker::operator ();
	virtual void operator()(VariableDeclaration const& _varDecl) override;
	virtual void operator()(FunctionDefinition const& _funDef) override;

	std::set<YulString> names() const { return m_names; }
private:
	std::set<YulString> m_names;
};

/**
 * Specific AST walker that counts all references to all declarations.
 */
class ReferencesCounter: public ASTWalker
{
public:
	using ASTWalker::operator ();
	virtual void operator()(Identifier const& _identifier);
	virtual void operator()(FunctionCall const& _funCall);

	static std::map<YulString, size_t> countReferences(Block const& _block);
	static std::map<YulString, size_t> countReferences(Expression const& _expression);

	std::map<YulString, size_t> const& references() const { return m_references; }
private:
	std::map<YulString, size_t> m_references;
};

/**
 * Specific AST walker that finds all variables that are assigned to.
 */
class Assignments: public ASTWalker
{
public:
	using ASTWalker::operator ();
	virtual void operator()(Assignment const& _assignment) override;

	std::set<YulString> const& names() const { return m_names; }
private:
	std::set<YulString> m_names;
};

}
}
