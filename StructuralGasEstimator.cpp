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
 * @date 2015
 * Gas consumption estimator working alongside the AST.
 */

#include "StructuralGasEstimator.h"
#include <map>
#include <functional>
#include <memory>
#include <libevmasm/ControlFlowGraph.h>
#include <libevmasm/KnownState.h>
#include <libsolidity/AST.h>
#include <libsolidity/ASTVisitor.h>

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::solidity;

StructuralGasEstimator::ASTGasConsumptionSelfAccumulated StructuralGasEstimator::performEstimation(
	AssemblyItems const& _items,
	vector<ASTNode const*> const& _ast
)
{
	solAssert(std::count(_ast.begin(), _ast.end(), nullptr) == 0, "");
	map<SourceLocation, GasMeter::GasConsumption> particularCosts;

	ControlFlowGraph cfg(_items);
	for (BasicBlock const& block: cfg.optimisedBlocks())
	{
		assertThrow(!!block.startState, OptimizerException, "");
		GasMeter meter(block.startState->copy());
		auto const end = _items.begin() + block.end;
		for (auto iter = _items.begin() + block.begin; iter != end; ++iter)
			particularCosts[iter->getLocation()] += meter.estimateMax(*iter);
	}

	set<ASTNode const*> finestNodes = finestNodesAtLocation(_ast);
	ASTGasConsumptionSelfAccumulated gasCosts;
	auto onNode = [&](ASTNode const& _node)
	{
		if (!finestNodes.count(&_node))
			return true;
		gasCosts[&_node][0] = gasCosts[&_node][1] = particularCosts[_node.getLocation()];
		return true;
	};
	auto onEdge = [&](ASTNode const& _parent, ASTNode const& _child)
	{
		gasCosts[&_parent][1] += gasCosts[&_child][1];
	};
	ASTReduce folder(onNode, onEdge);
	for (ASTNode const* ast: _ast)
		ast->accept(folder);

	return gasCosts;
}

map<ASTNode const*, GasMeter::GasConsumption> StructuralGasEstimator::breakToStatementLevel(
	ASTGasConsumptionSelfAccumulated const& _gasCosts,
	vector<ASTNode const*> const& _roots
)
{
	solAssert(std::count(_roots.begin(), _roots.end(), nullptr) == 0, "");
	// first pass: statementDepth[node] is the distance from the deepend statement to node
	// in direction of the tree root (or undefined if not possible)
	map<ASTNode const*, int> statementDepth;
	auto onNodeFirstPass = [&](ASTNode const& _node)
	{
		if (dynamic_cast<Statement const*>(&_node))
			statementDepth[&_node] = 0;
		return true;
	};
	auto onEdgeFirstPass = [&](ASTNode const& _parent, ASTNode const& _child)
	{
		if (statementDepth.count(&_child))
			statementDepth[&_parent] = max(statementDepth[&_parent], statementDepth[&_child] + 1);
	};
	ASTReduce firstPass(onNodeFirstPass, onEdgeFirstPass);
	for (ASTNode const* node: _roots)
		node->accept(firstPass);

	// we use the location of a node if
	//  - its statement depth is 0 or
	//  - its statement depth is undefined but the parent's statement depth is at least 1
	map<ASTNode const*, GasMeter::GasConsumption> gasCosts;
	auto onNodeSecondPass = [&](ASTNode const& _node)
	{
		return statementDepth.count(&_node);
	};
	auto onEdgeSecondPass = [&](ASTNode const& _parent, ASTNode const& _child)
	{
		bool useNode = false;
		if (statementDepth.count(&_child))
			useNode = statementDepth[&_child] == 0;
		else
			useNode = statementDepth.count(&_parent) && statementDepth.at(&_parent) > 0;
		if (useNode)
			gasCosts[&_child] = _gasCosts.at(&_child)[1];
	};
	ASTReduce secondPass(onNodeSecondPass, onEdgeSecondPass);
	for (ASTNode const* node: _roots)
		node->accept(secondPass);
	// gasCosts should only contain non-overlapping locations
	return gasCosts;
}

set<ASTNode const*> StructuralGasEstimator::finestNodesAtLocation(
	vector<ASTNode const*> const& _roots
)
{
	map<SourceLocation, ASTNode const*> locations;
	set<ASTNode const*> nodes;
	SimpleASTVisitor visitor(function<bool(ASTNode const&)>(), [&](ASTNode const& _n)
	{
		if (!locations.count(_n.getLocation()))
		{
			locations[_n.getLocation()] = &_n;
			nodes.insert(&_n);
		}
	});

	for (ASTNode const* root: _roots)
		root->accept(visitor);
	return nodes;
}

