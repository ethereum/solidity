// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/analysis/ControlFlowGraph.h>

#include <libsolidity/analysis/ControlFlowBuilder.h>
#include <algorithm>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend;

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
	m_nodes.emplace_back(std::make_unique<CFGNode>());
	return m_nodes.back().get();
}
