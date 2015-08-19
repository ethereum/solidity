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
//sol FixedFeeRegistrar
// Simple global registrar with fixed-fee reservations.
// @authors:
//   Gav Wood <g@ethdev.com>

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

	modifier onlyrecordowner(string _name) { if (m_record(_name).owner == msg.sender) _ }

	function reserve(string _name) {
		Record rec = m_record(_name);
		if (rec.owner == 0 && msg.value >= c_fee) {
			rec.owner = msg.sender;
			Changed(_name);
		}
	}
	function disown(string _name, address _refund) onlyrecordowner(_name) {
		delete m_recordData[uint(sha3(_name)) / 8];
		_refund.send(c_fee);
		Changed(_name);
	}
	function transfer(string _name, address _newOwner) onlyrecordowner(_name) {
		m_record(_name).owner = _newOwner;
		Changed(_name);
	}
	function setAddr(string _name, address _a) onlyrecordowner(_name) {
		m_record(_name).addr = _a;
		Changed(_name);
	}
	function setSubRegistrar(string _name, address _registrar) onlyrecordowner(_name) {
		m_record(_name).subRegistrar = _registrar;
		Changed(_name);
	}
	function setContent(string _name, bytes32 _content) onlyrecordowner(_name) {
		m_record(_name).content = _content;
		Changed(_name);
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
		return m_recordData[uint(sha3(_name)) / 8];
	}
	uint constant c_fee = 69 ether;
}
)DELIMITER";

static unique_ptr<bytes> s_compiledRegistrar;

class RegistrarTestFramework: public ExecutionFramework
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
			s_compiledRegistrar.reset(new bytes(m_compiler.getBytecode("FixedFeeRegistrar")));
		}
		sendMessage(*s_compiledRegistrar, true);
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
	m_sender = Address(0x123);
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name[0])) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name[0])) == encodeArgs(h256(0x123)));
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee + 1, encodeDyn(name[1])) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name[1])) == encodeArgs(h256(0x123)));
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee - 1, encodeDyn(name[2])) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name[2])) == encodeArgs(h256(0)));
}

BOOST_AUTO_TEST_CASE(double_reserve)
{
	// Test that it is not possible to re-reserve from a different address.
	deployRegistrar();
	string name = "abc";
	m_sender = Address(0x123);
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(h256(0x123)));

	m_sender = Address(0x124);
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(h256(0x123)));
}

BOOST_AUTO_TEST_CASE(properties)
{
	// Test setting and retrieving  the various properties works.
	deployRegistrar();
	string names[] = {"abc", "def", "ghi"};
	size_t addr = 0x9872543;
	for (string const& name: names)
	{
		addr++;
		size_t sender = addr + 10007;
		m_sender = Address(sender);
		// setting by sender works
		BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
		BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(u256(sender)));
		BOOST_CHECK(callContractFunction("setAddr(string,address)", u256(0x40), u256(addr), u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("addr(string)", encodeDyn(name)) == encodeArgs(addr));
		BOOST_CHECK(callContractFunction("setSubRegistrar(string,address)", u256(0x40), addr + 20, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("subRegistrar(string)", encodeDyn(name)) == encodeArgs(addr + 20));
		BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), addr + 90, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(addr + 90));
		// but not by someone else
		m_sender = Address(h256(addr + 10007 - 1));
		BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(sender));
		BOOST_CHECK(callContractFunction("setAddr(string,address)", u256(0x40), addr + 1, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("addr(string)", encodeDyn(name)) == encodeArgs(addr));
		BOOST_CHECK(callContractFunction("setSubRegistrar(string,address)", u256(0x40), addr + 20 + 1, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("subRegistrar(string)", encodeDyn(name)) == encodeArgs(addr + 20));
		BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), addr + 90 + 1, u256(name.length()), name) == encodeArgs());
		BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(addr + 90));
	}
}

BOOST_AUTO_TEST_CASE(transfer)
{
	deployRegistrar();
	string name = "abc";
	m_sender = Address(0x123);
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), u256(123), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("transfer(string,address)", u256(0x40), u256(555), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(u256(555)));
	BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(u256(123)));
}

BOOST_AUTO_TEST_CASE(disown)
{
	deployRegistrar();
	string name = "abc";
	m_sender = Address(0x123);
	BOOST_REQUIRE(callContractFunctionWithValue("reserve(string)", m_fee, encodeDyn(name)) == encodeArgs());
	BOOST_CHECK(callContractFunction("setContent(string,bytes32)", u256(0x40), u256(123), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("setAddr(string,address)", u256(0x40), u256(124), u256(name.length()), name) == encodeArgs());
	BOOST_CHECK(callContractFunction("setSubRegistrar(string,address)", u256(0x40), u256(125), u256(name.length()), name) == encodeArgs());

	BOOST_CHECK_EQUAL(m_state.balance(Address(0x124)), 0);
	BOOST_CHECK(callContractFunction("disown(string,address)", u256(0x40), u256(0x124), name.size(), name) == encodeArgs());
	BOOST_CHECK_EQUAL(m_state.balance(Address(0x124)), m_fee);

	BOOST_CHECK(callContractFunction("owner(string)", encodeDyn(name)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("content(string)", encodeDyn(name)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("addr(string)", encodeDyn(name)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("subRegistrar(string)", encodeDyn(name)) == encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
