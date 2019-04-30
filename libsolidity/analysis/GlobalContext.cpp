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
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Container of the (implicit and explicit) global objects.
 */

#include <libsolidity/analysis/GlobalContext.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>
#include <memory>

using namespace std;

namespace dev
{
namespace solidity
{

inline vector<shared_ptr<MagicVariableDeclaration const>> constructMagicVariables()
{
	static auto const magicVarDecl = [](string const& _name, Type const* _type) {
		return make_shared<MagicVariableDeclaration>(_name, _type);
	};

	return {
		magicVarDecl("abi", TypeProvider::magic(MagicType::Kind::ABI)),
		magicVarDecl("addmod", TypeProvider::function(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::AddMod, false, StateMutability::Pure)),
		magicVarDecl("assert", TypeProvider::function(strings{"bool"}, strings{}, FunctionType::Kind::Assert, false, StateMutability::Pure)),
		magicVarDecl("block", TypeProvider::magic(MagicType::Kind::Block)),
		magicVarDecl("blockhash", TypeProvider::function(strings{"uint256"}, strings{"bytes32"}, FunctionType::Kind::BlockHash, false, StateMutability::View)),
		magicVarDecl("ecrecover", TypeProvider::function(strings{"bytes32", "uint8", "bytes32", "bytes32"}, strings{"address"}, FunctionType::Kind::ECRecover, false, StateMutability::Pure)),
		magicVarDecl("gasleft", TypeProvider::function(strings(), strings{"uint256"}, FunctionType::Kind::GasLeft, false, StateMutability::View)),
		magicVarDecl("keccak256", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
		magicVarDecl("log0", TypeProvider::function(strings{"bytes32"}, strings{}, FunctionType::Kind::Log0)),
		magicVarDecl("log1", TypeProvider::function(strings{"bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log1)),
		magicVarDecl("log2", TypeProvider::function(strings{"bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log2)),
		magicVarDecl("log3", TypeProvider::function(strings{"bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log3)),
		magicVarDecl("log4", TypeProvider::function(strings{"bytes32", "bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log4)),
		magicVarDecl("msg", TypeProvider::magic(MagicType::Kind::Message)),
		magicVarDecl("mulmod", TypeProvider::function(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::MulMod, false, StateMutability::Pure)),
		magicVarDecl("now", TypeProvider::uint256()),
		magicVarDecl("require", TypeProvider::function(strings{"bool"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
		magicVarDecl("require", TypeProvider::function(strings{"bool", "string memory"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
		magicVarDecl("revert", TypeProvider::function(strings(), strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
		magicVarDecl("revert", TypeProvider::function(strings{"string memory"}, strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
		magicVarDecl("ripemd160", TypeProvider::function(strings{"bytes memory"}, strings{"bytes20"}, FunctionType::Kind::RIPEMD160, false, StateMutability::Pure)),
		magicVarDecl("selfdestruct", TypeProvider::function(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVarDecl("sha256", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::SHA256, false, StateMutability::Pure)),
		magicVarDecl("sha3", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
		magicVarDecl("suicide", TypeProvider::function(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVarDecl("tx", TypeProvider::magic(MagicType::Kind::Transaction)),
		magicVarDecl("type", TypeProvider::function(
			strings{"address"} /* accepts any contract type, handled by the type checker */,
			strings{} /* returns a MagicType, handled by the type checker */,
			FunctionType::Kind::MetaType,
			false,
			StateMutability::Pure
		)),
	};
}

GlobalContext::GlobalContext(): m_magicVariables{constructMagicVariables()}
{
}

void GlobalContext::setCurrentContract(ContractDefinition const& _contract)
{
	m_currentContract = &_contract;
}

vector<Declaration const*> GlobalContext::declarations() const
{
	vector<Declaration const*> declarations;
	declarations.reserve(m_magicVariables.size());
	for (ASTPointer<Declaration const> const& variable: m_magicVariables)
		declarations.push_back(variable.get());
	return declarations;
}

MagicVariableDeclaration const* GlobalContext::currentThis() const
{
	if (!m_thisPointer[m_currentContract])
		m_thisPointer[m_currentContract] = make_shared<MagicVariableDeclaration>("this", TypeProvider::contract(*m_currentContract));
	return m_thisPointer[m_currentContract].get();

}

MagicVariableDeclaration const* GlobalContext::currentSuper() const
{
	if (!m_superPointer[m_currentContract])
		m_superPointer[m_currentContract] = make_shared<MagicVariableDeclaration>("super", TypeProvider::contract(*m_currentContract, true));
	return m_superPointer[m_currentContract].get();
}

}
}
