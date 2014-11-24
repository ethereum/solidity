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
 * Container of the (implicit and explicit) global objects.
 */

#include <memory>
#include <libsolidity/GlobalContext.h>
#include <libsolidity/AST.h>
#include <libsolidity/Types.h>

using namespace std;

namespace dev
{
namespace solidity
{

GlobalContext::GlobalContext():
	m_magicVariables{make_shared<MagicVariableDeclaration>(MagicVariableDeclaration::VariableKind::BLOCK,
														   "block",
														   make_shared<MagicType>(MagicType::Kind::BLOCK)),
					 make_shared<MagicVariableDeclaration>(MagicVariableDeclaration::VariableKind::MSG,
														   "msg",
														   make_shared<MagicType>(MagicType::Kind::MSG)),
					 make_shared<MagicVariableDeclaration>(MagicVariableDeclaration::VariableKind::TX,
														   "tx",
														   make_shared<MagicType>(MagicType::Kind::TX))}
{
}

void GlobalContext::setCurrentContract(ContractDefinition const& _contract)
{
	m_currentContract = &_contract;
}

vector<Declaration*> GlobalContext::getDeclarations() const
{
	vector<Declaration*> declarations;
	declarations.reserve(m_magicVariables.size() + 1);
	for (ASTPointer<Declaration> const& variable: m_magicVariables)
		declarations.push_back(variable.get());
	declarations.push_back(getCurrentThis());
	return declarations;
}

MagicVariableDeclaration*GlobalContext::getCurrentThis() const
{
	if (!m_thisPointer[m_currentContract])
		m_thisPointer[m_currentContract] = make_shared<MagicVariableDeclaration>(
													MagicVariableDeclaration::VariableKind::THIS,
													"this", make_shared<ContractType>(*m_currentContract));
	return m_thisPointer[m_currentContract].get();

}

vector<MagicVariableDeclaration const*> GlobalContext::getMagicVariables() const
{
	vector<MagicVariableDeclaration const*> declarations;
	declarations.reserve(m_magicVariables.size() + 1);
	for (ASTPointer<MagicVariableDeclaration const> const& variable: m_magicVariables)
		declarations.push_back(variable.get());
	declarations.push_back(getCurrentThis());
	return declarations;
}

}
}
