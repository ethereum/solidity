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
/**
 * Unit tests for the swarm hash computation routine.
 */

#include <libdevcore/SwarmHash.h>

#include "../TestHelper.h"

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SwarmHash)

string swarmHashHex(bytes const& _input)
{
	return toHex(swarmHash(_input).asBytes());
}

BOOST_AUTO_TEST_CASE(test_zeros)
{
	BOOST_CHECK_EQUAL(swarmHashHex(bytes()), string("011b4d03dd8c01f1049143cf9c4c817e4b167f1d1b83e5c6f0f10d89ba1e7bce"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x1000 - 1, 0)), string("32f0faabc4265ac238cd945087133ce3d7e9bb2e536053a812b5373c54043adb"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x1000, 0)), string("411dd45de7246e94589ff5888362c41e85bd3e582a92d0fda8f0e90b76439bec"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x1000 + 1, 0)), string("970b0b1fe0bf90549af9aba54e8ce18884cce97d63069efe92f73fd50037c3e2"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x2000 - 1, 0)), string("1e7bd4d46836b63a2e7c313b68d24ad6be56ed6c8b30634005fd213bb580b0b4"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x2000, 0)), string("ab184c15eee316dd54e8887614a535f589478e99b922a1ede30ec418344c8324"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x2000 + 1, 0)), string("c8915d9244ad5d7428a41b207e083de6eafffac629b45ad462062ea8dc5ac4a3"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x80000, 0)), string("d0bf003bbc34a68f608f8bc88b497c19ce4ad61f38436ee3120b33db9cc9a116"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x80020, 0)), string("d001908f9942ad56d5343574e33bd74b30092115b1c29a67c20869a9ddbbedc3"));
	BOOST_CHECK_EQUAL(swarmHashHex(bytes(0x800020, 0)), string("cdeedeccfe3eaa1c4095713af579b9c2b151d2b3f45ce6652ba2f5cd537a60ba"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
