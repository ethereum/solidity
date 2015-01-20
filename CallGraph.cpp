
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
 * Callgraph of functions inside a contract.
 */

#include <libsolidity/AST.h>
#include <libsolidity/CallGraph.h>

using namespace std;

namespace dev
{
namespace solidity
{

void CallGraph::addNode(ASTNode const& _node)
{
	_node.accept(*this);
}

set<FunctionDefinition const*> const& CallGraph::getCalls()
{
	computeCallGraph();
	return m_functionsSeen;
}

void CallGraph::computeCallGraph()
{
	while (!m_workQueue.empty())
	{
		m_workQueue.front()->accept(*this);
		m_workQueue.pop();
	}
}

bool CallGraph::visit(Identifier const& _identifier)
{
	FunctionDefinition const* fun = dynamic_cast<FunctionDefinition const*>(_identifier.getReferencedDeclaration());
	if (fun)
	{
		if (m_overrideResolver)
			fun = (*m_overrideResolver)(fun->getName());
		solAssert(fun, "");
		addFunction(*fun);
	}
	return true;
}

bool CallGraph::visit(FunctionDefinition const& _function)
{
	addFunction(_function);
	return true;
}

bool CallGraph::visit(MemberAccess const& _memberAccess)
{
	// used for "BaseContract.baseContractFunction"
	if (_memberAccess.getExpression().getType()->getCategory() == Type::Category::TYPE)
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_memberAccess.getExpression().getType());
		if (type.getMembers().getMemberType(_memberAccess.getMemberName()))
		{
			ContractDefinition const& contract = dynamic_cast<ContractType const&>(*type.getActualType())
													.getContractDefinition();
			for (ASTPointer<FunctionDefinition> const& function: contract.getDefinedFunctions())
				if (function->getName() == _memberAccess.getMemberName())
				{
					addFunction(*function);
					return true;
				}
		}
	}
	return true;
}

void CallGraph::addFunction(FunctionDefinition const& _function)
{
	if (!m_functionsSeen.count(&_function))
	{
		m_functionsSeen.insert(&_function);
		m_workQueue.push(&_function);
	}
}

}
}
