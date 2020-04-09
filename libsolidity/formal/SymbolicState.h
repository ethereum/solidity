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

#pragma once

#include <libsolidity/formal/Sorts.h>
#include <libsolidity/formal/SolverInterface.h>
#include <libsolidity/formal/SymbolicVariables.h>

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
	Expression thisAddress();
	/// @returns the symbolic balance of address `this`.
	Expression balance();
	/// @returns the symbolic balance of an address.
	Expression balance(Expression _address);
	/// Transfer _value from _from to _to.
	void transfer(Expression _from, Expression _to, Expression _value);
	//@}

private:
	/// Adds _value to _account's balance.
	void addBalance(Expression _account, Expression _value);

	EncodingContext& m_context;

	/// Symbolic `this` address.
	SymbolicAddressVariable m_thisAddress{
		"this",
		m_context
	};

	/// Symbolic balances.
	SymbolicArrayVariable m_balances{
		std::make_shared<ArraySort>(SortProvider::intSort, SortProvider::intSort),
		"balances",
		m_context
	};
};

}
