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
 * Unit tests for the ipfs hash computation routine.
 */

#include <libdevcore/IpfsHash.h>

#include <test/Options.h>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(IpfsHash)

BOOST_AUTO_TEST_CASE(test_small)
{
	BOOST_CHECK_EQUAL(ipfsHashBase58({}), "QmbFMke1KXqnYyBBWxB74N4c5SBnJMVAiMNRcGu6x1AwQH");
	BOOST_CHECK_EQUAL(ipfsHashBase58("x"), "QmULKig5Fxrs2sC4qt9nNduucXfb92AFYQ6Hi3YRqDmrYC");
	BOOST_CHECK_EQUAL(ipfsHashBase58("Solidity\n"), "QmSsm9M7PQRBnyiz1smizk8hZw3URfk8fSeHzeTo3oZidS");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(size_t(200), char(0))), "QmSXR1N23uWzsANi8wpxMPw5dmmhqBVUAb4hUrHVLpNaMr");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(size_t(10250), char(0))), "QmVJJBB3gKKBWYC9QTywpH8ZL1bDeTDJ17B63Af5kino9i");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(size_t(100000), char(0))), "QmYgKa25YqEGpQmmZtPPFMNK3kpqqneHk6nMSEUYryEX1C");
	BOOST_CHECK_EQUAL(ipfsHashBase58(string(size_t(121071), char(0))), "QmdMdRshQmqvyc92N82r7AKYdUF5FRh4DJo6GtrmEk3wgj");
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

// TODO This needs chunking implemented
//BOOST_AUTO_TEST_CASE(test_large)
//{
//	size_t length = 1310710;
//	string data;
//	data.resize(length, 0);
//	BOOST_REQUIRE_EQUAL(data.size(), length);
//	BOOST_CHECK_EQUAL(ipfsHashBase58(data), "QmNg7BJo8gEMDK8yGQbHEwPtycesnE6FUULX5iVd5TAL9f");
//}

BOOST_AUTO_TEST_SUITE_END()

}
}
