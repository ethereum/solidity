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
 * Scope - object that holds declaration of names.
 */

#pragma once

#include <map>

#include <libsolidity/ASTForward.h>

namespace dev {
namespace solidity {

class Scope
{
public:
	explicit Scope(Scope* _outerScope = nullptr) : m_outerScope(_outerScope) {}
	/// Registers the name _name in the scope unless it is already declared. Returns true iff
	/// it was not yet declared.
	bool registerName(ASTString const& _name, ASTNode& _declaration)
	{
		if (m_declaredNames.find(_name) != m_declaredNames.end())
			return false;
		m_declaredNames[_name] = &_declaration;
		return true;
	}
	ASTNode* resolveName(ASTString const& _name, bool _recursive = false) const
	{
		auto result = m_declaredNames.find(_name);
		if (result != m_declaredNames.end())
			return result->second;
		if (_recursive && m_outerScope != nullptr)
			return m_outerScope->resolveName(_name, true);
		return nullptr;
	}
	Scope* getOuterScope() const { return m_outerScope; }

private:
	Scope* m_outerScope;
	std::map<ASTString, ASTNode*> m_declaredNames;
};

} }
