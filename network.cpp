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
/** @file network.cpp
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 * Basic networking tests
 */

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <libethereum/Client.h>
#include <libethereum/BlockChain.h>
#include <libethereum/PeerServer.h>
#include "TestHelper.h"
using namespace std;
using namespace eth;

// Disabled since tests shouldn't block (not the worst offender, but timeout should be reduced anyway).
/*
BOOST_AUTO_TEST_CASE(listen_port_busy)
{
	short port = 20000;

	//make use of the port ahead of our client
	ba::io_service ioService;
	bi::tcp::endpoint endPoint(bi::tcp::v4(), port);
	bi::tcp::acceptor acceptor(ioService, endPoint);
	acceptor.listen(10);

	//prepare client and try to listen on same, used, port
	Client c1("TestClient1", KeyPair::create().address(),
			(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());

	c1.startNetwork(port);

	BOOST_REQUIRE(c1.haveNetwork());
	BOOST_REQUIRE(c1.peerServer()->listenPort() != 0);
	BOOST_REQUIRE(c1.peerServer()->listenPort() != port);
}
*/
