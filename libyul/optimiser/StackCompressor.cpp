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
 */
class RematCandidateSelector: public DataFlowAnalyzer
{
public:
	explicit RematCandidateSelector(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}

	/// @returns a map from rematerialisation costs to a vector of variables to rematerialise
	/// and variables that occur in their expression.
	/// While the map is sorted by cost, the contained vectors are sorted by the order of occurrence.
	map<size_t, vector<tuple<YulString, set<YulString>>>> candidates()
	{
		map<size_t, vector<tuple<YulString, set<YulString>>>> cand;
		for (auto const& candidate: m_candidates)
		{
			if (size_t const* cost = util::valueOrNullptr(m_expressionCodeCost, candidate))
			{
				size_t numRef = m_numReferences[candidate];
				set<YulString> const* ref = references(candidate);
				cand[*cost * numRef].emplace_back(candidate, ref ? move(*ref) : set<YulString>{});
			}
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
			if (AssignedValue const* value = variableValue(varName))
			{
				yulAssert(!m_expressionCodeCost.count(varName), "");
				m_candidates.emplace_back(varName);
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

	/// All candidate variables in order of occurrence.
	vector<YulString> m_candidates;
	/// Candidate variables and the code cost of their value.
	map<YulString, size_t> m_expressionCodeCost;
	/// Number of references to each candidate variable.
	map<YulString, size_t> m_numReferences;
};

/// Selects at most @a _numVariables among @a _candidates.
set<YulString> chooseVarsToEliminate(
	map<size_t, vector<tuple<YulString, set<YulString>>>> const& _candidates,
	size_t _numVariables
)
{
	set<YulString> varsToEliminate;
	for (auto&& [cost, candidates]: _candidates)
		for (auto&& [candidate, references]: candidates)
		{
			if (varsToEliminate.size() >= _numVariables)
				return varsToEliminate;
			// If a variable we would like to eliminate references another one
			// we already selected for elimination, then stop selecting
			// candidates. If we would add that variable, then the cost calculation
			// for the previous variable would be off. Furthermore, we
			// do not skip the variable because it would be better to properly re-compute
			// the costs of all other variables instead.
			for (YulString const& referencedVar: references)
				if (varsToEliminate.count(referencedVar))
					return varsToEliminate;
			varsToEliminate.insert(candidate);
		}
	return varsToEliminate;
}

template <typename ASTNode>
void eliminateVariables(
	Dialect const& _dialect,
	ASTNode& _node,
	size_t _numVariables,
	bool _allowMSizeOptimization
)
{
	RematCandidateSelector selector{_dialect};
	selector(_node);
	Rematerialiser::run(_dialect, _node, chooseVarsToEliminate(selector.candidates(), _numVariables));
	UnusedPruner::runUntilStabilised(_dialect, _node, _allowMSizeOptimization);
}

void eliminateVariables(
	Dialect const& _dialect,
	Block& _block,
	vector<StackLayoutGenerator::StackTooDeep> const& _unreachables,
	bool _allowMSizeOptimization
)
{
	RematCandidateSelector selector{_dialect};
	selector(_block);
	std::map<YulString, size_t> candidates;
	for (auto [cost, candidatesWithCost]: selector.candidates())
		for (auto candidate: candidatesWithCost)
			candidates[get<0>(candidate)] = cost;

	set<YulString> varsToEliminate;

	// TODO: this currently ignores the fact that variables may reference other variables we want to eliminate.
	for (auto const& unreachable: _unreachables)
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
	Rematerialiser::run(_dialect, _block, std::move(varsToEliminate), true);
	UnusedPruner::runUntilStabilised(_dialect, _block, _allowMSizeOptimization);
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
		Block& mainBlock = std::get<Block>(_object.code->statements.at(0));
		if (
			auto stackTooDeepErrors = StackLayoutGenerator::reportStackTooDeep(*cfg, YulString{});
			!stackTooDeepErrors.empty()
		)
			eliminateVariables(_dialect, mainBlock, stackTooDeepErrors, allowMSizeOptimzation);
		for (size_t i = 1; i < _object.code->statements.size(); ++i)
		{
			auto& fun = std::get<FunctionDefinition>(_object.code->statements[i]);
			if (
				auto stackTooDeepErrors = StackLayoutGenerator::reportStackTooDeep(*cfg, fun.name);
				!stackTooDeepErrors.empty()
			)
				eliminateVariables(_dialect, fun.body, stackTooDeepErrors, allowMSizeOptimzation);
		}
	}
	else
		for (size_t iterations = 0; iterations < _maxIterations; iterations++)
		{
			map<YulString, int> stackSurplus = CompilabilityChecker(_dialect, _object, _optimizeStackAllocation).stackDeficit;
			if (stackSurplus.empty())
				return true;

			if (stackSurplus.count(YulString{}))
			{
				yulAssert(stackSurplus.at({}) > 0, "Invalid surplus value.");
				eliminateVariables(
					_dialect,
					std::get<Block>(_object.code->statements.at(0)),
					static_cast<size_t>(stackSurplus.at({})),
					allowMSizeOptimzation
				);
			}

			for (size_t i = 1; i < _object.code->statements.size(); ++i)
			{
				auto& fun = std::get<FunctionDefinition>(_object.code->statements[i]);
				if (!stackSurplus.count(fun.name))
					continue;

				yulAssert(stackSurplus.at(fun.name) > 0, "Invalid surplus value.");
				eliminateVariables(
					_dialect,
					fun,
					static_cast<size_t>(stackSurplus.at(fun.name)),
					allowMSizeOptimzation
				);
			}
		}
	return false;
}

