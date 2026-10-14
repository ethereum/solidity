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
 * Keywords of the textual SSA CFG format, shared between parser and printer.
 */

#pragma once

#include <fmt/format.h>

#include <optional>
#include <string_view>

namespace solidity::yul::ssa::io
{

enum class Keyword
{
	// Module and function headers
	MemoryGuard,
	Func,
	Args,
	NoContinue,
	Preds,
	// Instructions
	Builtin,
	Call,
	Upsilon,
	Const,
	Phi,
	Arg,
	Proj,
	Identity,
	Nop,
	Unreachable,
	Tombstone,
	True,
	// Block exits
	Jump,
	Branch,
	Return,
	MainExit,
	Terminated
};

std::string_view keywordToString(Keyword _keyword);
std::optional<Keyword> keywordFromString(std::string_view _spelling);

}

template<>
struct fmt::formatter<solidity::yul::ssa::io::Keyword>: fmt::formatter<std::string_view>
{
	auto format(solidity::yul::ssa::io::Keyword const _keyword, fmt::format_context& _ctx) const
	{
		return fmt::formatter<std::string_view>::format(solidity::yul::ssa::io::keywordToString(_keyword), _ctx);
	}
};
