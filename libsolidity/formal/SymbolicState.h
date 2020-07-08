// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/formal/SymbolicVariables.h>

#include <libsmtutil/Sorts.h>
#include <libsmtutil/SolverInterface.h>

namespace solidity::frontend::smt
{

class EncodingContext;

/**
 * Symbolic representation of the blockchain state.
 */
class SymbolicState
{
public:
	SymbolicState(EncodingContext& _context);

	void reset();

	/// Blockchain.
	//@{
	/// Value of `this` address.
	smtutil::Expression thisAddress();
	/// @returns the symbolic balance of address `this`.
	smtutil::Expression balance();
	/// @returns the symbolic balance of an address.
	smtutil::Expression balance(smtutil::Expression _address);
	/// Transfer _value from _from to _to.
	void transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value);
	//@}

private:
	/// Adds _value to _account's balance.
	void addBalance(smtutil::Expression _account, smtutil::Expression _value);

	EncodingContext& m_context;

	/// Symbolic `this` address.
	SymbolicAddressVariable m_thisAddress{
		"this",
		m_context
	};

	/// Symbolic balances.
	SymbolicArrayVariable m_balances{
		std::make_shared<smtutil::ArraySort>(smtutil::SortProvider::uintSort, smtutil::SortProvider::uintSort),
		"balances",
		m_context
	};
};

}
