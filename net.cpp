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

/**
 * Only used for testing. Not useful beyond tests.
 */
class TestHost: public Worker
{
public:
	TestHost(): Worker("test",0), m_io() {};
	~TestHost() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }
	
protected:
	ba::io_service m_io;
};

/**
 * Only used for testing. Not useful beyond tests.
 */
class TestNodeHost: public TestHost
{
public:
	TestNodeHost(): m_nodes(m_io) {};
	~TestNodeHost() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }

	NodeTable m_nodes;
};

class TestUDPSocket: UDPSocketEvents, public TestHost
{
public:
	TestUDPSocket(): m_socket(new UDPSocket<TestUDPSocket, 1024>(m_io, *this, 30300)) {}
	~TestUDPSocket() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }
	
	void onDisconnected(UDPSocketFace*) {};
	void onReceived(UDPSocketFace*, bi::udp::endpoint const&, bytesConstRef _packet) { if (_packet.toString() == "AAAA") success = true; }

	shared_ptr<UDPSocket<TestUDPSocket, 1024>> m_socket;
	
	bool success = false;
};

BOOST_AUTO_TEST_SUITE(p2p)

BOOST_AUTO_TEST_CASE(kademlia)
{
	TestNodeHost nodeHost;
	
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

