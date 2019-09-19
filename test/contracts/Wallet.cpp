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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Tests for a (comparatively) complex multisig wallet contract.
 */

#include <string>
#include <tuple>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4535) // calling _set_se_translator requires /EHa
#endif
#include <boost/test/unit_test.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <test/libsolidity/SolidityExecutionFramework.h>

using namespace std;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

static char const* walletCode = R"DELIMITER(
//sol Wallet
// Multi-sig, daily-limited account proxy/wallet.
// @authors:
// Gav Wood <g@ethdev.com>
// inheritable "property" contract that enables methods to be protected by requiring the acquiescence of either a
// single, or, crucially, each of a number of, designated owners.
// usage:
// use modifiers onlyowner (just own owned) or onlymanyowners(hash), whereby the same hash must be provided by
// some number (specified in constructor) of the set of owners (specified in the constructor, modifiable) before the
// interior is executed.

pragma solidity >=0.4.0 <0.7.0;

contract multiowned {

	// TYPES

	// struct for the status of a pending operation.
	struct PendingState {
		uint yetNeeded;
		uint ownersDone;
		uint index;
	}

	// EVENTS

	// this contract only has five types of events: it can accept a confirmation, in which case
	// we record owner and operation (hash) alongside it.
	event Confirmation(address owner, bytes32 operation);
	event Revoke(address owner, bytes32 operation);
	// some others are in the case of an owner changing.
	event OwnerChanged(address oldOwner, address newOwner);
	event OwnerAdded(address newOwner);
	event OwnerRemoved(address oldOwner);
	// the last one is emitted if the required signatures change
	event RequirementChanged(uint newRequirement);

	// MODIFIERS

	// simple single-sig function modifier.
	modifier onlyowner {
		if (isOwner(msg.sender))
			_;
	}
	// multi-sig function modifier: the operation must have an intrinsic hash in order
	// that later attempts can be realised as the same underlying operation and
	// thus count as confirmations.
	modifier onlymanyowners(bytes32 _operation) {
		if (confirmAndCheck(_operation))
			_;
	}

	// METHODS

	// constructor is given number of sigs required to do protected "onlymanyowners" transactions
	// as well as the selection of addresses capable of confirming them.
	constructor(address[] memory _owners, uint _required) public {
		m_numOwners = _owners.length + 1;
		m_owners[1] = uint(msg.sender);
		m_ownerIndex[uint(msg.sender)] = 1;
		for (uint i = 0; i < _owners.length; ++i)
		{
			m_owners[2 + i] = uint(_owners[i]);
			m_ownerIndex[uint(_owners[i])] = 2 + i;
		}
		m_required = _required;
	}

	// Revokes a prior confirmation of the given operation
	function revoke(bytes32 _operation) external {
		uint ownerIndex = m_ownerIndex[uint(msg.sender)];
		// make sure they're an owner
		if (ownerIndex == 0) return;
		uint ownerIndexBit = 2**ownerIndex;
		PendingState storage pending = m_pending[_operation];
		if (pending.ownersDone & ownerIndexBit > 0) {
			pending.yetNeeded++;
			pending.ownersDone -= ownerIndexBit;
			emit Revoke(msg.sender, _operation);
		}
	}

	// Replaces an owner `_from` with another `_to`.
	function changeOwner(address _from, address _to) onlymanyowners(keccak256(msg.data)) public {
		if (isOwner(_to)) return;
		uint ownerIndex = m_ownerIndex[uint(_from)];
		if (ownerIndex == 0) return;

		clearPending();
		m_owners[ownerIndex] = uint(_to);
		m_ownerIndex[uint(_from)] = 0;
		m_ownerIndex[uint(_to)] = ownerIndex;
		emit OwnerChanged(_from, _to);
	}

	function addOwner(address _owner) onlymanyowners(keccak256(msg.data)) external {
		if (isOwner(_owner)) return;

		clearPending();
		if (m_numOwners >= c_maxOwners)
			reorganizeOwners();
		if (m_numOwners >= c_maxOwners)
			return;
		m_numOwners++;
		m_owners[m_numOwners] = uint(_owner);
		m_ownerIndex[uint(_owner)] = m_numOwners;
		emit OwnerAdded(_owner);
	}

	function removeOwner(address _owner) onlymanyowners(keccak256(msg.data)) external {
		uint ownerIndex = m_ownerIndex[uint(_owner)];
		if (ownerIndex == 0) return;
		if (m_required > m_numOwners - 1) return;

		m_owners[ownerIndex] = 0;
		m_ownerIndex[uint(_owner)] = 0;
		clearPending();
		reorganizeOwners(); //make sure m_numOwner is equal to the number of owners and always points to the optimal free slot
		emit OwnerRemoved(_owner);
	}

	function changeRequirement(uint _newRequired) onlymanyowners(keccak256(msg.data)) external {
		if (_newRequired > m_numOwners) return;
		m_required = _newRequired;
		clearPending();
		emit RequirementChanged(_newRequired);
	}

	function isOwner(address _addr) public returns (bool) {
		return m_ownerIndex[uint(_addr)] > 0;
	}

	function hasConfirmed(bytes32 _operation, address _owner) public view returns (bool) {
		PendingState storage pending = m_pending[_operation];
		uint ownerIndex = m_ownerIndex[uint(_owner)];

		// make sure they're an owner
		if (ownerIndex == 0) return false;

		// determine the bit to set for this owner.
		uint ownerIndexBit = 2**ownerIndex;
		if (pending.ownersDone & ownerIndexBit == 0) {
			return false;
		} else {
			return true;
		}
	}

	// INTERNAL METHODS

	function confirmAndCheck(bytes32 _operation) internal returns (bool) {
		// determine what index the present sender is:
		uint ownerIndex = m_ownerIndex[uint(msg.sender)];
		// make sure they're an owner
		if (ownerIndex == 0) return false;

		PendingState storage pending = m_pending[_operation];
		// if we're not yet working on this operation, switch over and reset the confirmation status.
		if (pending.yetNeeded == 0) {
			// reset count of confirmations needed.
			pending.yetNeeded = m_required;
			// reset which owners have confirmed (none) - set our bitmap to 0.
			pending.ownersDone = 0;
			pending.index = m_pendingIndex.length++;
			m_pendingIndex[pending.index] = _operation;
		}
		// determine the bit to set for this owner.
		uint ownerIndexBit = 2**ownerIndex;
		// make sure we (the message sender) haven't confirmed this operation previously.
		if (pending.ownersDone & ownerIndexBit == 0) {
			emit Confirmation(msg.sender, _operation);
			// ok - check if count is enough to go ahead.
			if (pending.yetNeeded <= 1) {
				// enough confirmations: reset and run interior.
				delete m_pendingIndex[m_pending[_operation].index];
				delete m_pending[_operation];
				return true;
			}
			else
			{
				// not enough: record that this owner in particular confirmed.
				pending.yetNeeded--;
				pending.ownersDone |= ownerIndexBit;
				return false;
			}
		}
	}

	function reorganizeOwners() private returns (bool) {
		uint free = 1;
		while (free < m_numOwners)
		{
			while (free < m_numOwners && m_owners[free] != 0) free++;
			while (m_numOwners > 1 && m_owners[m_numOwners] == 0) m_numOwners--;
			if (free < m_numOwners && m_owners[m_numOwners] != 0 && m_owners[free] == 0)
			{
				m_owners[free] = m_owners[m_numOwners];
				m_ownerIndex[m_owners[free]] = free;
				m_owners[m_numOwners] = 0;
			}
		}
	}

	function clearPending() internal {
		uint length = m_pendingIndex.length;
		for (uint i = 0; i < length; ++i)
			if (m_pendingIndex[i] != 0)
				delete m_pending[m_pendingIndex[i]];
		delete m_pendingIndex;
	}

	// FIELDS

	// the number of owners that must confirm the same operation before it is run.
	uint public m_required;
	// pointer used to find a free slot in m_owners
	uint public m_numOwners;

	// list of owners
	uint[256] m_owners;
	uint constant c_maxOwners = 250;
	// index on the list of owners to allow reverse lookup
	mapping(uint => uint) m_ownerIndex;
	// the ongoing operations.
	mapping(bytes32 => PendingState) m_pending;
	bytes32[] m_pendingIndex;
}

// inheritable "property" contract that enables methods to be protected by placing a linear limit (specifiable)
// on a particular resource per calendar day. is multiowned to allow the limit to be altered. resource that method
// uses is specified in the modifier.
abstract contract daylimit is multiowned {

	// MODIFIERS

	// simple modifier for daily limit.
	modifier limitedDaily(uint _value) {
		if (underLimit(_value))
			_;
	}

	// METHODS

	// constructor - stores initial daily limit and records the present day's index.
	constructor(uint _limit) public {
		m_dailyLimit = _limit;
		m_lastDay = today();
	}
	// (re)sets the daily limit. needs many of the owners to confirm. doesn't alter the amount already spent today.
	function setDailyLimit(uint _newLimit) onlymanyowners(keccak256(msg.data)) external {
		m_dailyLimit = _newLimit;
	}
	// (re)sets the daily limit. needs many of the owners to confirm. doesn't alter the amount already spent today.
	function resetSpentToday() onlymanyowners(keccak256(msg.data)) external {
		m_spentToday = 0;
	}

	// INTERNAL METHODS

	// checks to see if there is at least `_value` left from the daily limit today. if there is, subtracts it and
	// returns true. otherwise just returns false.
	function underLimit(uint _value) internal onlyowner returns (bool) {
		// reset the spend limit if we're on a different day to last time.
		if (today() > m_lastDay) {
			m_spentToday = 0;
			m_lastDay = today();
		}
		// check to see if there's enough left - if so, subtract and return true.
		if (m_spentToday + _value >= m_spentToday && m_spentToday + _value <= m_dailyLimit) {
			m_spentToday += _value;
			return true;
		}
		return false;
	}
	// determines today's index.
	function today() private view returns (uint) { return now / 1 days; }

	// FIELDS

	uint public m_dailyLimit;
	uint m_spentToday;
	uint m_lastDay;
}

// interface contract for multisig proxy contracts; see below for docs.
abstract contract multisig {

	// EVENTS

	// logged events:
	// Funds has arrived into the wallet (record how much).
	event Deposit(address from, uint value);
	// Single transaction going out of the wallet (record who signed for it, how much, and to whom it's going).
	event SingleTransact(address owner, uint value, address to, bytes data);
	// Multi-sig transaction going out of the wallet (record who signed for it last, the operation hash, how much, and to whom it's going).
	event MultiTransact(address owner, bytes32 operation, uint value, address to, bytes data);
	// Confirmation still needed for a transaction.
	event ConfirmationNeeded(bytes32 operation, address initiator, uint value, address to, bytes data);

	// FUNCTIONS

	// TODO: document
	function changeOwner(address _from, address _to) public;
	function execute(address _to, uint _value, bytes calldata _data) external returns (bytes32);
	function confirm(bytes32 _h) public returns (bool);
}

// usage:
// bytes32 h = Wallet(w).from(oneOwner).transact(to, value, data);
// Wallet(w).from(anotherOwner).confirm(h);
contract Wallet is multisig, multiowned, daylimit {

	// TYPES

	// Transaction structure to remember details of transaction lest it need be saved for a later call.
	struct Transaction {
		address to;
		uint value;
		bytes data;
	}

	// METHODS

	// constructor - just pass on the owner array to the multiowned and
	// the limit to daylimit
	constructor(address[] memory _owners, uint _required, uint _daylimit) public payable
			multiowned(_owners, _required) daylimit(_daylimit) {
	}

	function changeOwner(address _from, address _to) public override(multiowned, multisig) {
		multiowned.changeOwner(_from, _to);
	}
	// destroys the contract sending everything to `_to`.
	function kill(address payable _to) onlymanyowners(keccak256(msg.data)) external {
		selfdestruct(_to);
	}

	// gets called when no other function matches
	function() external payable {
		// just being sent some cash?
		if (msg.value > 0)
			emit Deposit(msg.sender, msg.value);
	}

	// Outside-visible transact entry point. Executes transaction immediately if below daily spend limit.
	// If not, goes into multisig process. We provide a hash on return to allow the sender to provide
	// shortcuts for the other confirmations (allowing them to avoid replicating the _to, _value
	// and _data arguments). They still get the option of using them if they want, anyways.
	function execute(address _to, uint _value, bytes calldata _data) external override onlyowner returns (bytes32 _r) {
		// first, take the opportunity to check that we're under the daily limit.
		if (underLimit(_value)) {
			emit SingleTransact(msg.sender, _value, _to, _data);
			// yes - just execute the call.
			_to.call.value(_value)(_data);
			return 0;
		}
		// determine our operation hash.
		_r = keccak256(abi.encodePacked(msg.data, block.number));
		if (!confirm(_r) && m_txs[_r].to == 0x0000000000000000000000000000000000000000) {
			m_txs[_r].to = _to;
			m_txs[_r].value = _value;
			m_txs[_r].data = _data;
			emit ConfirmationNeeded(_r, msg.sender, _value, _to, _data);
		}
	}

	// confirm a transaction through just the hash. we use the previous transactions map, m_txs, in order
	// to determine the body of the transaction from the hash provided.
	function confirm(bytes32 _h) onlymanyowners(_h) public override returns (bool) {
		if (m_txs[_h].to != 0x0000000000000000000000000000000000000000) {
			m_txs[_h].to.call.value(m_txs[_h].value)(m_txs[_h].data);
			emit MultiTransact(msg.sender, _h, m_txs[_h].value, m_txs[_h].to, m_txs[_h].data);
			delete m_txs[_h];
			return true;
		}
	}

	// INTERNAL METHODS

	function clearPending() internal override {
		uint length = m_pendingIndex.length;
		for (uint i = 0; i < length; ++i)
			delete m_txs[m_pendingIndex[i]];
		super.clearPending();
	}

	// FIELDS

	// pending transactions we have at present.
	mapping (bytes32 => Transaction) m_txs;
}
)DELIMITER";

static unique_ptr<bytes> s_compiledWallet;

class WalletTestFramework: public SolidityExecutionFramework
{
protected:
	void deployWallet(
		u256 const& _value = 0,
		vector<u256> const& _owners = vector<u256>{},
		u256 _required = 1,
		u256 _dailyLimit = 0
	)
	{
		if (!s_compiledWallet)
			s_compiledWallet.reset(new bytes(compileContract(walletCode, "Wallet")));

		bytes args = encodeArgs(u256(0x60), _required, _dailyLimit, u256(_owners.size()), _owners);
		sendMessage(*s_compiledWallet + args, true, _value);
		BOOST_REQUIRE(m_transactionSuccessful);
		BOOST_REQUIRE(!m_output.empty());
	}
};

/// This is a test suite that tests optimised code!
BOOST_FIXTURE_TEST_SUITE(SolidityWallet, WalletTestFramework)

BOOST_AUTO_TEST_CASE(creation)
{
	deployWallet(200);
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(m_sender, h256::AlignRight)) == encodeArgs(true));
	bool v2 = dev::test::Options::get().useABIEncoderV2;
	BOOST_REQUIRE(callContractFunction("isOwner(address)", ~h256(m_sender, h256::AlignRight)) == (v2 ? encodeArgs() : encodeArgs(false)));
}

BOOST_AUTO_TEST_CASE(add_owners)
{
	deployWallet(200);
	Address originalOwner = m_sender;
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs(true));
	// now let the new owner add someone
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x13)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x13)) == encodeArgs(true));
	// and check that a non-owner cannot add a new owner
	m_sender = account(0);
	sendEther(account(2), 10 * ether);
	m_sender = account(2);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x20)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x20)) == encodeArgs(false));
	// finally check that all the owners are there
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(originalOwner, h256::AlignRight)) == encodeArgs(true));
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs(true));
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x13)) == encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(change_owners)
{
	deployWallet(200);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x12)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x12)) == encodeArgs(true));
	BOOST_REQUIRE(callContractFunction("changeOwner(address,address)", h256(0x12), h256(0x13)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x12)) == encodeArgs(false));
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x13)) == encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(remove_owner)
{
	deployWallet(200);
	// add 10 owners
	for (unsigned i = 0; i < 10; ++i)
	{
		BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x12 + i)) == encodeArgs());
		BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x12 + i)) == encodeArgs(true));
	}
	// check they are there again
	for (unsigned i = 0; i < 10; ++i)
		BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x12 + i)) == encodeArgs(true));
	// remove the odd owners
	for (unsigned i = 0; i < 10; ++i)
		if (i % 2 == 1)
			BOOST_REQUIRE(callContractFunction("removeOwner(address)", h256(0x12 + i)) == encodeArgs());
	// check the result
	for (unsigned i = 0; i < 10; ++i)
		BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x12 + i)) == encodeArgs(i % 2 == 0));
	// add them again
	for (unsigned i = 0; i < 10; ++i)
		if (i % 2 == 1)
			BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x12 + i)) == encodeArgs());
	// check everyone is there
	for (unsigned i = 0; i < 10; ++i)
		BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x12 + i)) == encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(initial_owners)
{
	vector<u256> owners{
		u256("0x00000000000000000000000042c56279432962a17176998a4747d1b4d6ed4367"),
		u256("0x000000000000000000000000d4d4669f5ba9f4c27d38ef02a358c339b5560c47"),
		u256("0x000000000000000000000000e6716f9544a56c530d868e4bfbacb172315bdead"),
		u256("0x000000000000000000000000775e18be7a50a0abb8a4e82b1bd697d79f31fe04"),
		u256("0x000000000000000000000000f4dd5c3794f1fd0cdc0327a83aa472609c806e99"),
		u256("0x0000000000000000000000004c9113886af165b2de069d6e99430647e94a9fff"),
		u256("0x0000000000000000000000003fb1cd2cd96c6d5c0b5eb3322d807b34482481d4")
	};
	deployWallet(0, owners, 4, 2);
	BOOST_CHECK(callContractFunction("m_numOwners()") == encodeArgs(u256(8)));
	BOOST_CHECK(callContractFunction("isOwner(address)", h256(m_sender, h256::AlignRight)) == encodeArgs(true));
	for (u256 const& owner: owners)
	{
		BOOST_CHECK(callContractFunction("isOwner(address)", owner) == encodeArgs(true));
	}
}

BOOST_AUTO_TEST_CASE(multisig_value_transfer)
{
	deployWallet(200);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(2), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(3), h256::AlignRight)) == encodeArgs());
	// 4 owners, set required to 3
	BOOST_REQUIRE(callContractFunction("changeRequirement(uint256)", u256(3)) == encodeArgs());
	Address destination = Address("0x5c6d6026d3fb35cd7175fd0054ae8df50d8f8b41");
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	auto ophash = callContractFunction("execute(address,uint256,bytes)", h256(destination, h256::AlignRight), 100, 0x60, 0x00);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(2), 10 * ether);
	m_sender = account(2);
	callContractFunction("confirm(bytes32)", ophash);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(3), 10 * ether);
	m_sender = account(3);
	callContractFunction("confirm(bytes32)", ophash);
	// now it should go through
	BOOST_CHECK_EQUAL(balanceAt(destination), 100);
}

BOOST_AUTO_TEST_CASE(revoke_addOwner)
{
	deployWallet();
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(2), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(3), h256::AlignRight)) == encodeArgs());
	// 4 owners, set required to 3
	BOOST_REQUIRE(callContractFunction("changeRequirement(uint256)", u256(3)) == encodeArgs());
	// add a new owner
	Address deployer = m_sender;
	h256 opHash = dev::keccak256(FixedHash<4>(dev::keccak256("addOwner(address)")).asBytes() + h256(0x33).asBytes());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x33)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x33)) == encodeArgs(false));
	m_sender = account(0);
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x33)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x33)) == encodeArgs(false));
	// revoke one confirmation
	m_sender = deployer;
	BOOST_REQUIRE(callContractFunction("revoke(bytes32)", opHash) == encodeArgs());
	m_sender = account(0);
	sendEther(account(2), 10 * ether);
	m_sender = account(2);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x33)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x33)) == encodeArgs(false));
	m_sender = account(0);
	sendEther(account(3), 10 * ether);
	m_sender = account(3);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(0x33)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("isOwner(address)", h256(0x33)) == encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(revoke_transaction)
{
	deployWallet(200);
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(2), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(3), h256::AlignRight)) == encodeArgs());
	// 4 owners, set required to 3
	BOOST_REQUIRE(callContractFunction("changeRequirement(uint256)", u256(3)) == encodeArgs());
	// create a transaction
	Address deployer = m_sender;
	Address destination = Address("0x5c6d6026d3fb35cd7175fd0054ae8df50d8f8b41");
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	auto opHash = callContractFunction("execute(address,uint256,bytes)", h256(destination, h256::AlignRight), 100, 0x60, 0x00);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(2), 10 * ether);
	m_sender = account(2);
	callContractFunction("confirm(bytes32)", opHash);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunction("revoke(bytes32)", opHash) == encodeArgs());
	m_sender = deployer;
	callContractFunction("confirm(bytes32)", opHash);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	m_sender = account(0);
	sendEther(account(3), 10 * ether);
	m_sender = account(3);
	callContractFunction("confirm(bytes32)", opHash);
	// now it should go through
	BOOST_CHECK_EQUAL(balanceAt(destination), 100);
}

BOOST_AUTO_TEST_CASE(daylimit)
{
	deployWallet(200);
	BOOST_REQUIRE(callContractFunction("m_dailyLimit()") == encodeArgs(u256(0)));
	BOOST_REQUIRE(callContractFunction("setDailyLimit(uint256)", h256(100)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("m_dailyLimit()") == encodeArgs(u256(100)));
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(1), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(2), h256::AlignRight)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("addOwner(address)", h256(account(3), h256::AlignRight)) == encodeArgs());
	// 4 owners, set required to 3
	BOOST_REQUIRE(callContractFunction("changeRequirement(uint256)", u256(3)) == encodeArgs());

	// try to send tx over daylimit
	Address destination = Address("0x5c6d6026d3fb35cd7175fd0054ae8df50d8f8b41");
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	BOOST_REQUIRE(
		callContractFunction("execute(address,uint256,bytes)", h256(destination, h256::AlignRight), 150, 0x60, 0x00) !=
		encodeArgs(u256(0))
	);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	// try to send tx under daylimit by stranger
	m_sender = account(0);
	sendEther(account(4), 10 * ether);
	m_sender = account(4);
	BOOST_REQUIRE(
		callContractFunction("execute(address,uint256,bytes)", h256(destination, h256::AlignRight), 90, 0x60, 0x00) ==
		encodeArgs(u256(0))
	);
	BOOST_CHECK_EQUAL(balanceAt(destination), 0);
	// now send below limit by owner
	m_sender = account(0);
	sendEther(account(1), 10 * ether);
	BOOST_REQUIRE(
		callContractFunction("execute(address,uint256,bytes)", h256(destination, h256::AlignRight), 90, 0x60, 0x00) ==
		encodeArgs(u256(0))
	);
	BOOST_CHECK_EQUAL(balanceAt(destination), 90);
}

BOOST_AUTO_TEST_CASE(daylimit_constructor)
{
	deployWallet(200, {}, 1, 20);
	BOOST_REQUIRE(callContractFunction("m_dailyLimit()") == encodeArgs(u256(20)));
	BOOST_REQUIRE(callContractFunction("setDailyLimit(uint256)", h256(30)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("m_dailyLimit()") == encodeArgs(u256(30)));
}

//@todo test data calls

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
