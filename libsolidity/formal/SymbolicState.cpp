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

#include <libsolidity/formal/SymbolicState.h>

#include <libsolidity/formal/EncodingContext.h>

using namespace std;
using namespace solidity;
using namespace solidity::frontend::smt;

SymbolicState::SymbolicState(EncodingContext& _context):
	m_context(_context)
{
}

void SymbolicState::reset()
{
	m_thisAddress.resetIndex();
	m_balances.resetIndex();
}

// Blockchain

smtutil::Expression SymbolicState::thisAddress()
{
	return m_thisAddress.currentValue();
}

smtutil::Expression SymbolicState::balance()
{
	return balance(m_thisAddress.currentValue());
}

smtutil::Expression SymbolicState::balance(smtutil::Expression _address)
{
	return smtutil::Expression::select(m_balances.elements(), move(_address));
}

void SymbolicState::transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value)
{
	unsigned indexBefore = m_balances.index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_balances.index();
	solAssert(indexAfter > indexBefore, "");
	m_balances.increaseIndex();
	/// Do not apply the transfer operation if _from == _to.
	auto newBalances = smtutil::Expression::ite(
		move(_from) == move(_to),
		m_balances.valueAtIndex(indexBefore),
		m_balances.valueAtIndex(indexAfter)
	);
	m_context.addAssertion(m_balances.currentValue() == newBalances);
}

/// Private helpers.

void SymbolicState::addBalance(smtutil::Expression _address, smtutil::Expression _value)
{
	auto newBalances = smtutil::Expression::store(
		m_balances.elements(),
		_address,
		balance(_address) + move(_value)
	);
	auto oldLength = m_balances.length();
	m_balances.increaseIndex();
	m_context.addAssertion(m_balances.elements() == newBalances);
	m_context.addAssertion(m_balances.length() == oldLength);
}

