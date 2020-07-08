// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Container of the (implicit and explicit) global objects.
 */

#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <boost/noncopyable.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace solidity::frontend
{

class Type; // forward

/**
 * Container for all global objects which look like AST nodes, but are not part of the AST
 * that is currently being compiled.
 * @note must not be destroyed or moved during compilation as its objects can be referenced from
 * other objects.
 */
class GlobalContext: private boost::noncopyable
{
public:
	GlobalContext();
	void setCurrentContract(ContractDefinition const& _contract);
	MagicVariableDeclaration const* currentThis() const;
	MagicVariableDeclaration const* currentSuper() const;

	/// @returns a vector of all implicit global declarations excluding "this".
	std::vector<Declaration const*> declarations() const;

private:
	std::vector<std::shared_ptr<MagicVariableDeclaration const>> m_magicVariables;
	ContractDefinition const* m_currentContract = nullptr;
	std::map<ContractDefinition const*, std::shared_ptr<MagicVariableDeclaration const>> mutable m_thisPointer;
	std::map<ContractDefinition const*, std::shared_ptr<MagicVariableDeclaration const>> mutable m_superPointer;
};

}
