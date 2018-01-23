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
 * Specific AST walker that collects all defined names.
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
 * Specific AST walker that collects all defined names.
 */
class NameCollector: public ASTWalker
{
public:
	using ASTWalker::operator ();
	virtual void operator()(VariableDeclaration const& _varDecl) override;
	virtual void operator()(FunctionDefinition const& _funDef) override;

	std::set<std::string> const& names() const { return m_names; }
	std::map<std::string, FunctionDefinition const*> const& functions() const { return m_functions; }
private:
	std::set<std::string> m_names;
	std::map<std::string, FunctionDefinition const*> m_functions;
};

}
}
