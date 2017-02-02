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
 * Unit tests for the address checksum.
 */

#include <libdevcore/CommonData.h>

#include "../TestHelper.h"

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(Checksum)

BOOST_AUTO_TEST_CASE(regular)
{
	BOOST_CHECK(passesAddressChecksum("0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed", true));
	BOOST_CHECK(passesAddressChecksum("0xfB6916095ca1df60bB79Ce92cE3Ea74c37c5d359", true));
	BOOST_CHECK(passesAddressChecksum("0xdbF03B407c01E7cD3CBea99509d93f8DDDC8C6FB", true));
	BOOST_CHECK(passesAddressChecksum("0xD1220A0cf47c7B9Be7A2E6BA89F429762e7b9aDb", true));
}

BOOST_AUTO_TEST_CASE(regular_negative)
{
	BOOST_CHECK(!passesAddressChecksum("0x6aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed", true));
	BOOST_CHECK(!passesAddressChecksum("0xeB6916095ca1df60bB79Ce92cE3Ea74c37c5d359", true));
	BOOST_CHECK(!passesAddressChecksum("0xebF03B407c01E7cD3CBea99509d93f8DDDC8C6FB", true));
	BOOST_CHECK(!passesAddressChecksum("0xE1220A0cf47c7B9Be7A2E6BA89F429762e7b9aDb", true));
}

BOOST_AUTO_TEST_CASE(regular_invalid_length)
{
	BOOST_CHECK(passesAddressChecksum("0x9426cbfc57389778d313268E7F85F1CDc2fdad60", true));
	BOOST_CHECK(!passesAddressChecksum("0x9426cbfc57389778d313268E7F85F1CDc2fdad6", true));
	BOOST_CHECK(passesAddressChecksum("0x08A61851FFa4637dE289D630Ae8c5dFb0ff9171F", true));
	BOOST_CHECK(!passesAddressChecksum("0x8A61851FFa4637dE289D630Ae8c5dFb0ff9171F", true));
	BOOST_CHECK(passesAddressChecksum("0x00c40cC30cb4675673c9ee382de805c19734986A", true));
	BOOST_CHECK(!passesAddressChecksum("0xc40cC30cb4675673c9ee382de805c19734986A", true));
	BOOST_CHECK(passesAddressChecksum("0xC40CC30cb4675673C9ee382dE805c19734986a00", true));
	BOOST_CHECK(!passesAddressChecksum("0xC40CC30cb4675673C9ee382dE805c19734986a", true));
}

BOOST_AUTO_TEST_CASE(homocaps_valid)
{
	BOOST_CHECK(passesAddressChecksum("0x52908400098527886E0F7030069857D2E4169EE7", true));
	BOOST_CHECK(passesAddressChecksum("0x8617E340B3D01FA5F11F306F4090FD50E238070D", true));
	BOOST_CHECK(passesAddressChecksum("0xde709f2102306220921060314715629080e2fb77", true));
	BOOST_CHECK(passesAddressChecksum("0x27b1fdb04752bbc536007a920d24acb045561c26", true));
}

BOOST_AUTO_TEST_CASE(homocaps_invalid)
{
	string upper = "0x00AA0000000012400000000DDEEFF000000000BB";
	BOOST_CHECK(passesAddressChecksum(upper, false));
	BOOST_CHECK(!passesAddressChecksum(upper, true));
	string lower = "0x11aa000000000000000d00cc00000000000000bb";
	BOOST_CHECK(passesAddressChecksum(lower, false));
	BOOST_CHECK(!passesAddressChecksum(lower, true));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
