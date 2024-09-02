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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Forward declaration of classes for inline assembly / Yul AST
 */

#pragma once

#include <variant>

namespace solidity::yul
{

enum class LiteralKind;
class LiteralValue;
struct Literal;
struct Label;
struct Identifier;
struct Assignment;
struct VariableDeclaration;
struct FunctionDefinition;
struct FunctionCall;
struct If;
struct Switch;
struct Case;
struct ForLoop;
struct Break;
struct Continue;
struct Leave;
struct ExpressionStatement;
struct Block;
class AST;

struct NameWithDebugData;

using Expression = std::variant<FunctionCall, Identifier, Literal>;
using Statement = std::variant<ExpressionStatement, Assignment, VariableDeclaration, FunctionDefinition, If, Switch, ForLoop, Break, Continue, Leave, Block>;

}
