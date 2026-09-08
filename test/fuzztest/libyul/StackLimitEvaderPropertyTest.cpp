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

/// Property test for the Yul stack limit evader
///
/// The evader relocates local variables that cannot be reached on the stack to fixed memory offsets. This test
/// builds a random call graph, asks the evader to relocate every variable of every function and then checks
/// that the rewritten program behaves exactly like the original. Which variables actually end up in memory is
/// implementation detail and deliberately untested here.
///
/// Both programs are run by a small interpreter for exactly the constructs the generator and the evader emit
/// and have to produce the identical sequence of storage writes.
///
/// The generated cycles have no base case, so execution budget in terms of call depth is bounded per top-level function.

#include <libyul/AST.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Object.h>
#include <libyul/YulStack.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/StackLimitEvader.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/Visitor.h>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <range/v3/view/transform.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace solidity;
using namespace solidity::yul;

namespace solidity::yul::test
{

namespace
{

using FunctionID = std::uint32_t;
/// Maximum amount of user functions
FunctionID constexpr maxFunctions = 8;
/// Maximum amount of edges in the call graph
std::size_t constexpr maxEdges = 16;
/// Maximum number of variables inside each of the user functions
std::uint32_t constexpr maxVariablesPerFunction = 3;
/// Initial value of memoryguard
std::uint32_t constexpr reservedMemoryStart = 128;
/// Storage slot of the activation counter every generated function increments on entry
std::uint32_t constexpr activationCounterSlot = 0xffff;
/// Storage slot every variable is read back into
std::uint32_t constexpr readBackSlot = 0;
/// Initial values or variables are variableIndex + activationStamp * activationCounterValue
std::uint32_t constexpr activationStamp = 1000;
/// Calls nested deeper than this are skipped
std::size_t constexpr maxCallDepth = 12;
/// Calls are also skipped once a top-level call has spawned this many nested calls
std::size_t constexpr maxInterpretedCallsPerTopLevelCall = 512;

static_assert(
	maxVariablesPerFunction <= activationStamp,
	"Variable indices must stay below the stamp factor, otherwise initial values are not unique per variable and activation."
);
static_assert(
	activationCounterSlot != readBackSlot,
	"The activation counter key must not collide with the storage key the read-backs write."
);
static_assert(
	maxCallDepth > maxFunctions,
	"A call path must be able to visit every function and still come back around a cycle to revisit one."
);

struct Edge
{
	FunctionID from;
	FunctionID to;
};

struct FuzzInput
{
	FunctionID numFunctions;
	std::vector<Edge> edges;
	std::vector<std::uint32_t> variableCounts;
};

std::string functionName(FunctionID const _function)
{
	return fmt::format("f{}", _function);
}

std::string variableName(FunctionID const _function, std::uint32_t const _variable)
{
	return fmt::format("v{}_{}", _function, _variable);
}

/// Builds a Yul program whose call graph is @a _adjacency. Each function bumps the activation counter,
/// declares @a _variableCounts variables initialized with values unique per variable and activation, then
/// performs its calls, then reads the variables back, so that all of them are live across every call the
/// function makes.
///
/// For two functions with the single edge f0 -> f1, one variable in f0 and two in f1, the generated program is:
///
///	{
///		mstore(0x40, memoryguard(128))
///		f0()
///		f1()
///		function f0()
///		{
///			sstore(0xffff, add(sload(0xffff), 1))         // bump the activation counter (key 0xffff)
///			let v0_0 := add(0, mul(sload(0xffff), 1000))  // variable index stamped with the activation counter
///			f1()
///			sstore(0, v0_0)                               // read-back, keeps v0_0 live across the calls
///		}
///		function f1()
///		{
///			sstore(0xffff, add(sload(0xffff), 1))
///			let v1_0 := add(0, mul(sload(0xffff), 1000))
///			let v1_1 := add(1, mul(sload(0xffff), 1000))
///			sstore(0, v1_0)
///			sstore(0, v1_1)
///		}
///	}
std::string generateProgram(
	FunctionID const _numFunctions,
	std::vector<std::set<FunctionID>> const& _adjacency,
	std::vector<std::uint32_t> const& _variableCounts
)
{
	std::vector<std::string> topLevelCalls;
	std::string functions;
	for (FunctionID function = 0; function < _numFunctions; ++function)
	{
		// The evader only assigns offsets to functions reachable from the top level block.
		topLevelCalls.emplace_back(fmt::format("{}()", functionName(function)));

		std::vector<std::string> body;
		body.emplace_back(fmt::format(
			"sstore({0}, add(sload({0}), 1))",
			activationCounterSlot
		));
		for (std::uint32_t variable = 0; variable < _variableCounts[function]; ++variable)
			body.emplace_back(fmt::format(
				"let {} := add({}, mul(sload({}), {}))",
				variableName(function, variable),
				variable,
				activationCounterSlot,
				activationStamp
			));
		for (FunctionID const callee: _adjacency[function])
			body.emplace_back(fmt::format("{}()", functionName(callee)));
		for (std::uint32_t variable = 0; variable < _variableCounts[function]; ++variable)
			body.emplace_back(fmt::format(
				"sstore({}, {})",
				readBackSlot,
				variableName(function, variable)
			));

		functions += fmt::format(
			R"(function {name}() {{ {body} }} )",
			fmt::arg("name", functionName(function)),
			fmt::arg("body", fmt::join(body, " "))
		);
	}

	return fmt::format(
		R"(
			{{
				mstore(0x40, memoryguard({reservedMemoryStart}))
				{topLevelCalls}
				{functions}
			}}
		)",
		fmt::arg("reservedMemoryStart", reservedMemoryStart),
		fmt::arg("topLevelCalls", fmt::join(topLevelCalls, " ")),
		fmt::arg("functions", functions)
	);
}

std::vector<std::set<FunctionID>> buildAdjacency(FunctionID const _numFunctions, std::vector<Edge> const& _edges)
{
	std::vector<std::set<FunctionID>> adjacency(_numFunctions);
	for (auto const& [from, to]: _edges)
		adjacency[from].insert(to);
	return adjacency;
}

/// Executes the small Yul subset that the generator and the evader emit, bounded by call depth and budget, see
/// maxCallDepth and maxInterpretedCallsPerTopLevelCall.
/// Records traces of storage writes and the set of accessed memory addresses.
struct BoundedInterpreter
{
	void run(Block const& _root)
	{
		std::map<YulName, u256> environment;
		for (Statement const& statement: _root.statements)
		{
			// reset call budget for top-level calls
			remainingCalls = maxInterpretedCallsPerTopLevelCall;
			executeStatement(statement, environment, 0);
		}
	}

	void execute(Block const& _block, std::map<YulName, u256>& _environment, std::size_t const _depth)
	{
		for (Statement const& statement: _block.statements)
			executeStatement(statement, _environment, _depth);
	}

	void executeStatement(Statement const& _statement, std::map<YulName, u256>& _environment, std::size_t const _depth)
	{
		if (std::holds_alternative<FunctionDefinition>(_statement))
			return;
		if (auto const* declaration = std::get_if<VariableDeclaration>(&_statement))
		{
			yulAssert(declaration->variables.size() == 1 && declaration->value, "unsupported variable declaration");
			_environment[declaration->variables.front().name] = evaluate(*declaration->value, _environment);
		}
		else if (auto const* expressionStatement = std::get_if<ExpressionStatement>(&_statement))
			executeCall(expressionStatement->expression, _environment, _depth);
		else
			yulAssert(false, "unsupported statement");
	}

	void executeCall(Expression const& _expression, std::map<YulName, u256> const& _environment, std::size_t const _depth)
	{
		auto const* call = std::get_if<FunctionCall>(&_expression);
		yulAssert(call, "unsupported expression statement");
		std::visit(util::GenericVisitor{
			[&](BuiltinName const& _builtin)
			{
				yulAssert(call->arguments.size() == dialect.builtin(_builtin.handle).numParameters);
				// Argument evaluation order does not matter, the subset's expressions are effect-free.
				if (_builtin.handle == mstore)
					writeWord(evaluate(call->arguments.front(), _environment), evaluate(call->arguments.back(), _environment));
				else if (_builtin.handle == sstore)
				{
					u256 const key = evaluate(call->arguments.front(), _environment);
					u256 const value = evaluate(call->arguments.back(), _environment);
					storage[key] = value;
					storageWrites.emplace_back(key, value);
				}
				else
					yulAssert(false, "unsupported builtin call");
			},
			[&](Identifier const& _callee)
			{
				auto const functionIt = functions.find(_callee.name);
				yulAssert(
					functionIt != functions.end() && call->arguments.empty() && functionIt->second->parameters.empty(),
					"unsupported user function call"
				);
				if (_depth < maxCallDepth && remainingCalls > 0)
				{
					--remainingCalls;
					std::map<YulName, u256> calleeEnvironment;
					execute(functionIt->second->body, calleeEnvironment, _depth + 1);
				}
				// else just skip the call
			}
		}, call->functionName);
	}

	u256 evaluate(Expression const& _expression, std::map<YulName, u256> const& _environment)
	{
		if (auto const* literal = std::get_if<Literal>(&_expression))
			return literal->value.value();
		if (auto const* identifier = std::get_if<Identifier>(&_expression))
		{
			auto const variable = _environment.find(identifier->name);
			yulAssert(variable != _environment.end(), "read of an undeclared variable");
			return variable->second;
		}
		if (auto const* call = std::get_if<FunctionCall>(&_expression))
			if (auto const* builtin = std::get_if<BuiltinName>(&call->functionName))
			{
				yulAssert(call->arguments.size() == dialect.builtin(builtin->handle).numParameters);
				std::vector<u256> const results =
					call->arguments |
					ranges::views::transform([&](Expression const& _expr) { return evaluate(_expr, _environment); }) |
					ranges::to<std::vector>;

				if (builtin->handle == mload)
					return readWord(results.front());

				if (builtin->handle == sload)
					return util::valueOrDefault(storage, results.front(), 0);

				if (builtin->handle == memoryGuard)
				{
					memoryGuardValue = results.front();
					return results.front();
				}

				if (builtin->handle == add)
					return results.front() + results.back();

				if (builtin->handle == mul)
					return results.front() * results.back();
			}
		yulAssert(false, "unsupported expression");
	}

	u256 readWord(u256 const& _offset)
	{
		accessedWords.insert(_offset);
		util::h256 word;
		for (unsigned byte = 0; byte < 32; ++byte)
			if (
				auto const value = bytewiseMemory.find(_offset + byte);
				value != bytewiseMemory.end()
			)
				word[byte] = value->second;
		return u256(word);
	}

	void writeWord(u256 const& _offset, u256 const& _value)
	{
		accessedWords.insert(_offset);
		util::h256 const word{_value};
		for (unsigned byte = 0; byte < 32; ++byte)
			bytewiseMemory[_offset + byte] = word[byte];
	}

	Dialect const& dialect;
	std::map<YulName, FunctionDefinition const*> functions;
	BuiltinHandle mstore;
	BuiltinHandle mload;
	BuiltinHandle sstore;
	BuiltinHandle sload;
	BuiltinHandle add;
	BuiltinHandle mul;
	BuiltinHandle memoryGuard;
	std::size_t remainingCalls = 0;
	std::map<u256, std::uint8_t> bytewiseMemory;
	std::map<u256, u256> storage;
	std::set<u256> accessedWords;
	u256 memoryGuardValue = 0;
	std::vector<std::pair<u256, u256>> storageWrites;
};

std::string formatTrace(std::vector<std::pair<u256, u256>> const& _writes)
{
	std::vector<std::string> entries;
	for (auto const& [key, value]: _writes)
		entries.emplace_back(fmt::format("sstore({}, {})", key.str(), value.str()));
	return fmt::format("{}", fmt::join(entries, " "));
}

auto inputDomain()
{
	return fuzztest::FlatMap(
		[](FunctionID const _numFunctions) {
			return fuzztest::StructOf<FuzzInput>(
				fuzztest::Just(_numFunctions),
				fuzztest::VectorOf(
					fuzztest::StructOf<Edge>(
						fuzztest::InRange<FunctionID>(0, _numFunctions - 1),
						fuzztest::InRange<FunctionID>(0, _numFunctions - 1)
					)
				).WithMaxSize(maxEdges),
				fuzztest::VectorOf(
					fuzztest::InRange<std::uint32_t>(0, maxVariablesPerFunction)
				).WithSize(_numFunctions)
			);
		},
		fuzztest::InRange<FunctionID>(1, maxFunctions)
	);
}

}

static void RelocationPreservesProgramBehaviour(FuzzInput const& _input)
{
	auto const& [numFunctions, edges, variableCounts] = _input;

	std::vector<std::set<FunctionID>> const adjacency = buildAdjacency(numFunctions, edges);
	std::string const source = generateProgram(numFunctions, adjacency, variableCounts);

	static auto constexpr printAst = [](Dialect const& _dialect, Block const& _root) { return AsmPrinter{_dialect}(_root); };

	YulStack yulStack;
	ASSERT_TRUE(yulStack.parseAndAnalyze("source", source)) << source;
	std::shared_ptr<Object const> const object = yulStack.parserResult();
	ASSERT_TRUE(object && object->hasCode()) << yulStack.print();
	auto const* dialect = dynamic_cast<EVMDialect const*>(object->dialect());
	ASSERT_TRUE(dialect && dialect->providesObjectAccess()) << yulStack.print();

	// all the variables are "unreachable" for the stack limit evader
	std::map<YulName, std::vector<YulName>> unreachableVariables;
	for (FunctionID function = 0; function < numFunctions; ++function)
		for (std::uint32_t variable = 0; variable < variableCounts[function]; ++variable)
			unreachableVariables[YulName{functionName(function)}].emplace_back(variableName(function, variable));

	Block const transformedAst = [&] -> Block
	{
		Block astRoot = std::get<Block>(ASTCopier{}(object->code()->root()));
		std::set<YulName> const reservedIdentifiers;
		NameDispenser dispenser(*dialect, astRoot, reservedIdentifiers);
		OptimiserStepContext context{
			.dialect = *dialect,
			.dispenser = dispenser,
			.reservedIdentifiers = reservedIdentifiers,
			.expectedExecutionsPerDeployment = std::nullopt
		};
		StackLimitEvader::run(context, astRoot, unreachableVariables);
		return astRoot;
	}();

	auto const builtin = [&](std::string const& _name) {
		std::optional<BuiltinHandle> const handle = dialect->findBuiltin(_name);
		yulAssert(handle.has_value());
		return *handle;
	};
	auto const interpret = [&](Block const& _root) {
		BoundedInterpreter interpreter{
			.dialect = *dialect,
			.functions = allFunctionDefinitions(_root),
			.mstore = builtin("mstore"),
			.mload = builtin("mload"),
			.sstore = builtin("sstore"),
			.sload = builtin("sload"),
			.add = builtin("add"),
			.mul = builtin("mul"),
			.memoryGuard = builtin("memoryguard")
		};
		interpreter.run(_root);
		return interpreter;
	};

	BoundedInterpreter const originalRun = interpret(object->code()->root());
	BoundedInterpreter const rewrittenRun = interpret(transformedAst);

	// We must have identical storage write sequences under the same bounded execution
	ASSERT_TRUE(originalRun.storageWrites == rewrittenRun.storageWrites) << fmt::format(
		"the rewritten program writes different values to storage\noriginal:  {}\nrewritten: {}\n{}\n{}",
		formatTrace(originalRun.storageWrites),
		formatTrace(rewrittenRun.storageWrites),
		yulStack.print(),
		printAst(*dialect, transformedAst)
	);
	// memoryguard value didn't shrink
	ASSERT_TRUE(rewrittenRun.memoryGuardValue >= originalRun.memoryGuardValue);
	// words that were accessed only after transformation must lie within the allocated spill-address-space
	for (u256 const& offset: rewrittenRun.accessedWords)
	{
		if (originalRun.accessedWords.contains(offset))
			continue;
		ASSERT_TRUE(offset >= u256(reservedMemoryStart) && offset <= rewrittenRun.memoryGuardValue - 32) << fmt::format(
			"the rewritten program touches the word at offset {}, which is not entirely inside the "
			"reserved range [{}, {})\n{}\n{}",
			offset.str(),
			reservedMemoryStart,
			rewrittenRun.memoryGuardValue.str(),
			yulStack.print(),
			printAst(*dialect, transformedAst)
		);
	}
}

FUZZ_TEST(StackLimitEvaderProperty, RelocationPreservesProgramBehaviour)
	.WithDomains(inputDomain());

/// f0 and f1 are mutually recursive and need no slots themselves, but f0 reaches f2, which does. f3 calls
/// into the cycle and needs a slot of its own, so it must not reuse the slot of f2. f0 being visited before
/// f3 is what makes the requirements of the cycle observable to f3.
TEST(StackLimitEvaderProperty, MutualRecursionWithSpillingCaller)
{
	RelocationPreservesProgramBehaviour(FuzzInput{
		.numFunctions = 4,
		.edges = {{0, 2}, {0, 1}, {1, 0}, {3, 1}},
		.variableCounts = {0, 0, 1, 1}
	});
}

}
