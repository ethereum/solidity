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
 *
 * - Mutually recursive functions cannot be given individual slot counts, because each of them
 *   reaches all the others. They are therefore handled as a unit: the traversal walks the strongly-
 *   connected components of the call graph rather than its individual functions, and every member of
 *   a component is considered to depend on as many slots as the component as a whole.
 *
 * - `slotsRequiredForComponent` maps a component to the number of slots it requires (which is also
 *   the next available slot that can be used by another function that calls into the component).
 *
 * - For each component starting from the one containing the root of the call graph:
 *   - Visit all components called by any of its members that are not already visited. Calls within the
 *     component are ignored as they cannot contribute a requirement, see below.
 *   - Determine the maximum value `n` of the values of `slotsRequiredForComponent` among them.
 *   - Assign each variable of each member its slot starting from `n` (incrementing it).
 *   - Assign `n` to `slotsRequiredForComponent` of the component.
 *
 * Since the components form an acyclic graph, a component is never reached again while it is still
 * being processed, and every value read from `slotsRequiredForComponent` is final.
 * Note that members of a component of more than one function never get a slot assigned at all:
 * `StackLimitEvader::run` bails out before reaching this point if any recursive function has
 * variables that need to be moved to memory.
 */
struct MemoryOffsetAllocator
{
	std::uint64_t run(FunctionHandle const& _function = YulName{})
	{
		std::size_t const componentIndex = callCycles.componentIndexOf(_function);
		if (std::uint64_t const* slotsRequired = util::valueOrNullptr(slotsRequiredForComponent, componentIndex))
			return *slotsRequired;

		// gather required slots from component-adjacent nodes in the call graph
		std::uint64_t requiredSlots = 0;
		for (FunctionHandle const& member: callCycles.component(componentIndex))
			if (auto const* children = util::valueOrNullptr(callGraph, member))
				for (FunctionHandle const& child: *children)
					if (callCycles.componentIndexOf(child) != componentIndex)
						// recurse and max if we leave SCC
						requiredSlots = std::max(run(child), requiredSlots);

		for (FunctionHandle const& member: callCycles.component(componentIndex))
		{
			if (!std::holds_alternative<YulName>(member))
				continue;

			if (auto const* unreachables = util::valueOrNullptr(unreachableVariables, std::get<YulName>(member)))
			{
				yulAssert(
					!callCycles.isRecursive(member),
					"Cannot move variables of a recursive function to fixed memory offsets."
				);

				if (FunctionDefinition const* functionDefinition = util::valueOrDefault(functionDefinitions, std::get<YulName>(member), nullptr))
					if (
						std::size_t const totalArgCount = functionDefinition->returnVariables.size() + functionDefinition->parameters.size();
						totalArgCount > reachableStackDepth
					)
						for (NameWithDebugData const& var: ranges::concat_view(
							functionDefinition->parameters,
							functionDefinition->returnVariables
						) | ranges::views::take(totalArgCount - reachableStackDepth))
							slotAllocations[var.name] = requiredSlots++;

				// Assign slots for all variables that become unreachable in the function body, if the above did not
				// assign a slot for them already.
				for (YulName variable: *unreachables)
					// The empty case is a function with too many arguments or return values,
					// which was already handled above.
					if (!variable.empty() && !slotAllocations.contains(variable))
						slotAllocations[variable] = requiredSlots++;
			}
		}

		return slotsRequiredForComponent[componentIndex] = requiredSlots;
	}

	/// Maps function names to the set of unreachable variables in that function.
	/// An empty variable name means that the function has too many arguments or return variables.
	std::map<YulName, std::vector<YulName>> const& unreachableVariables;
	/// The graph of immediate function calls of all functions.
	std::map<FunctionHandle, std::vector<FunctionHandle>> const& callGraph;
	/// The cycle structure of the call graph
	CallGraphCycles const& callCycles;
	/// Maps the name of each user-defined function to its definition.
	std::map<YulName, FunctionDefinition const*> const& functionDefinitions;
	/// Max stack slots reachable via DUP/SWAP for the current backend configuration.
	size_t reachableStackDepth;

	/// Maps variable names to the memory slot the respective variable is assigned.
	std::map<YulName, uint64_t> slotAllocations{};
	/// Maps strongly connected components to the number of memory slots the respective component requires.
	std::map<std::size_t, std::uint64_t> slotsRequiredForComponent{};
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
		yul::AsmAnalysisInfo analysisInfo = yul::AsmAnalyzer::analyzeStrictAssertCorrect(
			*evmDialect,
			astRoot,
			_object.summarizeStructure()
		);
		std::unique_ptr<CFG> cfg = ControlFlowGraphBuilder::build(analysisInfo, *evmDialect, astRoot);
		run(_context, astRoot, StackLayoutGenerator::reportStackTooDeep(*cfg, *evmDialect));
	}
	else
	{
		run(_context, astRoot, CompilabilityChecker{
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
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackLimitEvader can only be run on objects using the EVMDialect with object access."
	);
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

	auto const memoryGuardHandle = evmDialect->findBuiltin("memoryguard");
	yulAssert(memoryGuardHandle, "Compiling with object access, memoryguard should be available as builtin.");
	std::vector<FunctionCall*> const memoryGuardCalls = findFunctionCalls(_astRoot, *memoryGuardHandle);
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
	CallGraphCycles const callCycles = callGraph.analyzeCallCycles();

	// We cannot move variables in recursive functions to fixed memory offsets.
	for (FunctionHandle function: callCycles.recursiveFunctions)
	{
		yulAssert(std::holds_alternative<YulName>(function), "Builtins are not recursive.");
		if (_unreachableVariables.count(std::get<YulName>(function)))
			return;
	}

	std::map<YulName, FunctionDefinition const*> functionDefinitions = allFunctionDefinitions(_astRoot);

	// Functions that call each other have to share their slot requirements, so the allocator below
	// traverses the strongly-connected components of the call graph
	MemoryOffsetAllocator memoryOffsetAllocator{
		.unreachableVariables = _unreachableVariables,
		.callGraph = callGraph.functionCalls,
		.callCycles = callCycles,
		.functionDefinitions = functionDefinitions,
		.reachableStackDepth = evmDialect->reachableStackDepth()
	};
	uint64_t requiredSlots = memoryOffsetAllocator.run();
	yulAssert(requiredSlots < (uint64_t(1) << 32) - 1, "");

	StackToMemoryMover::run(_context, reservedMemory, memoryOffsetAllocator.slotAllocations, requiredSlots, _astRoot);

	reservedMemory += 32 * requiredSlots;
	for (FunctionCall* memoryGuardCall: findFunctionCalls(_astRoot, *memoryGuardHandle))
	{
		Literal* literal = std::get_if<Literal>(&memoryGuardCall->arguments.front());
		yulAssert(literal && literal->kind == LiteralKind::Number, "");
		literal->value = LiteralValue{reservedMemory, toCompactHexWithPrefix(reservedMemory)};
	}
}
