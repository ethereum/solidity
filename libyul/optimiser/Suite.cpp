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

#include <libyul/backends/evm/NoOutputAssembly.h>

#include <libsolutil/CommonData.h>

#include <libyul/CompilabilityChecker.h>

#include <range/v3/view/map.hpp>
#include <range/v3/action/remove.hpp>

#include <limits>
#include <tuple>

#ifdef PROFILE_OPTIMIZER_STEPS
#include <chrono>
#include <fmt/format.h>
#endif

using namespace solidity;
using namespace solidity::yul;
#ifdef PROFILE_OPTIMIZER_STEPS
using namespace std::chrono;
#endif
using namespace std::string_literals;

namespace
{

#ifdef PROFILE_OPTIMIZER_STEPS
void outputPerformanceMetrics(map<string, int64_t> const& _metrics)
{
	vector<pair<string, int64_t>> durations(_metrics.begin(), _metrics.end());
	sort(
		durations.begin(),
		durations.end(),
		[](pair<string, int64_t> const& _lhs, pair<string, int64_t> const& _rhs) -> bool
		{
			return _lhs.second < _rhs.second;
		}
	);

	int64_t totalDurationInMicroseconds = 0;
	for (auto&& [step, durationInMicroseconds]: durations)
		totalDurationInMicroseconds += durationInMicroseconds;

	cerr << "Performance metrics of optimizer steps" << endl;
	cerr << "======================================" << endl;
	constexpr double microsecondsInSecond = 1000000;
	for (auto&& [step, durationInMicroseconds]: durations)
	{
		double percentage = 100.0 * static_cast<double>(durationInMicroseconds) / static_cast<double>(totalDurationInMicroseconds);
		double sec = static_cast<double>(durationInMicroseconds) / microsecondsInSecond;
		cerr << fmt::format("{:>7.3f}% ({} s): {}", percentage, sec, step) << endl;
	}
	double totalDurationInSeconds = static_cast<double>(totalDurationInMicroseconds) / microsecondsInSecond;
	cerr << "--------------------------------------" << endl;
	cerr << fmt::format("{:>7}% ({:.3f} s)", 100, totalDurationInSeconds) << endl;
}
#endif

}


void OptimiserSuite::run(
	Dialect const& _dialect,
	GasMeter const* _meter,
	Object& _object,
	bool _optimizeStackAllocation,
	std::string_view _optimisationSequence,
	std::string_view _optimisationCleanupSequence,
	std::optional<size_t> _expectedExecutionsPerDeployment,
	std::set<YulString> const& _externallyUsedIdentifiers
)
{
	EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(&_dialect);
	bool usesOptimizedCodeGenerator =
		_optimizeStackAllocation &&
		evmDialect &&
		evmDialect->evmVersion().canOverchargeGasForCall() &&
		evmDialect->providesObjectAccess();
	std::set<YulString> reservedIdentifiers = _externallyUsedIdentifiers;
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

	// Run the user-supplied clean up sequence
	suite.runSequence(_optimisationCleanupSequence, ast);
	// Hard-coded FunctionGrouper step is used to bring the AST into a canonical form required by the StackCompressor
	// and StackLimitEvader. This is hard-coded as the last step, as some previously executed steps may break the
	// aforementioned form, thus causing the StackCompressor/StackLimitEvader to throw.
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

	dispenser.reset(ast);
	NameSimplifier::run(suite.m_context, ast);
	VarNameCleaner::run(suite.m_context, ast);

#ifdef PROFILE_OPTIMIZER_STEPS
	outputPerformanceMetrics(suite.m_durationPerStepInMicroseconds);
#endif

	*_object.analysisInfo = AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, _object);
}

namespace
{

template <class... Step>
std::map<std::string, std::unique_ptr<OptimiserStep>> optimiserStepCollection()
{
	std::map<std::string, std::unique_ptr<OptimiserStep>> ret;
	for (std::unique_ptr<OptimiserStep>& s: util::make_vector<std::unique_ptr<OptimiserStep>>(
		(std::make_unique<OptimiserStepInstance<Step>>())...
	))
	{
		yulAssert(!ret.count(s->name), "");
		ret[s->name] = std::move(s);
	}
	return ret;
}

}

std::map<std::string, std::unique_ptr<OptimiserStep>> const& OptimiserSuite::allSteps()
{
	static std::map<std::string, std::unique_ptr<OptimiserStep>> instance;
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

std::map<std::string, char> const& OptimiserSuite::stepNameToAbbreviationMap()
{
	static std::map<std::string, char> lookupTable{
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
			util::convertContainer<std::set<char>>(std::string(NonStepAbbreviations)) -
			util::convertContainer<std::set<char>>(lookupTable | ranges::views::values)
		).size() == std::string(NonStepAbbreviations).size(),
		"Step abbreviation conflicts with a character reserved for another syntactic element"
	);

	return lookupTable;
}

std::map<char, std::string> const& OptimiserSuite::stepAbbreviationToNameMap()
{
	static std::map<char, std::string> lookupTable = util::invertMap(stepNameToAbbreviationMap());

	return lookupTable;
}

void OptimiserSuite::validateSequence(std::string_view _stepAbbreviations)
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
			assertThrow(nestingLevel < std::numeric_limits<int8_t>::max(), OptimizerException, "Brackets nested too deep");
			nestingLevel++;
			break;
		case ']':
			nestingLevel--;
			assertThrow(nestingLevel >= 0, OptimizerException, "Unbalanced brackets");
			break;
		case ':':
			++colonDelimiters;
			assertThrow(nestingLevel == 0, OptimizerException, "Cleanup sequence delimiter cannot be placed inside the brackets");
			assertThrow(colonDelimiters <= 1, OptimizerException, "Too many cleanup sequence delimiters");
			break;
		default:
		{
			yulAssert(
				std::string(NonStepAbbreviations).find(abbreviation) == std::string::npos,
				"Unhandled syntactic element in the abbreviation sequence"
			);
			assertThrow(
				stepAbbreviationToNameMap().find(abbreviation) != stepAbbreviationToNameMap().end(),
				OptimizerException,
				"'"s + abbreviation + "' is not a valid step abbreviation"
			);
			std::optional<std::string> invalid = allSteps().at(stepAbbreviationToNameMap().at(abbreviation))->invalidInCurrentEnvironment();
			assertThrow(
				!invalid.has_value(),
				OptimizerException,
				"'"s + abbreviation + "' is invalid in the current environment: " + *invalid
			);
		}
		}
	assertThrow(nestingLevel == 0, OptimizerException, "Unbalanced brackets");
}

bool OptimiserSuite::isEmptyOptimizerSequence(std::string const& _sequence)
{
	size_t delimiterCount{0};
	for (char const step: _sequence)
		switch (step)
		{
		case ':':
			if (++delimiterCount > 1)
				return false;
			break;
		case ' ':
		case '\n':
			break;
		default:
			return false;
		}
	return true;
}

void OptimiserSuite::runSequence(std::string_view _stepAbbreviations, Block& _ast, bool _repeatUntilStable)
{
	validateSequence(_stepAbbreviations);

	// This splits 'aaa[bbb]ccc...' into 'aaa' and '[bbb]ccc...'.
	auto extractNonNestedPrefix = [](std::string_view _tail) -> std::tuple<std::string_view, std::string_view>
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
	auto extractBracketContent = [](std::string_view _tail) -> std::tuple<std::string_view, std::string_view>
	{
		yulAssert(!_tail.empty() && _tail[0] == '[');

		size_t contentLength = 0;
		int8_t nestingLevel = 1;
		for (char abbreviation: _tail.substr(1))
		{
			if (abbreviation == '[')
			{
				yulAssert(nestingLevel < std::numeric_limits<int8_t>::max());
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

	auto abbreviationsToSteps = [](std::string_view _sequence) -> std::vector<std::string>
	{
		std::vector<std::string> steps;
		for (char abbreviation: _sequence)
			if (abbreviation != ' ' && abbreviation != '\n')
				steps.emplace_back(stepAbbreviationToNameMap().at(abbreviation));
		return steps;
	};

	std::vector<std::tuple<std::string_view, bool>> subsequences;
	std::string_view tail = _stepAbbreviations;
	while (!tail.empty())
	{
		std::string_view subsequence;
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

void OptimiserSuite::runSequence(std::vector<std::string> const& _steps, Block& _ast)
{
	std::unique_ptr<Block> copy;
	if (m_debug == Debug::PrintChanges)
		copy = std::make_unique<Block>(std::get<Block>(ASTCopier{}(_ast)));
	for (std::string const& step: _steps)
	{
		if (m_debug == Debug::PrintStep)
			std::cout << "Running " << step << std::endl;
#ifdef PROFILE_OPTIMIZER_STEPS
		steady_clock::time_point startTime = steady_clock::now();
#endif
		allSteps().at(step)->run(m_context, _ast);
#ifdef PROFILE_OPTIMIZER_STEPS
		steady_clock::time_point endTime = steady_clock::now();
		m_durationPerStepInMicroseconds[step] += duration_cast<microseconds>(endTime - startTime).count();
#endif
		if (m_debug == Debug::PrintChanges)
		{
			// TODO should add switch to also compare variable names!
			if (SyntacticallyEqual{}.statementEqual(_ast, *copy))
				std::cout << "== Running " << step << " did not cause changes." << std::endl;
			else
			{
				std::cout << "== Running " << step << " changed the AST." << std::endl;
				std::cout << AsmPrinter{}(_ast) << std::endl;
				copy = std::make_unique<Block>(std::get<Block>(ASTCopier{}(_ast)));
			}
		}
	}
}
