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

GlobalContext::GlobalContext()
{
	// CurrentContract this; // @todo type depends on context -> switch prior to entering contract
	// Message msg;
	// Transaction tx;
	// Block block;

	//@todo type will be a custom complex type, maybe the same type class for msg tx and block.
	//addVariable("msg", );
}

void GlobalContext::setCurrentContract(ContractDefinition const& _contract)
{
	m_this = createVariable("this", make_shared<ContractType>(_contract));
}

vector<Declaration*> GlobalContext::getDeclarations() const
{
	vector<Declaration*> declarations;
	declarations.reserve(m_objects.size() + 1);
	for (ASTPointer<Declaration> const& declaration: m_objects)
		declarations.push_back(declaration.get());
	declarations.push_back(m_this.get());
	return declarations;
}

ASTPointer<VariableDeclaration> GlobalContext::createVariable(const string& _name, shared_ptr<const Type> const& _type)
{
	ASTPointer<VariableDeclaration> variable = make_shared<VariableDeclaration>(Location(), ASTPointer<TypeName>(),
																				make_shared<ASTString>(_name));
	variable->setType(_type);
	return variable;
}

}
}
