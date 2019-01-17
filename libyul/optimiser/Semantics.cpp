/*(
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
 * Specific AST walkers that collect semantical facts.
 */

#include <libyul/optimiser/Semantics.h>

#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <libevmasm/SemanticInformation.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

MovableChecker::MovableChecker(Dialect const& _dialect):
	m_dialect(_dialect)
{
}

MovableChecker::MovableChecker(Dialect const& _dialect, Expression const& _expression):
	MovableChecker(_dialect)
{
	visit(_expression);
}

void MovableChecker::operator()(Identifier const& _identifier)
{
	ASTWalker::operator()(_identifier);
	m_variableReferences.emplace(_identifier.name);
}

void MovableChecker::operator()(FunctionalInstruction const& _instr)
{
	if (!eth::SemanticInformation::movable(_instr.instruction))
		m_movable = false;
	else
		ASTWalker::operator()(_instr);
}

void MovableChecker::operator()(FunctionCall const& _functionCall)
{
	if (BuiltinFunction const* f = m_dialect.builtin(_functionCall.functionName.name))
		if (f->movable)
		{
			ASTWalker::operator()(_functionCall);
			return;
		}
	m_movable = false;
}

void MovableChecker::visit(Statement const&)
{
	assertThrow(false, OptimizerException, "Movability for statement requested.");
}
