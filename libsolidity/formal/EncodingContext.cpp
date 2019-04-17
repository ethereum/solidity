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

#include <libsolidity/formal/EncodingContext.h>

#include <libsolidity/formal/SymbolicTypes.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::smt;

EncodingContext::EncodingContext(SolverInterface& _solver):
	m_solver(_solver),
	m_thisAddress(make_unique<SymbolicAddressVariable>("this", m_solver))
{
}

void EncodingContext::reset()
{
	m_thisAddress->increaseIndex();
}

smt::Expression EncodingContext::thisAddress()
{
	return m_thisAddress->currentValue();
}
