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
 * @date 2017
 * Desugars assembly, i.e. converts the parsed ast and removes functions, switches and loops.
 */

#pragma once

#include <libdevcore/Assertions.h>

//@TODO try to get rid of this
#include <libsolidity/inlineasm/AsmData.h>

#include <boost/variant.hpp>

namespace dev
{
namespace solidity
{
namespace assembly
{

struct DesugaringError: virtual Exception {};

struct Instruction;
struct Literal;
struct Identifier;
struct FunctionalInstruction;
struct Label;
struct Assignment;
struct FunctionalAssignment;
struct VariableDeclaration;
struct FunctionDefinition;
struct FunctionCall;
struct Block;
using Statement = boost::variant<Instruction, Literal, Label, Assignment, Identifier, FunctionalAssignment, FunctionCall, FunctionalInstruction, VariableDeclaration, FunctionDefinition, Block>;

struct ASTNodeReplacement
{
	ASTNodeReplacement() = default;
	ASTNodeReplacement(ASTNodeReplacement const&) = default;
	ASTNodeReplacement(ASTNodeReplacement&&) = default;
	ASTNodeReplacement& operator=(ASTNodeReplacement const&) = default;
	ASTNodeReplacement& operator=(ASTNodeReplacement&&) = default;
	ASTNodeReplacement(assembly::Statement&& _statement):
		node(std::move(_statement))
	{}
	ASTNodeReplacement(assembly::Statement const& _statement):
		node(_statement)
	{}
	ASTNodeReplacement(assembly::Statement const& _st1, assembly::Statement const& _st2):
		node(_st1), secondNode(_st2), secondValid(true)
	{}
	operator assembly::Statement() &&;
	/// Converts to single node (throws if secondValid) and MOVES contents there.
	assembly::Statement asStatement();

	/// @returns true iff the node is a single block.
	bool isBlock() const;

	assembly::Statement node;
	assembly::Statement secondNode;
	bool secondValid = false;
};

class AsmDesugar: public boost::static_visitor<ASTNodeReplacement>
{
public:
	Block run(Block const& _in);

	ASTNodeReplacement operator()(assembly::Instruction const& _instruction);
	ASTNodeReplacement operator()(assembly::Literal const& _literal);
	ASTNodeReplacement operator()(assembly::Identifier const& _identifier);
	ASTNodeReplacement operator()(assembly::FunctionalInstruction const& _functionalInstruction);
	ASTNodeReplacement operator()(assembly::Label const& _label);
	ASTNodeReplacement operator()(assembly::Assignment const& _assignment);
	ASTNodeReplacement operator()(assembly::FunctionalAssignment const& _functionalAssignment);
	ASTNodeReplacement operator()(assembly::VariableDeclaration const& _variableDeclaration);
	ASTNodeReplacement operator()(assembly::FunctionDefinition const& _functionDefinition);
	ASTNodeReplacement operator()(assembly::FunctionCall const& _functionCall);
	ASTNodeReplacement operator()(assembly::Block const& _block);
};

}
}
}
