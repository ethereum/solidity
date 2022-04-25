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

#include <libsmtutil/DLSolver.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>

using namespace std;
using namespace solidity::smtutil;
using namespace solidity::util;

DLSolver::DLSolver(std::optional<unsigned> _queryTimeout):
	SolverInterface(_queryTimeout)
{
	reset();
}

void DLSolver::reset()
{
	m_graphs.clear();
	initGraphs();
}

void DLSolver::push()
{
	m_graphs.push_back(m_graphs.back());
}

void DLSolver::pop()
{
	smtAssert(!m_graphs.empty());
	m_graphs.pop_back();
	if (m_graphs.empty())
		initGraphs();
}

void DLSolver::declareVariable(string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort, "");
	smtAssert(_sort->kind == Kind::Int);

	if (m_variables.count(_name))
		return;

	m_variables[_name] = uniqueId();
}

void DLSolver::addAssertion(Expression const& _expr)
{
	smtAssert(_expr.sort->kind == Kind::Bool);
	smtAssert(_expr.name == "<=");
	smtAssert(_expr.arguments.size() == 2);

	Expression const& lhs = _expr.arguments.at(0);
	Expression const& rhs = _expr.arguments.at(1);

	smtAssert(lhs.sort->kind == Kind::Int);
	smtAssert(lhs.name == "-");
	smtAssert(lhs.arguments.size() == 2);

	Expression const& a = lhs.arguments.at(0);
	Expression const& b = lhs.arguments.at(1);

	smtAssert(a.sort->kind == Kind::Int);
	smtAssert(a.arguments.empty());
	smtAssert(m_variables.count(a.name));

	smtAssert(b.sort->kind == Kind::Int);
	smtAssert(b.arguments.empty());
	smtAssert(m_variables.count(b.name));

	smtAssert(rhs.sort->kind == Kind::Int);
	smtAssert(rhs.arguments.empty());

	bigint constK;
	try
	{
		constK = bigint(rhs.name);
	}
	catch (...)
	{
		smtAssert(false);
	}

	unsigned nodeA = m_variables.at(a.name);
	unsigned nodeB = m_variables.at(b.name);

	std::map<unsigned, std::map<unsigned, bigint>>& graph = m_graphs.back();

	if (graph.count(nodeA) && graph.at(nodeA).count(nodeB))
	{
		bigint oldK = graph.at(nodeA).at(nodeB);
		if (oldK <= constK)
			return;
	}

	graph[nodeA][nodeB] = constK;
}

pair<CheckResult, vector<string>> DLSolver::check(vector<Expression> const& _expressionsToEvaluate)
{
	vector<string> values;

	CheckResult result = solve();

	if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
	{
		// TODO model stuff
	}

	return make_pair(result, values);
}

CheckResult DLSolver::solve()
{
	smtAssert(!m_graphs.empty());
	std::map<unsigned, std::map<unsigned, bigint>> const& graph = m_graphs.back();
	unsigned const nNodes = m_lastId;

	// ==== Bellman-Ford negative cycle detection ====

	// 1. Single source shortest path

	//vector<bigint> dist(nNodes, 0);
	vector<bigint> dist(nNodes, bigint(1) << 256);

	for (unsigned i = 0; i < nNodes; ++i)
		for (auto const& [a, edges]: graph)
			for (auto const& [b, k]: edges)
				dist[b] = min(dist[b], dist[a] + k);

	// 2. Negative cycle detection

	bool neg = false;
	for (auto const& [a, edges]: graph)
		for (auto const& [b, k]: edges)
			if (dist[b] > dist[a] + k) {
				neg = true;
				break;
			}

	return neg ? CheckResult::UNSATISFIABLE : CheckResult::SATISFIABLE;
}

void DLSolver::initGraphs()
{
	smtAssert(m_graphs.empty());
	m_graphs.push_back({ {} });
}

unsigned DLSolver::uniqueId()
{
	return m_lastId++;
}
