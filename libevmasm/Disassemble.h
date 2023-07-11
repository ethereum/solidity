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

#pragma once

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>

#include <libevmasm/Instruction.h>

#include <functional>
#include <string>

namespace solidity::evmasm
{

/// Iterate through EVM code and call a function on each instruction.
void eachInstruction(bytes const& _mem, langutil::EVMVersion _evmVersion, std::function<void(Instruction, u256 const&)> const& _onInstruction);

/// Convert from EVM code to simple EVM assembly language.
std::string disassemble(bytes const& _mem, langutil::EVMVersion _evmVersion, std::string const& _delimiter = " ");

}
