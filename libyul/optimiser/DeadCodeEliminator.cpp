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
 * Optimisation stage that removes unreachable code.
 */

#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/AsmData.h>

#include <libevmasm/SemanticInformation.h>
#include <libevmasm/AssemblyItem.h>

#include <algorithm>

using namespace std;
using namespace dev;
using namespace yul;

namespace
{
bool isTerminating(yul::ExpressionStatement const& _exprStmnt)
{
	if (_exprStmnt.expression.type() != typeid(FunctionalInstruction))
		return false;

	auto const& funcInstr = boost::get<FunctionalInstruction>(_exprStmnt.expression);

	return eth::SemanticInformation::terminatesControlFlow(funcInstr.instruction);
}

/// Returns an iterator to the first terminating statement or
/// `_block.statements.end()()` when none was found
auto findFirstTerminatingStatement(Block& _block)
{
	return find_if(
		_block.statements.begin(),
		_block.statements.end(),
		[](Statement const& _stmnt)
		{
			if (
				_stmnt.type() == typeid(ExpressionStatement) &&
				isTerminating(boost::get<ExpressionStatement>(_stmnt))
			)
				return true;
			else if (_stmnt.type() == typeid(Break))
				return true;
			else if (_stmnt.type() == typeid(Continue))
				return true;

			return false;
		}
	);
}
}

void DeadCodeEliminator::operator()(Block& _block)
{
	auto& statements = _block.statements;

	auto firstTerminatingStatment = findFirstTerminatingStatement(_block);

	if (
		firstTerminatingStatment != statements.end() &&
		firstTerminatingStatment + 1 != statements.end()
	)
		statements.erase(
			std::remove_if(
				firstTerminatingStatment + 1,
				statements.end(),
				[] (Statement const& _s)
				{
					return _s.type() != typeid(yul::FunctionDefinition);
				}
			),
			statements.end()
		);

	ASTModifier::operator()(_block);
}

