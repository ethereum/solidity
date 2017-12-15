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
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 */

#pragma once

#include <libjulia/optimiser/ASTWalker.h>

#include <string>
#include <map>
#include <set>

namespace dev
{
namespace julia
{

/**
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 *
 * Prerequisite: Disambiguator
 */
class Rematerialiser: public ASTModifier
{
public:
	using ASTModifier::operator();
	virtual void operator()(Assignment& _assignment) override;
	virtual void operator()(VariableDeclaration& _varDecl) override;
	virtual void operator()(If& _if) override;
	virtual void operator()(Switch& _switch) override;
	virtual void operator()(ForLoop&) override;

protected:
	virtual void visit(Expression& _e) override;

private:
	void handleAssignment(std::set<std::string> const& _names, Expression* _value);

	/// Substitutions to be performed, if possible.
	std::map<std::string, Expression const*> m_substitutions;
	/// m_references[a].contains(b) <=> the current expression assigned to a references b
	std::map<std::string, std::set<std::string>> m_references;
	/// m_referencedBy[b].contains(a) <=> the current expression assigned to a references b
	std::map<std::string, std::set<std::string>> m_referencedBy;
};

}
}
