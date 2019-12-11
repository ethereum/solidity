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

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>

namespace solidity::frontend
{

VariableDeclaration const* rootVariableDeclaration(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "Constant variable expected");

	VariableDeclaration const* rootDecl = &_varDecl;
	Identifier const* identifier;
	while ((identifier = dynamic_cast<Identifier const*>(rootDecl->value().get())))
	{
		auto referencedVarDecl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration);
		solAssert(referencedVarDecl && referencedVarDecl->isConstant(), "Identifier is not referencing a variable declaration");
		rootDecl = referencedVarDecl;
	}
	return rootDecl;
}

}
