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

#include <libsolidity/formal/SymbolicBoolVariable.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicBoolVariable::SymbolicBoolVariable(
	Declaration const* _decl,
	smt::SolverInterface&_interface
):
	SymbolicVariable(_decl, _interface)
{
	solAssert(m_declaration->type()->category() == Type::Category::Bool, "");
	m_expression = make_shared<smt::Expression>(m_interface.newFunction(uniqueSymbol(), smt::Sort::Int, smt::Sort::Bool));
}

void SymbolicBoolVariable::setZeroValue(int _seq)
{
	m_interface.addAssertion(valueAtSequence(_seq) == smt::Expression(false));
}

void SymbolicBoolVariable::setUnknownValue(int)
{
}
