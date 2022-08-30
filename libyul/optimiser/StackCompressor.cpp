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

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/Semantics.h>

#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/StackLayoutGenerator.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/CompilabilityChecker.h>

#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

namespace
{

/**
 * Class that discovers all variables that can be fully eliminated by rematerialization,
 * and the corresponding approximate costs.
 *
 * Prerequisite: Disambiguator, Function Grouper
 */
class RematCandidateSelector: public DataFlowAnalyzer
{
public:
	explicit RematCandidateSelector(Dialect const& _dialect): DataFlowAnalyzer(_dialect, MemoryAndStorage::Ignore) {}

	/// @returns a map from function name to rematerialisation costs to a vector of variables to rematerialise
	/// and variables that occur in their expression.
	/// While the map is sorted by cost, the contained vectors are sorted by the order of occurrence.
	map<YulString, map<size_t, vector<YulString>>> candidates()
	{
		map<YulString, map<size_t, vector<YulString>>> cand;
		for (auto const& [functionName, candidate]: m_candidates)
		{
			if (size_t const* cost = util::valueOrNullptr(m_expressionCodeCost, candidate))
			{
				size_t numRef = m_numReferences[candidate];
				cand[functionName][*cost * numRef].emplace_back(candidate);
			}
		}
		return cand;
	}

	using DataFlowAnalyzer::operator();
	void operator()(FunctionDefinition& _function) override
	{
		yulAssert(m_currentFunctionName.empty());
		m_currentFunctionName = _function.name;
		DataFlowAnalyzer::operator()(_function);
		m_currentFunctionName = {};
	}

	void operator()(VariableDeclaration& _varDecl) override
	{
		DataFlowAnalyzer::operator()(_varDecl);
		if (_varDecl.variables.size() == 1)
		{
			YulString varName = _varDecl.variables.front().name;
			if (AssignedValue const* value = variableValue(varName))
			{
				yulAssert(!m_expressionCodeCost.count(varName), "");
				m_candidates.emplace_back(m_currentFunctionName, varName);
				m_expressionCodeCost[varName] = CodeCost::codeCost(m_dialect, *value->value);
			}
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
		if (holds_alternative<Identifier>(_e))
		{
			YulString name = std::get<Identifier>(_e).name;
			if (m_expressionCodeCost.count(name))
			{
				if (!variableValue(name))
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

	YulString m_currentFunctionName = {};

	/// All candidate variables by function name, in order of occurrence.
	vector<pair<YulString, YulString>> m_candidates;
	/// Candidate variables and the code cost of their value.
	map<YulString, size_t> m_expressionCodeCost;
	/// Number of references to each candidate variable.
	map<YulString, size_t> m_numReferences;
};

/// Selects at most @a _numVariables among @a _candidates.
set<YulString> chooseVarsToEliminate(
	map<size_t, vector<YulString>> const& _candidates,
	size_t _numVariables
)
{
	set<YulString> varsToEliminate;
	for (auto&& [cost, candidates]: _candidates)
		for (auto&& candidate: candidates)
		{
			if (varsToEliminate.size() >= _numVariables)
				return varsToEliminate;
			varsToEliminate.insert(candidate);
		}
	return varsToEliminate;
}

void eliminateVariables(
	Dialect const& _dialect,
	Block& _ast,
	map<YulString, int> const& _numVariables,
	bool _allowMSizeOptimization
)
{
	RematCandidateSelector selector{_dialect};
	selector(_ast);
	map<YulString, map<size_t, vector<YulString>>> candidates = selector.candidates();

	set<YulString> varsToEliminate;
	for (auto const& [functionName, numVariables]: _numVariables)
	{
		yulAssert(numVariables > 0);
		varsToEliminate += chooseVarsToEliminate(candidates[functionName], static_cast<size_t>(numVariables));
	}

	Rematerialiser::run(_dialect, _ast, std::move(varsToEliminate));
	// Do not remove functions.
	set<YulString> allFunctions = NameCollector{_ast, NameCollector::OnlyFunctions}.names();
	UnusedPruner::runUntilStabilised(_dialect, _ast, _allowMSizeOptimization, nullptr, allFunctions);
}

void eliminateVariablesOptimizedCodegen(
	Dialect const& _dialect,
	Block& _ast,
	map<YulString, vector<StackLayoutGenerator::StackTooDeep>> const& _unreachables,
	bool _allowMSizeOptimization
)
{
	if (std::all_of(_unreachables.begin(), _unreachables.end(), [](auto const& _item) { return _item.second.empty(); }))
		return;

	RematCandidateSelector selector{_dialect};
	selector(_ast);

	map<YulString, size_t> candidates;
	for (auto const& [functionName, candidatesInFunction]: selector.candidates())
		for (auto [cost, candidatesWithCost]: candidatesInFunction)
			for (auto candidate: candidatesWithCost)
				candidates[candidate] = cost;

	set<YulString> varsToEliminate;

	// TODO: this currently ignores the fact that variables may reference other variables we want to eliminate.
	for (auto const& [functionName, unreachables]: _unreachables)
		for (auto const& unreachable: unreachables)
		{
			map<size_t, vector<YulString>> suitableCandidates;
			size_t neededSlots = unreachable.deficit;
			for (auto varName: unreachable.variableChoices)
			{
				if (varsToEliminate.count(varName))
					--neededSlots;
				else if (size_t* cost = util::valueOrNullptr(candidates, varName))
					if (!util::contains(suitableCandidates[*cost], varName))
						suitableCandidates[*cost].emplace_back(varName);
			}
			for (auto candidatesByCost: suitableCandidates)
			{
				for (auto candidate: candidatesByCost.second)
					if (neededSlots--)
						varsToEliminate.emplace(candidate);
					else
						break;
				if (!neededSlots)
					break;
			}
		}
	Rematerialiser::run(_dialect, _ast, std::move(varsToEliminate), true);
	// Do not remove functions.
	set<YulString> allFunctions = NameCollector{_ast, NameCollector::OnlyFunctions}.names();
	UnusedPruner::runUntilStabilised(_dialect, _ast, _allowMSizeOptimization, nullptr, allFunctions);
}

}

bool StackCompressor::run(
	Dialect const& _dialect,
	Object& _object,
	bool _optimizeStackAllocation,
	size_t _maxIterations
)
{
	yulAssert(
		_object.code &&
		_object.code->statements.size() > 0 && holds_alternative<Block>(_object.code->statements.at(0)),
		"Need to run the function grouper before the stack compressor."
	);
	bool usesOptimizedCodeGenerator = false;
	if (auto evmDialect = dynamic_cast<EVMDialect const*>(&_dialect))
		usesOptimizedCodeGenerator =
			_optimizeStackAllocation &&
			evmDialect->evmVersion().canOverchargeGasForCall() &&
			evmDialect->providesObjectAccess();
	bool allowMSizeOptimzation = !MSizeFinder::containsMSize(_dialect, *_object.code);
	if (usesOptimizedCodeGenerator)
	{
		yul::AsmAnalysisInfo analysisInfo = yul::AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, _object);
		unique_ptr<CFG> cfg = ControlFlowGraphBuilder::build(analysisInfo, _dialect, *_object.code);
		eliminateVariablesOptimizedCodegen(
			_dialect,
			*_object.code,
			StackLayoutGenerator::reportStackTooDeep(*cfg),
			allowMSizeOptimzation
		);
	}
	else
		for (size_t iterations = 0; iterations < _maxIterations; iterations++)
		{
			map<YulString, int> stackSurplus = CompilabilityChecker(_dialect, _object, _optimizeStackAllocation).stackDeficit;
			if (stackSurplus.empty())
				return true;
			eliminateVariables(
				_dialect,
				*_object.code,
				stackSurplus,
				allowMSizeOptimzation
			);
		}
	return false;
}

