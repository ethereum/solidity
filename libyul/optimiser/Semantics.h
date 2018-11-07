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
 * Specific AST walkers that collect semantical facts.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <set>

namespace dev
{
namespace yul
{

/**
 * Specific AST walker that determines whether an expression is movable.
 */
class MovableChecker: public ASTWalker
{
public:
	MovableChecker() = default;
	explicit MovableChecker(Expression const& _expression);

	virtual void operator()(Identifier const& _identifier) override;
	virtual void operator()(FunctionalInstruction const& _functionalInstruction) override;
	virtual void operator()(FunctionCall const& _functionCall) override;

	/// Disallow visiting anything apart from Expressions (this throws).
	virtual void visit(Statement const&) override;
	using ASTWalker::visit;

	bool movable() const { return m_movable; }
	std::set<YulString> const& referencedVariables() const { return m_variableReferences; }

private:
	/// Which variables the current expression references.
	std::set<YulString> m_variableReferences;
	/// Is the current expression movable or not.
	bool m_movable = true;
};

}
}
