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
/** @file genesis.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Trie test functions.
 */

#include <fstream>
#include <random>
#include "JsonSpiritHeaders.h"
#include <libdevcore/CommonIO.h>
#include <libethereum/CanonBlockChain.h>
#include <boost/test/unit_test.hpp>
#include "TestHelper.h"

using namespace std;
using namespace dev;
using namespace dev::eth;

namespace js = json_spirit;

BOOST_AUTO_TEST_SUITE(BasicTests)

BOOST_AUTO_TEST_CASE(emptySHA3Types)
{
	h256 emptyListSHA3(fromHex("1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347"));
	BOOST_REQUIRE_EQUAL(emptyListSHA3, EmptyListSHA3);
	
	h256 emptySHA3(fromHex("c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"));
	BOOST_REQUIRE_EQUAL(emptySHA3, EmptySHA3);
}

BOOST_AUTO_TEST_CASE(genesis_tests)
{
	string testPath = test::getTestPath();
	testPath += "/BasicTests";

	cnote << "Testing Genesis block...";
	js::mValue v;
	string s = asString(contents(testPath + "/genesishashestest.json"));
	BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of 'genesishashestest.json' is empty. Have you cloned the 'tests' repo branch develop?");
	js::read_string(s, v);

	js::mObject o = v.get_obj();

	BOOST_CHECK_EQUAL(CanonBlockChain::genesis().stateRoot, h256(o["genesis_state_root"].get_str()));
	BOOST_CHECK_EQUAL(toHex(CanonBlockChain::createGenesisBlock()), toHex(fromHex(o["genesis_rlp_hex"].get_str())));
	BOOST_CHECK_EQUAL(BlockInfo::headerHash(CanonBlockChain::createGenesisBlock()), h256(o["genesis_hash"].get_str()));
}

BOOST_AUTO_TEST_SUITE_END()

