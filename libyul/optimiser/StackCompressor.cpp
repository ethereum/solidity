/*(
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
/**
 * Optimisation stage that aggressively rematerializes certain variables ina a function to free
 * space on the stack until it is compilable.
 */

#include <libyul/optimiser/StackCompressor.h>

#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/Semantics.h>

#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace yul;

namespace
{

/**
 * Class that discovers all variables that can be fully eliminated by rematerialization,
 * and the corresponding approximate costs.
 */
class RematCandidateSelector: public DataFlowAnalyzer
{
public:
	explicit RematCandidateSelector(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}

	/// @returns a set of pairs of rematerialisation costs and variable to rematerialise.
	/// Note that this set is sorted by cost.
	set<pair<size_t, YulString>> candidates()
	{
		set<pair<size_t, YulString>> cand;
		for (auto const& codeCost: m_expressionCodeCost)
		{
			size_t numRef = m_numReferences[codeCost.first];
			cand.emplace(make_pair(codeCost.second * numRef, codeCost.first));
		}
		return cand;
	}

	using DataFlowAnalyzer::operator();
	void operator()(VariableDeclaration& _varDecl) override
	{
		DataFlowAnalyzer::operator()(_varDecl);
		if (_varDecl.variables.size() == 1)
		{
			YulString varName = _varDecl.variables.front().name;
			if (m_value.count(varName))
				m_expressionCodeCost[varName] = CodeCost::codeCost(m_dialect, *m_value[varName]);
		}
	}

	void operator()(Assignment& _assignment) override
	{
		for (auto const& var: _assignment.variableNames)
			rematImpossible(var.name);
		DataFlowAnalyzer::operator()(_assignment);
	}

	// We use visit(Expression) because operator()(Identifier) would also
	// get called on left-hand-sides of assignments.
	void visit(Expression& _e) override
	{
		if (_e.type() == typeid(Identifier))
		{
			YulString name = boost::get<Identifier>(_e).name;
			if (m_expressionCodeCost.count(name))
			{
				if (!m_value.count(name))
					rematImpossible(name);
				else
					++m_numReferences[name];
			}
		}
		DataFlowAnalyzer::visit(_e);
	}

	/// Remove the variable from the candidate set.
	void rematImpossible(YulString _variable)
	{
		m_numReferences.erase(_variable);
		m_expressionCodeCost.erase(_variable);
	}

	/// Candidate variables and the code cost of their value.
	map<YulString, size_t> m_expressionCodeCost;
	/// Number of references to each candidate variable.
	map<YulString, size_t> m_numReferences;
};

template <typename ASTNode>
void eliminateVariables(Dialect const& _dialect, ASTNode& _node, size_t _numVariables)
{
	RematCandidateSelector selector{_dialect};
	selector(_node);

	// Select at most _numVariables
	set<YulString> varsToEliminate;
	for (auto const& costs: selector.candidates())
	{
		if (varsToEliminate.size() >= _numVariables)
			break;
		varsToEliminate.insert(costs.second);
	}

	Rematerialiser::run(_dialect, _node, std::move(varsToEliminate));
	UnusedPruner::runUntilStabilised(_dialect, _node);
}

}

bool StackCompressor::run(
	Dialect const& _dialect,
	Block& _ast,
	bool _optimizeStackAllocation,
	size_t _maxIterations
)
{
	yulAssert(
		_ast.statements.size() > 0 && _ast.statements.at(0).type() == typeid(Block),
		"Need to run the function grouper before the stack compressor."
	);
	for (size_t iterations = 0; iterations < _maxIterations; iterations++)
	{
		map<YulString, int> stackSurplus = CompilabilityChecker::run(_dialect, _ast, _optimizeStackAllocation);
		if (stackSurplus.empty())
			return true;

		if (stackSurplus.count(YulString{}))
		{
			yulAssert(stackSurplus.at({}) > 0, "Invalid surplus value.");
			eliminateVariables(_dialect, boost::get<Block>(_ast.statements.at(0)), stackSurplus.at({}));
		}

		for (size_t i = 1; i < _ast.statements.size(); ++i)
		{
			FunctionDefinition& fun = boost::get<FunctionDefinition>(_ast.statements[i]);
			if (!stackSurplus.count(fun.name))
				continue;

			yulAssert(stackSurplus.at(fun.name) > 0, "Invalid surplus value.");
			eliminateVariables(_dialect, fun, stackSurplus.at(fun.name));
		}
	}
	return false;
}

