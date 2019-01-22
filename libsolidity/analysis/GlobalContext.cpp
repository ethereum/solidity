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
#include <libsolidity/ast/Types.h>
#include <memory>

using namespace std;

namespace dev
{
namespace solidity
{

GlobalContext::GlobalContext():
m_magicVariables(vector<shared_ptr<MagicVariableDeclaration const>>{
	make_shared<MagicVariableDeclaration>("abi", make_shared<MagicType>(MagicType::Kind::ABI)),
	make_shared<MagicVariableDeclaration>("addmod", make_shared<FunctionType>(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::AddMod, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("assert", make_shared<FunctionType>(strings{"bool"}, strings{}, FunctionType::Kind::Assert, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("block", make_shared<MagicType>(MagicType::Kind::Block)),
	make_shared<MagicVariableDeclaration>("blockhash", make_shared<FunctionType>(strings{"uint256"}, strings{"bytes32"}, FunctionType::Kind::BlockHash, false, StateMutability::View)),
	make_shared<MagicVariableDeclaration>("ecrecover", make_shared<FunctionType>(strings{"bytes32", "uint8", "bytes32", "bytes32"}, strings{"address"}, FunctionType::Kind::ECRecover, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("gasleft", make_shared<FunctionType>(strings(), strings{"uint256"}, FunctionType::Kind::GasLeft, false, StateMutability::View)),
	make_shared<MagicVariableDeclaration>("keccak256", make_shared<FunctionType>(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("log0", make_shared<FunctionType>(strings{"bytes32"}, strings{}, FunctionType::Kind::Log0)),
	make_shared<MagicVariableDeclaration>("log1", make_shared<FunctionType>(strings{"bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log1)),
	make_shared<MagicVariableDeclaration>("log2", make_shared<FunctionType>(strings{"bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log2)),
	make_shared<MagicVariableDeclaration>("log3", make_shared<FunctionType>(strings{"bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log3)),
	make_shared<MagicVariableDeclaration>("log4", make_shared<FunctionType>(strings{"bytes32", "bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log4)),
	make_shared<MagicVariableDeclaration>("msg", make_shared<MagicType>(MagicType::Kind::Message)),
	make_shared<MagicVariableDeclaration>("mulmod", make_shared<FunctionType>(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::MulMod, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("now", make_shared<IntegerType>(256)),
	make_shared<MagicVariableDeclaration>("require", make_shared<FunctionType>(strings{"bool"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("require", make_shared<FunctionType>(strings{"bool", "string memory"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("revert", make_shared<FunctionType>(strings(), strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("revert", make_shared<FunctionType>(strings{"string memory"}, strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("ripemd160", make_shared<FunctionType>(strings{"bytes memory"}, strings{"bytes20"}, FunctionType::Kind::RIPEMD160, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("selfdestruct", make_shared<FunctionType>(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
	make_shared<MagicVariableDeclaration>("sha256", make_shared<FunctionType>(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::SHA256, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("sha3", make_shared<FunctionType>(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
	make_shared<MagicVariableDeclaration>("suicide", make_shared<FunctionType>(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
	make_shared<MagicVariableDeclaration>("tx", make_shared<MagicType>(MagicType::Kind::Transaction)),
	make_shared<MagicVariableDeclaration>("type", make_shared<FunctionType>(
		strings{"address"} /* accepts any contract type, handled by the type checker */,
		strings{} /* returns a MagicType, handled by the type checker */,
		FunctionType::Kind::MetaType,
		false,
		StateMutability::Pure
	)),
})
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
		m_thisPointer[m_currentContract] = make_shared<MagicVariableDeclaration>("this", make_shared<ContractType>(*m_currentContract));
	return m_thisPointer[m_currentContract].get();

}

MagicVariableDeclaration const* GlobalContext::currentSuper() const
{
	if (!m_superPointer[m_currentContract])
		m_superPointer[m_currentContract] = make_shared<MagicVariableDeclaration>("super", make_shared<ContractType>(*m_currentContract, true));
	return m_superPointer[m_currentContract].get();
}

}
}
