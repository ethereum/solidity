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

#include <libsolidity/formal/VariableUsage.h>

#include <libsolidity/formal/SMTChecker.h>

#include <libsolidity/ast/ASTVisitor.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

VariableUsage::VariableUsage(ASTNode const& _node)
{
	auto nodeFun = [&](ASTNode const& n) -> bool
	{
		if (auto identifier = dynamic_cast<Identifier const*>(&n))
		{
			Declaration const* declaration = identifier->annotation().referencedDeclaration;
			solAssert(declaration, "");
			if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
				if (identifier->annotation().lValueRequested)
					m_touchedVariable[&n] = varDecl;
		}
		else if (auto funCall = dynamic_cast<FunctionCall const*>(&n))
		{
			if (FunctionDefinition const* funDef = SMTChecker::inlinedFunctionCallToDefinition(*funCall))
				m_children[&n].push_back(funDef);
		}
		return true;
	};
	auto edgeFun = [&](ASTNode const& _parent, ASTNode const& _child)
	{
		if (m_touchedVariable.count(&_child) || m_children.count(&_child))
			m_children[&_parent].push_back(&_child);
	};

	ASTReduce reducer(nodeFun, edgeFun);
	_node.accept(reducer);
}

vector<VariableDeclaration const*> VariableUsage::touchedVariables(ASTNode const& _node) const
{
	if (!m_children.count(&_node) && !m_touchedVariable.count(&_node))
		return {};

	set<VariableDeclaration const*> touched;
	set<ASTNode const*> visitedFunctions;
	vector<ASTNode const*> toVisit;
	toVisit.push_back(&_node);

	while (!toVisit.empty())
	{
		ASTNode const* n = toVisit.back();
		toVisit.pop_back();

		if (auto funDef = dynamic_cast<FunctionDefinition const*>(n))
			visitedFunctions.insert(funDef);

		if (m_children.count(n))
		{
			solAssert(!m_touchedVariable.count(n), "");
			for (auto const& child: m_children.at(n))
				if (!visitedFunctions.count(child))
					toVisit.push_back(child);
		}
		else
		{
			solAssert(m_touchedVariable.count(n), "");
			touched.insert(m_touchedVariable.at(n));
		}
	}

	return {touched.begin(), touched.end()};
}
