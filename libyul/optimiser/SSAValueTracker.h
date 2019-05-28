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
 * Component that collects variables that are never assigned to and their
 * initial values.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmData.h>

#include <map>
#include <set>

namespace yul
{

/**
 * Class that walks the AST and stores the initial value of each variable
 * that is never assigned to.
 *
 * A special zero constant expression is used for the default value of variables.
 *
 * Prerequisite: Disambiguator
 */
class SSAValueTracker: public ASTWalker
{
public:
	using ASTWalker::operator();
	void operator()(FunctionDefinition const& _funDef) override;
	void operator()(VariableDeclaration const& _varDecl) override;
	void operator()(Assignment const& _assignment) override;

	std::map<YulString, Expression const*> const& values() const { return m_values; }
	Expression const* value(YulString _name) const { return m_values.at(_name); }

private:
	void setValue(YulString _name, Expression const* _value);

	/// Special expression whose address will be used in m_values.
	/// YulString does not need to be reset because SSAValueTracker is short-lived.
	Expression const m_zero{Literal{{}, LiteralKind::Number, YulString{"0"}, {}}};
	std::map<YulString, Expression const*> m_values;
};

}
