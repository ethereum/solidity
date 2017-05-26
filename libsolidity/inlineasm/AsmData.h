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

#include <boost/variant.hpp>
#include <libevmasm/Instruction.h>
#include <libevmasm/SourceLocation.h>

namespace dev
{
namespace solidity
{
namespace assembly
{

using Type = std::string;

struct TypedName { SourceLocation location; std::string name; Type type; };
using TypedNameList = std::vector<TypedName>;

/// What follows are the AST nodes for assembly.

struct Instruction;
struct Literal;
struct Label;
struct StackAssignment;
struct Identifier;
struct Assignment;
struct VariableDeclaration;
struct FunctionalInstruction;
struct FunctionDefinition;
struct FunctionCall;
struct Switch;
struct Block;

using Statement = boost::variant<Instruction, Literal, Label, StackAssignment, Identifier, Assignment, FunctionCall, FunctionalInstruction, VariableDeclaration, FunctionDefinition, Switch, Block>;

/// Direct EVM instruction (except PUSHi and JUMPDEST)
struct Instruction { SourceLocation location; solidity::Instruction instruction; };
/// Literal number or string (up to 32 bytes)
enum class LiteralKind { Number, Boolean, String };
struct Literal { SourceLocation location; LiteralKind kind; std::string value; Type type; };
/// External / internal identifier or label reference
struct Identifier { SourceLocation location; std::string name; };
/// Jump label ("name:")
struct Label { SourceLocation location; std::string name; };
/// Assignment from stack (":= x", moves stack top into x, potentially multiple slots)
struct StackAssignment { SourceLocation location; Identifier variableName; };
/// Assignment ("x := mload(20:u256)", expects push-1-expression on the right hand
/// side and requires x to occupy exactly one stack slot.
struct Assignment { SourceLocation location; Identifier variableName; std::shared_ptr<Statement> value; };
/// Functional instruction, e.g. "mul(mload(20:u256), add(2:u256, x))"
struct FunctionalInstruction { SourceLocation location; Instruction instruction; std::vector<Statement> arguments; };
struct FunctionCall { SourceLocation location; Identifier functionName; std::vector<Statement> arguments; };
/// Block-scope variable declaration ("let x:u256 := mload(20:u256)"), non-hoisted
struct VariableDeclaration { SourceLocation location; TypedNameList variables; std::shared_ptr<Statement> value; };
/// Block that creates a scope (frees declared stack variables)
struct Block { SourceLocation location; std::vector<Statement> statements; };
/// Function definition ("function f(a, b) -> (d, e) { ... }")
struct FunctionDefinition { SourceLocation location; std::string name; TypedNameList arguments; TypedNameList returns; Block body; };
/// Switch case or default case
struct Case { SourceLocation location; std::shared_ptr<Literal> value; Block body; };
/// Switch statement
struct Switch { SourceLocation location; std::shared_ptr<Statement> expression; std::vector<Case> cases; };

struct LocationExtractor: boost::static_visitor<SourceLocation>
{
	template <class T> SourceLocation operator()(T const& _node) const
	{
		return _node.location;
	}
};

/// Extracts the source location from an inline assembly node.
template <class T> inline SourceLocation locationOf(T const& _node)
{
	return boost::apply_visitor(LocationExtractor(), _node);
}

}
}
}
