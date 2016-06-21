/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Account.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/SHA3.h>
#include <libethcore/Common.h>

namespace dev
{
namespace eth
{

/**
 * Models the state of a single Ethereum account.
 * Used to cache a portion of the full Ethereum state. State keeps a mapping of Address's to Accounts.
 *
 * Aside from storing the nonce and balance, the account may also be "dead" (where isAlive() returns false).
 * This allows State to explicitly store the notion of a deleted account in it's cache. kill() can be used
 * for this.
 *
 * For the account's storage, the class operates a cache. baseRoot() specifies the base state of the storage
 * given as the Trie root to be looked up in the state database. Alterations beyond this base are specified
 * in the overlay, stored in this class and retrieved with storageOverlay(). setStorage allows the overlay
 * to be altered.
 *
 * The code handling explicitly supports a two-stage commit model needed for contract-creation. When creating
 * a contract (running the initialisation code), the code of the account is considered empty. The attribute
 * of emptiness can be retrieved with codeBearing(). After initialisation one must set the code accordingly;
 * the code of the Account can be set with setCode(). To validate a setCode() call, this class records the
 * state of being in contract-creation (and thus in a state where setCode may validly be called). It can be
 * determined through isFreshCode().
 *
 * The code can be retrieved through code(), and its hash through codeHash(). codeHash() is only valid when
 * the account is not in the contract-creation phase (i.e. when isFreshCode() returns false). This class
 * supports populating code on-demand from the state database. To determine if the code has been prepopulated
 * call codeCacheValid(). To populate the code, look it up with codeHash() and populate with noteCode().
 *
 * @todo: need to make a noteCodeCommitted().
 *
 * The constructor allows you to create an one of a number of "types" of accounts. The default constructor
 * makes a dead account (this is ignored by State when writing out the Trie). Another three allow a basic
 * or contract account to be specified along with an initial balance. The fina two allow either a basic or
 * a contract account to be created with arbitrary values.
 */
class Account
{
public:
	/// Type of account to create.
	enum NewAccountType
	{
		/// Normal account.
		NormalCreation,
		/// Contract account - we place this object into the contract-creation state (and as such we
		/// expect setCode(), but codeHash() won't work).
		ContractConception
	};

	/// Changedness of account to create.
	enum Changedness
	{
		/// Account starts as though it has been changed.
		Changed,
		/// Account starts as though it has not been changed.
		Unchanged
	};

	/// Construct a dead Account.
	Account() {}

	/// Construct an alive Account, with given endowment, for either a normal (non-contract) account or for a
	/// contract account in the
	/// conception phase, where the code is not yet known.
	Account(u256 _nonce, u256 _balance, NewAccountType _t, Changedness _c = Changed): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance), m_codeHash(_t == NormalCreation ? EmptySHA3 : c_contractConceptionCodeHash) {}
	/// Explicit constructor for wierd cases of construction of a normal account.
	Account(u256 _nonce, u256 _balance, Changedness _c = Changed): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance) {}

	/// Explicit constructor for wierd cases of construction or a contract account.
	Account(u256 _nonce, u256 _balance, h256 _contractRoot, h256 _codeHash, Changedness _c): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance), m_storageRoot(_contractRoot), m_codeHash(_codeHash) { assert(_contractRoot); }


	/// Kill this account. Useful for the suicide opcode. Following this call, isAlive() returns false.
	void kill() { m_isAlive = false; m_storageOverlay.clear(); m_codeHash = EmptySHA3; m_storageRoot = EmptyTrie; m_balance = 0; m_nonce = 0; changed(); }

	/// @returns true iff this object represents an account in the state. Returns false if this object
	/// represents an account that should no longer exist in the trie (an account that never existed or was
	/// suicided).
	bool isAlive() const { return m_isAlive; }

	/// @returns true if the account is unchanged from creation.
	bool isDirty() const { return !m_isUnchanged; }


	/// @returns the balance of this account. Can be altered in place.
	u256& balance() { return m_balance; }

	/// @returns the balance of this account.
	u256 const& balance() const { return m_balance; }

	/// Increments the balance of this account by the given amount. It's a bigint, so can be negative.
	void addBalance(bigint _i) { if (!_i) return; m_balance = (u256)((bigint)m_balance + _i); changed(); }

	/// @returns the nonce of the account. Can be altered in place.
	u256& nonce() { return m_nonce; }

	/// @returns the nonce of the account.
	u256 const& nonce() const { return m_nonce; }

	/// Increment the nonce of the account by one.
	void incNonce() { m_nonce++; changed(); }


	/// @returns the root of the trie (whose nodes are stored in the state db externally to this class)
	/// which encodes the base-state of the account's storage (upon which the storage is overlaid).
	h256 baseRoot() const { assert(m_storageRoot); return m_storageRoot; }

	/// @returns the storage overlay as a simple hash map.
	std::unordered_map<u256, u256> const& storageOverlay() const { return m_storageOverlay; }

	/// Set a key/value pair in the account's storage. This actually goes into the overlay, for committing
	/// to the trie later.
	void setStorage(u256 _p, u256 _v) { m_storageOverlay[_p] = _v; changed(); }

	/// @returns true if we are in the contract-conception state and setCode is valid to call.
	bool isFreshCode() const { return m_codeHash == c_contractConceptionCodeHash; }

	/// @returns true if we are either in the contract-conception state or if the account's code is not
	/// empty.
	bool codeBearing() const { return m_codeHash != EmptySHA3; }

	/// @returns the hash of the account's code. Must only be called when isFreshCode() returns false.
	h256 codeHash() const { assert(!isFreshCode()); return m_codeHash; }

	/// Sets the code of the account. Must only be called when isFreshCode() returns true.
	void setCode(bytes&& _code) { assert(isFreshCode()); m_codeCache = std::move(_code); changed(); }

	/// @returns true if the account's code is available through code().
	bool codeCacheValid() const { return m_codeHash == EmptySHA3 || m_codeHash == c_contractConceptionCodeHash || m_codeCache.size(); }

	/// Specify to the object what the actual code is for the account. @a _code must have a SHA3 equal to
	/// codeHash() and must only be called when isFreshCode() returns false.
	void noteCode(bytesConstRef _code) { assert(sha3(_code) == m_codeHash); m_codeCache = _code.toBytes(); }

	/// @returns the account's code. Must only be called when codeCacheValid returns true.
	bytes const& code() const { assert(codeCacheValid()); return m_codeCache; }

private:
	/// Note that we've altered the account.
	void changed() { m_isUnchanged = false; }

	/// Is this account existant? If not, it represents a deleted account.
	bool m_isAlive = false;

	/// True if we've not made any alteration to the account having been given it's properties directly.
	bool m_isUnchanged = false;

	/// Account's nonce.
	u256 m_nonce;

	/// Account's balance.
	u256 m_balance = 0;

	/// The base storage root. Used with the state DB to give a base to the storage. m_storageOverlay is
	/// overlaid on this and takes precedence for all values set.
	h256 m_storageRoot = EmptyTrie;

	/** If c_contractConceptionCodeHash then we're in the limbo where we're running the initialisation code.
	 * We expect a setCode() at some point later.
	 * If EmptySHA3, then m_code, which should be empty, is valid.
	 * If anything else, then m_code is valid iff it's not empty, otherwise, State::ensureCached() needs to
	 * be called with the correct args.
	 */
	h256 m_codeHash = EmptySHA3;

	/// The map with is overlaid onto whatever storage is implied by the m_storageRoot in the trie.
	std::unordered_map<u256, u256> m_storageOverlay;

	/// The associated code for this account. The SHA3 of this should be equal to m_codeHash unless m_codeHash
	/// equals c_contractConceptionCodeHash.
	bytes m_codeCache;

	/// Value for m_codeHash when this account is having its code determined.
	static const h256 c_contractConceptionCodeHash;
};

class AccountMask
{
public:
	AccountMask(bool _all = false):
		m_hasBalance(_all),
		m_hasNonce(_all),
		m_hasCode(_all),
		m_hasStorage(_all)
	{}

	AccountMask(
		bool _hasBalance,
		bool _hasNonce,
		bool _hasCode,
		bool _hasStorage,
		bool _shouldNotExist = false
	):
		m_hasBalance(_hasBalance),
		m_hasNonce(_hasNonce),
		m_hasCode(_hasCode),
		m_hasStorage(_hasStorage),
		m_shouldNotExist(_shouldNotExist)
	{}

	bool allSet() const { return m_hasBalance && m_hasNonce && m_hasCode && m_hasStorage; }
	bool hasBalance() const { return m_hasBalance; }
	bool hasNonce() const { return m_hasNonce; }
	bool hasCode() const { return m_hasCode; }
	bool hasStorage() const { return m_hasStorage; }
	bool shouldExist() const { return !m_shouldNotExist; }

private:
	bool m_hasBalance;
	bool m_hasNonce;
	bool m_hasCode;
	bool m_hasStorage;
	bool m_shouldNotExist = false;
};

using AccountMap = std::unordered_map<Address, Account>;
using AccountMaskMap = std::unordered_map<Address, AccountMask>;

class PrecompiledContract;
using PrecompiledContractMap = std::unordered_map<Address, PrecompiledContract>;

AccountMap jsonToAccountMap(std::string const& _json, u256 const& _defaultNonce = 0, AccountMaskMap* o_mask = nullptr, PrecompiledContractMap* o_precompiled = nullptr);

}
}
