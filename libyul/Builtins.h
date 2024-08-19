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

#include <libyul/ASTForward.h>

#include <cstddef>
#include <tuple>

namespace solidity::yul
{

struct Dialect;

/// Handle to reference a builtin function in the AST
struct BuiltinHandle
{
	size_t id;

	bool operator==(BuiltinHandle const& _other) const { return id == _other.id; }
	bool operator<(BuiltinHandle const& _other) const { return id < _other.id; }
};

/// Handle to reference a verbatim function in the AST
struct VerbatimHandle
{
	size_t numArgs;
	size_t numRets;

	bool operator==(VerbatimHandle const& _other) const { return numArgs == _other.numArgs && numRets == _other.numRets; }
	bool operator<(VerbatimHandle const& _other) const { return std::make_tuple(numArgs, numRets) < std::make_tuple(_other.numArgs, _other.numRets); }
};

}
