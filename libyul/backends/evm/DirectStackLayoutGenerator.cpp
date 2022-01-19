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
 * Stack layout generator for Yul to EVM code generation.
 */

#include <libyul/backends/evm/DirectStackLayoutGenerator.h>

#include <libyul/AST.h>

#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/reverse.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

void DirectStackLayoutGenerator::run(Block const& _block)
{
	yulAssert(_block.statements.size() > 0, "");
	yulAssert(holds_alternative<Block>(_block.statements.front()), "");

	{
		DirectStackLayoutGenerator generator;
		generator(get<Block>(_block.statements.front()));
	}
	for (Statement const& statement: _block.statements | ranges::views::drop_exactly(1))
	{
		FunctionDefinition const* functionDefinition = get_if<FunctionDefinition>(&statement);
		yulAssert(functionDefinition, "");
		DirectStackLayoutGenerator generator;
		generator(*functionDefinition);
	}
}

void DirectStackLayoutGenerator::operator()(Block const& _block)
{
	for (Statement const& statement: _block.statements | ranges::views::reverse)
		visit(statement);
}

void DirectStackLayoutGenerator::operator()(VariableDeclaration const& _variableDeclaration)
{

}

void DirectStackLayoutGenerator::operator()(Assignment const& _assignment)
{

}

void DirectStackLayoutGenerator::operator()(ExpressionStatement const& _expressionStatement)
{

}

void DirectStackLayoutGenerator::operator()(If const& _if)
{
}
void DirectStackLayoutGenerator::operator()(Switch const& _switch)
{
}
void DirectStackLayoutGenerator::operator()(ForLoop const& _forLoop)
{
}

void DirectStackLayoutGenerator::operator()(FunctionDefinition const& _functionDefinition)
{
}