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
/** @file peer.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Peer Network test functions.
 */

#include <boost/test/unit_test.hpp>
#include <chrono>
#include <thread>
#include <libp2p/Host.h>
using namespace std;
using namespace dev;
using namespace dev::p2p;

BOOST_AUTO_TEST_SUITE(p2p)

BOOST_AUTO_TEST_CASE(host)
{
	NetworkPreferences host1prefs(30301, "127.0.0.1", true, true);
	NetworkPreferences host2prefs(30302, "127.0.0.1", true, true);
	
	Host host1("Test", host1prefs);
	host1.start();
		
	Host host2("Test", host2prefs);
	auto node2 = host2.id();
	host2.start();
	
	host1.addNode(node2, "127.0.0.1", host2prefs.listenPort, host2prefs.listenPort);
	
	this_thread::sleep_for(chrono::seconds(1));
	
	BOOST_REQUIRE_EQUAL(host1.peerCount(), 1);
	BOOST_REQUIRE_EQUAL(host2.peerCount(), host1.peerCount());
}

BOOST_AUTO_TEST_SUITE_END()

int peerTest(int argc, char** argv)
{
	Public remoteAlias;
	short listenPort = 30303;
	string remoteHost;
	short remotePort = 30303;
	
	for (int i = 1; i < argc; ++i)
	{
		string arg = argv[i];
		if (arg == "-l" && i + 1 < argc)
			listenPort = (short)atoi(argv[++i]);
		else if (arg == "-r" && i + 1 < argc)
			remoteHost = argv[++i];
		else if (arg == "-p" && i + 1 < argc)
			remotePort = (short)atoi(argv[++i]);
		else if (arg == "-ra" && i + 1 < argc)
			remoteAlias = Public(dev::fromHex(argv[++i]));
		else
			remoteHost = argv[i];
	}

	Host ph("Test", NetworkPreferences(listenPort));

	if (!remoteHost.empty() && !remoteAlias)
		ph.addNode(remoteAlias, remoteHost, remotePort, remotePort);

	this_thread::sleep_for(chrono::milliseconds(200));

	return 0;
}

