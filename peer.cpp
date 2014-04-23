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

#include <chrono>
#include <thread>
#include <boost/filesystem/operations.hpp>
#include <libethereum/BlockChain.h>
#include <libethereum/PeerServer.h>
using namespace std;
using namespace eth;
using boost::asio::ip::tcp;

int peerTest(int argc, char** argv)
{
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
		else
			remoteHost = argv[i];
	}

	BlockChain ch(boost::filesystem::temp_directory_path().string());
	PeerServer pn("Test", ch, 0, listenPort);

	if (!remoteHost.empty())
		pn.connect(remoteHost, remotePort);

	for (int i = 0; ; ++i)
	{
		this_thread::sleep_for(chrono::milliseconds(100));
		pn.sync();
		if (!(i % 10))
			pn.pingAll();
	}

	return 0;
}

