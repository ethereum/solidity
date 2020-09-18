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

#include <libsolidity/formal/SymbolicTypes.h>
#include <libsolidity/formal/EncodingContext.h>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;
using namespace solidity::frontend::smt;

SymbolicState::SymbolicState(EncodingContext& _context):
	m_context(_context)
{
	m_stateMembers.emplace("balances", make_shared<smtutil::ArraySort>(smtutil::SortProvider::uintSort, smtutil::SortProvider::uintSort));

	vector<string> members;
	vector<SortPointer> sorts;
	for (auto const& [component, sort]: m_stateMembers)
	{
		members.emplace_back(component);
		sorts.emplace_back(sort);
		m_componentIndices[component] = members.size() - 1;
	}
	m_stateTuple = make_unique<SymbolicTupleVariable>(
		make_shared<smtutil::TupleSort>("state_type", members, sorts),
		"state",
		m_context
	);
}

void SymbolicState::reset()
{
	m_error.resetIndex();
	m_thisAddress.resetIndex();
	m_stateTuple->resetIndex();
}

// Blockchain

SymbolicIntVariable& SymbolicState::errorFlag()
{
	return m_error;
}

SortPointer SymbolicState::errorFlagSort()
{
	return m_error.sort();
}

smtutil::Expression SymbolicState::thisAddress()
{
	return m_thisAddress.currentValue();
}

smtutil::Expression SymbolicState::thisAddress(unsigned _idx)
{
	return m_thisAddress.valueAtIndex(_idx);
}

SortPointer SymbolicState::thisAddressSort()
{
	return m_thisAddress.sort();
}

smtutil::Expression SymbolicState::state()
{
	return m_stateTuple->currentValue();
}

smtutil::Expression SymbolicState::state(unsigned _idx)
{
	return m_stateTuple->valueAtIndex(_idx);
}

SortPointer SymbolicState::stateSort()
{
	return m_stateTuple->sort();
}

void SymbolicState::newState()
{
	m_stateTuple->increaseIndex();
}

smtutil::Expression SymbolicState::balances()
{
	return m_stateTuple->component(m_componentIndices.at("balances"));
}

smtutil::Expression SymbolicState::balance()
{
	return balance(thisAddress());
}

smtutil::Expression SymbolicState::balance(smtutil::Expression _address)
{
	return smtutil::Expression::select(balances(), move(_address));
}

void SymbolicState::transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value)
{
	unsigned indexBefore = m_stateTuple->index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_stateTuple->index();
	solAssert(indexAfter > indexBefore, "");
	m_stateTuple->increaseIndex();
	/// Do not apply the transfer operation if _from == _to.
	auto newState = smtutil::Expression::ite(
		move(_from) == move(_to),
		m_stateTuple->valueAtIndex(indexBefore),
		m_stateTuple->valueAtIndex(indexAfter)
	);
	m_context.addAssertion(m_stateTuple->currentValue() == newState);
}

/// Private helpers.

void SymbolicState::addBalance(smtutil::Expression _address, smtutil::Expression _value)
{
	auto newBalances = smtutil::Expression::store(
		balances(),
		_address,
		balance(_address) + move(_value)
	);
	assignStateMember("balances", newBalances);
}

smtutil::Expression SymbolicState::assignStateMember(string const& _member, smtutil::Expression const& _value)
{
	vector<smtutil::Expression> args;
	for (auto const& member: m_stateMembers)
		if (member.first == _member)
			args.emplace_back(_value);
		else
			args.emplace_back(m_stateTuple->component(m_componentIndices.at(member.first)));
	m_stateTuple->increaseIndex();
	auto tuple = m_stateTuple->currentValue();
	auto sortExpr = smtutil::Expression(make_shared<smtutil::SortSort>(tuple.sort), tuple.name);
	m_context.addAssertion(tuple == smtutil::Expression::tuple_constructor(sortExpr, args));
	return m_stateTuple->currentValue();
}
