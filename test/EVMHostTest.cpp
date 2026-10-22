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
 * Unit tests for EVMHost.
 */

#include <test/EVMHost.h>

#include <boost/test/unit_test.hpp>

namespace solidity::test
{

BOOST_AUTO_TEST_SUITE(EVMHostTest, *boost::unit_test::label("nooptions"))

// Cross-checked against evmone's own compute_create_address() unit tests
// (test/unittests/create_address_test.cpp, TEST(create_address, create_nonces) and
// TEST(create_address, create_examples) in evmone 0.23.0), which is the RLP/keccak
// derivation the EVMC 18 VM actually performs for CREATE.
BOOST_AUTO_TEST_CASE(compute_create_address)
{
	util::h160 const sender0("0x0000000000000000000000000000000000000000");
	util::h160 const sender1("0x0000000000000000000000000000000000000001");

	// Nonce 0: RLP empty string (0x80).
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender0), 0),
		util::h160("0xbd770416a3345f91e4b34576cb804a576fa48eb1")
	);

	// Nonce 1..127: RLP single byte, no prefix.
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender1), 1),
		util::h160("0x535b3d7a252fa034ed71f0c53ec0c6f784cb64e1")
	);

	// Nonce 128..255: RLP short-string, 1-byte payload (prefix 0x81).
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender1), 0x80),
		util::h160("0x09c1ef8f55c61b94e8b92a55d0891d408a991e18")
	);

	// Nonce 256..65535: RLP short-string, 2-byte payload (prefix 0x82). This is exactly
	// the case that used to be miscomputed with the "long string" prefix 0xb9 instead of
	// the correct "short string" prefix 0x82.
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender0), 0x100),
		util::h160("0x1183a5a83c1fa113618603abc4509077ec672699")
	);
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender0), 0xffff),
		util::h160("0xae80be2f887b0efb148934160afd38459969571a")
	);

	// Nonce beyond 0xffff: previously unsupported (solUnimplemented). Now handled via
	// minimal big-endian encoding, with a 4-byte and an 8-byte (full uint64_t range) payload.
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender0), 0xffffffff),
		util::h160("0x83317d2df02af8fe91040765f49719e8115c0f04")
	);
	BOOST_CHECK_EQUAL(
		EVMHost::computeCreateAddress(EVMHost::convertToEVMC(sender0), 0xffffffffffffffffULL),
		util::h160("0x1262d73ea59d3a661bf8751d16cf1a5377149e75")
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
