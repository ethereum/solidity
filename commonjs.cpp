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
/** @file commonjs.cpp
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2014
 */

#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libethcore/CommonJS.h>

BOOST_AUTO_TEST_SUITE(commonjs)
using namespace std;
using namespace dev;
using namespace dev::eth;

BOOST_AUTO_TEST_CASE(jsToPublic)
{
	cnote << "Testing jsToPublic...";
	KeyPair kp = KeyPair::create();
	string string = toJS(kp.pub());
	Public pub = dev::jsToPublic(string);
	BOOST_CHECK_EQUAL(kp.pub(), pub);
}

BOOST_AUTO_TEST_CASE(jsToAddress)
{
	cnote << "Testing jsToPublic...";
	KeyPair kp = KeyPair::create();
	string string = toJS(kp.address());
	Address address = dev::eth::jsToAddress(string);
	BOOST_CHECK_EQUAL(kp.address(), address);
}

BOOST_AUTO_TEST_CASE(jsToSecret)
{
	cnote << "Testing jsToPublic...";
	KeyPair kp = KeyPair::create();
	string string = toJS(kp.secret());
	Secret secret = dev::jsToSecret(string);
	BOOST_CHECK_EQUAL(kp.secret(), secret);
}

BOOST_AUTO_TEST_SUITE_END()
