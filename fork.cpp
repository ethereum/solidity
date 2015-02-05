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
/** @file fork.cpp
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 * Tests for different forking behavior
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
BOOST_AUTO_TEST_CASE(simple_chain_fork)
{
	//start a client and mine a short chain
	Client c1("TestClient1", KeyPair::create().address(),
			(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());
	mine(c1, 4);

	//start another client and mine a longer chain
	Client c2("TestClient2", KeyPair::create().address(),
			(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()).string());
	mine(c2, 6);

	//connect the two clients up to resolve chain
	c1.startNetwork(20000);
	c2.startNetwork(21000);
	c2.connect("127.0.0.1", 20000);

	//mine an extra block to cement it
	mine(c1, 1);

	//check the balances are where they should be
	//c1's chain should have been clobbered by c2
	BOOST_REQUIRE(c1.state().balance(c1.address()) == 0);
	BOOST_REQUIRE(c2.state().balance(c2.address()) > 0);
}
*/
