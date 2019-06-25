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
 * Unit tests for keccak256.
 */
#include <libdevcore/Keccak256.h>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(Keccak256)

BOOST_AUTO_TEST_CASE(empty)
{
	BOOST_CHECK_EQUAL(
		keccak256(bytes()),
		FixedHash<32>("0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470")
	);
}

BOOST_AUTO_TEST_CASE(zeros)
{
	BOOST_CHECK_EQUAL(
		keccak256(bytes(1, '\0')),
		FixedHash<32>("0xbc36789e7a1e281436464229828f817d6612f7b477d66591ff96a9e064bcc98a")
	);
	BOOST_CHECK_EQUAL(
		keccak256(bytes(2, '\0')),
		FixedHash<32>("0x54a8c0ab653c15bfb48b47fd011ba2b9617af01cb45cab344acd57c924d56798")
	);
	BOOST_CHECK_EQUAL(
		keccak256(bytes(5, '\0')),
		FixedHash<32>("0xc41589e7559804ea4a2080dad19d876a024ccb05117835447d72ce08c1d020ec")
	);
	BOOST_CHECK_EQUAL(
		keccak256(bytes(10, '\0')),
		FixedHash<32>("0x6bd2dd6bd408cbee33429358bf24fdc64612fbf8b1b4db604518f40ffd34b607")
	);
}

BOOST_AUTO_TEST_CASE(strings)
{
	BOOST_CHECK_EQUAL(
		keccak256("test"),
		FixedHash<32>("0x9c22ff5f21f0b81b113e63f7db6da94fedef11b2119b4088b89664fb9a3cb658")
	);
	BOOST_CHECK_EQUAL(
		keccak256("longer test string"),
		FixedHash<32>("0x47bed17bfbbc08d6b5a0f603eff1b3e932c37c10b865847a7bc73d55b260f32a")
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
