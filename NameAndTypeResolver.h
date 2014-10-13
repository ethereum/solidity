/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Parser part that determines the declarations corresponding to names and the types of expressions.
 */

#pragma once

#include <map>

#include <boost/noncopyable.hpp>

#include <libsolidity/Scope.h>
#include <libsolidity/ASTVisitor.h>

namespace dev {
namespace solidity {

class NameAndTypeResolver : private boost::noncopyable
{
public:
	NameAndTypeResolver();

	void resolveNamesAndTypes(ContractDefinition& _contract);
private:
	class ScopeHelper; //< RIIA helper to open and close scopes

	void reset();

	void handleContract(ContractDefinition& _contract);
	void handleFunction(FunctionDefinition& _function);
	void registerVariablesInFunction(FunctionDefinition& _function);
	void resolveReferencesInFunction(ParameterList& _returnParameters,
									 Block& _functionBody);

	void registerVariableDeclarationAndResolveType(VariableDeclaration& _variable);
	void registerDeclaration(Declaration& _declaration);
	Declaration* getNameFromCurrentScope(ASTString const& _name, bool _recursive = true);

	void enterNewSubScope(ASTNode& _node);
	void closeCurrentScope();

	Scope m_globalScope; // not part of the map
	std::map<ASTNode*, Scope> m_scopes;

	Scope* m_currentScope;
};

} }
