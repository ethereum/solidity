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

#include <libsolidity/analysis/Scoper.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;

void Scoper::assignScopes(ASTNode const& _astRoot)
{
	Scoper scoper;
	_astRoot.accept(scoper);
}

bool Scoper::visit(ContractDefinition const& _contract)
{
	solAssert(m_contract == nullptr, "");
	m_contract = &_contract;
	return ASTConstVisitor::visit(_contract);
}

void Scoper::endVisit(ContractDefinition const& _contract)
{
	solAssert(m_contract == &_contract, "");
	m_contract = nullptr;
	ASTConstVisitor::endVisit(_contract);
}

bool Scoper::visitNode(ASTNode const& _node)
{
	if (auto const* scopable = dynamic_cast<Scopable const*>(&_node))
	{
		scopable->annotation().scope = m_scopes.empty() ? nullptr : m_scopes.back();
		scopable->annotation().contract = m_contract;
	}
	if (dynamic_cast<ScopeOpener const*>(&_node))
		m_scopes.push_back(&_node);
	return true;
}

void Scoper::endVisitNode(ASTNode const& _node)
{
	if (dynamic_cast<ScopeOpener const*>(&_node))
		m_scopes.pop_back();
}
