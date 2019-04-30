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
	auto sort = make_shared<smt::ArraySort>(
		make_shared<smt::Sort>(smt::Kind::Int),
		make_shared<smt::Sort>(smt::Kind::Int)
	);
	m_balances = make_unique<SymbolicVariable>(sort, "balances", m_solver);
}

void EncodingContext::reset()
{
	m_thisAddress->increaseIndex();
	m_balances->increaseIndex();
}

smt::Expression EncodingContext::thisAddress()
{
	return m_thisAddress->currentValue();
}

smt::Expression EncodingContext::balance()
{
	return balance(m_thisAddress->currentValue());
}

smt::Expression EncodingContext::balance(smt::Expression _address)
{
	return smt::Expression::select(m_balances->currentValue(), move(_address));
}

void EncodingContext::transfer(smt::Expression _from, smt::Expression _to, smt::Expression _value)
{
	unsigned indexBefore = m_balances->index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_balances->index();
	solAssert(indexAfter > indexBefore, "");
	m_balances->increaseIndex();
	/// Do not apply the transfer operation if _from == _to.
	auto newBalances = smt::Expression::ite(
		move(_from) == move(_to),
		m_balances->valueAtIndex(indexBefore),
		m_balances->valueAtIndex(indexAfter)
	);
	m_solver.addAssertion(m_balances->currentValue() == newBalances);
}

void EncodingContext::addBalance(smt::Expression _address, smt::Expression _value)
{
	auto newBalances = smt::Expression::store(
		m_balances->currentValue(),
		_address,
		balance(_address) + move(_value)
	);
	m_balances->increaseIndex();
	m_solver.addAssertion(newBalances == m_balances->currentValue());
}
