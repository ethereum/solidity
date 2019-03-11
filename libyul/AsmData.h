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
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Parsed inline assembly to be used by the AST
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>

#include <libevmasm/Instruction.h>
#include <liblangutil/SourceLocation.h>

#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>

#include <map>
#include <memory>

namespace yul
{

using Type = YulString;

struct TypedName { langutil::SourceLocation location; YulString name; Type type; };
using TypedNameList = std::vector<TypedName>;

/// Direct EVM instruction (except PUSHi and JUMPDEST)
struct Instruction { langutil::SourceLocation location; dev::solidity::Instruction instruction; };
/// Literal number or string (up to 32 bytes)
enum class LiteralKind { Number, Boolean, String };
struct Literal { langutil::SourceLocation location; LiteralKind kind; YulString value; Type type; };
/// External / internal identifier or label reference
struct Identifier { langutil::SourceLocation location; YulString name; };
/// Jump label ("name:")
struct Label { langutil::SourceLocation location; YulString name; };
/// Assignment from stack (":= x", moves stack top into x, potentially multiple slots)
struct StackAssignment { langutil::SourceLocation location; Identifier variableName; };
/// Assignment ("x := mload(20:u256)", expects push-1-expression on the right hand
/// side and requires x to occupy exactly one stack slot.
///
/// Multiple assignment ("x, y := f()"), where the left hand side variables each occupy
/// a single stack slot and expects a single expression on the right hand returning
/// the same amount of items as the number of variables.
struct Assignment { langutil::SourceLocation location; std::vector<Identifier> variableNames; std::unique_ptr<Expression> value; };
/// Functional instruction, e.g. "mul(mload(20:u256), add(2:u256, x))"
struct FunctionalInstruction { langutil::SourceLocation location; dev::solidity::Instruction instruction; std::vector<Expression> arguments; };
struct FunctionCall { langutil::SourceLocation location; Identifier functionName; std::vector<Expression> arguments; };
/// Statement that contains only a single expression
struct ExpressionStatement { langutil::SourceLocation location; Expression expression; };
/// Block-scope variable declaration ("let x:u256 := mload(20:u256)"), non-hoisted
struct VariableDeclaration { langutil::SourceLocation location; TypedNameList variables; std::unique_ptr<Expression> value; };
/// Block that creates a scope (frees declared stack variables)
struct Block { langutil::SourceLocation location; std::vector<Statement> statements; };
/// Function definition ("function f(a, b) -> (d, e) { ... }")
struct FunctionDefinition { langutil::SourceLocation location; YulString name; TypedNameList parameters; TypedNameList returnVariables; Block body; };
/// Conditional execution without "else" part.
struct If { langutil::SourceLocation location; std::unique_ptr<Expression> condition; Block body; };
/// Switch case or default case
struct Case { langutil::SourceLocation location; std::unique_ptr<Literal> value; Block body; };
/// Switch statement
struct Switch { langutil::SourceLocation location; std::unique_ptr<Expression> expression; std::vector<Case> cases; };
struct ForLoop { langutil::SourceLocation location; Block pre; std::unique_ptr<Expression> condition; Block post; Block body; };
/// Break statement (valid within for loop)
struct Break { langutil::SourceLocation location; };
/// Continue statement (valid within for loop)
struct Continue { langutil::SourceLocation location; };

struct LocationExtractor: boost::static_visitor<langutil::SourceLocation>
{
	template <class T> langutil::SourceLocation operator()(T const& _node) const
	{
		return _node.location;
	}
};

/// Extracts the source location from an inline assembly node.
template <class T> inline langutil::SourceLocation locationOf(T const& _node)
{
	return boost::apply_visitor(LocationExtractor(), _node);
}

}
