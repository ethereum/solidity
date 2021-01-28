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
// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>

#include <libsolutil/Algorithms.h>

namespace solidity::frontend
{

bool isConstantVariableRecursive(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "Constant variable expected");

	auto referencedDeclaration = [&](Expression const* _e) -> VariableDeclaration const*
	{
		if (auto identifier = dynamic_cast<Identifier const*>(_e))
			return dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration);
		else if (auto memberAccess = dynamic_cast<MemberAccess const*>(_e))
			return dynamic_cast<VariableDeclaration const*>(memberAccess->annotation().referencedDeclaration);
		return nullptr;
	};

	auto visitor = [&](VariableDeclaration const& _variable, util::CycleDetector<VariableDeclaration>& _cycleDetector, size_t _depth)
	{
		solAssert(_depth < 256, "Recursion depth limit reached");

		if (auto referencedVarDecl = referencedDeclaration(_variable.value().get()))
			if (referencedVarDecl->isConstant())
				if (_cycleDetector.run(*referencedVarDecl))
					return;
	};

	return util::CycleDetector<VariableDeclaration>(visitor).run(_varDecl);
}

VariableDeclaration const* rootConstVariableDeclaration(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "Constant variable expected");
	solAssert(!isConstantVariableRecursive(_varDecl), "Recursive declaration");

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

Declaration const* referencedDeclaration(Expression const& _expression)
{
	if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_expression))
		return memberAccess->annotation().referencedDeclaration;
	else if (auto const* identifier = dynamic_cast<Identifier const*>(&_expression))
		return identifier->annotation().referencedDeclaration;
	else
		return nullptr;
}

}
