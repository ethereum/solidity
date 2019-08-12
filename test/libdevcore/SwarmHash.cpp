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

#include <test/Options.h>

#include <libdevcore/Keccak256.h>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SwarmHash)

string bzzr0HashHex(string const& _input)
{
	return toHex(bzzr0Hash(_input).asBytes());
}

string bzzr1HashHex(bytes const& _input)
{
	return toHex(bzzr1Hash(_input).asBytes());
}

bytes sequence(size_t _length)
{
	bytes data;
	for (size_t i = 0; i < _length; i++)
		data.push_back(uint8_t((i % 255) & 0xff));
	return data;
}

BOOST_AUTO_TEST_CASE(test_zeros)
{
	BOOST_CHECK_EQUAL(bzzr0HashHex(string()), string("011b4d03dd8c01f1049143cf9c4c817e4b167f1d1b83e5c6f0f10d89ba1e7bce"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x1000 - 1, 0)), string("32f0faabc4265ac238cd945087133ce3d7e9bb2e536053a812b5373c54043adb"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x1000, 0)), string("411dd45de7246e94589ff5888362c41e85bd3e582a92d0fda8f0e90b76439bec"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x1000 + 1, 0)), string("69754a0098432bbc2e84fe1205276870748a61a065ab6ef44d6a2e7b13ce044d"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x2000 - 1, 0)), string("69ad3c581043404f775ffa8d6f1b25ad4a9ee812971190e90209c0966116a321"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x2000, 0)), string("f00222373ff82d0a178dc6271c78953e9c88f74130a52d401f5ec51475f63c43"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x2000 + 1, 0)), string("86d6773e79e02fd8145ee1aedba89ace0c15f2566db1249654000039a9a134bf"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x80000, 0)), string("cc0854fe2c6b98e920d5c14b1a88e6d4223e55b8f78883f60939aa2485e361bf"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x80020, 0)), string("ee9ffca246e70d3704740ba4df450fa6988d14a1c2439c7e734c7a77a4eb6fd3"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(0x800020, 0)), string("78b90b20c90559fb904535181a7c28929ea2f30a2329dbc25232de579709f12f"));
	BOOST_CHECK_EQUAL(bzzr0HashHex(string(2095104, 0)), string("a9958184589fc11b4027a4c233e777ebe2e99c66f96b74aef2a0638a94dd5439"));
}

BOOST_AUTO_TEST_CASE(bzz_hash_short)
{
	// Special case: 32 zero bytes
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes()), toHex(bytes(32, 0)));
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(1, 0)), "fe60ba40b87599ddfb9e8947c1c872a4a1a5b56f7d1b80f0a646005b38db52a5");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(31, 0)), "36fe2d14c5fe9ed380dc67afd9da6c5824bffcb01ac7972219c3cc3b1c8cd6b1");
	BOOST_CHECK_EQUAL(bzzr1HashHex(asBytes("hello world")), "92672a471f4419b255d7cb0cf313474a6f5856fb347c5ece85fb706d644b630f");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(64, 0)), "24090f674316c306ea2a98bdd08f042d6f776d0ae1c23b27fca52750a9c7d4e5");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(65, 0)), "6ab1eaa91095215e30cacf47131d06ce5e9fc01611e406409705e190ee4440c6");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(4096, 0)), "09ae927d0f3aaa37324df178928d3826820f3dd3388ce4aaebfc3af410bde23a");
}

BOOST_AUTO_TEST_CASE(bzz_hash_large)
{
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(4097, 0)), "c082943c4cb8a97c67947f290f5421cf4c61d021eb303c8df77de6fe208df516");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(4096 * 128, 0)), "392edbfc185187265cb5d50c2507965f2bb99ce8c255a24d3eb14257e40f2e33");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(4096 * 128 * 2, 0)), "f89af84ac550cdaa79639d5f6a1591ff1c9b3cb5d1fc55651ca63d4f80375447");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(4096 * 129, 0)), "8ab36734a15aef65221b024e47ee9a6bb727d8101eec42db364eef42285beb1d");
	BOOST_CHECK_EQUAL(bzzr1HashHex(bytes(4096 * 130, 0)), "21eafb87f2a2a7e51d96212296f8e970103add0527f2fc61fae99f244847a42d");
}

BOOST_AUTO_TEST_CASE(bzz_hash_nonzero)
{
	BOOST_CHECK_EQUAL(bzzr1HashHex(sequence(65)), "541552bae05e9a63a6cb561f69edf36ffe073e441667dbf7a0e9a3864bb744ea");
	BOOST_CHECK_EQUAL(bzzr1HashHex(sequence(4096)), "c10090961e7682a10890c334d759a28426647141213abda93b096b892824d2ef");
	BOOST_CHECK_EQUAL(bzzr1HashHex(sequence(4096 * 128 + 31)), "e5c76afa931e33ac94bce2e754b1bb6407d07f738f67856783d93934ca8fc576");
	BOOST_CHECK_EQUAL(bzzr1HashHex(sequence(4096 * 128 + 33)), "c2489aebad937b19384b61c7bd4ba494f9b14a710b2bcccce792856fb8fcfb3d");
	BOOST_CHECK_EQUAL(bzzr1HashHex(sequence(4096 * 129)), "b7e298f61b1bf23e21d8f45bf545eb1d6c0c4eaaca7d2c2690fb86038404a6d6");
	BOOST_CHECK_EQUAL(bzzr1HashHex(sequence(4096 * 130)), "59de730bf6c67a941f3b2ffa2f920acfaa1713695ad5deea12b4a121e5f23fa1");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
