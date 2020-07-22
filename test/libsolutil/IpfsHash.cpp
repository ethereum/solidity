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
// SPDX-License-Identifier: GPL-3.0
/**
 * Unit tests for the ipfs hash computation routine.
 */

#include <libsolutil/IpfsHash.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(IpfsHash, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(test_small)
{
	BOOST_CHECK_EQUAL(ipfsHashBase58({}), "QmbFMke1KXqnYyBBWxB74N4c5SBnJMVAiMNRcGu6x1AwQH");
	BOOST_CHECK_EQUAL(ipfsHashBase58("x"), "QmULKig5Fxrs2sC4qt9nNduucXfb92AFYQ6Hi3YRqDmrYC");
	BOOST_CHECK_EQUAL(ipfsHashBase58("Solidity\n"), "QmSsm9M7PQRBnyiz1smizk8hZw3URfk8fSeHzeTo3oZidS");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(200ul, char(0))), "QmSXR1N23uWzsANi8wpxMPw5dmmhqBVUAb4hUrHVLpNaMr");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(10250ul, char(0))), "QmVJJBB3gKKBWYC9QTywpH8ZL1bDeTDJ17B63Af5kino9i");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(100000ul, char(0))), "QmYgKa25YqEGpQmmZtPPFMNK3kpqqneHk6nMSEUYryEX1C");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(121071ul, char(0))), "QmdMdRshQmqvyc92N82r7AKYdUF5FRh4DJo6GtrmEk3wgj");
}

BOOST_AUTO_TEST_CASE(test_medium)
{
	size_t length = 131071;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmSxYSToKHsPqqRdRnsM9gmr3EYS6dakhVaHgbFdgYQWi6");
}

BOOST_AUTO_TEST_CASE(test_largest_unchunked)
{
	size_t length = 1024 * 256 - 1;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmbNDspMkzkMFKyS3eCJGedG7GWRQHSCzJCZLjxP7wyVAx");
}

BOOST_AUTO_TEST_CASE(test_smallest_chunked)
{
	size_t length = 1024 * 256 + 1;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmbVuw4C4vcmVKqxoWtgDVobvcHrSn51qsmQmyxjk4sB2Q");
}

BOOST_AUTO_TEST_CASE(test_large)
{
	size_t length = 1310710;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmNg7BJo8gEMDK8yGQbHEwPtycesnE6FUULX5iVd5TAL9f");
}

BOOST_AUTO_TEST_CASE(test_largest_one_level)
{
	size_t length = 45613056; // 1024 * 256 * 174;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmY4HSz1oVGdUzb8poVYPLsoqBZjH6LZrtgnme9wWn2Qko");
}

BOOST_AUTO_TEST_CASE(test_smallest_multi_level)
{
	size_t length = 45613057; // 1024 * 256 * 174 + 1;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmehMASWcBsX7VcEQqs6rpR5AHoBfKyBVEgmkJHjpPg8jq");
}

BOOST_AUTO_TEST_CASE(test_multi_level_tree)
{
	size_t length = 46661632;
	string data;
	data.resize(length, 0);
	BOOST_REQUIRE_EQUAL(data.size(), length);
	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmaTb1sT9hrSXJLmf8bxJ9NuwndiHuMLsgNLgkS2eXu3Xj");
}

BOOST_AUTO_TEST_SUITE_END()

}
