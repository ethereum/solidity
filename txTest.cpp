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
#include <libethereum/Client.h>
#include <libethereum/CanonBlockChain.h>
#include <libethereum/EthereumHost.h>
#include "TestHelper.h"
using namespace std;
using namespace dev;
using namespace dev::eth;

// Disabled since tests shouldn't block. Need a short cut to avoid real mining.
/*
BOOST_AUTO_TEST_CASE(mine_local_simple_tx)
{
	KeyPair kp1 = KeyPair::create();
	KeyPair kp2 = KeyPair::create();

	Client c1("TestClient1", kp1.address(), (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());

	//mine some blocks so that client 1 has a balance
	mine(c1, 1);
	auto c1bal = c1.state().balance(kp1.address());
	BOOST_REQUIRE(c1bal > 0);

	//send c2 some eth from c1
	auto txAmount = c1bal / 2u;
	auto gasPrice = 10 * szabo;
	auto gas = dev::eth::c_callGas;
	c1.transact(kp1.secret(), txAmount, kp2.address(), bytes(), gas, gasPrice);

	//mine some more to include the transaction on chain
	mine(c1, 1);
	auto c2bal = c1.state().balance(kp2.address());
	BOOST_REQUIRE(c2bal > 0);
	BOOST_REQUIRE(c2bal == txAmount);
}

BOOST_AUTO_TEST_CASE(mine_and_send_to_peer)
{
	KeyPair kp1 = KeyPair::create();
	KeyPair kp2 = KeyPair::create();

	Client c1("TestClient1", kp1.address(), (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());
	Client c2("TestClient2", kp2.address(), (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());

	connectClients(c1, c2);

	//mine some blocks so that client 1 has a balance
	mine(c1, 1);
	auto c1bal = c1.state().balance(kp1.address());
	BOOST_REQUIRE(c1bal > 0);

	//send c2 some eth from c1
	auto txAmount = c1bal / 2u;
	auto gasPrice = 10 * szabo;
	auto gas = dev::eth::c_callGas;
	c1.transact(kp1.secret(), txAmount, kp2.address(), bytes(), gas, gasPrice);

	//mine some more to include the transaction on chain
	mine(c1, 1);
	auto c2bal = c2.state().balance(kp2.address());
	BOOST_REQUIRE(c2bal > 0);
	BOOST_REQUIRE(c2bal == txAmount);
}

BOOST_AUTO_TEST_CASE(mine_and_send_to_peer_fee_check)
{
	KeyPair kp1 = KeyPair::create();
	KeyPair kp2 = KeyPair::create();

	Client c1("TestClient1", kp1.address(), (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());
	Client c2("TestClient2", kp2.address(), (boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());

	connectClients(c1, c2);

	//mine some blocks so that client 1 has a balance
	mine(c1, 1);

	auto c1StartBalance = c1.state().balance(kp1.address());
	auto c2StartBalance = c2.state().balance(kp2.address());
	BOOST_REQUIRE(c1StartBalance > 0);
	BOOST_REQUIRE(c2StartBalance == 0);

	//send c2 some eth from c1
	auto txAmount = c1StartBalance / 2u;
	auto gasPrice = 10 * szabo;
	auto gas = dev::eth::c_callGas;
	c1.transact(kp1.secret(), txAmount, c2.address(), bytes(), gas, gasPrice);

	//mine some more, this time with second client (so he can get fees from first client's tx)
	mine(c2, 1);

	auto c1EndBalance = c1.state().balance(kp1.address());
	auto c2EndBalance = c2.state().balance(kp2.address());
	BOOST_REQUIRE(c1EndBalance > 0);
	BOOST_REQUIRE(c1EndBalance == c1StartBalance - txAmount - gasPrice * gas);
	BOOST_REQUIRE(c2EndBalance > 0);
}
*/
