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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Tests for a fixed fee registrar contract.
 */

#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <libdevcore/Hash.h>
#include <libethcore/ABI.h>
#include <test/libsolidity/solidityExecutionFramework.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

static char const* registrarCode = R"DELIMITER(
//sol

contract NameRegister {
	function addr(string _name) constant returns (address o_owner);
	function name(address _owner) constant returns (string o_name);
}

contract Registrar is NameRegister {
	event Changed(string indexed name);
	event PrimaryChanged(string indexed name, address indexed addr);

	function owner(string _name) constant returns (address o_owner);
	function addr(string _name) constant returns (address o_address);
	function subRegistrar(string _name) constant returns (address o_subRegistrar);
	function content(string _name) constant returns (bytes32 o_content);

	function name(address _owner) constant returns (string o_name);
}

contract AuctionSystem {
	event AuctionEnded(string indexed _name, address _winner);
	event NewBid(string indexed _name, address _bidder, uint _value);

	/// Function that is called once an auction ends.
	function onAuctionEnd(string _name) internal;

	function bid(string _name, address _bidder, uint _value) internal {
		var auction = m_auctions[_name];
		if (auction.endDate > 0 && now > auction.endDate)
		{
			AuctionEnded(_name, auction.highestBidder);
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

			NewBid(_name, _bidder, _value);
		}
	}

	uint constant c_biddingTime = 7 days;

	struct Auction {
		address highestBidder;
		uint highestBid;
		uint secondHighestBid;
		uint sumOfBids;
		uint endDate;
	}
	mapping(string => Auction) m_auctions;
}

contract GlobalRegistrar is Registrar, AuctionSystem {
	struct Record {
		address owner;
		address primary;
		address subRegistrar;
		bytes32 content;
		uint renewalDate;
	}

	uint constant c_renewalInterval = 1 years;
	uint constant c_freeBytes = 12;

	function Registrar() {
		// TODO: Populate with hall-of-fame.
	}

	function() {
		// prevent people from just sending funds to the registrar
		__throw();
	}

	function onAuctionEnd(string _name) internal {
		var auction = m_auctions[_name];
		var record = m_toRecord[_name];
		if (record.owner != 0)
			record.owner.send(auction.sumOfBids - auction.highestBid / 100);
		else
			auction.highestBidder.send(auction.highestBid - auction.secondHighestBid);
		record.renewalDate = now + c_renewalInterval;
		record.owner = auction.highestBidder;
		Changed(_name);
	}

	function reserve(string _name) external {
		if (bytes(_name).length == 0)
			__throw();
		bool needAuction = requiresAuction(_name);
		if (needAuction)
		{
			if (now < m_toRecord[_name].renewalDate)
				__throw();
			bid(_name, msg.sender, msg.value);
		}
		else
		{
			Record record = m_toRecord[_name];
			if (record.owner != 0)
				__throw();
			m_toRecord[_name].owner = msg.sender;
			Changed(_name);
		}
	}

	function requiresAuction(string _name) internal returns (bool) {
		return bytes(_name).length < c_freeBytes;
	}

	modifier onlyrecordowner(string _name) { if (m_toRecord[_name].owner == msg.sender) _ }

	function transfer(string _name, address _newOwner) onlyrecordowner(_name) {
		m_toRecord[_name].owner = _newOwner;
		Changed(_name);
	}

	function disown(string _name) onlyrecordowner(_name) {
		if (stringsEqual(m_toName[m_toRecord[_name].primary], _name))
		{
			PrimaryChanged(_name, m_toRecord[_name].primary);
			m_toName[m_toRecord[_name].primary] = "";
		}
		delete m_toRecord[_name];
		Changed(_name);
	}

	function setAddress(string _name, address _a, bool _primary) onlyrecordowner(_name) {
		m_toRecord[_name].primary = _a;
		if (_primary)
		{
			PrimaryChanged(_name, _a);
			m_toName[_a] = _name;
		}
		Changed(_name);
	}
	function setSubRegistrar(string _name, address _registrar) onlyrecordowner(_name) {
		m_toRecord[_name].subRegistrar = _registrar;
		Changed(_name);
	}
	function setContent(string _name, bytes32 _content) onlyrecordowner(_name) {
		m_toRecord[_name].content = _content;
		Changed(_name);
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

	function owner(string _name) constant returns (address) { return m_toRecord[_name].owner; }
	function addr(string _name) constant returns (address) { return m_toRecord[_name].primary; }
	function subRegistrar(string _name) constant returns (address) { return m_toRecord[_name].subRegistrar; }
	function content(string _name) constant returns (bytes32) { return m_toRecord[_name].content; }
	function name(address _addr) constant returns (string o_name) { return m_toName[_addr]; }

	function __throw() internal {
		// workaround until we have "throw"
		uint[] x; x[1];
	}

	mapping (address => string) m_toName;
	mapping (string => Record) m_toRecord;
}
)DELIMITER";

static unique_ptr<bytes> s_compiledRegistrar;

class AuctionRegistrarTestFramework: public ExecutionFramework
{
protected:
	void deployRegistrar()
	{
		if (!s_compiledRegistrar)
		{
			m_optimize = true;
			m_compiler.reset(false, m_addStandardSources);
			m_compiler.addSource("", registrarCode);
			ETH_TEST_REQUIRE_NO_THROW(m_compiler.compile(m_optimize, m_optimizeRuns), "Compiling contract failed");
			s_compiledRegistrar.reset(new bytes(m_compiler.getBytecode("GlobalRegistrar")));
		}
		sendMessage(*s_compiledRegistrar, true);
		BOOST_REQUIRE(!m_output.empty());
	}

	using ContractInterface = ExecutionFramework::ContractInterface;
	class RegistrarInterface: public ContractInterface
	{
	public:
		RegistrarInterface(ExecutionFramework& _framework): ContractInterface(_framework) {}
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
	m_sender = Address(0x123);

	RegistrarInterface registrar(*this);

	// should not work
	registrar.reserve("");
	BOOST_CHECK_EQUAL(registrar.owner(""), u160(0));

	for (auto const& name: names)
	{
		registrar.reserve(name);
		BOOST_CHECK_EQUAL(registrar.owner(name), u160(0x123));
	}
}

BOOST_AUTO_TEST_CASE(double_reserve_long)
{
	// Test that it is not possible to re-reserve from a different address.
	deployRegistrar();
	string name = "abcabcabcabcabcabcabcabcabcabca";
	m_sender = Address(0x123);
	RegistrarInterface registrar(*this);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), u160(0x123));

	m_sender = Address(0x124);
	registrar.reserve(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), u160(0x123));
}

BOOST_AUTO_TEST_CASE(properties)
{
	// Test setting and retrieving  the various properties works.
	deployRegistrar();
	RegistrarInterface registrar(*this);
	string names[] = {"abcaeouoeuaoeuaoeu", "defncboagufra,fui", "ghagpyajfbcuajouhaeoi"};
	size_t addr = 0x9872543;
	for (string const& name: names)
	{
		addr++;
		size_t sender = addr + 10007;
		m_sender = Address(sender);
		// setting by sender works
		registrar.reserve(name);
		BOOST_CHECK_EQUAL(registrar.owner(name), u160(sender));
		registrar.setAddress(name, addr, true);
		BOOST_CHECK_EQUAL(registrar.addr(name), u160(addr));
		registrar.setSubRegistrar(name, addr + 20);
		BOOST_CHECK_EQUAL(registrar.subRegistrar(name), u160(addr + 20));
		registrar.setContent(name, h256(u256(addr + 90)));
		BOOST_CHECK_EQUAL(registrar.content(name), h256(u256(addr + 90)));

		// but not by someone else
		m_sender = Address(h256(addr + 10007 - 1));
		BOOST_CHECK_EQUAL(registrar.owner(name), u160(sender));
		registrar.setAddress(name, addr + 1, true);
		BOOST_CHECK_EQUAL(registrar.addr(name), u160(addr));
		registrar.setSubRegistrar(name, addr + 20 + 1);
		BOOST_CHECK_EQUAL(registrar.subRegistrar(name), u160(addr + 20));
		registrar.setContent(name, h256(u256(addr + 90 + 1)));
		BOOST_CHECK_EQUAL(registrar.content(name), h256(u256(addr + 90)));
	}
}

BOOST_AUTO_TEST_CASE(transfer)
{
	deployRegistrar();
	string name = "abcaoeguaoucaeoduceo";
	m_sender = Address(0x123);
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
	m_sender = Address(0x123);
	RegistrarInterface registrar(*this);
	registrar.reserve(name);
	registrar.setContent(name, h256(u256(123)));
	registrar.setAddress(name, u160(124), true);
	registrar.setSubRegistrar(name, u160(125));

	// someone else tries disowning
	m_sender = Address(0x128);
	registrar.disown(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), 0x123);

	m_sender = Address(0x123);
	registrar.disown(name);
	BOOST_CHECK_EQUAL(registrar.owner(name), 0);
	BOOST_CHECK_EQUAL(registrar.addr(name), 0);
	BOOST_CHECK_EQUAL(registrar.subRegistrar(name), 0);
	BOOST_CHECK_EQUAL(registrar.content(name), h256());
}

//@todo:
// - reverse lookup
// - actual auction

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
