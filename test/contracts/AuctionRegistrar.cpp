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
 * Tests for a fixed fee registrar contract.
 */

#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <test/libsolidity/SolidityExecutionFramework.h>
#include <test/contracts/ContractInterface.h>

using namespace std;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

static char const* registrarCode = R"DELIMITER(
pragma solidity ^0.4.0;

contract NameRegister {
	function addr(string memory _name) public view returns (address o_owner);
	function name(address _owner) public view returns (string memory o_name);
}

contract Registrar is NameRegister {
	event Changed(string indexed name);
	event PrimaryChanged(string indexed name, address indexed addr);

	function owner(string memory _name) public view returns (address o_owner);
	function addr(string memory _name) public view returns (address o_address);
	function subRegistrar(string memory _name) public view returns (address o_subRegistrar);
	function content(string memory _name) public view returns (bytes32 o_content);

	function name(address _owner) public view returns (string memory o_name);
}

contract AuctionSystem {
	event AuctionEnded(string indexed _name, address _winner);
	event NewBid(string indexed _name, address _bidder, uint _value);

	/// Function that is called once an auction ends.
	function onAuctionEnd(string memory _name) internal;

	function bid(string memory _name, address payable _bidder, uint _value) internal {
		Auction storage auction = m_auctions[_name];
		if (auction.endDate > 0 && now > auction.endDate)
		{
			emit AuctionEnded(_name, auction.highestBidder);
			onAuctionEnd(_name);
			delete m_auctions[_name];
			return;
		}
		if (msg.value > auction.highestBid)
		{
			// new bid on auction
			auction.secondHighestBid = auction.highestBid;
			auction.sumOfBids += _value;
			auction.highestBid = _value;
			auction.highestBidder = _bidder;
			auction.endDate = now + c_biddingTime;

			emit NewBid(_name, _bidder, _value);
		}
	}

	uint constant c_biddingTime = 7 days;

	struct Auction {
		address payable highestBidder;
		uint highestBid;
		uint secondHighestBid;
		uint sumOfBids;
		uint endDate;
	}
	mapping(string => Auction) m_auctions;
}

contract GlobalRegistrar is Registrar, AuctionSystem {
	struct Record {
		address payable owner;
		address primary;
		address subRegistrar;
		bytes32 content;
		uint renewalDate;
	}

	uint constant c_renewalInterval = 365 days;
	uint constant c_freeBytes = 12;

	function Registrar() public {
		// TODO: Populate with hall-of-fame.
	}

	function onAuctionEnd(string memory _name) internal {
		Auction storage auction = m_auctions[_name];
		Record storage record = m_toRecord[_name];
		address previousOwner = record.owner;
		record.renewalDate = now + c_renewalInterval;
		record.owner = auction.highestBidder;
		emit Changed(_name);
		if (previousOwner != 0x0000000000000000000000000000000000000000) {
			if (!record.owner.send(auction.sumOfBids - auction.highestBid / 100))
				revert();
		} else {
			if (!auction.highestBidder.send(auction.highestBid - auction.secondHighestBid))
				revert();
		}
	}

	function reserve(string calldata _name) external payable {
		if (bytes(_name).length == 0)
			revert();
		bool needAuction = requiresAuction(_name);
		if (needAuction)
		{
			if (now < m_toRecord[_name].renewalDate)
				revert();
			bid(_name, msg.sender, msg.value);
		} else {
			Record storage record = m_toRecord[_name];
			if (record.owner != 0x0000000000000000000000000000000000000000)
				revert();
			m_toRecord[_name].owner = msg.sender;
			emit Changed(_name);
		}
	}

	function requiresAuction(string memory _name) internal returns (bool) {
		return bytes(_name).length < c_freeBytes;
	}

	modifier onlyrecordowner(string memory _name) { if (m_toRecord[_name].owner == msg.sender) _; }

	function transfer(string memory _name, address payable _newOwner) onlyrecordowner(_name) public {
		m_toRecord[_name].owner = _newOwner;
		emit Changed(_name);
	}

	function disown(string memory _name) onlyrecordowner(_name) public {
		if (stringsEqual(m_toName[m_toRecord[_name].primary], _name))
		{
			emit PrimaryChanged(_name, m_toRecord[_name].primary);
			m_toName[m_toRecord[_name].primary] = "";
		}
		delete m_toRecord[_name];
		emit Changed(_name);
	}

	function setAddress(string memory _name, address _a, bool _primary) onlyrecordowner(_name) public {
		m_toRecord[_name].primary = _a;
		if (_primary)
		{
			emit PrimaryChanged(_name, _a);
			m_toName[_a] = _name;
		}
		emit Changed(_name);
	}
	function setSubRegistrar(string memory _name, address _registrar) onlyrecordowner(_name) public {
		m_toRecord[_name].subRegistrar = _registrar;
		emit Changed(_name);
	}
	function setContent(string memory _name, bytes32 _content) onlyrecordowner(_name) public {
		m_toRecord[_name].content = _content;
		emit Changed(_name);
	}

	function stringsEqual(string storage _a, string memory _b) internal returns (bool) {
		bytes storage a = bytes(_a);
		bytes memory b = bytes(_b);
		if (a.length != b.length)
			return false;
		// @todo unroll this loop
		for (uint i = 0; i < a.length; i ++)
			if (a[i] != b[i])
				return false;
		return true;
	}

	function owner(string memory _name) public view returns (address) { return m_toRecord[_name].owner; }
	function addr(string memory _name) public view returns (address) { return m_toRecord[_name].primary; }
	function subRegistrar(string memory _name) public view returns (address) { return m_toRecord[_name].subRegistrar; }
	function content(string memory _name) public view returns (bytes32) { return m_toRecord[_name].content; }
	function name(address _addr) public view returns (string memory o_name) { return m_toName[_addr]; }

	mapping (address => string) m_toName;
	mapping (string => Record) m_toRecord;
}
)DELIMITER";

static unique_ptr<bytes> s_compiledRegistrar;

class AuctionRegistrarTestFramework: public SolidityExecutionFramework
{
protected:
	void deployRegistrar()
	{
		if (!s_compiledRegistrar)
			s_compiledRegistrar.reset(new bytes(compileContract(registrarCode, "GlobalRegistrar")));

		sendMessage(*s_compiledRegistrar, true);
		BOOST_REQUIRE(m_transactionSuccessful);
		BOOST_REQUIRE(!m_output.empty());
	}

	class RegistrarInterface: public ContractInterface
	{
	public:
		RegistrarInterface(SolidityExecutionFramework& _framework): ContractInterface(_framework) {}
		void reserve(string const& _name)
		{
			callString("reserve", _name);
		}
		u160 owner(string const& _name)
		{
			return callStringReturnsAddress("owner", _name);
		}
		void setAddress(string const& _name, u160 const& _address, bool _primary)
		{
			callStringAddressBool("setAddress", _name, _address, _primary);
		}
		u160 addr(string const& _name)
		{
			return callStringReturnsAddress("addr", _name);
		}
		string name(u160 const& _addr)
		{
			return callAddressReturnsString("name", _addr);
		}
		void setSubRegistrar(string const& _name, u160 const& _address)
		{
			callStringAddress("setSubRegistrar", _name, _address);
		}
		u160 subRegistrar(string const& _name)
		{
			return callStringReturnsAddress("subRegistrar", _name);
		}
		void setContent(string const& _name, h256 const& _content)
		{
			callStringBytes32("setContent", _name, _content);
		}
		h256 content(string const& _name)
		{
			return callStringReturnsBytes32("content", _name);
		}
		void transfer(string const& _name, u160 const& _target)
		{
			return callStringAddress("transfer", _name, _target);
		}
		void disown(string const& _name)
		{
			return callString("disown", _name);
		}
	};

	size_t const m_biddingTime = size_t(7 * 24 * 3600);
	size_t const m_renewalInterval = size_t(365 * 24 * 3600);
};

}

/// This is a test suite that tests optimised code!
BOOST_FIXTURE_TEST_SUITE(SolidityAuctionRegistrar, AuctionRegistrarTestFramework)

BOOST_AUTO_TEST_CASE(creation)
{
	deployRegistrar();
}

BOOST_AUTO_TEST_CASE(reserve)
{
	// Test that reserving works for long strings
	deployRegistrar();
	vector<string> names{"abcabcabcabcabc", "defdefdefdefdef", "ghighighighighighighighighighighighighighighi"};

	RegistrarInterface registrar(*this);

	// should not work
	registrar.reserve("");
	BOOST_CHECK_EQUAL(registrar.owner(""), u160(0));

	for (auto const& name: names)
	{
		registrar.reserve(name);
		BOOST_CHECK_EQUAL(registrar.owner(name), u160(m_sender));
	}
}

BOOST_AUTO_TEST_CASE(double_reserve_long)
{
	// Test that it is not possible to re-reserve from a different address.
	deployRegistrar();
	string name = "abcabcabcabcabcabcabcabcabcabca";
	RegistrarInterface registrar(*this);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), m_sender);

	sendEther(account(1), u256(10) * ether);
	m_sender = account(1);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), account(0));
}

BOOST_AUTO_TEST_CASE(properties)
{
	// Test setting and retrieving  the various properties works.
	deployRegistrar();
	RegistrarInterface registrar(*this);
	string names[] = {"abcaeouoeuaoeuaoeu", "defncboagufra,fui", "ghagpyajfbcuajouhaeoi"};
	size_t addr = 0x9872543;
	size_t count = 1;
	for (string const& name: names)
	{
		m_sender = account(0);
		sendEther(account(count), u256(20) * ether);
		m_sender = account(count);
		auto sender = m_sender;
		addr += count;
		// setting by sender works
		registrar.reserve(name);
		BOOST_CHECK_EQUAL(registrar.owner(name), sender);
		registrar.setAddress(name, addr, true);
		BOOST_CHECK_EQUAL(registrar.addr(name), u160(addr));
		registrar.setSubRegistrar(name, addr + 20);
		BOOST_CHECK_EQUAL(registrar.subRegistrar(name), u160(addr + 20));
		registrar.setContent(name, h256(u256(addr + 90)));
		BOOST_CHECK_EQUAL(registrar.content(name), h256(u256(addr + 90)));

		// but not by someone else
		m_sender = account(count - 1);
		BOOST_CHECK_EQUAL(registrar.owner(name), sender);
		registrar.setAddress(name, addr + 1, true);
		BOOST_CHECK_EQUAL(registrar.addr(name), u160(addr));
		registrar.setSubRegistrar(name, addr + 20 + 1);
		BOOST_CHECK_EQUAL(registrar.subRegistrar(name), u160(addr + 20));
		registrar.setContent(name, h256(u256(addr + 90 + 1)));
		BOOST_CHECK_EQUAL(registrar.content(name), h256(u256(addr + 90)));
		count++;
	}
}

BOOST_AUTO_TEST_CASE(transfer)
{
	deployRegistrar();
	string name = "abcaoeguaoucaeoduceo";
	RegistrarInterface registrar(*this);
	registrar.reserve(name);
	registrar.setContent(name, h256(u256(123)));
	registrar.transfer(name, u160(555));
	BOOST_CHECK_EQUAL(registrar.owner(name), u160(555));
	BOOST_CHECK_EQUAL(registrar.content(name), h256(u256(123)));
}

BOOST_AUTO_TEST_CASE(disown)
{
	deployRegistrar();
	string name = "abcaoeguaoucaeoduceo";

	RegistrarInterface registrar(*this);
	registrar.reserve(name);
	registrar.setContent(name, h256(u256(123)));
	registrar.setAddress(name, u160(124), true);
	registrar.setSubRegistrar(name, u160(125));
	BOOST_CHECK_EQUAL(registrar.name(u160(124)), name);

	// someone else tries disowning
	sendEther(account(1), u256(10) * ether);
	m_sender = account(1);
	registrar.disown(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), account(0));

	m_sender = account(0);
	registrar.disown(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), 0);
	BOOST_CHECK_EQUAL(registrar.addr(name), 0);
	BOOST_CHECK_EQUAL(registrar.subRegistrar(name), 0);
	BOOST_CHECK_EQUAL(registrar.content(name), h256());
	BOOST_CHECK_EQUAL(registrar.name(u160(124)), "");
}

BOOST_AUTO_TEST_CASE(auction_simple)
{
	deployRegistrar();
	string name = "x";

	RegistrarInterface registrar(*this);
	// initiate auction
	registrar.setNextValue(8);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), 0);
	// "wait" until auction end
	m_rpc.test_modifyTimestamp(currentTimestamp() + m_biddingTime + 10);
	// trigger auction again
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), m_sender);
}

BOOST_AUTO_TEST_CASE(auction_bidding)
{
	deployRegistrar();
	string name = "x";

	unsigned startTime = 0x776347e2;
	m_rpc.test_modifyTimestamp(startTime);

	RegistrarInterface registrar(*this);
	// initiate auction
	registrar.setNextValue(8);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), 0);
	// overbid self
	m_rpc.test_modifyTimestamp(startTime + m_biddingTime - 10);
	registrar.setNextValue(12);
	registrar.reserve(name);
	// another bid by someone else
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	m_rpc.test_modifyTimestamp(startTime + 2 * m_biddingTime - 50);
	registrar.setNextValue(13);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), 0);
	// end auction by first bidder (which is not highest) trying to overbid again (too late)
	m_sender = account(0);
	m_rpc.test_modifyTimestamp(startTime + 4 * m_biddingTime);
	registrar.setNextValue(20);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), account(1));
}

BOOST_AUTO_TEST_CASE(auction_renewal)
{
	deployRegistrar();

	string name = "x";
	RegistrarInterface registrar(*this);
	size_t startTime = currentTimestamp();
	// register name by auction
	registrar.setNextValue(8);
	registrar.reserve(name);
	m_rpc.test_modifyTimestamp(startTime + 4 * m_biddingTime);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), m_sender);

	// try to re-register before interval end
	sendEther(account(1), 10 * ether);
	m_sender = account(1);
	m_rpc.test_modifyTimestamp(currentTimestamp() + m_renewalInterval - 1);
	registrar.setNextValue(80);
	registrar.reserve(name);
	m_rpc.test_modifyTimestamp(currentTimestamp() + m_biddingTime);
	// if there is a bug in the renewal logic, this would transfer the ownership to account(1),
	// but if there is no bug, this will initiate the auction, albeit with a zero bid
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), account(0));

	registrar.setNextValue(80);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), account(1));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
