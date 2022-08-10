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
/**
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 */

#include <libyul/optimiser/Suite.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/CircularReferencesPruner.h>
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/EqualStoreEliminator.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopConditionOutOfBody.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/FunctionSpecializer.h>
#include <libyul/optimiser/ReasoningBasedSimplifier.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/StackLimitEvader.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libyul/optimiser/UnusedAssignEliminator.h>
#include <libyul/optimiser/UnusedStoreEliminator.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/LoadResolver.h>
#include <libyul/optimiser/LoopInvariantCodeMotion.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/NameSimplifier.h>
#include <libyul/backends/evm/ConstantOptimiser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/Object.h>

#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/evm/NoOutputAssembly.h>

#include <libsolutil/CommonData.h>

#include <libyul/CompilabilityChecker.h>

#include <range/v3/view/map.hpp>
#include <range/v3/action/remove.hpp>

#include <limits>
#include <tuple>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void OptimiserSuite::run(
	Dialect const& _dialect,
	GasMeter const* _meter,
	Object& _object,
	bool _optimizeStackAllocation,
	string_view _optimisationSequence,
	string_view _optimisationCleanupSequence,
	optional<size_t> _expectedExecutionsPerDeployment,
	set<YulString> const& _externallyUsedIdentifiers
)
{
	EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(&_dialect);
	bool usesOptimizedCodeGenerator =
		_optimizeStackAllocation &&
		evmDialect &&
		evmDialect->evmVersion().canOverchargeGasForCall() &&
		evmDialect->providesObjectAccess();
	set<YulString> reservedIdentifiers = _externallyUsedIdentifiers;
	reservedIdentifiers += _dialect.fixedFunctionNames();

	*_object.code = std::get<Block>(Disambiguator(
		_dialect,
		*_object.analysisInfo,
		reservedIdentifiers
	)(*_object.code));
	Block& ast = *_object.code;

	NameDispenser dispenser{_dialect, ast, reservedIdentifiers};
	OptimiserStepContext context{_dialect, dispenser, reservedIdentifiers, _expectedExecutionsPerDeployment};

	OptimiserSuite suite(context, Debug::None);

	// Some steps depend on properties ensured by FunctionHoister, BlockFlattener, FunctionGrouper and
	// ForLoopInitRewriter. Run them first to be able to run arbitrary sequences safely.
	suite.runSequence("hgfo", ast);

	NameSimplifier::run(suite.m_context, ast);
	// Now the user-supplied part
	suite.runSequence(_optimisationSequence, ast);

	// This is a tuning parameter, but actually just prevents infinite loops.
	size_t stackCompressorMaxIterations = 16;
	suite.runSequence("g", ast);

	// We ignore the return value because we will get a much better error
	// message once we perform code generation.
	if (!usesOptimizedCodeGenerator)
		StackCompressor::run(
			_dialect,
			_object,
			_optimizeStackAllocation,
			stackCompressorMaxIterations
		);

	// Run the user supplied (otherwise default) clean up sequence
	suite.runSequence(_optimisationCleanupSequence, ast);
	suite.runSequence("g", ast);

	if (evmDialect)
	{
		yulAssert(_meter, "");
		ConstantOptimiser{*evmDialect, *_meter}(ast);
		if (usesOptimizedCodeGenerator)
		{
			StackCompressor::run(
				_dialect,
				_object,
				_optimizeStackAllocation,
				stackCompressorMaxIterations
			);
			if (evmDialect->providesObjectAccess())
				StackLimitEvader::run(suite.m_context, _object);
		}
		else if (evmDialect->providesObjectAccess() && _optimizeStackAllocation)
			StackLimitEvader::run(suite.m_context, _object);
	}
	else if (dynamic_cast<WasmDialect const*>(&_dialect))
	{
		// If the first statement is an empty block, remove it.
		// We should only have function definitions after that.
		if (ast.statements.size() > 1 && std::get<Block>(ast.statements.front()).statements.empty())
			ast.statements.erase(ast.statements.begin());
	}

	dispenser.reset(ast);
	NameSimplifier::run(suite.m_context, ast);
	VarNameCleaner::run(suite.m_context, ast);

	*_object.analysisInfo = AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, _object);
}

namespace
{


template <class... Step>
map<string, unique_ptr<OptimiserStep>> optimiserStepCollection()
{
	map<string, unique_ptr<OptimiserStep>> ret;
	for (unique_ptr<OptimiserStep>& s: util::make_vector<unique_ptr<OptimiserStep>>(
		(make_unique<OptimiserStepInstance<Step>>())...
	))
	{
		yulAssert(!ret.count(s->name), "");
		ret[s->name] = std::move(s);
	}
	return ret;
}

}

map<string, unique_ptr<OptimiserStep>> const& OptimiserSuite::allSteps()
{
	static map<string, unique_ptr<OptimiserStep>> instance;
	if (instance.empty())
		instance = optimiserStepCollection<
			BlockFlattener,
			CircularReferencesPruner,
			CommonSubexpressionEliminator,
			ConditionalSimplifier,
			ConditionalUnsimplifier,
			ControlFlowSimplifier,
			DeadCodeEliminator,
			EqualStoreEliminator,
			EquivalentFunctionCombiner,
			ExpressionInliner,
			ExpressionJoiner,
			ExpressionSimplifier,
			ExpressionSplitter,
			ForLoopConditionIntoBody,
			ForLoopConditionOutOfBody,
			ForLoopInitRewriter,
			FullInliner,
			FunctionGrouper,
			FunctionHoister,
			FunctionSpecializer,
			LiteralRematerialiser,
			LoadResolver,
			LoopInvariantCodeMotion,
			UnusedAssignEliminator,
			UnusedStoreEliminator,
			ReasoningBasedSimplifier,
			Rematerialiser,
			SSAReverser,
			SSATransform,
			StructuralSimplifier,
			UnusedFunctionParameterPruner,
			UnusedPruner,
			VarDeclInitializer
		>();
	// Does not include VarNameCleaner because it destroys the property of unique names.
	// Does not include NameSimplifier.
	return instance;
}

map<string, char> const& OptimiserSuite::stepNameToAbbreviationMap()
{
	static map<string, char> lookupTable{
		{BlockFlattener::name,                'f'},
		{CircularReferencesPruner::name,      'l'},
		{CommonSubexpressionEliminator::name, 'c'},
		{ConditionalSimplifier::name,         'C'},
		{ConditionalUnsimplifier::name,       'U'},
		{ControlFlowSimplifier::name,         'n'},
		{DeadCodeEliminator::name,            'D'},
		{EqualStoreEliminator::name,          'E'},
		{EquivalentFunctionCombiner::name,    'v'},
		{ExpressionInliner::name,             'e'},
		{ExpressionJoiner::name,              'j'},
		{ExpressionSimplifier::name,          's'},
		{ExpressionSplitter::name,            'x'},
		{ForLoopConditionIntoBody::name,      'I'},
		{ForLoopConditionOutOfBody::name,     'O'},
		{ForLoopInitRewriter::name,           'o'},
		{FullInliner::name,                   'i'},
		{FunctionGrouper::name,               'g'},
		{FunctionHoister::name,               'h'},
		{FunctionSpecializer::name,           'F'},
		{LiteralRematerialiser::name,         'T'},
		{LoadResolver::name,                  'L'},
		{LoopInvariantCodeMotion::name,       'M'},
		{ReasoningBasedSimplifier::name,      'R'},
		{UnusedAssignEliminator::name,        'r'},
		{UnusedStoreEliminator::name,         'S'},
		{Rematerialiser::name,                'm'},
		{SSAReverser::name,                   'V'},
		{SSATransform::name,                  'a'},
		{StructuralSimplifier::name,          't'},
		{UnusedFunctionParameterPruner::name, 'p'},
		{UnusedPruner::name,                  'u'},
		{VarDeclInitializer::name,            'd'},
	};
	yulAssert(lookupTable.size() == allSteps().size(), "");
	yulAssert((
			util::convertContainer<set<char>>(string(NonStepAbbreviations)) -
			util::convertContainer<set<char>>(lookupTable | ranges::views::values)
		).size() == string(NonStepAbbreviations).size(),
		"Step abbreviation conflicts with a character reserved for another syntactic element"
	);

	return lookupTable;
}

map<char, string> const& OptimiserSuite::stepAbbreviationToNameMap()
{
	static map<char, string> lookupTable = util::invertMap(stepNameToAbbreviationMap());

	return lookupTable;
}

void OptimiserSuite::validateSequence(string_view _stepAbbreviations)
{
	int8_t nestingLevel = 0;
	int8_t colonDelimiters = 0;
	for (char abbreviation: _stepAbbreviations)
		switch (abbreviation)
		{
		case ' ':
		case '\n':
			break;
		case '[':
			assertThrow(nestingLevel < numeric_limits<int8_t>::max(), OptimizerException, "Brackets nested too deep");
			nestingLevel++;
			break;
		case ']':
			nestingLevel--;
			assertThrow(nestingLevel >= 0, OptimizerException, "Unbalanced brackets");
			break;
		case ':':
			assertThrow(nestingLevel == 0, OptimizerException, "Cleanup delimiter may only be placed at nesting level zero");
			assertThrow(++colonDelimiters <=1, OptimizerException, "Too many colon delimiters");
			break;
		default:
		{
			yulAssert(
				string(NonStepAbbreviations).find(abbreviation) == string::npos,
				"Unhandled syntactic element in the abbreviation sequence"
			);
			assertThrow(
				stepAbbreviationToNameMap().find(abbreviation) != stepAbbreviationToNameMap().end(),
				OptimizerException,
				"'"s + abbreviation + "' is not a valid step abbreviation"
			);
			optional<string> invalid = allSteps().at(stepAbbreviationToNameMap().at(abbreviation))->invalidInCurrentEnvironment();
			assertThrow(
				!invalid.has_value(),
				OptimizerException,
				"'"s + abbreviation + "' is invalid in the current environment: " + *invalid
			);
		}
		}
	assertThrow(nestingLevel == 0, OptimizerException, "Unbalanced brackets");
}

void OptimiserSuite::runSequence(string_view _stepAbbreviations, Block& _ast, bool _repeatUntilStable)
{
	validateSequence(_stepAbbreviations);

	// This splits 'aaa[bbb]ccc...' into 'aaa' and '[bbb]ccc...'.
	auto extractNonNestedPrefix = [](string_view _tail) -> tuple<string_view, string_view>
	{
		for (size_t i = 0; i < _tail.size(); ++i)
		{
			yulAssert(_tail[i] != ']');
			if (_tail[i] == '[')
				return {_tail.substr(0, i), _tail.substr(i)};
		}
		return {_tail, {}};
	};

	// This splits '[bbb]ccc...' into 'bbb' and 'ccc...'.
	auto extractBracketContent = [](string_view _tail) -> tuple<string_view, string_view>
	{
		yulAssert(!_tail.empty() && _tail[0] == '[');

		size_t contentLength = 0;
		int8_t nestingLevel = 1;
		for (char abbreviation: _tail.substr(1))
		{
			if (abbreviation == '[')
			{
				yulAssert(nestingLevel < numeric_limits<int8_t>::max());
				++nestingLevel;
			}
			else if (abbreviation == ']')
			{
				--nestingLevel;
				if (nestingLevel == 0)
					break;
			}
			++contentLength;
		}
		yulAssert(nestingLevel == 0);
		yulAssert(_tail[contentLength + 1] == ']');

		return {_tail.substr(1, contentLength), _tail.substr(contentLength + 2)};
	};

	auto abbreviationsToSteps = [](string_view _sequence) -> vector<string>
	{
		vector<string> steps;
		for (char abbreviation: _sequence)
			if (abbreviation != ' ' && abbreviation != '\n')
				steps.emplace_back(stepAbbreviationToNameMap().at(abbreviation));
		return steps;
	};

	vector<tuple<string_view, bool>> subsequences;
	string_view tail = _stepAbbreviations;
	while (!tail.empty())
	{
		string_view subsequence;
		tie(subsequence, tail) = extractNonNestedPrefix(tail);
		if (subsequence.size() > 0)
			subsequences.push_back({subsequence, false});

		if (tail.empty())
			break;

		tie(subsequence, tail) = extractBracketContent(tail);
		if (subsequence.size() > 0)
			subsequences.push_back({subsequence, true});
	}

	size_t codeSize = 0;
	for (size_t round = 0; round < MaxRounds; ++round)
	{
		for (auto const& [subsequence, repeat]: subsequences)
		{
			if (repeat)
				runSequence(subsequence, _ast, true);
			else
				runSequence(abbreviationsToSteps(subsequence), _ast);
		}

		if (!_repeatUntilStable)
			break;

		size_t newSize = CodeSize::codeSizeIncludingFunctions(_ast);
		if (newSize == codeSize)
			break;
		codeSize = newSize;
	}
}

void OptimiserSuite::runSequence(std::vector<string> const& _steps, Block& _ast)
{
	unique_ptr<Block> copy;
	if (m_debug == Debug::PrintChanges)
		copy = make_unique<Block>(std::get<Block>(ASTCopier{}(_ast)));
	for (string const& step: _steps)
	{
		if (m_debug == Debug::PrintStep)
			cout << "Running " << step << endl;
		allSteps().at(step)->run(m_context, _ast);
		if (m_debug == Debug::PrintChanges)
		{
			// TODO should add switch to also compare variable names!
			if (SyntacticallyEqual{}.statementEqual(_ast, *copy))
				cout << "== Running " << step << " did not cause changes." << endl;
			else
			{
				cout << "== Running " << step << " changed the AST." << endl;
				cout << AsmPrinter{}(_ast) << endl;
				copy = make_unique<Block>(std::get<Block>(ASTCopier{}(_ast)));
			}
		}
	}
}
