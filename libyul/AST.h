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
 * Parsed inline assembly to be used by the AST
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

#include <liblangutil/SourceLocation.h>

#include <memory>

namespace solidity::yul
{

using Type = YulString;

struct DebugData
{
	explicit DebugData(langutil::SourceLocation _yulLocation = {}, langutil::SourceLocation _sollLocation = {}): irLocation(std::move(_yulLocation)), originLocation(std::move(_sollLocation))
	{}
	langutil::SourceLocation irLocation;
	langutil::SourceLocation originLocation;
};

struct TypedName { std::shared_ptr<DebugData> debugData; YulString name; Type type; };
using TypedNameList = std::vector<TypedName>;

/// Literal number or string (up to 32 bytes)
enum class LiteralKind { Number, Boolean, String };
struct Literal { std::shared_ptr<DebugData> debugData; LiteralKind kind; YulString value; Type type; };
/// External / internal identifier or label reference
struct Identifier { std::shared_ptr<DebugData> debugData; YulString name; };
/// Assignment ("x := mload(20:u256)", expects push-1-expression on the right hand
/// side and requires x to occupy exactly one stack slot.
///
/// Multiple assignment ("x, y := f()"), where the left hand side variables each occupy
/// a single stack slot and expects a single expression on the right hand returning
/// the same amount of items as the number of variables.
struct Assignment { std::shared_ptr<DebugData> debugData; std::vector<Identifier> variableNames; std::unique_ptr<Expression> value; };
struct FunctionCall { std::shared_ptr<DebugData> debugData; Identifier functionName; std::vector<Expression> arguments; };
/// Statement that contains only a single expression
struct ExpressionStatement { std::shared_ptr<DebugData> debugData; Expression expression; };
/// Block-scope variable declaration ("let x:u256 := mload(20:u256)"), non-hoisted
struct VariableDeclaration { std::shared_ptr<DebugData> debugData; TypedNameList variables; std::unique_ptr<Expression> value; };
/// Block that creates a scope (frees declared stack variables)
struct Block { std::shared_ptr<DebugData> debugData; std::vector<Statement> statements; };
/// Function definition ("function f(a, b) -> (d, e) { ... }")
struct FunctionDefinition { std::shared_ptr<DebugData> debugData; YulString name; TypedNameList parameters; TypedNameList returnVariables; Block body; };
/// Conditional execution without "else" part.
struct If { std::shared_ptr<DebugData> debugData; std::unique_ptr<Expression> condition; Block body; };
/// Switch case or default case
struct Case { std::shared_ptr<DebugData> debugData; std::unique_ptr<Literal> value; Block body; };
/// Switch statement
struct Switch { std::shared_ptr<DebugData> debugData; std::unique_ptr<Expression> expression; std::vector<Case> cases; };
struct ForLoop { std::shared_ptr<DebugData> debugData; Block pre; std::unique_ptr<Expression> condition; Block post; Block body; };
/// Break statement (valid within for loop)
struct Break { std::shared_ptr<DebugData> debugData; };
/// Continue statement (valid within for loop)
struct Continue { std::shared_ptr<DebugData> debugData; };
/// Leave statement (valid within function)
struct Leave { std::shared_ptr<DebugData> debugData; };

struct LocationExtractor
{
	template <class T> std::shared_ptr<DebugData> operator()(T const& _node) const
	{
		return _node.debugData;
	}
};

/// Extracts the source location from an inline assembly node.
template <class T> inline std::shared_ptr<DebugData> locationOf(T const& _node)
{
	return std::visit(LocationExtractor(), _node);
}

}
