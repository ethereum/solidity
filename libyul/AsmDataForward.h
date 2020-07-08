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

struct TypedName;

using Expression = std::variant<FunctionCall, Identifier, Literal>;
using Statement = std::variant<ExpressionStatement, Assignment, VariableDeclaration, FunctionDefinition, If, Switch, ForLoop, Break, Continue, Leave, Block>;

}
