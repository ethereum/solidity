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
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Main test functions.
 */

#include <fstream>
#include "JsonSpiritHeaders.h"
#include <libdevcore/Log.h>
#include <libdevcore/CommonIO.h>
#include <libdevcrypto/TrieCommon.h>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace js = json_spirit;

BOOST_AUTO_TEST_CASE(hexPrefix_test)
{
	cnote << "Testing Hex-Prefix-Encode...";
	js::mValue v;
	string s = asString(contents("../../../tests/hexencodetest.json"));
	BOOST_REQUIRE_MESSAGE(s.length() > 0, "Content from 'hexencodetest.json' is empty. Have you cloned the 'tests' repo branch develop?");
	js::read_string(s, v);
	for (auto& i: v.get_obj())
	{
		js::mObject& o = i.second.get_obj();
		cnote << i.first;
		bytes v;
		for (auto& i: o["seq"].get_array())
			v.push_back((byte)i.get_int());
		auto e = hexPrefixEncode(v, o["term"].get_bool());
		BOOST_REQUIRE( ! o["out"].is_null() ); 
		BOOST_CHECK( o["out"].get_str() == toHex(e) ); 
	}
}

