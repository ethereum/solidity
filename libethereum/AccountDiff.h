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
/** @file AccountDiff.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/Diff.h>
#include <libethcore/Common.h>

namespace dev
{
namespace eth
{

/// Type of change that an account can have from state to state.
enum class AccountChange
{
	None,			///< Nothing changed at all.
	Creation,		///< Account came into existance.
	Deletion,		///< Account was deleted.
	Intrinsic,		///< Account was already in existance and some internal aspect of the account altered such as balance, nonce or storage.
	CodeStorage,	///< Account was already in existance and the code of the account changed.
	All				///< Account was already in existance and all aspects of the account changed.
};

/// @returns a three-character code that expresses the type of change.
char const* lead(AccountChange _c);

/**
 * @brief Stores the difference between two accounts (typically the same account at two times).
 *
 * In order to determine what about an account has altered, this struct can be used to specify
 * alterations. Use changed() and changeType() to determine what, if anything, is different.
 *
 * Five members are accessible: to determine the nature of the changes.
 */
struct AccountDiff
{
	/// @returns true if the account has changed at all.
	inline bool changed() const { return storage.size() || code || nonce || balance || exist; }
	/// @returns a three-character code that expresses the change.
	AccountChange changeType() const;

	Diff<bool> exist;						///< The account's existance; was it created/deleted or not?
	Diff<u256> balance;						///< The account's balance; did it alter?
	Diff<u256> nonce;						///< The account's nonce; did it alter?
	std::map<u256, Diff<u256>> storage;		///< The account's storage addresses; each has its own Diff.
	Diff<bytes> code;						///< The account's code; in general this should only have changed if exist also changed.
};

/**
 * @brief Stores the difference between two states; this is just their encumbent accounts.
 */
struct StateDiff
{
	std::map<Address, AccountDiff> accounts;	///< The state's account changes; each has its own AccountDiff.
};

}

/// Simple stream output for the StateDiff.
std::ostream& operator<<(std::ostream& _out, dev::eth::StateDiff const& _s);
/// Simple stream output for the AccountDiff.
std::ostream& operator<<(std::ostream& _out, dev::eth::AccountDiff const& _s);

}

