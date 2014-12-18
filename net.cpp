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
#include <libp2p/UDP.h>
using namespace std;
using namespace dev;
using namespace dev::p2p;
namespace ba = boost::asio;
namespace bi = ba::ip;

class Kademlia: UDPSocketEvents
{
public:
	Kademlia(): Worker("test",0), m_io(), m_socket(new UDPSocket<Kademlia, 1024>(m_io, *this, 30300)) {}
	~Kademlia() { m_io.stop(); stopWorking(); }

	void onDisconnected(UDPSocketFace*) {};
	void onReceived(UDPSocketFace*, bi::udp::endpoint const& _from, bytesConstRef _packet) { if (_packet.toString() == "AAAA") success = true; }

	ba::io_service m_io;
	shared_ptr<UDPSocket<Kademlia, 1024>> m_socket;
	
	bool success = false;
};

class TestA: UDPSocketEvents, public Worker
{
public:
	TestA(): Worker("test",0), m_io(), m_socket(new UDPSocket<TestA, 1024>(m_io, *this, 30300)) {}
	~TestA() { m_io.stop(); stopWorking(); }
	
	void start() { startWorking(); }
	void doWork() { m_io.run(); }
	
	void onDisconnected(UDPSocketFace*) {};
	void onReceived(UDPSocketFace*, bi::udp::endpoint const& _from, bytesConstRef _packet) { if (_packet.toString() == "AAAA") success = true; }

	ba::io_service m_io;
	shared_ptr<UDPSocket<TestA, 1024>> m_socket;
	
	bool success = false;
};

//struct TestBProtocol: UDPSocketEvents
//{
//	void onDisconnected(UDPSocketFace*) {};
//	void onReceived(UDPSocketFace*, bi::udp::endpoint const& _from, bytesConstRef _packet) { cout << "received TestBProtocol" << endl; };
//};
//
//class TestB: TestBProtocol
//{
//public:
//	TestB(): m_io(), m_socket(m_io, *this, 30300) {}
////private:
//	ba::io_service m_io;
//	UDPSocket<TestBProtocol, 1024> m_socket;
//};
//
//class TestC
//{
//public:
//	TestC(): m_io(), m_socket(m_io, m_rpc, 30300) {}
////private:
//	ba::io_service m_io;
//	TestBProtocol m_rpc;
//	UDPSocket<TestBProtocol, 1024> m_socket;
//};

BOOST_AUTO_TEST_SUITE(p2p)

BOOST_AUTO_TEST_CASE(test_txrx_one)
{
	UDPDatagram d;
	d.to = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 30300);
	d.data = bytes({65,65,65,65});
	
	TestA a; a.start(); a.m_socket->connect();
	a.m_socket->send(d);
	sleep(1);
	BOOST_REQUIRE_EQUAL(true, a.success);
}

BOOST_AUTO_TEST_SUITE_END()

