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
/** @file txTest.cpp
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 * Simple peer transaction send test.
 */

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <Client.h>
#include <BlockChain.h>
#include <PeerServer.h>
using namespace std;
using namespace eth;

void mine(Client &c, int numBlocks)
{
	auto startBlock = c.blockChain().details().number;

	c.startMining();
	while(c.blockChain().details().number < startBlock + numBlocks)
		this_thread::sleep_for(chrono::milliseconds(1000));
	c.stopMining();
}

BOOST_AUTO_TEST_CASE(mine_and_send_to_peer)
{
	KeyPair kp1 = KeyPair::create();
	KeyPair kp2 = KeyPair::create();

	string c1DataDir = (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string();
	string c2DataDir = (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string();

	Client c1("TestClient1", kp1.address(), c1DataDir);
	Client c2("TestClient2", kp2.address(), c2DataDir);

	short c1Port = 20000;
	short c2Port = 21000;

	//connect the two clients up
	c1.startNetwork(c1Port);
	c2.startNetwork(c2Port);
	c2.connect("127.0.0.1", c1Port);

	//mine some blocks so that client 1 has a balance
	mine(c1, 1);
	auto c1bal = c1.state().balance(kp1.address());
	BOOST_REQUIRE(c1bal > 0);
//	BOOST_REQUIRE(c1bal > c1.state().fee());

	//send c2 some eth from c1
//	auto txAmount = c1bal - c1.state().fee();
//	c1.transact(kp1.secret(), c2.address(), txAmount);

	//mine some more to include the transaction on chain
	mine(c1, 1);
	auto c2bal = c2.state().balance(kp2.address());
	BOOST_REQUIRE(c2bal > 0);
//	BOOST_REQUIRE(c2bal == txAmount);
}
