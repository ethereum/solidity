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
 * Simplified in-memory representation of a Wasm AST.
 */

#pragma once

#include <boost/variant.hpp>
#include <string>
#include <vector>

namespace yul
{
namespace wasm
{

struct Literal;
struct Identifier;
struct Label;
struct FunctionCall;
struct BuiltinCall;
struct LocalAssignment;
struct Block;
struct If;
struct Loop;
struct Break;
struct Continue;
using Expression = boost::variant<Literal, Identifier, Label, FunctionCall, BuiltinCall, LocalAssignment, Block, If, Loop, Break, Continue>;

struct Literal { uint64_t value; };
struct Identifier { std::string name; };
struct Label { std::string name; };
struct FunctionCall { std::string functionName; std::vector<Expression> arguments; };
struct BuiltinCall { std::string functionName; std::vector<Expression> arguments; };
struct LocalAssignment { std::string variableName; std::unique_ptr<Expression> value; };
struct Block { std::string labelName; std::vector<Expression> statements; };
struct If { std::unique_ptr<Expression> condition; std::vector<Expression> statements; };
struct Loop { std::string labelName; std::vector<Expression> statements; };
struct Break { Label label; };
struct Continue { Label label; };

struct VariableDeclaration { std::string variableName; };
struct FunctionDefinition
{
	std::string name;
	std::vector<std::string> parameterNames;
	bool returns;
	std::vector<VariableDeclaration> locals;
	std::vector<Expression> body;
};


}
}
