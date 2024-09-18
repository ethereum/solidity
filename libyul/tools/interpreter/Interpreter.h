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
 * Yul interpreter.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulName.h>
#include <libyul/Exceptions.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/CommonData.h>

#include <libsolutil/Exceptions.h>

#include <map>

namespace solidity::yul
{
struct Dialect;
}

namespace solidity::yul::tools::interpreter
{

template<typename T>
class ExecutionTerminatedCommon
{
public:
	bool operator==(T) const { return true; }
	bool operator!=(T) const { return false; }
};

class ExplicitlyTerminated : public ExecutionTerminatedCommon<ExplicitlyTerminated>
{
};

class ExplicitlyTerminatedWithReturn : public ExecutionTerminatedCommon<ExplicitlyTerminatedWithReturn>
{
};

class StepLimitReached : public ExecutionTerminatedCommon<StepLimitReached>
{
};

class TraceLimitReached : public ExecutionTerminatedCommon<TraceLimitReached>
{
};

class ExpressionNestingLimitReached : public ExecutionTerminatedCommon<ExpressionNestingLimitReached>
{
};

using ExecutionTerminated = std::variant<
	ExplicitlyTerminated,
	ExplicitlyTerminatedWithReturn,
	StepLimitReached,
	TraceLimitReached,
	ExpressionNestingLimitReached
>;

enum class ControlFlowState
{
	Default,
	Continue,
	Break,
	Leave
};

struct ExecutionOk
{
	ControlFlowState state;

	bool operator==(ExecutionOk other) const
	{
		return state == other.state;
	}

	bool operator!=(ExecutionOk other) const
	{
		return state != other.state;
	}
};

struct EvaluationOk
{
	std::vector<u256> values;

	EvaluationOk(u256 _x):
		values{_x}
	{
	}

	EvaluationOk(std::vector<u256> const& _values):
		values(_values)
	{
	}
};

using ExecutionResult = std::variant<ExecutionOk, ExecutionTerminated>;
using EvaluationResult = std::variant<EvaluationOk, ExecutionTerminated>;

struct InterpreterState
{
	bytes calldata;
	bytes returndata;
	std::map<u256, uint8_t> memory;
	/// This is different than memory.size() because we ignore gas.
	u256 msize;
	std::map<util::h256, util::h256> storage;
	std::map<util::h256, util::h256> transientStorage;
	util::h160 address = util::h160("0x0000000000000000000000000000000011111111");
	u256 balance = 0x22222222;
	u256 selfbalance = 0x22223333;
	util::h160 origin = util::h160("0x0000000000000000000000000000000033333333");
	util::h160 caller = util::h160("0x0000000000000000000000000000000044444444");
	u256 callvalue = 0x55555555;
	/// Deployed code
	bytes code = util::asBytes("codecodecodecodecode");
	u256 gasprice = 0x66666666;
	util::h160 coinbase = util::h160("0x0000000000000000000000000000000077777777");
	u256 timestamp = 0x88888888;
	u256 blockNumber = 1024;
	u256 difficulty = 0x9999999;
	u256 prevrandao = (u256(1) << 64) + 1;
	u256 gaslimit = 4000000;
	u256 chainid = 0x01;
	/// The minimum value of basefee: 7 wei.
	u256 basefee = 0x07;
	/// The minimum value of blobbasefee: 1 wei.
	u256 blobbasefee = 0x01;
	/// Log of changes / effects. Sholud be structured data in the future.
	std::vector<std::string> trace;
	/// This is actually an input parameter that more or less limits the runtime.
	size_t maxTraceSize = 0;
	size_t maxSteps = 0;
	size_t numSteps = 0;
	size_t maxExprNesting = 0;

	/// Number of the current state instance, used for recursion protection
	size_t numInstance = 0;

	// Blob commitment hash version
	util::FixedHash<1> const blobHashVersion = util::FixedHash<1>(1);
	// Blob commitments
	std::array<u256, 2> const blobCommitments = {0x01, 0x02};

	/// Prints execution trace and non-zero storage to @param _out.
	/// Flag @param _disableMemoryTrace, if set, does not produce a memory dump. This
	/// avoids false positives reports by the fuzzer when certain optimizer steps are
	/// activated e.g., Redundant store eliminator, Equal store eliminator.
	void dumpTraceAndState(std::ostream& _out, bool _disableMemoryTrace) const;
	/// Prints non-zero storage to @param _out.
	void dumpStorage(std::ostream& _out) const;
	/// Prints non-zero transient storage to @param _out.
	void dumpTransientStorage(std::ostream& _out) const;

	bytes readMemory(u256 const& _offset, u256 const& _size)
	{
		yulAssert(_size <= 0xffff, "Too large read.");
		bytes data(size_t(_size), uint8_t(0));
		for (size_t i = 0; i < data.size(); ++i)
			data[i] = memory[_offset + i];
		return data;
	}
};

/**
 * Scope structure built and maintained during execution.
 */
struct Scope
{
	/// Used for variables and functions. Value is nullptr for variables.
	std::map<YulName, FunctionDefinition const*> names;
	std::map<Block const*, std::unique_ptr<Scope>> subScopes;
	Scope* parent = nullptr;
};

/**
 * Yul interpreter.
 */
class Interpreter
{
public:
	/// Executes the Yul interpreter. Flag @param _disableMemoryTracing if set ensures that
	/// instructions that write to memory do not affect @param _state. This
	/// avoids false positives reports by the fuzzer when certain optimizer steps are
	/// activated e.g., Redundant store eliminator, Equal store eliminator.
	static void run(
		InterpreterState& _state,
		Dialect const& _dialect,
		Block const& _ast,
		bool _disableMemoryTracing
	);

	Interpreter(
		InterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		bool _disableMemoryTracing,
		std::map<YulName, u256> _variables = {}
	):
		m_dialect(_dialect),
		m_state(_state),
		m_variables(std::move(_variables)),
		m_scope(&_scope),
		m_disableMemoryTrace(_disableMemoryTracing)
	{
	}

	ExecutionResult operator()(ExpressionStatement const& _statement);
	ExecutionResult operator()(Assignment const& _assignment);
	ExecutionResult operator()(VariableDeclaration const& _varDecl);
	ExecutionResult operator()(If const& _if);
	ExecutionResult operator()(Switch const& _switch);
	ExecutionResult operator()(FunctionDefinition const&);
	ExecutionResult operator()(ForLoop const&);
	ExecutionResult operator()(Break const&);
	ExecutionResult operator()(Continue const&);
	ExecutionResult operator()(Leave const&);
	ExecutionResult operator()(Block const& _block);

	ExecutionResult visit(Statement const& _st);

	bytes returnData() const { return m_state.returndata; }
	std::vector<std::string> const& trace() const { return m_state.trace; }

	u256 valueOfVariable(YulName _name) const { return m_variables.at(_name); }

protected:
	// evaluate the expression and assert that the number of return variable is _numReturnVars
	virtual EvaluationResult evaluate(Expression const& _expression, size_t _numReturnVars);

	void enterScope(Block const& _block);
	void leaveScope();

	/// Increment interpreter step count, throwing exception if step limit
	/// is reached.
	std::optional<ExecutionTerminated> incrementStep();

	Dialect const& m_dialect;
	InterpreterState& m_state;
	/// Values of variables.
	std::map<YulName, u256> m_variables;
	Scope* m_scope;
	bool m_disableMemoryTrace;
};

/**
 * Yul expression evaluator.
 */
class ExpressionEvaluator
{
public:
	ExpressionEvaluator(
		InterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		std::map<YulName, u256> const& _variables,
		bool _disableMemoryTrace
	):
		m_state(_state),
		m_dialect(_dialect),
		m_variables(_variables),
		m_scope(_scope),
		m_disableMemoryTrace(_disableMemoryTrace)
	{}

	EvaluationResult operator()(Literal const&);
	EvaluationResult operator()(Identifier const&);
	EvaluationResult operator()(FunctionCall const& _funCall);

	EvaluationResult visit(Expression const& _st);

protected:
	virtual std::unique_ptr<Interpreter> makeInterpreterCopy(std::map<YulName, u256> _variables = {}) const
	{
		return std::make_unique<Interpreter>(
			m_state,
			m_dialect,
			m_scope,
			m_disableMemoryTrace,
			std::move(_variables)
		);
	}
	virtual std::unique_ptr<Interpreter> makeInterpreterNew(InterpreterState& _state, Scope& _scope) const
	{
		return std::make_unique<Interpreter>(
			_state,
			m_dialect,
			_scope,
			m_disableMemoryTrace
		);
	}

	/// Evaluates the given expression from right to left and
	/// stores it in m_value.
	EvaluationResult evaluateArgs(
		std::vector<Expression> const& _expr,
		std::vector<std::optional<LiteralKind>> const* _literalArguments
	);

	/// Increment evaluation count, throwing exception if the
	/// nesting level is beyond the upper bound configured in
	/// the interpreter state.
	std::optional<ExecutionTerminated> incrementStep();

	InterpreterState& m_state;
	Dialect const& m_dialect;
	/// Values of variables.
	std::map<YulName, u256> const& m_variables;
	Scope& m_scope;
	/// Current expression nesting level
	unsigned m_nestingLevel = 0;
	/// Flag to disable memory tracing
	bool m_disableMemoryTrace;
};

}
