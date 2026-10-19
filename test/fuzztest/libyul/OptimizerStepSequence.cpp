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

#include <test/fuzztest/libyul/OptimizerStepSequence.h>

#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Suite.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/map.hpp>

#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <vector>

namespace solidity::yul::test
{

namespace
{

/// The steps other steps need to have run, in the order they have to run in:
///   h = FunctionHoister
///   g = FunctionGrouper
///   o = ForLoopInitRewriter
std::string_view constexpr prerequisiteOrder = "hgo";

/// The steps that have to run before a step, transitively, going by what its implementation asserts and its
/// documentation states. Steps not listed need none.
/// The steps built on the DataFlowAnalyzer only document the ForLoopInitRewriter, but assert that no function is
/// defined inside a loop.
std::map<char, std::string> const prerequisites{
	{'a', "o"},   // SSATransform
	{'c', "ho"},  // CommonSubexpressionEliminator
	{'D', "hgo"}, // DeadCodeEliminator
	{'E', "ho"},  // EqualStoreEliminator
	{'f', "hg"},  // BlockFlattener
	{'F', "h"},   // FunctionSpecializer
	{'g', "h"},   // FunctionGrouper
	{'i', "h"},   // FullInliner
	{'l', "h"},   // CircularReferencesPruner
	{'L', "ho"},  // LoadResolver
	{'m', "ho"},  // Rematerialiser
	{'M', "ho"},  // LoopInvariantCodeMotion
	{'n', "ho"},  // ControlFlowSimplifier
	{'p', "h"},   // UnusedFunctionParameterPruner
	{'r', "o"},   // UnusedAssignEliminator
	{'s', "ho"},  // ExpressionSimplifier
	{'S', "o"},   // UnusedStoreEliminator
	{'T', "ho"},  // LiteralRematerialiser
	{'v', "ho"}   // EquivalentFunctionCombiner
};
}

fuzztest::Domain<std::string> stepsDomain(std::size_t const _maxSize)
{
	std::vector<char> const allSteps =
		OptimiserSuite::stepAbbreviationToNameMap() | ranges::views::keys | ranges::to<std::vector>;
	return fuzztest::StringOf(fuzztest::ElementOf(allSteps)).WithMaxSize(_maxSize);
}

std::string prerequisitesOf(std::string const& _steps)
{
	std::string needed;
	for (char const step: _steps)
		if (auto const entry = prerequisites.find(step); entry != prerequisites.end())
			needed += entry->second;
	yulAssert(needed.find_first_not_of(prerequisiteOrder) == std::string::npos, "prerequisite without an order");
	std::string ordered;
	for (char const prerequisite: prerequisiteOrder)
		if (needed.find(prerequisite) != std::string::npos)
			ordered += prerequisite;
	return ordered;
}

Block runSteps(Object const& _object, std::string const& _steps)
{
	Dialect const& dialect = *_object.dialect();
	Block ast = std::get<Block>(Disambiguator(dialect, *_object.analysisInfo)(_object.code()->root()));
	std::set<YulName> const reservedIdentifiers;
	NameDispenser dispenser(dialect, ast, reservedIdentifiers);
	OptimiserStepContext context{
		.dialect = dialect,
		.dispenser = dispenser,
		.reservedIdentifiers = reservedIdentifiers,
		.expectedExecutionsPerDeployment = std::nullopt
	};
	OptimiserSuite{context}.runSequence(_steps, ast);
	return ast;
}

}
