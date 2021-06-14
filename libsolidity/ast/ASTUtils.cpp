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
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/ASTUtils.h>

#include <libsolutil/Algorithms.h>

namespace solidity::frontend
{

namespace
{

class ASTNodeLocator: public ASTConstVisitor
{
public:
	bool visitNode(ASTNode const& _node) override
	{
		if (_node.location().contains(m_pos))
		{
			m_closestMatch = &_node;
			return true;
		}
		return false;
	}

	explicit ASTNodeLocator(int _pos): m_pos{_pos}, m_closestMatch{nullptr} {};

	int const m_pos;
	ASTNode const* m_closestMatch;
};

}

ASTNode const* locateASTNode(int _pos, SourceUnit const& _sourceUnit)
{
	ASTNodeLocator locator{_pos};
	_sourceUnit.accept(locator);
	return locator.m_closestMatch;
}

bool isConstantVariableRecursive(VariableDeclaration const& _varDecl)
{
	solAssert(_varDecl.isConstant(), "Constant variable expected");

	auto visitor = [](VariableDeclaration const& _variable, util::CycleDetector<VariableDeclaration>& _cycleDetector, size_t _depth)
	{
		solAssert(_depth < 256, "Recursion depth limit reached");
		if (!_variable.value())
			// This should result in an error later on.
			return;

		if (auto referencedVarDecl = dynamic_cast<VariableDeclaration const*>(
			ASTNode::referencedDeclaration(*_variable.value()))
		)
			if (referencedVarDecl->isConstant())
				_cycleDetector.run(*referencedVarDecl);
	};

	return util::CycleDetector<VariableDeclaration>(visitor).run(_varDecl) != nullptr;
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

}
