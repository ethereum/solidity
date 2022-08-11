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
 * Yul interpreter module that evaluates EVM instructions.
 */

#pragma once

#include <libyul/ASTForward.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>

#include <vector>

namespace solidity::evmasm
{
enum class Instruction: uint8_t;
}

namespace solidity::yul
{
class YulString;
struct BuiltinFunctionForEVM;
}

namespace solidity::yul::test
{

/// Copy @a _size bytes of @a _source at offset @a _sourceOffset to
/// @a _target at offset @a _targetOffset. Behaves as if @a _source would
/// continue with an infinite sequence of zero bytes beyond its end.
void copyZeroExtended(
	std::map<u256, uint8_t>& _target, bytes const& _source,
	size_t _targetOffset, size_t _sourceOffset, size_t _size
);

struct InterpreterState;

/**
 * Interprets EVM instructions based on the current state and logs instructions with
 * side-effects.
 *
 * Since this is mainly meant to be used for differential fuzz testing, it is focused
 * on a single contract only, does not do any gas counting and differs from the correct
 * implementation in many ways:
 *
 * - If memory access to a "large" memory position is performed, a deterministic
 *   value is returned. Data that is stored in a "large" memory position is not
 *   retained.
 * - The blockhash instruction returns a fixed value if the argument is in range.
 * - Extcodesize returns a deterministic value depending on the address.
 * - Extcodecopy copies a deterministic value depending on the address.
 * - And many other things
 *
 * The main focus is that the generated execution trace is the same for equivalent executions
 * and likely to be different for non-equivalent executions.
 */
class EVMInstructionInterpreter
{
public:
	explicit EVMInstructionInterpreter(InterpreterState& _state, bool _disableMemWriteTrace):
		m_state(_state),
		m_disableMemoryWriteInstructions(_disableMemWriteTrace)
	{}
	/// Evaluate instruction
	u256 eval(evmasm::Instruction _instruction, std::vector<u256> const& _arguments);
	/// Evaluate builtin function
	u256 evalBuiltin(
		BuiltinFunctionForEVM const& _fun,
		std::vector<Expression> const& _arguments,
		std::vector<u256> const& _evaluatedArguments
	);

private:
	/// Checks if the memory access is not too large for the interpreter and adjusts
	/// msize accordingly.
	/// @returns false if the amount of bytes read is lager than 0xffff
	bool accessMemory(u256 const& _offset, u256 const& _size = 32);
	/// @returns the memory contents at the provided address.
	/// Does not adjust msize, use @a accessMemory for that
	bytes readMemory(u256 const& _offset, u256 const& _size = 32);
	/// @returns the memory contents at the provided address.
	/// Does not adjust msize, use @a accessMemory for that
	u256 readMemoryWord(u256 const& _offset);
	/// @returns writes a word to memory
	/// Does not adjust msize, use @a accessMemory for that
	void writeMemoryWord(u256 const& _offset, u256 const& _value);

	void logTrace(
		evmasm::Instruction _instruction,
		std::vector<u256> const& _arguments = {},
		bytes const& _data = {}
	);
	/// Appends a log to the trace representing an instruction or similar operation by string,
	/// with arguments and auxiliary data (if nonempty). Flag @param _writesToMemory indicates
	/// whether the instruction writes to (true) or does not write to (false) memory.
	void logTrace(
		std::string const& _pseudoInstruction,
		bool _writesToMemory,
		std::vector<u256> const& _arguments = {},
		bytes const& _data = {}
	);
	/// @returns disable trace flag.
	bool memWriteTracingDisabled()
	{
		return m_disableMemoryWriteInstructions;
	}

	InterpreterState& m_state;
	/// Flag to disable trace of instructions that write to memory.
	bool m_disableMemoryWriteInstructions;
};

} // solidity::yul::test
