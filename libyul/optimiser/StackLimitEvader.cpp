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
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/StackToMemoryMover.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Utilities.h>
#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

namespace
{
// Walks the call graph using a Depth-First-Search assigning memory offsets to variables.
// - The leaves of the call graph will get the lowest offsets, increasing towards the root.
// - ``nextAvailableSlot`` maps a function to the next available slot that can be used by another
//   function that calls it.
// - For each function starting from the root of the call graph:
//   - Visit all children that are not already visited.
//   - Determine the maximum value ``n`` of the values of ``nextAvailableSlot`` among the children.
//   - If the function itself contains variables that need memory slots, but is contained in a cycle,
//     abort the process as failure.
//   - If not, assign each variable its slot starting from ``n`` (incrementing it).
//   - Assign ``n`` to ``nextAvailableSlot`` of the function.
struct MemoryOffsetAllocator
{
	uint64_t run(YulString _function = YulString{})
	{
		if (nextAvailableSlot.count(_function))
			return nextAvailableSlot[_function];

		// Assign to zero early to guard against recursive calls.
		nextAvailableSlot[_function] = 0;

		uint64_t nextSlot = 0;
		if (callGraph.count(_function))
			for (YulString child: callGraph.at(_function))
				nextSlot = std::max(run(child), nextSlot);

		if (unreachableVariables.count(_function))
		{
			yulAssert(!slotAllocations.count(_function), "");
			auto& assignedSlots = slotAllocations[_function];
			for (YulString variable: unreachableVariables.at(_function))
				if (variable.empty())
				{
					// TODO: Too many function arguments or return parameters.
				}
				else
					assignedSlots[variable] = nextSlot++;
		}

		return nextAvailableSlot[_function] = nextSlot;
	}

	map<YulString, set<YulString>> const& unreachableVariables;
	map<YulString, set<YulString>> const& callGraph;

	map<YulString, map<YulString, uint64_t>> slotAllocations{};
	map<YulString, uint64_t> nextAvailableSlot{};
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
	for (FunctionCall const* getFreeMemoryStartCall: memoryGuardCalls)
		if (reservedMemory != literalArgumentValue(*getFreeMemoryStartCall))
			return;

	CallGraph callGraph = CallGraphGenerator::callGraph(*_object.code);

	// We cannot move variables in recursive functions to fixed memory offsets.
	for (YulString function: callGraph.recursiveFunctions())
		if (_unreachableVariables.count(function))
			return;

	MemoryOffsetAllocator memoryOffsetAllocator{_unreachableVariables, callGraph.functionCalls};
	uint64_t requiredSlots = memoryOffsetAllocator.run();

	StackToMemoryMover{_context, reservedMemory, memoryOffsetAllocator.slotAllocations}(*_object.code);
	reservedMemory += 32 * requiredSlots;
	YulString reservedMemoryString{util::toCompactHexWithPrefix(reservedMemory)};
	for (FunctionCall* memoryGuardCall: memoryGuardCalls)
	{
		Literal* literal = std::get_if<Literal>(&memoryGuardCall->arguments.front());
		yulAssert(literal && literal->kind == LiteralKind::Number, "");
		literal->value = reservedMemoryString;
	}
}
