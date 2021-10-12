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

#include <libsolidity/formal/SymbolicTypes.h>
#include <libsolidity/formal/SymbolicVariables.h>

#include <libsmtutil/Sorts.h>
#include <libsmtutil/SolverInterface.h>

namespace solidity::frontend::smt
{

class EncodingContext;
class SymbolicAddressVariable;
class SymbolicArrayVariable;

class BlockchainVariable
{
public:
	BlockchainVariable(std::string _name, std::map<std::string, smtutil::SortPointer> _members, EncodingContext& _context);
	/// @returns the variable data as a tuple.
	smtutil::Expression value() const { return m_tuple->currentValue(); }
	smtutil::Expression value(unsigned _idx) const { return m_tuple->valueAtIndex(_idx); }
	smtutil::SortPointer const& sort() const { return m_tuple->sort(); }
	unsigned index() const { return m_tuple->index(); }
	void newVar() { m_tuple->increaseIndex(); }
	void reset() { m_tuple->resetIndex(); }

	/// @returns the symbolic _member.
	smtutil::Expression member(std::string const& _member) const;
	/// Generates a new tuple where _member is assigned _value.
	smtutil::Expression assignMember(std::string const& _member, smtutil::Expression const& _value);

private:
	std::string const m_name;
	std::map<std::string, smtutil::SortPointer> const m_members;
	EncodingContext& m_context;
	std::map<std::string, unsigned> m_componentIndices;
	std::unique_ptr<SymbolicTupleVariable> m_tuple;
};

/**
 * Symbolic representation of the blockchain context:
 * - error flag
 * - this (the address of the currently executing contract)
 * - state, represented as a tuple of:
 *   - balances
 *   - array of address => bool representing whether an address is used by a contract
 *   - storage of contracts
 * - block and transaction properties, represented as a tuple of:
 *   - blockhash
 *   - block basefee
 *   - block chainid
 *   - block coinbase
 *   - block difficulty
 *   - block gaslimit
 *   - block number
 *   - block timestamp
 *   - TODO gasleft
 *   - msg data
 *   - msg sender
 *   - msg sig
 *   - msg value
 *   - tx gasprice
 *   - tx origin
 */
class SymbolicState
{
public:
	SymbolicState(EncodingContext& _context): m_context(_context) {}

	void reset();

	/// Error flag.
	//@{
	SymbolicIntVariable& errorFlag() { return m_error; }
	SymbolicIntVariable const& errorFlag() const { return m_error; }
	smtutil::SortPointer const& errorFlagSort() const { return m_error.sort(); }
	//@}

	/// This.
	//@{
	/// @returns the symbolic value of the currently executing contract's address.
	smtutil::Expression thisAddress() const { return m_thisAddress.currentValue(); }
	smtutil::Expression thisAddress(unsigned _idx) const { return m_thisAddress.valueAtIndex(_idx); }
	smtutil::Expression newThisAddress() { return m_thisAddress.increaseIndex(); }
	smtutil::SortPointer const& thisAddressSort() const { return m_thisAddress.sort(); }
	//@}

	/// Blockchain state.
	//@{
	smtutil::Expression state() const { solAssert(m_state, ""); return m_state->value(); }
	smtutil::Expression state(unsigned _idx) const { solAssert(m_state, ""); return m_state->value(_idx); }
	smtutil::SortPointer const& stateSort() const { solAssert(m_state, ""); return m_state->sort(); }
	void newState() { solAssert(m_state, ""); m_state->newVar(); }

	void newBalances();

	/// Balance.
	/// @returns the symbolic balances.
	smtutil::Expression balances() const;
	/// @returns the symbolic balance of address `this`.
	smtutil::Expression balance() const;
	/// @returns the symbolic balance of an address.
	smtutil::Expression balance(smtutil::Expression _address) const;
	/// Transfer _value from _from to _to.
	void transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value);

	/// Adds _value to _account's balance.
	void addBalance(smtutil::Expression _account, smtutil::Expression _value);

	/// Storage.
	smtutil::Expression storage(ContractDefinition const& _contract) const;
	smtutil::Expression storage(ContractDefinition const& _contract, smtutil::Expression _address) const;
	smtutil::Expression addressActive(smtutil::Expression _address) const;
	void setAddressActive(smtutil::Expression _address, bool _active);

	void writeStateVars(ContractDefinition const& _contract, smtutil::Expression _address);
	void readStateVars(ContractDefinition const& _contract, smtutil::Expression _address);
	//@}

	/// Transaction data.
	//@{
	/// @returns the tx data as a tuple.
	smtutil::Expression tx() const { return m_tx.value(); }
	smtutil::Expression tx(unsigned _idx) const { return m_tx.value(_idx); }
	smtutil::SortPointer const& txSort() const { return m_tx.sort(); }
	void newTx() { m_tx.newVar(); }
	smtutil::Expression txMember(std::string const& _member) const;
	smtutil::Expression txFunctionConstraints(FunctionDefinition const& _function) const;
	smtutil::Expression txTypeConstraints() const;
	smtutil::Expression txNonPayableConstraint() const;
	smtutil::Expression blockhash(smtutil::Expression _blockNumber) const;
	//@}

	/// Crypto functions.
	//@{
	/// @returns the crypto functions represented as a tuple of arrays.
	smtutil::Expression crypto() const { return m_crypto.value(); }
	smtutil::Expression crypto(unsigned _idx) const { return m_crypto.value(_idx); }
	smtutil::SortPointer const& cryptoSort() const { return m_crypto.sort(); }
	void newCrypto() { m_crypto.newVar(); }
	smtutil::Expression cryptoFunction(std::string const& _member) const { return m_crypto.member(_member); }
	//@}

	/// Calls the internal methods that build
	/// - the symbolic ABI functions based on the abi.* calls
	///   in _source and referenced sources.
	/// - the symbolic storages for all contracts in _source and
	///   referenced sources.
	void prepareForSourceUnit(SourceUnit const& _source, bool _storage);

	/// ABI functions.
	//@{
	smtutil::Expression abiFunction(FunctionCall const* _funCall);
	using SymbolicABIFunction = std::tuple<
		std::string,
		std::vector<frontend::Type const*>,
		std::vector<frontend::Type const*>
	>;
	SymbolicABIFunction const& abiFunctionTypes(FunctionCall const* _funCall) const;

	smtutil::Expression abi() const { solAssert(m_abi, ""); return m_abi->value(); }
	smtutil::Expression abi(unsigned _idx) const { solAssert(m_abi, ""); return m_abi->value(_idx); }
	smtutil::SortPointer const& abiSort() const { solAssert(m_abi, ""); return m_abi->sort(); }
	void newABI() { solAssert(m_abi, ""); m_abi->newVar(); }
	//@}

private:
	std::string contractSuffix(ContractDefinition const& _contract) const;
	std::string contractStorageKey(ContractDefinition const& _contract) const;
	std::string stateVarStorageKey(VariableDeclaration const& _var, ContractDefinition const& _contract) const;

	/// Builds state.storage based on _contracts.
	void buildState(std::set<ContractDefinition const*, ASTCompareByID<ContractDefinition>> const& _contracts, bool _allStorages);

	/// Builds m_abi based on the abi.* calls _abiFunctions.
	void buildABIFunctions(std::set<FunctionCall const*, ASTCompareByID<FunctionCall>> const& _abiFunctions);

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

	/// m_state is a tuple of
	/// - balances: array of address to balance of address.
	/// - isActive: array of address to Boolean, where element is true iff address is used.
	/// - storage: tuple containing the storage of every contract, where
	/// each element of the tuple represents a contract,
	/// and is defined by an array where the index is the contract's address
	/// and the element is a tuple containing the state variables of that contract.
	std::unique_ptr<BlockchainVariable> m_state;

	BlockchainVariable m_tx{
		"tx",
		{
			{"block.basefee", smtutil::SortProvider::uintSort},
			{"block.chainid", smtutil::SortProvider::uintSort},
			{"block.coinbase", smt::smtSort(*TypeProvider::address())},
			{"block.difficulty", smtutil::SortProvider::uintSort},
			{"block.gaslimit", smtutil::SortProvider::uintSort},
			{"block.number", smtutil::SortProvider::uintSort},
			{"block.timestamp", smtutil::SortProvider::uintSort},
			{"blockhash", std::make_shared<smtutil::ArraySort>(smtutil::SortProvider::uintSort, smtutil::SortProvider::uintSort)},
			// TODO gasleft
			{"msg.data", smt::smtSort(*TypeProvider::bytesMemory())},
			{"msg.sender", smt::smtSort(*TypeProvider::address())},
			{"msg.sig", smtutil::SortProvider::uintSort},
			{"msg.value", smtutil::SortProvider::uintSort},
			{"tx.gasprice", smtutil::SortProvider::uintSort},
			{"tx.origin", smt::smtSort(*TypeProvider::address())}
		},
		m_context
	};

	BlockchainVariable m_crypto{
		"crypto",
		{
			{"keccak256", std::make_shared<smtutil::ArraySort>(
				smt::smtSort(*TypeProvider::bytesStorage()),
				smtSort(*TypeProvider::fixedBytes(32))
			)},
			{"sha256", std::make_shared<smtutil::ArraySort>(
				smt::smtSort(*TypeProvider::bytesStorage()),
				smtSort(*TypeProvider::fixedBytes(32))
			)},
			{"ripemd160", std::make_shared<smtutil::ArraySort>(
				smt::smtSort(*TypeProvider::bytesStorage()),
				smtSort(*TypeProvider::fixedBytes(20))
			)},
			{"ecrecover", std::make_shared<smtutil::ArraySort>(
				std::make_shared<smtutil::TupleSort>(
					"ecrecover_input_type",
					std::vector<std::string>{"hash", "v", "r", "s"},
					std::vector<smtutil::SortPointer>{
						smt::smtSort(*TypeProvider::fixedBytes(32)),
						smt::smtSort(*TypeProvider::uint(8)),
						smt::smtSort(*TypeProvider::fixedBytes(32)),
						smt::smtSort(*TypeProvider::fixedBytes(32))
					}
				),
				smtSort(*TypeProvider::address())
			)}
		},
		m_context
	};

	/// Tuple containing all used ABI functions.
	std::unique_ptr<BlockchainVariable> m_abi;
	/// Maps ABI functions calls to their tuple names generated by
	/// `buildABIFunctions`.
	std::map<FunctionCall const*, SymbolicABIFunction> m_abiMembers;
};

}
