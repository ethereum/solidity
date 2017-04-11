/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
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

string swarmHashHex(string const& _input)
{
	return toHex(swarmHash(_input).asBytes());
}

BOOST_AUTO_TEST_CASE(test_zeros)
{
	BOOST_CHECK_EQUAL(swarmHashHex(string()), string("011b4d03dd8c01f1049143cf9c4c817e4b167f1d1b83e5c6f0f10d89ba1e7bce"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x1000 - 1, 0)), string("32f0faabc4265ac238cd945087133ce3d7e9bb2e536053a812b5373c54043adb"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x1000, 0)), string("411dd45de7246e94589ff5888362c41e85bd3e582a92d0fda8f0e90b76439bec"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x1000 + 1, 0)), string("e1adcce5812ca01b9dc1abab5429e393f2ee54575a9e3f4f1acc2827fbd48ef2"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x2000 - 1, 0)), string("ec8e9140197ef7707d897d6e870128913da7a75fe76a3aa40e11bbb5a9f4c304"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x2000, 0)), string("fe7c80b9383415132706adadb39c4982acca5fc969cb79af1105a34ac2dcffcc"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x2000 + 1, 0)), string("bb8668c849c2fa47997161b16b99fc5b6f3a96247f0af6ac68855a85bc718533"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x80000, 0)), string("4b37e998b558329ac806f69b492704b334103e66e2fe9f212dbb4cec58ff16d3"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x80020, 0)), string("0f86dcd141ae963f4e1ae23d8a0649a2a2dfaf9a768316420ad7ba5c81bd710c"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(0x800020, 0)), string("a6d51187334c169456b67c0798fe512efd97fa23ccfa811221f4c392b3676faf"));
	BOOST_CHECK_EQUAL(swarmHashHex(string(2095104, 0)), string("abbc50e1168bfc68f214a94f0f5e23ab2a28789bc67b14b1e4a16912752ca1e1"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
