// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>

namespace solidity::frontend
{

VariableDeclaration const* rootConstVariableDeclaration(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "Constant variable expected");

	VariableDeclaration const* rootDecl = &_varDecl;
	Identifier const* identifier;
	while ((identifier = dynamic_cast<Identifier const*>(rootDecl->value().get())))
	{
		auto referencedVarDecl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration);
		if (!referencedVarDecl || !referencedVarDecl->isConstant())
			return nullptr;
		rootDecl = referencedVarDecl;
	}
	return rootDecl;
}

}
