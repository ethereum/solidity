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
