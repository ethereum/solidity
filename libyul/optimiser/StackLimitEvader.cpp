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

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/StackLimitEvader.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/StackToMemoryMover.h>
#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AST.h>
#include <libyul/CompilabilityChecker.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Utilities.h>
#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/take.hpp>

using namespace solidity;
using namespace solidity::yul;

namespace
{
/**
 * Walks the call graph using a Depth-First-Search assigning memory slots to variables.
 * - The leaves of the call graph will get the lowest slot, increasing towards the root.
 * - ``slotsRequiredForFunction`` maps a function to the number of slots it requires (which is also the
 *   next available slot that can be used by another function that calls this function).
 * - For each function starting from the root of the call graph:
 * - Visit all children that are not already visited.
 * - Determine the maximum value ``n`` of the values of ``slotsRequiredForFunction`` among the children.
 * - If the function itself contains variables that need memory slots, but is contained in a cycle,
 *   abort the process as failure.
 * - If not, assign each variable its slot starting from ``n`` (incrementing it).
 * - Assign ``n`` to ``slotsRequiredForFunction`` of the function.
 */
struct MemoryOffsetAllocator
{
	uint64_t run(YulName _function = YulName{})
	{
		if (slotsRequiredForFunction.count(_function))
			return slotsRequiredForFunction[_function];

		// Assign to zero early to guard against recursive calls.
		slotsRequiredForFunction[_function] = 0;

		uint64_t requiredSlots = 0;
		if (callGraph.count(_function))
			for (YulName child: callGraph.at(_function))
				requiredSlots = std::max(run(child), requiredSlots);

		if (auto const* unreachables = util::valueOrNullptr(unreachableVariables, _function))
		{
			if (FunctionDefinition const* functionDefinition = util::valueOrDefault(functionDefinitions, _function, nullptr, util::allow_copy))
				if (
					size_t totalArgCount = functionDefinition->returnVariables.size() + functionDefinition->parameters.size();
					totalArgCount > 16
				)
					for (NameWithDebugData const& var: ranges::concat_view(
						functionDefinition->parameters,
						functionDefinition->returnVariables
					) | ranges::views::take(totalArgCount - 16))
						slotAllocations[var.name] = requiredSlots++;

			// Assign slots for all variables that become unreachable in the function body, if the above did not
			// assign a slot for them already.
			for (YulName variable: *unreachables)
				// The empty case is a function with too many arguments or return values,
				// which was already handled above.
				if (!variable.empty() && !slotAllocations.count(variable))
					slotAllocations[variable] = requiredSlots++;
		}

		return slotsRequiredForFunction[_function] = requiredSlots;
	}

	/// Maps function names to the set of unreachable variables in that function.
	/// An empty variable name means that the function has too many arguments or return variables.
	std::map<YulName, std::vector<YulName>> const& unreachableVariables;
	/// The graph of immediate function calls of all functions.
	std::map<YulName, std::vector<YulName>> const& callGraph;
	/// Maps the name of each user-defined function to its definition.
	std::map<YulName, FunctionDefinition const*> const& functionDefinitions;

	/// Maps variable names to the memory slot the respective variable is assigned.
	std::map<YulName, uint64_t> slotAllocations{};
	/// Maps function names to the number of memory slots the respective function requires.
	std::map<YulName, uint64_t> slotsRequiredForFunction{};
};

u256 literalArgumentValue(FunctionCall const& _call)
{
	yulAssert(_call.arguments.size() == 1, "");
	Literal const* literal = std::get_if<Literal>(&_call.arguments.front());
	yulAssert(literal && literal->kind == LiteralKind::Number, "");
	return literal->value.value();
}
}

Block StackLimitEvader::run(
	OptimiserStepContext& _context,
	Object const& _object
)
{
	yulAssert(_object.hasCode());
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackLimitEvader can only be run on objects using the EVMDialect with object access."
	);
	auto astRoot = std::get<Block>(ASTCopier{}(_object.code()->root()));
	if (evmDialect && evmDialect->evmVersion().canOverchargeGasForCall())
	{
		yul::AsmAnalysisInfo analysisInfo = yul::AsmAnalyzer::analyzeStrictAssertCorrect(*evmDialect, astRoot, _object.qualifiedDataNames());
		std::unique_ptr<CFG> cfg = ControlFlowGraphBuilder::build(analysisInfo, *evmDialect, astRoot);
		run(_context, astRoot, StackLayoutGenerator::reportStackTooDeep(*cfg));
	}
	else
	{
		run(_context, astRoot, CompilabilityChecker{
			_context.dialect,
			_object,
			true,
		}.unreachableVariables);
	}
	return astRoot;
}

void StackLimitEvader::run(
	OptimiserStepContext& _context,
	Block& _astRoot,
	std::map<YulName, std::vector<StackLayoutGenerator::StackTooDeep>> const& _stackTooDeepErrors
)
{
	std::map<YulName, std::vector<YulName>> unreachableVariables;
	for (auto&& [function, stackTooDeepErrors]: _stackTooDeepErrors)
	{
		auto& unreachables = unreachableVariables[function];
		// TODO: choose wisely.
		for (auto const& stackTooDeepError: stackTooDeepErrors)
			for (auto variable: stackTooDeepError.variableChoices | ranges::views::take(stackTooDeepError.deficit))
				if (!util::contains(unreachables, variable))
					unreachables.emplace_back(variable);
	}
	run(_context, _astRoot, unreachableVariables);
}

void StackLimitEvader::run(
	OptimiserStepContext& _context,
	Block& _astRoot,
	std::map<YulName, std::vector<YulName>> const& _unreachableVariables
)
{
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackLimitEvader can only be run on objects using the EVMDialect with object access."
	);

	std::vector<FunctionCall*> memoryGuardCalls = findFunctionCalls(_astRoot, "memoryguard"_yulname);
	// Do not optimise, if no ``memoryguard`` call is found.
	if (memoryGuardCalls.empty())
		return;

	// Make sure all calls to ``memoryguard`` we found have the same value as argument (otherwise, abort).
	u256 reservedMemory = literalArgumentValue(*memoryGuardCalls.front());
	yulAssert(reservedMemory < u256(1) << 32 - 1, "");

	for (FunctionCall const* memoryGuardCall: memoryGuardCalls)
		if (reservedMemory != literalArgumentValue(*memoryGuardCall))
			return;

	CallGraph callGraph = CallGraphGenerator::callGraph(_astRoot);

	// We cannot move variables in recursive functions to fixed memory offsets.
	for (YulName function: callGraph.recursiveFunctions())
		if (_unreachableVariables.count(function))
			return;

	std::map<YulName, FunctionDefinition const*> functionDefinitions = allFunctionDefinitions(_astRoot);

	MemoryOffsetAllocator memoryOffsetAllocator{_unreachableVariables, callGraph.functionCalls, functionDefinitions};
	uint64_t requiredSlots = memoryOffsetAllocator.run();
	yulAssert(requiredSlots < (uint64_t(1) << 32) - 1, "");

	StackToMemoryMover::run(_context, reservedMemory, memoryOffsetAllocator.slotAllocations, requiredSlots, _astRoot);

	reservedMemory += 32 * requiredSlots;
	for (FunctionCall* memoryGuardCall: findFunctionCalls(_astRoot, "memoryguard"_yulname))
	{
		Literal* literal = std::get_if<Literal>(&memoryGuardCall->arguments.front());
		yulAssert(literal && literal->kind == LiteralKind::Number, "");
		literal->value = LiteralValue{reservedMemory, toCompactHexWithPrefix(reservedMemory)};
	}
}
