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

#include <libsolidity/formal/SSAVariable.h>

#include <libsolidity/formal/SymbolicIntVariable.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SSAVariable::SSAVariable(Declaration const* _decl,
	smt::SolverInterface& _interface)
{
	resetIndex();

	if (dynamic_cast<IntegerType const*>(_decl->type().get()))
		m_symbVar = make_shared<SymbolicIntVariable>(_decl, _interface);
	else
	{
		//solAssert(false, "");
	}
}

void SSAVariable::resetIndex()
{
	m_currentSequenceCounter = 0;
	m_nextFreeSequenceCounter.reset (new int);
	*m_nextFreeSequenceCounter = 1;
}

int SSAVariable::index() const
{
	return m_currentSequenceCounter;
}

int SSAVariable::next() const
{
	return *m_nextFreeSequenceCounter;
}

void SSAVariable::setZeroValue()
{
	m_symbVar->setZeroValue(index());
}

void SSAVariable::setUnknownValue()
{
	m_symbVar->setUnknownValue(index());
}
