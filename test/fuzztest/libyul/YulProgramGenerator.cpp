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

#include <test/fuzztest/libyul/YulProgramGenerator.h>

#include <libsolutil/Assertions.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <fmt/format.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <set>
#include <utility>

namespace solidity::yul::test
{

namespace
{

std::size_t constexpr minInstructions = 16; // FuzzTest starts out with vectors of minimal size
std::size_t constexpr maxInstructions = 96;
std::size_t constexpr numFunctionParameters = 2;
std::size_t constexpr maxCaseLength = 3; // instructions in one case of a switch
// Bound the execution time under ProgramProfile::Calls, where functions call more than f0.
std::size_t constexpr maxCallDepth = 2; // a function calls a function that calls a function
std::size_t constexpr maxCallsPerFunction = 2;

std::uint8_t constexpr smallValueBound = 5; // exclusive bound of literals and calldata words, so locations collide
std::size_t constexpr numCalldataWords = 4;
std::size_t constexpr distantSlotsStart = 100; // first of a few storage slots the locations rarely reach
std::size_t constexpr numDistantSlots = 8;
std::size_t constexpr returnedMemorySize = 128; // the final return keeps the stores to memory below this alive

static_assert(
	minInstructions <= maxInstructions,
	"The range of program sizes must not be empty."
);
static_assert(
	smallValueBound > 1,
	"The calldata of all ones must be made of small values, otherwise it is not what the programs expect to read."
);

struct OperationInfo
{
	Operation operation;
	char const* name;
	/// How often the operation is drawn, relative to the others
	std::size_t weight;
	/// The same under ProgramProfile::Calls
	std::size_t weightForCalls;
};

OperationInfo constexpr operationInfos[] = {
	{Operation::DeclareLocation,        "DeclareLocation",        2, 2},
	{Operation::DeclareDerivedLocation, "DeclareDerivedLocation", 3, 3},
	{Operation::Reassign,               "Reassign",               4, 4},
	{Operation::StorageStore,           "StorageStore",           4, 4},
	{Operation::MemoryStore,            "MemoryStore",            2, 2},
	{Operation::MemoryStoreByte,        "MemoryStoreByte",        1, 1},
	{Operation::CalldataCopy,           "CalldataCopy",           1, 1},
	{Operation::Load,                   "Load",                   2, 2},
	{Operation::Hash,                   "Hash",                   1, 1},
	{Operation::MemoryToStorage,        "MemoryToStorage",        1, 1},
	{Operation::OpenIf,                 "OpenIf",                 4, 4},
	{Operation::OpenSwitch,             "OpenSwitch",             2, 2},
	{Operation::OpenLoop,               "OpenLoop",               2, 2},
	{Operation::BreakContinueOrLeave,   "BreakContinueOrLeave",   1, 1},
	{Operation::OpenFunction,           "OpenFunction",           2, 4},
	{Operation::CallFunction,           "CallFunction",           3, 6}
};

static_assert(
	std::size(operationInfos) == numOperations,
	"Every operation needs a row in operationInfos."
);
static_assert(
	[] {
		for (std::size_t i = 0; i < std::size(operationInfos); ++i)
			if (static_cast<std::size_t>(operationInfos[i].operation) != i)
				return false;
		return true;
	}(),
	"Row i of operationInfos must describe the operation with value i: the statistics are indexed by that value."
);

double ratio(std::size_t const _part, std::size_t const _total)
{
	return _total == 0 ? 0.0 : static_cast<double>(_part) / static_cast<double>(_total);
}

/// Hands out the random bytes of an instruction one by one, and zeros once they are used up.
class Choices
{
public:
	explicit Choices(Instruction const& _instruction): m_bytes(_instruction.choices) {}

	/// @returns a number below @a _count.
	/// Draw into named variables: the evaluation order of function arguments is unspecified.
	std::size_t pick(std::size_t const _count)
	{
		return (m_next < m_bytes.size() ? m_bytes[m_next++] : 0) % _count;
	}

private:
	std::array<std::uint8_t, choicesPerInstruction> const& m_bytes;
	std::size_t m_next = 0;
};

class ProgramBuilder
{
public:
	ProgramBuilder(ProgramProfile const _profile, GeneratorStatistics& _statistics):
		m_profile(_profile),
		m_statistics(_statistics)
	{}

	std::string build(std::vector<Instruction> const& _instructions)
	{
		if (!_instructions.empty())
			declareInitialLocations(_instructions.front());
		for (Instruction const& instruction: _instructions)
			add(instruction);
		while (!m_constructs.empty())
			closeBody();
		// Keeps memory stores alive. As a statement, it is also a place to check what holds once the program is done.
		m_code += fmt::format("return(0, {})\n", returnedMemorySize);
		++m_statistics.programs;
		m_statistics.lines += static_cast<std::size_t>(std::count(m_code.begin(), m_code.end(), '\n'));
		m_statistics.variables += m_nextVariableIndex;
		m_statistics.bases += m_bases.size();
		++m_statistics.programsWithMaxDepth[m_maxDepth];
		return "{\n" + m_code + "}\n";
	}

private:
	/// A braced block: the body of an if, loop or function, or of one case of a switch.
	struct Body
	{
		std::string opening;
		std::string closing;
		std::size_t numInstructions;
	};

	/// What the body of a function may do to storage and memory. Only ProgramProfile::Calls restricts it.
	enum class FunctionKind { Unrestricted, ReadOnly, Pure };

	enum class ConstructKind { If, Switch, Loop, Function };

	struct Construct
	{
		ConstructKind kind;
		/// The open body, followed by those still to open: the remaining cases of a switch.
		std::vector<Body> bodies;
		/// Instructions still to add before the open body closes
		std::size_t remainingInstructions = 0;
		/// Sizes of m_locations and m_data when the construct was opened, restored when a body closes.
		std::size_t numLocations = 0;
		std::size_t numData = 0;
		/// Size of m_functions when the construct was opened: the functions defined in a body end with it.
		std::size_t numFunctions = 0;
	};

	struct Function
	{
		std::size_t index;
		/// Length of the longest chain of calls starting in the function
		std::size_t callDepth;
	};

	struct FunctionFrame
	{
		/// The variables of the code around the function, which the function cannot access
		std::vector<std::string> outerLocations;
		std::vector<std::string> outerData;
		std::size_t index = 0;
		std::size_t callDepth = 0;
		std::size_t numCalls = 0;
		FunctionKind kind = FunctionKind::Unrestricted;
	};

	void add(Instruction const& _instruction)
	{
		Choices choices{_instruction};
		std::size_t const depth = m_constructs.size();
		std::size_t const operationIndex = static_cast<std::size_t>(_instruction.operation);
		std::size_t const codeSizeBefore = m_code.size();
		++m_statistics.operations[operationIndex];
		++m_statistics.instructionsAtDepth[depth];
		m_statistics.locationsInScope += m_locations.size();
		m_statistics.dataInScope += m_data.size();
		switch (permitted(_instruction.operation))
		{
		case Operation::DeclareLocation:
			declareLocation(smallValue(choices));
			break;
		case Operation::DeclareDerivedLocation:
			declareDerivedLocation(choices);
			break;
		case Operation::Reassign:
			reassign(choices);
			break;
		case Operation::StorageStore:
			store("sstore", storageSlot(choices), choices);
			break;
		case Operation::MemoryStore:
			store("mstore", accessAddress(choices), choices);
			break;
		case Operation::MemoryStoreByte:
			store("mstore8", accessAddress(choices), choices);
			break;
		case Operation::CalldataCopy:
		{
			std::string const target = accessAddress(choices);
			std::string const source = calldataOffset(choices);
			m_code += fmt::format("calldatacopy({}, {}, {})\n", target, source, rangeSize(choices));
			break;
		}
		case Operation::Load:
		{
			if (choices.pick(2) == 0)
				declareData(fmt::format("mload({})", accessAddress(choices)));
			else
				declareData(fmt::format("sload({})", storageSlot(choices)));
			break;
		}
		case Operation::Hash:
		{
			std::string const start = accessAddress(choices);
			declareData(fmt::format("keccak256({}, {})", start, rangeSize(choices)));
			break;
		}
		case Operation::MemoryToStorage:
		{
			std::size_t const slot = distantSlotsStart + choices.pick(numDistantSlots);
			m_code += fmt::format("sstore({}, mload({}))\n", slot, accessAddress(choices));
			break;
		}
		case Operation::OpenIf:
			openIf(choices);
			break;
		case Operation::OpenSwitch:
			openSwitch(choices);
			break;
		case Operation::OpenLoop:
			openLoop(choices);
			break;
		case Operation::BreakContinueOrLeave:
		{
			std::vector<std::string> jumps;
			if (insideLoop())
				jumps = {"continue", "break"};
			if (insideFunction())
				jumps.push_back("leave");
			if (!jumps.empty())
			{
				std::string const jump = jumps[choices.pick(jumps.size())];
				m_code += fmt::format("if {} {{ {} }}\n", condition(choices), jump);
			}
			else
				// Not valid outside a loop or function. Reassigns instead, which invalidates what the analyzer knows
				// about the variable and everything derived from it.
				reassign(choices);
			break;
		}
		case Operation::OpenFunction:
			openFunction(choices);
			break;
		case Operation::CallFunction:
			callFunction(choices);
			break;
		}
		if (m_code.size() == codeSizeBefore)
			++m_statistics.noOps[operationIndex];
		// An instruction that opened a construct is only complete once that construct is closed.
		if (m_constructs.size() == depth)
			completeInstruction();
	}

	/// @returns the operation itself, or the one to perform in its place if the function being built must not
	/// contain it.
	Operation permitted(Operation const _operation) const
	{
		if (functionKind() == FunctionKind::Unrestricted)
			return _operation;
		bool const pure = functionKind() == FunctionKind::Pure;
		switch (_operation)
		{
		case Operation::StorageStore:
		case Operation::MemoryStore:
		case Operation::MemoryStoreByte:
		case Operation::CalldataCopy:
		case Operation::MemoryToStorage:
			return pure ? Operation::Reassign : Operation::Load;
		case Operation::Load:
		case Operation::Hash:
			return pure ? Operation::DeclareDerivedLocation : _operation;
		case Operation::OpenLoop:
			// A loop alone keeps a function from being movable.
			return pure ? Operation::OpenIf : _operation;
		case Operation::DeclareLocation:
		case Operation::DeclareDerivedLocation:
		case Operation::Reassign:
		case Operation::OpenIf:
		case Operation::OpenSwitch:
		case Operation::BreakContinueOrLeave:
		case Operation::OpenFunction:
		case Operation::CallFunction:
			return _operation;
		}
		util::unreachable();
	}

	/// Top-level locations, one of them derived, so that every instruction has some in scope.
	void declareInitialLocations(Instruction const& _instruction)
	{
		Choices choices{_instruction};
		declareLocation(choices.pick(2) == 0 ? smallLiteral(choices) : calldataWord(choices));
		declareLocation(smallValue(choices));
		declareDerivedLocation(choices);
	}

	void declareLocation(std::string const& _value)
	{
		m_locations.push_back(fmt::format("v{}", m_nextVariableIndex++));
		++m_statistics.locationsDeclared;
		m_code += fmt::format("let {} := {}\n", m_locations.back(), _value);
	}

	/// Declares a location equal to, or at a constant offset from, an existing location, its base.
	void declareDerivedLocation(Choices& _choices)
	{
		// Without a location to derive from, declares an independent one, so that the instruction is not wasted.
		if (m_locations.empty())
		{
			declareLocation(smallValue(_choices));
			return;
		}

		std::string const base = pickLocation(_choices);
		m_bases.insert(base);
		std::vector<std::string> const forms{
			base,
			fmt::format("add({}, 0)", base),
			fmt::format("sub({}, 0)", base),
			fmt::format("add({}, {})", base, 1 + _choices.pick(3))
		};
		declareLocation(forms[_choices.pick(forms.size())]);
	}

	void declareData(std::string const& _value)
	{
		m_data.push_back(fmt::format("d{}", m_nextVariableIndex++));
		++m_statistics.dataDeclared;
		m_code += fmt::format("let {} := {}\n", m_data.back(), _value);
	}

	void reassign(Choices& _choices)
	{
		if (m_locations.empty())
			return;
		std::string const target = reassignmentTarget(_choices);
		m_pickedLocations.clear();
		std::string const value = smallValue(_choices);
		++m_statistics.reassignments;
		m_statistics.reassignmentsOfBase += m_bases.count(target);
		m_statistics.reassignmentsMentioningTarget += m_pickedLocations.count(target);
		m_statistics.reassignmentsToItself += value == target ? 1 : 0;
		m_code += fmt::format("{} := {}\n", target, value);
	}

	void store(char const* _builtin, std::string const& _address, Choices& _choices)
	{
		m_code += fmt::format("{}({}, {})\n", _builtin, _address, storedValue(_choices));
	}

	/// Mostly short, so that code after the construct is common; sometimes long enough for a real loop body.
	std::size_t bodyLength(Choices& _choices) const
	{
		static std::size_t constexpr lengths[] = {1, 1, 2, 2, 2, 3, 3, 5};
		return lengths[_choices.pick(std::size(lengths))];
	}

	std::size_t caseLength(Choices& _choices) const { return 1 + _choices.pick(maxCaseLength); }

	/// Opens a construct made of the given bodies, one after the other. Does nothing at the maximum nesting depth.
	void openConstruct(ConstructKind const _kind, std::vector<Body> _bodies)
	{
		if (m_constructs.size() >= maxNestingDepth)
			return;
		m_constructs.push_back(Construct{
			.kind = _kind,
			.bodies = std::move(_bodies),
			.numLocations = m_locations.size(),
			.numData = m_data.size(),
			.numFunctions = m_functions.size()
		});
		m_maxDepth = std::max(m_maxDepth, m_constructs.size());
		openBody();
	}

	void openBody()
	{
		Construct& construct = m_constructs.back();
		m_code += construct.bodies.front().opening;
		construct.remainingInstructions = construct.bodies.front().numInstructions;
	}

	void openIf(Choices& _choices)
	{
		openConstruct(ConstructKind::If, {Body{
			.opening = fmt::format("if {} {{\n", condition(_choices)),
			.closing = "}\n",
			.numInstructions = bodyLength(_choices)
		}});
	}

	/// The bodies of the cases are short, so that a switch does not use up much more of the program than an if.
	void openSwitch(Choices& _choices)
	{
		std::vector<Body> cases;
		cases.push_back(Body{
			.opening = fmt::format("switch {} case 0 {{\n", smallValue(_choices)),
			.closing = "}\n",
			.numInstructions = caseLength(_choices)
		});
		cases.push_back(Body{.opening = "case 1 {\n", .closing = "}\n", .numInstructions = caseLength(_choices)});
		cases.push_back(Body{.opening = "default {\n", .closing = "}\n", .numInstructions = caseLength(_choices)});
		openConstruct(ConstructKind::Switch, std::move(cases));
	}

	/// Opens a loop of up to three iterations. Loops are not nested directly, which keeps executions short.
	void openLoop(Choices& _choices)
	{
		// Opens an if instead, so that the instruction is not wasted.
		if (insideLoop())
		{
			openIf(_choices);
			return;
		}

		std::string const counter = fmt::format("i{}", m_nextVariableIndex++);
		++m_statistics.loopCountersDeclared;
		std::string const bound = calldataWord(_choices);
		openConstruct(
			ConstructKind::Loop,
			{Body{
				.opening = fmt::format(
					"for {{ let {0} := 0 }} lt({0}, and({1}, 3)) {{ {0} := add({0}, 1) }} {{\n",
					counter,
					bound
				),
				.closing = "}\n",
				.numInstructions = bodyLength(_choices)
			}}
		);
	}

	/// Opens a function in the current block, so that the rest of that block can call it. Its parameters and its
	/// return variable are its locations: it cannot access the variables declared outside.
	void openFunction(Choices& _choices)
	{
		if (m_constructs.size() >= maxNestingDepth)
			return;

		std::vector<std::string> parameters;
		for (std::size_t i = 0; i < numFunctionParameters; ++i)
			parameters.push_back(fmt::format("v{}", m_nextVariableIndex++));
		std::string const returnVariable = fmt::format("v{}", m_nextVariableIndex++);

		m_statistics.locationsDeclared += parameters.size() + 1;
		// One function in eight never returns to its caller.
		std::string const closing = _choices.pick(8) == 0 ? "revert(0, 0)\n}\n" : "}\n";
		std::size_t const numInstructions = 2 * bodyLength(_choices);
		FunctionFrame frame{.index = m_numFunctions++};
		m_code += fmt::format(
			"function f{}({}) -> {} ",
			frame.index,
			util::joinHumanReadable(parameters),
			returnVariable
		);
		frame.outerLocations = std::exchange(m_locations, std::move(parameters));
		m_locations.push_back(returnVariable);
		frame.outerData = std::exchange(m_data, {});
		++m_statistics.functions;
		m_statistics.nestedFunctions += m_constructs.empty() ? 0 : 1;
		if (m_profile == ProgramProfile::Calls)
		{
			static FunctionKind constexpr kinds[] = {
				FunctionKind::Unrestricted, FunctionKind::Unrestricted, FunctionKind::Unrestricted,
				FunctionKind::ReadOnly, FunctionKind::ReadOnly,
				FunctionKind::Pure, FunctionKind::Pure, FunctionKind::Pure
			};
			frame.kind = kinds[_choices.pick(std::size(kinds))];
			m_statistics.readOnlyFunctions += frame.kind == FunctionKind::ReadOnly ? 1 : 0;
			m_statistics.pureFunctions += frame.kind == FunctionKind::Pure ? 1 : 0;
		}
		openConstruct(ConstructKind::Function, {Body{.opening = "{\n", .closing = closing, .numInstructions = numInstructions}});
		m_functionFrames.push_back(std::move(frame));
	}

	/// Chains of calls multiply the execution time. Unless the profile is ProgramProfile::Calls, a function can
	/// therefore only call functions that call none. Under that profile the number and the depth of its calls are
	/// bounded instead.
	bool callableFromFunction(Function const& _function) const
	{
		if (m_profile != ProgramProfile::Calls)
			return _function.callDepth == 0;
		return m_functionFrames.back().numCalls < maxCallsPerFunction && _function.callDepth < maxCallDepth;
	}

	/// Calls a function that is complete and in scope.
	void callFunction(Choices& _choices)
	{
		std::vector<Function> callable;
		for (Function const& function: m_functions)
			if (!insideFunction() || callableFromFunction(function))
				callable.push_back(function);
		// Without a function to call, reassigns a location instead, so that the instruction is not wasted.
		if (callable.empty())
		{
			reassign(_choices);
			return;
		}

		Function const function = callable[_choices.pick(callable.size())];
		std::vector<std::string> arguments;
		for (std::size_t i = 0; i < numFunctionParameters; ++i)
			arguments.push_back(smallValue(_choices));
		declareLocation(fmt::format("f{}({})", function.index, util::joinHumanReadable(arguments)));
		if (insideFunction())
		{
			++m_functionFrames.back().numCalls;
			noteCall(function);
		}
	}

	/// Calls the function that was just completed, so that no function goes unused. Behind a condition, as the
	/// function may not return.
	void callCompletedFunction(Function const& _function)
	{
		std::vector<std::string> arguments;
		for (std::size_t i = 0; i < numFunctionParameters; ++i)
			arguments.push_back(
				i % 2 == 0 && !m_locations.empty() ?
				m_locations[_function.index % m_locations.size()] :
				std::to_string(_function.index % smallValueBound)
			);
		m_code += fmt::format(
			"if calldataload({}) {{ let v{} := f{}({}) }}\n",
			32 * (_function.index % numCalldataWords),
			m_nextVariableIndex++,
			_function.index,
			util::joinHumanReadable(arguments)
		);
		if (insideFunction())
			noteCall(_function);
	}

	void noteCall(Function const& _function)
	{
		FunctionFrame& frame = m_functionFrames.back();
		frame.callDepth = std::max(frame.callDepth, _function.callDepth + 1);
	}

	/// A loop around the function being built does not count: break and continue cannot leave a function.
	bool insideLoop() const
	{
		for (
			auto construct = m_constructs.rbegin();
			construct != m_constructs.rend() && construct->kind != ConstructKind::Function;
			++construct
		)
			if (construct->kind == ConstructKind::Loop)
				return true;
		return false;
	}
	bool insideFunction() const { return !m_functionFrames.empty(); }
	FunctionKind functionKind() const
	{
		return m_functionFrames.empty() ? FunctionKind::Unrestricted : m_functionFrames.back().kind;
	}

	void completeInstruction()
	{
		while (!m_constructs.empty() && --m_constructs.back().remainingInstructions == 0)
			if (!closeBody())
				break;
	}

	/// @returns false if that opened the next case of a switch, true if the whole construct is closed.
	bool closeBody()
	{
		Construct& construct = m_constructs.back();
		m_code += construct.bodies.front().closing;
		construct.bodies.erase(construct.bodies.begin());
		m_locations.resize(construct.numLocations);
		m_data.resize(construct.numData);
		m_functions.resize(construct.numFunctions);
		if (!construct.bodies.empty())
		{
			openBody();
			return false;
		}
		ConstructKind const kind = construct.kind;
		m_constructs.pop_back();
		if (kind == ConstructKind::Function)
		{
			FunctionFrame frame = std::move(m_functionFrames.back());
			m_functionFrames.pop_back();
			m_locations = std::move(frame.outerLocations);
			m_data = std::move(frame.outerData);
			m_functions.push_back(Function{.index = frame.index, .callDepth = frame.callDepth});
			if (m_profile == ProgramProfile::Calls)
				callCompletedFunction(m_functions.back());
		}
		return true;
	}

	std::string smallLiteral(Choices& _choices) const
	{
		return std::to_string(_choices.pick(smallValueBound));
	}

	std::string calldataOffset(Choices& _choices) const
	{
		return std::to_string(32 * _choices.pick(numCalldataWords));
	}

	std::string calldataWord(Choices& _choices) const
	{
		return fmt::format("calldataload({})", calldataOffset(_choices));
	}

	/// Range sizes: under a word, one word, just over a word, two words.
	std::size_t rangeSize(Choices& _choices) const
	{
		static std::size_t constexpr sizes[] = {1, 32, 33, 64};
		return sizes[_choices.pick(std::size(sizes))];
	}

	/// Requires a declared location. Mostly picks among the four newest, which are likelier to be related.
	std::string pickLocation(Choices& _choices)
	{
		std::size_t const candidates =
			_choices.pick(10) < 7 ? std::min<std::size_t>(m_locations.size(), 4) : m_locations.size();
		std::string const& location = m_locations[m_locations.size() - 1 - _choices.pick(candidates)];
		m_pickedLocations.insert(location);
		return location;
	}

	/// Mostly picks a base: reassigning one invalidates what is known about locations derived from it.
	std::string reassignmentTarget(Choices& _choices)
	{
		std::vector<std::string> const bases =
			m_locations |
			ranges::views::filter([&](std::string const& _location) { return m_bases.contains(_location); }) |
			ranges::to<std::vector>;
		if (!bases.empty() && _choices.pick(10) < 7)
			return bases[_choices.pick(bases.size())];
		return pickLocation(_choices);
	}

	/// An expression that evaluates to a small value, nested up to maxSmallValueDepth levels deep.
	std::string smallValue(Choices& _choices, std::size_t const _depth = 0)
	{
		// Up to Location, the forms do not nest: they are the only ones at the maximum depth.
		enum class Form
		{
			Literal,
			CalldataWord,
			Location,
			LocationPlusValue,
			ValuePlusLiteral,
			ValueTimesLiteral,
			MaskedLoad
		};
		auto const numFormsUpTo = [](Form const _form) { return static_cast<std::size_t>(_form) + 1; };
		++m_statistics.smallValuesAtDepth[_depth];
		Form const lastForm = _depth < maxSmallValueDepth ? Form::MaskedLoad : Form::Location;
		Form form = static_cast<Form>(_choices.pick(numFormsUpTo(lastForm)));
		if (form == Form::MaskedLoad && functionKind() == FunctionKind::Pure)
			form = Form::ValuePlusLiteral;
		if (m_locations.empty())
			return smallLiteral(_choices);
		switch (form)
		{
		case Form::Literal:
			return smallLiteral(_choices);
		case Form::CalldataWord:
			return calldataWord(_choices);
		case Form::Location:
			return pickLocation(_choices);
		case Form::LocationPlusValue:
		{
			std::string const location = pickLocation(_choices);
			return fmt::format("add({}, {})", location, smallValue(_choices, _depth + 1));
		}
		case Form::ValuePlusLiteral:
		{
			std::string const operand = smallValue(_choices, _depth + 1);
			return fmt::format("add({}, {})", operand, 1 + _choices.pick(3));
		}
		case Form::ValueTimesLiteral:
		{
			std::string const operand = smallValue(_choices, _depth + 1);
			return fmt::format("mul({}, {})", operand, 1 + _choices.pick(2));
		}
		case Form::MaskedLoad:
			return fmt::format("and(sload({}), 7)", smallValue(_choices, _depth + 1));
		}
		util::unreachable();
	}

	/// A small value plus an offset that makes memory accesses overlap partially (1 to 3) or not at all (32).
	std::string accessAddress(Choices& _choices)
	{
		static std::uint8_t constexpr offsets[] = {0, 0, 1, 2, 3, 32};
		std::string const base = smallValue(_choices);
		std::uint8_t const offset = offsets[_choices.pick(std::size(offsets))];
		return offset == 0 ? base : fmt::format("add({}, {})", base, offset);
	}

	/// An access address, sometimes lowered by 1 to 3: below zero it wraps around to just under 2**256.
	/// Only for storage, as the interpreter does not model memory that high.
	std::string storageSlot(Choices& _choices)
	{
		static std::uint8_t constexpr distances[] = {0, 0, 0, 0, 0, 1, 2, 3};
		std::string const address = accessAddress(_choices);
		std::uint8_t const distance = distances[_choices.pick(std::size(distances))];
		++m_statistics.storageSlots;
		m_statistics.storageSlotsWithSubtraction += distance == 0 ? 0 : 1;
		return distance == 0 ? address : fmt::format("sub({}, {})", address, distance);
	}

	std::string storedValue(Choices& _choices)
	{
		enum class Form { Variable, StorageLoad, MemoryLoad, SmallValue };
		switch (static_cast<Form>(_choices.pick(static_cast<std::size_t>(Form::SmallValue) + 1)))
		{
		case Form::Variable:
		{
			std::size_t const numCandidates = m_data.size() + m_locations.size();
			if (numCandidates == 0)
				return smallValue(_choices);
			std::size_t const index = _choices.pick(numCandidates);
			return index < m_data.size() ? m_data[index] : m_locations[index - m_data.size()];
		}
		case Form::StorageLoad:
			return fmt::format("sload({})", storageSlot(_choices));
		case Form::MemoryLoad:
			return fmt::format("mload({})", accessAddress(_choices));
		case Form::SmallValue:
			return smallValue(_choices);
		}
		util::unreachable();
	}

	std::string condition(Choices& _choices)
	{
		std::size_t const form = _choices.pick(3);
		if (form == 0 || (form == 1 && m_locations.empty()))
			return calldataWord(_choices);
		std::string const left = form == 1 ? pickLocation(_choices) : smallValue(_choices);
		std::string const right = smallValue(_choices);
		return fmt::format("{}({}, {})", form == 1 ? "eq" : "lt", left, right);
	}

	std::string m_code;
	std::vector<Construct> m_constructs;
	/// Variables in scope that mostly hold small values and address storage and memory (v0, v1, ...)
	std::vector<std::string> m_locations;
	/// Variables in scope that hold loaded values of any size (d0, d1, ...)
	std::vector<std::string> m_data;
	/// Locations other locations were derived from, in scope or not
	std::set<std::string> m_bases;
	/// Locations picked since reassign cleared it, for the statistics
	std::set<std::string> m_pickedLocations;
	ProgramProfile m_profile;
	GeneratorStatistics& m_statistics;
	/// The complete functions in scope
	std::vector<Function> m_functions;
	/// The functions being built, the innermost last
	std::vector<FunctionFrame> m_functionFrames;
	/// Number of functions opened so far (f0, f1, ...)
	std::size_t m_numFunctions = 0;
	std::size_t m_nextVariableIndex = 0;
	std::size_t m_maxDepth = 0;
};

}

void GeneratorStatistics::print() const
{
	if (programs == 0)
		return;
	std::size_t instructions = 0;
	std::size_t depthSum = 0;
	for (std::size_t depth = 0; depth < instructionsAtDepth.size(); ++depth)
	{
		instructions += instructionsAtDepth[depth];
		depthSum += depth * instructionsAtDepth[depth];
	}
	fmt::print(stderr, "\n== generator statistics ==\n");
	fmt::print(stderr, "programs: {}\n", programs);
	fmt::print(
		stderr,
		"per program: {:.1f} instructions, {:.1f} lines, {:.1f} variables\n",
		ratio(instructions, programs),
		ratio(lines, programs),
		ratio(variables, programs)
	);
	fmt::print(
		stderr,
		"declared per program: {:.1f} locations (v), {:.1f} data variables (d), {:.1f} loop counters (i); "
		"{:.1f} locations used as a base\n",
		ratio(locationsDeclared, programs),
		ratio(dataDeclared, programs),
		ratio(loopCountersDeclared, programs),
		ratio(bases, programs)
	);
	fmt::print(
		stderr,
		"in scope at an instruction, on average: {:.1f} locations, {:.1f} data variables\n",
		ratio(locationsInScope, instructions),
		ratio(dataInScope, instructions)
	);
	fmt::print(stderr, "nesting depth of instructions: average {:.2f}", ratio(depthSum, instructions));
	for (std::size_t depth = 0; depth < instructionsAtDepth.size(); ++depth)
		fmt::print(stderr, ", depth {}: {:.1f}%", depth, 100 * ratio(instructionsAtDepth[depth], instructions));
	std::size_t maxDepthSum = 0;
	for (std::size_t depth = 0; depth < programsWithMaxDepth.size(); ++depth)
		maxDepthSum += depth * programsWithMaxDepth[depth];
	fmt::print(stderr, "\ndeepest nesting of a program: average {:.2f}", ratio(maxDepthSum, programs));
	for (std::size_t depth = 0; depth < programsWithMaxDepth.size(); ++depth)
		fmt::print(stderr, ", depth {}: {:.1f}%", depth, 100 * ratio(programsWithMaxDepth[depth], programs));
	fmt::print(
		stderr,
		"\n\n{:<24}{:>10}{:>9}{:>21}{:>21}\n",
		"operation",
		"drawn",
		"share",
		"of which no effect",
		"emitted per program"
	);
	for (std::size_t operation = 0; operation < operations.size(); ++operation)
		fmt::print(
			stderr,
			"{:<24}{:>10}{:>8.1f}%{:>20.1f}%{:>21.2f}\n",
			operationInfos[operation].name,
			operations[operation],
			100 * ratio(operations[operation], instructions),
			100 * ratio(noOps[operation], operations[operation]),
			ratio(operations[operation] - noOps[operation], programs)
		);
	fmt::print(
		stderr,
		"\nfunctions: {}, not in the outermost block: {:.1f}%, restricted to reading: {:.1f}%, "
		"to neither reading nor writing: {:.1f}%\n",
		functions,
		100 * ratio(nestedFunctions, functions),
		100 * ratio(readOnlyFunctions, functions),
		100 * ratio(pureFunctions, functions)
	);
	fmt::print(
		stderr,
		"reassignments emitted: {}, of a base: {:.1f}%, value mentions the target: {:.1f}%, "
		"value is the target: {:.1f}%\n",
		reassignments,
		100 * ratio(reassignmentsOfBase, reassignments),
		100 * ratio(reassignmentsMentioningTarget, reassignments),
		100 * ratio(reassignmentsToItself, reassignments)
	);
	fmt::print(
		stderr,
		"storage addresses: {}, with a subtraction: {:.1f}%\n",
		storageSlots,
		100 * ratio(storageSlotsWithSubtraction, storageSlots)
	);
	std::size_t smallValues = 0;
	for (std::size_t const count: smallValuesAtDepth)
		smallValues += count;
	fmt::print(stderr, "small values: {}", smallValues);
	for (std::size_t depth = 0; depth < smallValuesAtDepth.size(); ++depth)
		fmt::print(stderr, ", nested {} deep: {:.1f}%", depth, 100 * ratio(smallValuesAtDepth[depth], smallValues));
	fmt::print(stderr, "\n");
}

fuzztest::Domain<std::vector<Instruction>> instructionsDomain(ProgramProfile const _profile)
{
	std::vector<Operation> operations;
	for (OperationInfo const& info: operationInfos)
		operations.insert(
			operations.end(),
			_profile == ProgramProfile::Calls ? info.weightForCalls : info.weight,
			info.operation
		);
	return fuzztest::VectorOf(
		fuzztest::StructOf<Instruction>(
			fuzztest::ElementOf(operations),
			fuzztest::Arbitrary<std::array<std::uint8_t, choicesPerInstruction>>()
		)
	).WithMinSize(minInstructions).WithMaxSize(maxInstructions);
}

std::string buildProgram(
	std::vector<Instruction> const& _instructions,
	ProgramProfile const _profile,
	GeneratorStatistics* const _statistics
)
{
	GeneratorStatistics unused;
	return ProgramBuilder{_profile, _statistics ? *_statistics : unused}.build(_instructions);
}

bytes toCalldata(CalldataWords const& _words)
{
	bytes calldata;
	for (std::uint8_t const word: _words.values)
	{
		calldata += bytes(31, 0);
		calldata.push_back(word);
	}
	return calldata;
}

fuzztest::Domain<CalldataWords> calldataWordsDomain()
{
	auto const randomWords = [] {
		return fuzztest::StructOf<CalldataWords>(
			fuzztest::VectorOf(fuzztest::InRange<std::uint8_t>(0, smallValueBound - 1)).WithSize(numCalldataWords)
		);
	};
	return fuzztest::OneOf(
		randomWords(), randomWords(), randomWords(), randomWords(), randomWords(), randomWords(),
		fuzztest::Just(CalldataWords{std::vector<std::uint8_t>(numCalldataWords, 0)}),
		fuzztest::Just(CalldataWords{std::vector<std::uint8_t>(numCalldataWords, 1)})
	);
}

}
