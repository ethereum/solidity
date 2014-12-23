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

BOOST_AUTO_TEST_SUITE(p2p)

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
	
protected:
	ba::io_service m_io;
};

struct TestNodeTable: public NodeTable
{
	void generateTestNodes(int _count = 10)
	{
		asserts(_count < 1000);
		static uint16_t s_basePort = 30500;
		
		m_testNodes.clear();
		for (auto i = 0; i < _count; i++)
			m_testNodes.push_back(make_pair(KeyPair::create(),s_basePort++));
	}
	std::vector<std::pair<KeyPair,unsigned>> m_testNodes; // keypair and port

	/// Constructor
	using NodeTable::NodeTable;
	
	void setup()
	{
		/// Phase 1 test: populate with pings
		/// Phase 2 test: pre-populate *expected* ping-responses, send pings

		bi::address ourIp = bi::address::from_string("127.0.0.1");
		uint16_t ourPort = 30300;
		bi::udp::endpoint ourEndpoint(ourIp, ourPort);
		
		generateTestNodes();
		for (auto& n: m_testNodes)
			ping(bi::udp::endpoint(ourIp, n.second));
		
		// wait 1ms between each send
		// send PingNode for each s_bootstrapNodes
		// wait until nodecount is s_testNodes.count()
		
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
	TestNodeTableHost(): nodeTable(new TestNodeTable(m_io)) {};
	shared_ptr<TestNodeTable> nodeTable;
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

BOOST_AUTO_TEST_CASE(kademlia)
{
//	TestNodeTableHost node;
//	node.start();
//	node.nodeTable->join(); // ideally, joining with empty node table logs warning we can check for
//	node.nodeTable->setup();
//	sleep(1);
}

BOOST_AUTO_TEST_CASE(test_txrx_one)
{
	UDPDatagram d(bi::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 30300), bytes({65,65,65,65}));
	TestUDPSocket a; a.m_socket->connect(); a.start();
	a.m_socket->send(d);
	sleep(1);
	BOOST_REQUIRE_EQUAL(true, a.success);
}

BOOST_AUTO_TEST_SUITE_END()

