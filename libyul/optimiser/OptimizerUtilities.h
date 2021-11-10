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
 * Small useful snippets for the optimiser.
 */

#pragma once

#include <libsolutil/Common.h>
#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>
#include <libyul/YulString.h>

#include <optional>

namespace solidity::evmasm
{
enum class Instruction: uint8_t;
}

namespace solidity::yul
{

/// Removes statements that are just empty blocks (non-recursive).
/// If this is run on the outermost block, the FunctionGrouper should be run afterwards to keep
/// the canonical form.
void removeEmptyBlocks(Block& _block);

/// Returns true if a given literal can not be used as an identifier.
/// This includes Yul keywords and builtins of the given dialect.
bool isRestrictedIdentifier(Dialect const& _dialect, YulString const& _identifier);

/// Helper function that returns the instruction, if the `_name` is a BuiltinFunction
std::optional<evmasm::Instruction> toEVMInstruction(Dialect const& _dialect, YulString const& _name);

}
