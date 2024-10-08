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

#include <libyul/interpreter/Results.h>
#include <libyul/interpreter/PureInterpreterState.h>

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

namespace solidity::yul::interpreter
{

/**
 * Interprets EVM instructions based on the current state without side-effect.
 */
class PureEVMInstructionInterpreter
{
public:
	explicit PureEVMInstructionInterpreter(langutil::EVMVersion _evmVersion):
		m_evmVersion(_evmVersion)
	{}

	/// Evaluate instruction
	EvaluationResult eval(evmasm::Instruction _instruction, std::vector<u256> const& _arguments);

	/// Evaluate builtin function
	EvaluationResult evalBuiltin(
		BuiltinFunctionForEVM const& _fun,
		std::vector<Expression> const& _arguments,
		std::vector<u256> const& _evaluatedArguments
	);

private:

	langutil::EVMVersion m_evmVersion;
};

} // solidity::yul::test
