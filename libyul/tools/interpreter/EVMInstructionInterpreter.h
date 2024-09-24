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
#include <libsolutil/FixedHash.h>
#include <libsolutil/Numeric.h>

#include <liblangutil/EVMVersion.h>

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

namespace solidity::yul::tools::interpreter
{

/// Copy @a _size bytes of @a _source at offset @a _sourceOffset to
/// @a _target at offset @a _targetOffset. Behaves as if @a _source would
/// continue with an infinite sequence of zero bytes beyond its end.
void copyZeroExtended(
	std::map<u256, uint8_t>& _target,
	bytes const& _source,
	size_t _targetOffset,
	size_t _sourceOffset,
	size_t _size
);

/// Copy @a _size bytes of @a _source at offset @a _sourceOffset to
/// @a _target at offset @a _targetOffset. Behaves as if @a _source would
/// continue with an infinite sequence of zero bytes beyond its end.
/// When target and source areas overlap, behaves as if the data was copied
/// using an intermediate buffer.
void copyZeroExtendedWithOverlap(
	std::map<u256, uint8_t>& _target,
	std::map<u256, uint8_t> const& _source,
	size_t _targetOffset,
	size_t _sourceOffset,
	size_t _size
);

struct InterpreterState;

/**
 * Interprets EVM instructions based on the current state without side-effect.
 */
class EVMInstructionInterpreter
{
public:
	explicit EVMInstructionInterpreter(langutil::EVMVersion _evmVersion, InterpreterState& _state):
		m_evmVersion(_evmVersion),
		m_state(_state)
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

	langutil::EVMVersion m_evmVersion;
	InterpreterState& m_state;
public:
};

} // solidity::yul::test
