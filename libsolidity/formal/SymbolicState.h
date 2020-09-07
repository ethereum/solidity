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
class SymbolicAddressVariable;
class SymbolicArrayVariable;

/**
 * Symbolic representation of the blockchain context:
 * - error flag
 * - this (the address of the currently executing contract)
 * - state, represented as a tuple of:
 *   - balances
 *   - TODO: potentially storage of contracts
 * - TODO transaction variables
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
	/// @returns the symbolic balances.
	smtutil::Expression balances();
	/// @returns the symbolic balance of address `this`.
	smtutil::Expression balance();
	/// @returns the symbolic balance of an address.
	smtutil::Expression balance(smtutil::Expression _address);

	SymbolicIntVariable& errorFlag();

	/// @returns the state as a tuple.
	smtutil::Expression state();

	/// @returns the state sort.
	smtutil::SortPointer stateSort();

	/// Transfer _value from _from to _to.
	void transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value);
	//@}

private:
	/// Adds _value to _account's balance.
	void addBalance(smtutil::Expression _account, smtutil::Expression _value);

	/// Generates a new tuple where _member is assigned _value.
	smtutil::Expression assignStateMember(std::string const& _member, smtutil::Expression const& _value);

	EncodingContext& m_context;

	SymbolicIntVariable m_error{
		TypeProvider::uint256(),
		TypeProvider::uint256(),
		"error",
		m_context
	};

	SymbolicAddressVariable m_thisAddress{
		"this",
		m_context
	};

	std::map<std::string, unsigned> m_componentIndices;
	/// balances, TODO storage of other contracts
	std::map<std::string, smtutil::SortPointer> m_stateMembers;
	std::unique_ptr<SymbolicTupleVariable> m_stateTuple;
};

}
