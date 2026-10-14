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
 * Parser for the textual SSA CFG format produced by `io::print`.
 */

#pragma once

#include <cstddef>
#include <expected>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

namespace solidity::yul
{

class EVMDialect;

}

namespace solidity::yul::ssa
{

struct ControlFlowGraphs;

}

namespace solidity::yul::ssa::io
{

struct SourceRange
{
	std::size_t begin{std::numeric_limits<std::size_t>::max()};
	std::size_t end{std::numeric_limits<std::size_t>::max()};
};

struct ParseError
{
	SourceRange location;
	std::string message;
};

/// Parses the output of `io::print` back into control flow graphs.
///
/// Function names must be unique, i.e., the graphs must originate from a disambiguated AST.
/// Returns the first error on malformed or inconsistent input.
std::expected<std::unique_ptr<ControlFlowGraphs>, ParseError> parse(
	std::string_view _source,
	EVMDialect const& _dialect
);

}
