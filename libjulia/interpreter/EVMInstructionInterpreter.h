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
/**
 * IULIA interpreter module that evaluates EVM instructions.
 */

#pragma once

#include <libjulia/ASTDataForward.h>

#include <libdevcore/CommonData.h>

#include <vector>

namespace dev
{
namespace solidity
{
enum class Instruction: uint8_t;
}
namespace julia
{

struct InterpreterState;

/**
 * Interprets EVM instructions based on the current state and logs instructions with
 * side-effects.
 *
 * Since this is mainly meant to be used for testing, it is focused on a single
 * contract only, does not do any gas counting and differs from the correct
 * implementation in the following ways:
 *
 * - If memory access to a "large" memory position is performed, a deterministic
 *    value is returned. Data that is stored in a "large" memory position is not
 *    retained.
 * - The blockhash instruction returns a fixed value if the argument is in range.
 * - Extcodesize returns a deterministic value depending on the address.
 * - Extcodecopy copies a deterministic value depending on the address.
 * - And many other things
 * - TODO
 *
 */
class EVMInstructionInterpreter
{
public:
	explicit EVMInstructionInterpreter(InterpreterState& _state):
		m_state(_state)
	{}
	u256 eval(solidity::Instruction const& _instruction, std::vector<u256> const& _arguments);

private:
	/// Record a memory read in the trace. Also updaes m_state.msize
	/// @returns true if m_state.memory can be used at that offset.
	bool logMemoryRead(u256 const& _offset, u256 const& _size = 32);
	/// Record a memory write in the trace. Also updaes m_state.msize
	/// @returns true if m_state.memory can be used at that offset.
	bool logMemoryWrite(u256 const& _offset, u256 const& _size = 32);

	bool logMemory(std::string const& _type, u256 const& _offset, u256 const& _size = 32);

	void logTrace(std::string const& _message);

	InterpreterState& m_state;
};

}
}
