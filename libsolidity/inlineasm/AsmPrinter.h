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
 * Converts a parsed assembly into its textual form.
 */

#pragma once

#include <boost/variant.hpp>

namespace dev
{
namespace solidity
{
namespace assembly
{
struct Instruction;
struct Literal;
struct Identifier;
struct FunctionalInstruction;
struct Label;
struct StackAssignment;
struct Assignment;
struct VariableDeclaration;
struct FunctionDefinition;
struct FunctionCall;
struct Switch;
struct Block;

class AsmPrinter: public boost::static_visitor<std::string>
{
public:
	explicit AsmPrinter(bool _julia = false): m_julia(_julia) {}

	std::string operator()(assembly::Instruction const& _instruction);
	std::string operator()(assembly::Literal const& _literal);
	std::string operator()(assembly::Identifier const& _identifier);
	std::string operator()(assembly::FunctionalInstruction const& _functionalInstruction);
	std::string operator()(assembly::Label const& _label);
	std::string operator()(assembly::StackAssignment const& _assignment);
	std::string operator()(assembly::Assignment const& _assignment);
	std::string operator()(assembly::VariableDeclaration const& _variableDeclaration);
	std::string operator()(assembly::FunctionDefinition const& _functionDefinition);
	std::string operator()(assembly::FunctionCall const& _functionCall);
	std::string operator()(assembly::Switch const& _switch);
	std::string operator()(assembly::Block const& _block);

private:
	std::string appendTypeName(std::string const& _type);

	bool m_julia = false;
};

}
}
}
