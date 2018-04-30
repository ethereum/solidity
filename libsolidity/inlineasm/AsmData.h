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

#include <libsolidity/inlineasm/AsmDataForward.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/SourceLocation.h>

#include <boost/variant.hpp>

namespace dev
{
namespace solidity
{
namespace assembly
{

using Type = std::string;

struct TypedName
{
	TypedName() {}
	TypedName(SourceLocation const& location, std::string const& name, Type const& type):
			location(location), name(name), type(type) {}
	SourceLocation location{};
	std::string name{};
	Type type{};
};
using TypedNameList = std::vector<TypedName>;

/// Direct EVM instruction (except PUSHi and JUMPDEST)
struct Instruction
{
	Instruction() {}
	Instruction(SourceLocation const& location, solidity::Instruction instruction):
			location(location), instruction(instruction) {}
	SourceLocation location{};
	solidity::Instruction instruction{};
};

/// Literal number or string (up to 32 bytes)
enum class LiteralKind { Number, Boolean, String };

struct Literal
{
	Literal() {}
	Literal(SourceLocation const& location, LiteralKind kind, std::string const& value, Type const& type):
			location(location), kind(kind), value(value), type(type) {}
	SourceLocation location{};
	LiteralKind kind{};
	std::string value{};
	Type type{};
};

/// External / internal identifier or label reference
struct Identifier
{
	Identifier() {}
	Identifier(SourceLocation const& location, std::string const& name):
			location(location), name(name) {}
	SourceLocation location{};
	std::string name{};
};

/// Jump label ("name:")
struct Label
{
	Label() {}
	Label(SourceLocation const& location, std::string const& name):
			location(location), name(name) {}
	SourceLocation location{};
	std::string name{};
};

/// Assignment from stack (":= x", moves stack top into x, potentially multiple slots)
struct StackAssignment
{
	StackAssignment() {}
	StackAssignment(SourceLocation const& location, Identifier const& variableName):
			location(location), variableName(variableName) {}
	SourceLocation location{};
	Identifier variableName{};
};

/// Assignment ("x := mload(20:u256)", expects push-1-expression on the right hand
/// side and requires x to occupy exactly one stack slot.
///
/// Multiple assignment ("x, y := f()"), where the left hand side variables each occupy
/// a single stack slot and expects a single expression on the right hand returning
/// the same amount of items as the number of variables.
struct Assignment
{
	Assignment() {}
	Assignment(SourceLocation const& location, std::vector<Identifier> const& variableNames,
			   std::shared_ptr<Expression> const& value):
			location(location), variableNames(variableNames), value(value) {}
	SourceLocation location{};
	std::vector<Identifier> variableNames{};
	std::shared_ptr<Expression> value{};
};

/// Functional instruction, e.g. "mul(mload(20:u256), add(2:u256, x))"
struct FunctionalInstruction
{
	FunctionalInstruction() {}
	FunctionalInstruction(SourceLocation const& location, solidity::Instruction instruction,
						  std::vector<Expression> const& arguments):
			location(location), instruction(instruction), arguments(arguments) {}
	SourceLocation location{};
	solidity::Instruction instruction{};
	std::vector<Expression> arguments{};
};

struct FunctionCall
{
	FunctionCall() {}
	FunctionCall(SourceLocation const& location, Identifier const&functionName,
				 std::vector<Expression> const& arguments):
			location(location), functionName(functionName), arguments(arguments) {}
	SourceLocation location{};
	Identifier functionName{};
	std::vector<Expression> arguments{};
};

/// Statement that contains only a single expression
struct ExpressionStatement
{
	ExpressionStatement() {}
	ExpressionStatement(SourceLocation const& location, Expression const& expression):
			location(location), expression(expression) {}
	SourceLocation location{};
	Expression expression{};
};

/// Block-scope variable declaration ("let x:u256 := mload(20:u256)"), non-hoisted
struct VariableDeclaration
{
	VariableDeclaration() {}
	VariableDeclaration(SourceLocation const& location, TypedNameList const& variables,
						std::shared_ptr<Expression> const& value):
			location(location), variables(variables), value(value) {}
	SourceLocation location{};
	TypedNameList variables{};
	std::shared_ptr<Expression> value{};
};

/// Block that creates a scope (frees declared stack variables)
struct Block
{
	Block() {}
	Block(SourceLocation const& location, std::vector<Statement> const& statements):
			location(location), statements(statements) {}
	SourceLocation location{};
	std::vector<Statement> statements{};
};

/// Function definition ("function f(a, b) -> (d, e) { ... }")
struct FunctionDefinition
{
	FunctionDefinition() {}
	FunctionDefinition(SourceLocation const& location, std::string const& name, TypedNameList const& parameters,
					   TypedNameList const& returnVariables, Block const& body):
			location(location), name(name), parameters(parameters), returnVariables(returnVariables), body(body) {}
	SourceLocation location{};
	std::string name{};
	TypedNameList parameters{};
	TypedNameList returnVariables{};
	Block body{};
};

/// Conditional execution without "else" part.
struct If
{
	If() {}
	If(SourceLocation const& location, std::shared_ptr<Expression> const& condition, Block const& body):
			location(location), condition(condition), body(body) {}
	SourceLocation location{};
	std::shared_ptr<Expression> condition{};
	Block body{};
};

/// Switch case or default case
struct Case
{
	Case() {}
	Case(SourceLocation const& location, std::shared_ptr<Literal> const& value, Block const& body):
			location(location), value(value), body(body) {}
	SourceLocation location{};
	std::shared_ptr<Literal> value{};
	Block body{};
};

/// Switch statement
struct Switch
{
	Switch() {}
	Switch(SourceLocation const& location, std::shared_ptr<Expression> const& expression,
		   std::vector<Case> const &cases): location(location), expression(expression), cases(cases) {}
	SourceLocation location{};
	std::shared_ptr<Expression> expression{};
	std::vector<Case> cases{};
};

struct ForLoop
{
	ForLoop() {}
	ForLoop(SourceLocation const& location, Block const& pre, std::shared_ptr<Expression> const& condition,
			Block const &post, Block const& body):
			location(location), pre(pre), condition(condition), post(post), body(body) {}
	SourceLocation location{};
	Block pre{};
	std::shared_ptr<Expression> condition{};
	Block post{};
	Block body{};
};

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
