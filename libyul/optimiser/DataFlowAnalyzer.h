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
 * Base class to perform data flow analysis during AST walks.
 * Tracks assignments and is used as base class for both Rematerialiser and
 * Common Subexpression Eliminator.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <string>
#include <map>
#include <set>

namespace dev
{
namespace yul
{

/**
 * Base class to perform data flow analysis during AST walks.
 * Tracks assignments and is used as base class for both Rematerialiser and
 * Common Subexpression Eliminator.
 *
 * Prerequisite: Disambiguator
 */
class DataFlowAnalyzer: public ASTModifier
{
public:
	using ASTModifier::operator();
	virtual void operator()(Assignment& _assignment) override;
	virtual void operator()(VariableDeclaration& _varDecl) override;
	virtual void operator()(If& _if) override;
	virtual void operator()(Switch& _switch) override;
	virtual void operator()(FunctionDefinition&) override;
	virtual void operator()(ForLoop&) override;
	virtual void operator()(Block& _block) override;

protected:
	/// Registers the assignment.
	void handleAssignment(std::set<std::string> const& _names, Expression* _value);

	/// Clears information about the values assigned to the given variables,
	/// for example at points where control flow is merged.
	void clearValues(std::set<std::string> const& _names);

	/// Returns true iff the variable is in scope.
	bool inScope(std::string const& _variableName) const;

	/// Current values of variables, always movable.
	std::map<std::string, Expression const*> m_value;
	/// m_references[a].contains(b) <=> the current expression assigned to a references b
	std::map<std::string, std::set<std::string>> m_references;
	/// m_referencedBy[b].contains(a) <=> the current expression assigned to a references b
	std::map<std::string, std::set<std::string>> m_referencedBy;

	struct Scope
	{
		explicit Scope(bool _isFunction): isFunction(_isFunction) {}
		std::set<std::string> variables;
		bool isFunction;
	};
	/// List of scopes.
	std::vector<Scope> m_variableScopes;
};

}
}
