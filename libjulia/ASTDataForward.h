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
 * @date 2017
 * Pull in some identifiers from the solidity::assembly namespace.
 */

#pragma once

#include <libsolidity/inlineasm/AsmDataForward.h>

namespace dev
{
namespace julia
{

using Instruction = solidity::assembly::Instruction;
using Literal = solidity::assembly::Literal;
using Label = solidity::assembly::Label;
using StackAssignment = solidity::assembly::StackAssignment;
using Identifier = solidity::assembly::Identifier;
using Assignment = solidity::assembly::Assignment;
using VariableDeclaration = solidity::assembly::VariableDeclaration;
using FunctionalInstruction = solidity::assembly::FunctionalInstruction;
using FunctionDefinition = solidity::assembly::FunctionDefinition;
using FunctionCall = solidity::assembly::FunctionCall;
using If = solidity::assembly::If;
using Case = solidity::assembly::Case;
using Switch = solidity::assembly::Switch;
using ForLoop = solidity::assembly::ForLoop;
using ExpressionStatement = solidity::assembly::ExpressionStatement;
using Block = solidity::assembly::Block;

using TypedName = solidity::assembly::TypedName;

using Expression = boost::variant<FunctionalInstruction, FunctionCall, Identifier, Literal>;
using Statement = boost::variant<ExpressionStatement, Instruction, Label, StackAssignment, Assignment, VariableDeclaration, FunctionDefinition, If, Switch, ForLoop, Block>;

}
}
