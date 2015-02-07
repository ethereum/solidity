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
/** @file net.cpp
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 */

#include <boost/test/unit_test.hpp>
#include <libdevcore/Worker.h>
#include <libdevcrypto/Common.h>
#include <libp2p/UDP.h>
#include <libp2p/NodeTable.h>
using namespace std;
using namespace dev;
using namespace dev::p2p;
namespace ba = boost::asio;
namespace bi = ba::ip;

BOOST_AUTO_TEST_SUITE(net)

/**
 * Only used for testing. Not useful beyond tests.
 */
class TestHost: public Worker
{
public:
	TestHost(): Worker("test",0), m_io() {};
	virtual ~TestHost() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }
	void doneWorking() { m_io.reset(); m_io.poll(); m_io.reset(); }
	
protected:
	ba::io_service m_io;
};

struct TestNodeTable: public NodeTable
{
	/// Constructor
	TestNodeTable(ba::io_service& _io, KeyPair _alias, uint16_t _port = 30300): NodeTable(_io, _alias, _port) {}
	
	static std::vector<std::pair<KeyPair,unsigned>> createTestNodes(unsigned _count)
	{
		std::vector<std::pair<KeyPair,unsigned>> ret;
		asserts(_count < 1000);
		static uint16_t s_basePort = 30500;
		
		ret.clear();
		for (unsigned i = 0; i < _count; i++)
		{
			KeyPair k = KeyPair::create();
			ret.push_back(make_pair(k,s_basePort+i));
		}
		
		return std::move(ret);
	}
	
	void pingTestNodes(std::vector<std::pair<KeyPair,unsigned>> const& _testNodes)
	{
		bi::address ourIp = bi::address::from_string("127.0.0.1");
		for (auto& n: _testNodes)
		{
			ping(bi::udp::endpoint(ourIp, n.second));
			this_thread::sleep_for(chrono::milliseconds(2));
		}
	}
	
	void populateTestNodes(std::vector<std::pair<KeyPair,unsigned>> const& _testNodes, size_t _count = 0)
	{
		if (!_count)
			_count = _testNodes.size();

		bi::address ourIp = bi::address::from_string("127.0.0.1");
		for (auto& n: _testNodes)
			if (_count--)
				noteActiveNode(n.first.pub(), bi::udp::endpoint(ourIp, n.second));
			else
				break;
	}
	
	void reset()
	{
		Guard l(x_state);
		for (auto& n: m_state) n.nodes.clear();
	}
};

/**
 * Only used for testing. Not useful beyond tests.
 */
struct TestNodeTableHost: public TestHost
{
	TestNodeTableHost(unsigned _count = 8): m_alias(KeyPair::create()), nodeTable(new TestNodeTable(m_io, m_alias)), testNodes(TestNodeTable::createTestNodes(_count)) {};
	~TestNodeTableHost() { m_io.stop(); stopWorking(); }

	void setup() { for (auto n: testNodes) nodeTables.push_back(make_shared<TestNodeTable>(m_io,n.first,n.second)); }
	
	void pingAll() { for (auto& t: nodeTables) t->pingTestNodes(testNodes); }
	
	void populateAll(size_t _count = 0) { for (auto& t: nodeTables) t->populateTestNodes(testNodes, _count); }
	
	void populate(size_t _count = 0) { nodeTable->populateTestNodes(testNodes, _count); }
	
	KeyPair m_alias;
	shared_ptr<TestNodeTable> nodeTable;
	std::vector<std::pair<KeyPair,unsigned>> testNodes; // keypair and port
	std::vector<shared_ptr<TestNodeTable>> nodeTables;
};

class TestUDPSocket: UDPSocketEvents, public TestHost
{
public:
	TestUDPSocket(): m_socket(new UDPSocket<TestUDPSocket, 1024>(m_io, *this, 30300)) {}

	void onDisconnected(UDPSocketFace*) {};
	void onReceived(UDPSocketFace*, bi::udp::endpoint const&, bytesConstRef _packet) { if (_packet.toString() == "AAAA") success = true; }

	shared_ptr<UDPSocket<TestUDPSocket, 1024>> m_socket;
	
	bool success = false;
};

BOOST_AUTO_TEST_CASE(test_neighbours_packet)
{
	KeyPair k = KeyPair::create();
	std::vector<std::pair<KeyPair,unsigned>> testNodes(TestNodeTable::createTestNodes(16));
	bi::udp::endpoint to(boost::asio::ip::address::from_string("127.0.0.1"), 30000);
	
	Neighbours out(to);
	for (auto n: testNodes)
	{
		Neighbours::Node node;
		node.ipAddress = boost::asio::ip::address::from_string("127.0.0.1").to_string();
		node.port = n.second;
		node.node = n.first.pub();
		out.nodes.push_back(node);
	}
	out.sign(k.sec());

	bytesConstRef packet(out.data.data(), out.data.size());
	bytesConstRef rlpBytes(packet.cropped(h256::size + Signature::size + 1));
	Neighbours in = Neighbours::fromBytesConstRef(to, rlpBytes);
	int count = 0;
	for (auto n: in.nodes)
	{
		BOOST_REQUIRE_EQUAL(testNodes[count].second, n.port);
		BOOST_REQUIRE_EQUAL(testNodes[count].first.pub(), n.node);
		BOOST_REQUIRE_EQUAL(sha3(testNodes[count].first.pub()), sha3(n.node));
		count++;
	}
}

BOOST_AUTO_TEST_CASE(test_findnode_neighbours)
{
	// Executing findNode should result in a list which is serialized
	// into Neighbours packet. Neighbours packet should then be deserialized
	// into the same list of nearest nodes.
}

BOOST_AUTO_TEST_CASE(test_windows_template)
{
	bi::udp::endpoint ep;
	PingNode p(ep);
}

BOOST_AUTO_TEST_CASE(kademlia)
{
	// Not yet a 'real' test.
	TestNodeTableHost node(8);
	node.start();
	node.nodeTable->discover(); // ideally, joining with empty node table logs warning we can check for
	node.setup();
	node.populate();
	clog << "NodeTable:\n" << *node.nodeTable.get() << endl;
	
	node.populateAll();
	clog << "NodeTable:\n" << *node.nodeTable.get() << endl;
	
	auto nodes = node.nodeTable->nodes();
	nodes.sort();
	
	node.nodeTable->reset();
	clog << "NodeTable:\n" << *node.nodeTable.get() << endl;

	node.populate(1);
	clog << "NodeTable:\n" << *node.nodeTable.get() << endl;
	
	node.nodeTable->discover();
	this_thread::sleep_for(chrono::milliseconds(2000));
	clog << "NodeTable:\n" << *node.nodeTable.get() << endl;

	BOOST_REQUIRE_EQUAL(node.nodeTable->count(), 8);
	
	auto netNodes = node.nodeTable->nodes();
	netNodes.sort();

}

BOOST_AUTO_TEST_CASE(test_udp_once)
{
	UDPDatagram d(bi::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 30300), bytes({65,65,65,65}));
	TestUDPSocket a; a.m_socket->connect(); a.start();
	a.m_socket->send(d);
	this_thread::sleep_for(chrono::seconds(1));
	BOOST_REQUIRE_EQUAL(true, a.success);
}

BOOST_AUTO_TEST_SUITE_END()

