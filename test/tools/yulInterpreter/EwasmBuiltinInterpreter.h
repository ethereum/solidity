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
 * Yul interpreter module that evaluates Ewasm builtins.
 */

#pragma once

#include <libyul/ASTForward.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/FixedHash.h>

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

struct InterpreterState;

/**
 * Interprets Ewasm builtins based on the current state and logs instructions with
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
 *
 * The type names are following the Ewasm specification (https://github.com/ewasm/design/blob/master/eth_interface.md).
 */
class EwasmBuiltinInterpreter
{
public:
	explicit EwasmBuiltinInterpreter(InterpreterState& _state):
		m_state(_state)
	{}
	/// Evaluate builtin function
	u256 evalBuiltin(
		YulString _functionName,
		std::vector<Expression> const& _arguments,
		std::vector<u256> const& _evaluatedArguments
	);

private:
	template <typename Word>
	u256 evalWasmBuiltin(
		std::string const& _fun,
		std::vector<Word> const& _arguments
	);
	u256 evalEthBuiltin(
		std::string const& _fun,
		std::vector<uint64_t> const& _arguments
	);

	/// Checks if the memory access is not too large for the interpreter and adjusts
	/// msize accordingly.
	void accessMemory(u256 const& _offset, u256 const& _size = 32);
	/// @returns the memory contents at the provided address.
	/// Does not adjust msize, use @a accessMemory for that
	bytes readMemory(uint64_t _offset, uint64_t _size = 32);
	/// @returns the memory contents (8 bytes) at the provided address (little-endian).
	/// Does not adjust msize, use @a accessMemory for that
	uint64_t readMemoryWord(uint64_t _offset);
	/// @returns the memory contents (4 bytes) at the provided address (little-endian).
	/// Does not adjust msize, use @a accessMemory for that
	uint32_t readMemoryHalfWord(uint64_t _offset);
	/// Writes bytes to memory.
	/// Does not adjust msize, use @a accessMemory for that
	void writeMemory(uint64_t _offset, bytes const& _value);
	/// Writes a word to memory (little-endian)
	/// Does not adjust msize, use @a accessMemory for that
	void writeMemoryWord(uint64_t _offset, uint64_t _value);
	/// Writes a 4-byte value to memory (little-endian)
	/// Does not adjust msize, use @a accessMemory for that
	void writeMemoryHalfWord(uint64_t _offset, uint32_t _value);
	/// Writes a byte to memory
	/// Does not adjust msize, use @a accessMemory for that
	void writeMemoryByte(uint64_t _offset, uint8_t _value);

	/// Helper for eth.* builtins. Writes to memory (little-endian) and always returns zero.
	void writeU256(uint64_t _offset, u256 _value, size_t _croppedTo = 32);
	void writeU128(uint64_t _offset, u256 _value) { writeU256(_offset, std::move(_value), 16); }
	/// Helper for eth.* builtins. Writes to memory (as a byte string).
	void writeBytes32(uint64_t _offset, util::h256 _value) { accessMemory(_offset, 32); writeMemory(_offset, _value.asBytes()); }
	void writeAddress(uint64_t _offset, util::h160 _value) { accessMemory(_offset, 20); writeMemory(_offset, _value.asBytes()); }
	/// Helper for eth.* builtins. Reads from memory (little-endian) and returns the value.
	u256 readU256(uint64_t _offset, size_t _croppedTo = 32);
	u256 readU128(uint64_t _offset) { return readU256(_offset, 16); }
	/// Helper for eth.* builtins. Reads from memory (as a byte string).
	util::h256 readBytes32(uint64_t _offset) { accessMemory(_offset, 32); return util::h256(readMemory(_offset, 32)); }
	util::h160 readAddress(uint64_t _offset) { accessMemory(_offset, 20); return util::h160(readMemory(_offset, 20)); }

	void logTrace(evmasm::Instruction _instruction, std::vector<u256> const& _arguments = {}, bytes const& _data = {});
	/// Appends a log to the trace representing an instruction or similar operation by string,
	/// with arguments and auxiliary data (if nonempty).
	void logTrace(std::string const& _pseudoInstruction, std::vector<u256> const& _arguments = {}, bytes const& _data = {});

	InterpreterState& m_state;
};

}
