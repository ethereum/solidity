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

#include <libyul/optimiser/StackLimitEvader.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/optimiser/FunctionDefinitionCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/StackToMemoryMover.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Utilities.h>
#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>

#include <range/v3/view/concat.hpp>
#include <range/v3/view/take.hpp>

using namespace std;
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
	uint64_t run(YulString _function = YulString{})
	{
		if (slotsRequiredForFunction.count(_function))
			return slotsRequiredForFunction[_function];

		// Assign to zero early to guard against recursive calls.
		slotsRequiredForFunction[_function] = 0;

		uint64_t requiredSlots = 0;
		if (callGraph.count(_function))
			for (YulString child: callGraph.at(_function))
				requiredSlots = std::max(run(child), requiredSlots);

		if (auto const* unreachables = util::valueOrNullptr(unreachableVariables, _function))
		{
			if (FunctionDefinition const* functionDefinition = util::valueOrDefault(functionDefinitions, _function, nullptr, util::allow_copy))
				if (
					size_t totalArgCount = functionDefinition->returnVariables.size() + functionDefinition->parameters.size();
					totalArgCount > 16
				)
					for (TypedName const& var: ranges::concat_view(
						functionDefinition->parameters,
						functionDefinition->returnVariables
					) | ranges::views::take(totalArgCount - 16))
						slotAllocations[var.name] = requiredSlots++;

			// Assign slots for all variables that become unreachable in the function body, if the above did not
			// assign a slot for them already.
			for (YulString variable: *unreachables)
				// The empty case is a function with too many arguments or return values,
				// which was already handled above.
				if (!variable.empty() && !slotAllocations.count(variable))
					slotAllocations[variable] = requiredSlots++;
		}

		return slotsRequiredForFunction[_function] = requiredSlots;
	}

	/// Maps function names to the set of unreachable variables in that function.
	/// An empty variable name means that the function has too many arguments or return variables.
	map<YulString, set<YulString>> const& unreachableVariables;
	/// The graph of immediate function calls of all functions.
	map<YulString, set<YulString>> const& callGraph;
	/// Maps the name of each user-defined function to its definition.
	map<YulString, FunctionDefinition const*> const& functionDefinitions;

	/// Maps variable names to the memory slot the respective variable is assigned.
	map<YulString, uint64_t> slotAllocations{};
	/// Maps function names to the number of memory slots the respective function requires.
	map<YulString, uint64_t> slotsRequiredForFunction{};
};

u256 literalArgumentValue(FunctionCall const& _call)
{
	yulAssert(_call.arguments.size() == 1, "");
	Literal const* literal = std::get_if<Literal>(&_call.arguments.front());
	yulAssert(literal && literal->kind == LiteralKind::Number, "");
	return valueOfLiteral(*literal);
}
}

void StackLimitEvader::run(
	OptimiserStepContext& _context,
	Object& _object,
	map<YulString, set<YulString>> const& _unreachableVariables
)
{
	yulAssert(_object.code, "");
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackLimitEvader can only be run on objects using the EVMDialect with object access."
	);

	vector<FunctionCall*> memoryGuardCalls = FunctionCallFinder::run(
		*_object.code,
		"memoryguard"_yulstring
	);
	// Do not optimise, if no ``memoryguard`` call is found.
	if (memoryGuardCalls.empty())
		return;

	// Make sure all calls to ``memoryguard`` we found have the same value as argument (otherwise, abort).
	u256 reservedMemory = literalArgumentValue(*memoryGuardCalls.front());
	yulAssert(reservedMemory < u256(1) << 32 - 1, "");

	for (FunctionCall const* memoryGuardCall: memoryGuardCalls)
		if (reservedMemory != literalArgumentValue(*memoryGuardCall))
			return;

	CallGraph callGraph = CallGraphGenerator::callGraph(*_object.code);

	// We cannot move variables in recursive functions to fixed memory offsets.
	for (YulString function: callGraph.recursiveFunctions())
		if (_unreachableVariables.count(function))
			return;

	map<YulString, FunctionDefinition const*> functionDefinitions = FunctionDefinitionCollector::run(*_object.code);

	MemoryOffsetAllocator memoryOffsetAllocator{_unreachableVariables, callGraph.functionCalls, functionDefinitions};
	uint64_t requiredSlots = memoryOffsetAllocator.run();
	yulAssert(requiredSlots < (uint64_t(1) << 32) - 1, "");

	StackToMemoryMover::run(_context, reservedMemory, memoryOffsetAllocator.slotAllocations, requiredSlots, *_object.code);

	reservedMemory += 32 * requiredSlots;
	for (FunctionCall* memoryGuardCall: FunctionCallFinder::run(*_object.code, "memoryguard"_yulstring))
	{
		Literal* literal = std::get_if<Literal>(&memoryGuardCall->arguments.front());
		yulAssert(literal && literal->kind == LiteralKind::Number, "");
		literal->value = YulString{toCompactHexWithPrefix(reservedMemory)};
	}
}
