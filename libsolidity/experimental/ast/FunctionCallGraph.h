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

/// Data structure representing a function dependency graph.

#pragma once

#include <libsolidity/ast/AST.h>

#include <map>
#include <set>
#include <ostream>

namespace solidity::frontend::experimental
{

struct FunctionDependencyGraph
{
	/// Graph edges. Edges are directed and lead from the caller to the callee.
	/// The map contains a key for every function, even if does not actually perform
	/// any calls.
	std::map<FunctionDefinition const*, std::set<FunctionDefinition const*, ASTCompareByID<FunctionDefinition>>, ASTCompareByID<FunctionDefinition>> edges;
};

}
