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

namespace
{

static char const* registrarCode = R"DELIMITER(
//sol FixedFeeRegistrar
// Simple global registrar with fixed-fee reservations.
// @authors:
//   Gav Wood <g@ethdev.com>

pragma solidity ^0.4.0;

contract Registrar {
	event Changed(string indexed name);

	function owner(string _name) constant returns (address o_owner);
	function addr(string _name) constant returns (address o_address);
	function subRegistrar(string _name) constant returns (address o_subRegistrar);
	function content(string _name) constant returns (bytes32 o_content);
}

contract FixedFeeRegistrar is Registrar {
	struct Record {
		address addr;
		address subRegistrar;
		bytes32 content;
		address owner;
	}

	modifier onlyrecordowner(string _name) { if (m_record(_name).owner == msg.sender) _; }

	function reserve(string _name) payable {
		Record rec = m_record(_name);
		if (rec.owner == 0x0000000000000000000000000000000000000000 && msg.value >= c_fee) {
			rec.owner = msg.sender;
			emit Changed(_name);
		}
	}
	function disown(string _name, address _refund) onlyrecordowner(_name) {
		delete m_recordData[uint(keccak256(bytes(_name))) / 8];
		if (!_refund.send(c_fee))
			throw;
		emit Changed(_name);
	}
	function transfer(string _name, address _newOwner) onlyrecordowner(_name) {
		m_record(_name).owner = _newOwner;
		emit Changed(_name);
	}
	function setAddr(string _name, address _a) onlyrecordowner(_name) {
		m_record(_name).addr = _a;
		emit Changed(_name);
	}
	function setSubRegistrar(string _name, address _registrar) onlyrecordowner(_name) {
		m_record(_name).subRegistrar = _registrar;
		emit Changed(_name);
	}
	function setContent(string _name, bytes32 _content) onlyrecordowner(_name) {
		m_record(_name).content = _content;
		emit Changed(_name);
	}

	function record(string _name) constant returns (address o_addr, address o_subRegistrar, bytes32 o_content, address o_owner) {
		Record rec = m_record(_name);
		o_addr = rec.addr;
		o_subRegistrar = rec.subRegistrar;
		o_content = rec.content;
		o_owner = rec.owner;
	}
	function addr(string _name) constant returns (address) { return m_record(_name).addr; }
	function subRegistrar(string _name) constant returns (address) { return m_record(_name).subRegistrar; }
	function content(string _name) constant returns (bytes32) { return m_record(_name).content; }
	function owner(string _name) constant returns (address) { return m_record(_name).owner; }

	Record[2**253] m_recordData;
	function m_record(string _name) constant internal returns (Record storage o_record) {
		return m_recordData[uint(keccak256(bytes(_name))) / 8];
	}
	uint constant c_fee = 69 ether;
}
)DELIMITER";

static unique_ptr<bytes> s_compiledRegistrar;

class RegistrarTestFramework: public SolidityExecutionFramework
{
protected:
	void deployRegistrar()
	{
		if (!s_compiledRegistrar)
			s_compiledRegistrar.reset(new bytes(compileContract(registrarCode, "FixedFeeRegistrar")));

		sendMessage(*s_compiledRegistrar, true);
		BOOST_REQUIRE(m_transactionSuccessful);
		BOOST_REQUIRE(!m_output.empty());
	}
	u256 const m_fee = u256("69000000000000000000");
};

}

/// This is a test suite that tests optimised code!
BOOST_FIXTURE_TEST_SUITE(SolidityFixedFeeRegistrar, RegistrarTestFramework)

BOOST_AUTO_TEST_CASE(creation)
{
	deployRegistrar();
}

BOOST_AUTO_TEST_CASE(reserve)
{
	// Test that reserving works and fee is taken into account.
	deployRegistrar();
	string name[] = {"abc", "def", "ghi"};
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name[0])) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name[0])) == encodeArgs(h256(account(0), h256::AlignRight)));
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee + 1, encodeDyn(name[1])) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name[1])) == encodeArgs(h256(account(0), h256::AlignRight)));
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee - 1, encodeDyn(name[2])) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name[2])) == encodeArgs(h256(0)));
}

BOOST_AUTO_TEST_CASE(double_reserve)
{
	// Test that it is not possible to re-reserve from a different address.
	deployRegistrar();
	string name = "abc";
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(h256(account(0), h256::AlignRight)));

	sendEther(account(1), 100 * ether);
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(h256(account(0), h256::AlignRight)));
}

BOOST_AUTO_TEST_CASE(properties)
{
	// Test setting and retrieving  the various properties works.
	deployRegistrar();
	string names[] = {"abc", "def", "ghi"};
	size_t addr = 0x9872543;
	size_t count = 1;
	for (string const& name: names)
	{
		addr++;
		m_sender = account(0);
		sendEther(account(count), 100 * ether);
		m_sender = account(count);
		Address owner = m_sender;
		// setting by sender works
		BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
		BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(h256(owner, h256::AlignRight)));
		BOOST_CHECK(callContractFunction("setAddr(string,address)", u256(0x40), u256(addr), u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("addr(string)", encodeDyn(name)) == encodeArgs(addr));
		BOOST_CHECK(callContractFunction("setSubRegistrar(string,address)", u256(0x40), addr + 20, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("subRegistrar(string)", encodeDyn(name)) == encodeArgs(addr + 20));
		BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), addr + 90, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(addr + 90));
		count++;
		// but not by someone else
		m_sender = account(0);
		sendEther(account(count), 100 * ether);
		m_sender = account(count);
		BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(h256(owner, h256::AlignRight)));
		BOOST_CHECK(callContractFunction("setAddr(string,address)", u256(0x40), addr + 1, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("addr(string)", encodeDyn(name)) == encodeArgs(addr));
		BOOST_CHECK(callContractFunction("setSubRegistrar(string,address)", u256(0x40), addr + 20 + 1, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("subRegistrar(string)", encodeDyn(name)) == encodeArgs(addr + 20));
		BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), addr + 90 + 1, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(addr + 90));
		count++;
	}
}

BOOST_AUTO_TEST_CASE(transfer)
{
	deployRegistrar();
	string name = "abc";
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), h256(account(0), h256::AlignRight), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("transfer(string,address)", u256(0x40), u256(555), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(u256(555)));
	BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(h256(account(0), h256::AlignRight)));
}

BOOST_AUTO_TEST_CASE(disown)
{
	deployRegistrar();
	string name = "abc";
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), h256(account(0), h256::AlignRight), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("setAddr(string,address)", u256(0x40), u256(124), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("setSubRegistrar(string,address)", u256(0x40), u256(125), u256(name.length()), name) == encodeArgs());

	BOOST_CHECK_EQUAL(balanceAt(Address(0x124)), 0);
	BOOST_CHECK(callContractFunction("disown(string,address)", u256(0x40), u256(0x124), name.size(), name) == encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(Address(0x124)), m_fee);

	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("addr(string)", encodeDyn(name)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("subRegistrar(string)", encodeDyn(name)) == encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
