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

BlockchainVariable::BlockchainVariable(
	string _name,
	map<string, smtutil::SortPointer> _members,
	EncodingContext& _context
):
	m_name(move(_name)),
	m_members(move(_members)),
	m_context(_context)
{
	vector<string> members;
	vector<SortPointer> sorts;
	for (auto const& [component, sort]: m_members)
	{
		members.emplace_back(component);
		sorts.emplace_back(sort);
		m_componentIndices[component] = static_cast<unsigned>(members.size() - 1);
	}
	m_tuple = make_unique<SymbolicTupleVariable>(
		make_shared<smtutil::TupleSort>(m_name + "_type", members, sorts),
		m_name,
		m_context
	);
}

smtutil::Expression BlockchainVariable::member(string const& _member) const
{
	return m_tuple->component(m_componentIndices.at(_member));
}

smtutil::Expression BlockchainVariable::assignMember(string const& _member, smtutil::Expression const& _value)
{
	vector<smtutil::Expression> args;
	for (auto const& m: m_members)
		if (m.first == _member)
			args.emplace_back(_value);
		else
			args.emplace_back(member(m.first));
	m_tuple->increaseIndex();
	auto tuple = m_tuple->currentValue();
	auto sortExpr = smtutil::Expression(make_shared<smtutil::SortSort>(tuple.sort), tuple.name);
	m_context.addAssertion(tuple == smtutil::Expression::tuple_constructor(sortExpr, args));
	return m_tuple->currentValue();
}

void SymbolicState::reset()
{
	m_error.resetIndex();
	m_thisAddress.resetIndex();
	m_state.reset();
	m_tx.reset();
	m_crypto.reset();
}

smtutil::Expression SymbolicState::balances() const
{
	return m_state.member("balances");
}

smtutil::Expression SymbolicState::balance() const
{
	return balance(thisAddress());
}

smtutil::Expression SymbolicState::balance(smtutil::Expression _address) const
{
	return smtutil::Expression::select(balances(), move(_address));
}

smtutil::Expression SymbolicState::blockhash(smtutil::Expression _blockNumber) const
{
	return smtutil::Expression::select(m_tx.member("blockhash"), move(_blockNumber));
}

void SymbolicState::transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value)
{
	unsigned indexBefore = m_state.index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_state.index();
	solAssert(indexAfter > indexBefore, "");
	m_state.newVar();
	/// Do not apply the transfer operation if _from == _to.
	auto newState = smtutil::Expression::ite(
		move(_from) == move(_to),
		m_state.value(indexBefore),
		m_state.value(indexAfter)
	);
	m_context.addAssertion(m_state.value() == newState);
}

smtutil::Expression SymbolicState::txMember(string const& _member) const
{
	return m_tx.member(_member);
}

smtutil::Expression SymbolicState::txConstraints(FunctionDefinition const& _function) const
{
	smtutil::Expression conj = smt::symbolicUnknownConstraints(m_tx.member("block.coinbase"), TypeProvider::uint(160)) &&
		smt::symbolicUnknownConstraints(m_tx.member("msg.sender"), TypeProvider::uint(160)) &&
		smt::symbolicUnknownConstraints(m_tx.member("tx.origin"), TypeProvider::uint(160));

	if (_function.isPartOfExternalInterface())
	{
		auto sig = TypeProvider::function(_function)->externalIdentifier();
		conj = conj && m_tx.member("msg.sig") == sig;

		auto b0 = sig >> (3 * 8);
		auto b1 = (sig & 0x00ff0000) >> (2 * 8);
		auto b2 = (sig & 0x0000ff00) >> (1 * 8);
		auto b3 = (sig & 0x000000ff);
		auto data = smtutil::Expression::tuple_get(m_tx.member("msg.data"), 0);
		conj = conj && smtutil::Expression::select(data, 0) == b0;
		conj = conj && smtutil::Expression::select(data, 1) == b1;
		conj = conj && smtutil::Expression::select(data, 2) == b2;
		conj = conj && smtutil::Expression::select(data, 3) == b3;
		auto length = smtutil::Expression::tuple_get(m_tx.member("msg.data"), 1);
		// TODO add ABI size of function input parameters here \/
		conj = conj && length >= 4;
	}

	return conj;
}

/// Private helpers.

void SymbolicState::addBalance(smtutil::Expression _address, smtutil::Expression _value)
{
	auto newBalances = smtutil::Expression::store(
		balances(),
		_address,
		balance(_address) + move(_value)
	);
	m_state.assignMember("balances", newBalances);
}
