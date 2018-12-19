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

#include <libsolidity/analysis/ControlFlowGraph.h>

#include <libsolidity/analysis/ControlFlowBuilder.h>
#include <boost/range/adaptor/reversed.hpp>
#include <algorithm>

using namespace std;
using namespace langutil;
using namespace dev::solidity;

bool CFG::constructFlow(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return Error::containsOnlyWarnings(m_errorReporter.errors());
}


bool CFG::visit(FunctionDefinition const& _function)
{
	if (_function.isImplemented())
		m_functionControlFlow[&_function] = ControlFlowBuilder::createFunctionFlow(m_nodeContainer, _function);
	return false;
}

FunctionFlow const& CFG::functionFlow(FunctionDefinition const& _function) const
{
	solAssert(m_functionControlFlow.count(&_function), "");
	return *m_functionControlFlow.find(&_function)->second;
}

CFGNode* CFG::NodeContainer::newNode()
{
	m_nodes.emplace_back(new CFGNode());
	return m_nodes.back().get();
}
