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
/** @file rlp.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * RLP test functions.
 */

#include <fstream>
#include <sstream>
#include "JsonSpiritHeaders.h"
#include <libethsupport/Log.h>
#include <libethsupport/RLP.h>
#include <libethsupport/Common.h>
#include <boost/test/unit_test.hpp>
#include <algorithm>

using namespace std;
using namespace eth;
namespace js = json_spirit;

namespace eth 
{
	namespace test
	{
		static void buildRLP(js::mValue& _v, RLPStream& _rlp)
		{
			if (_v.type() == js::array_type)
			{
				RLPStream s;
				for (auto& i: _v.get_array())
					buildRLP(i, s);
				_rlp.appendList(s.out());
			}
			else if (_v.type() == js::int_type)
				_rlp.append(_v.get_uint64());
			else if (_v.type() == js::str_type)
			{
				auto s = _v.get_str();
				if (s.size() && s[0] == '#')
					_rlp.append(bigint(s.substr(1)));
				else
					_rlp.append(s);
			}
		}

		static void getRLPTestCases(js::mValue& v)
		{
			string s = asString(contents("../../../tests/rlptest.json"));
			BOOST_REQUIRE_MESSAGE( s.length() > 0, 
				"Contents of 'rlptest.json' is empty. Have you cloned the 'tests' repo branch develop?"); 
			js::read_string(s, v);
		} 	

		static void checkRLPTestCase(js::mObject& o)
		{ 
			BOOST_REQUIRE( o.count("in") > 0 ); 
			BOOST_REQUIRE( o.count("out") > 0 ); 
			BOOST_REQUIRE(!o["out"].is_null());			
		} 

		static void checkRLPAgainstJson(js::mValue& v, RLP& u)
		{
			if ( v.type() == js::str_type ) 
			{ 
				const std::string& expectedText = v.get_str();
				if ( expectedText.front() == '#' ) 
				{ 
					// Deal with bigint instead of a raw string 
					std::string bigIntStr = expectedText.substr(1,expectedText.length()-1); 
					std::stringstream bintStream(bigIntStr);  
					bigint val; 
					bintStream >> val; 
					BOOST_CHECK( !u.isList() ); 
					BOOST_CHECK( !u.isNull() );
					BOOST_CHECK( u );             // operator bool()
					BOOST_CHECK(u == val); 
				} 
				else 
				{ 
					BOOST_CHECK( !u.isList() ); 
					BOOST_CHECK( !u.isNull() ); 
					BOOST_CHECK( u.isData() ); 
					BOOST_CHECK( u ); 
					BOOST_CHECK( u.size() == expectedText.length() ); 
					BOOST_CHECK(u == expectedText); 
				}
			} 
			else if ( v.type() == js::int_type ) 
			{ 
				const int expectedValue = v.get_int(); 
				BOOST_CHECK( u.isInt() ); 
				BOOST_CHECK( !u.isList() ); 
				BOOST_CHECK( !u.isNull() );
				BOOST_CHECK( u );             // operator bool()
				BOOST_CHECK(u == expectedValue); 
			} 
			else if ( v.type() == js::array_type ) 
			{ 
				BOOST_CHECK( u.isList() ); 
				BOOST_CHECK( !u.isInt() ); 
				BOOST_CHECK( !u.isData() ); 
				js::mArray& arr = v.get_array(); 
				BOOST_CHECK( u.itemCount() == arr.size() ); 
				uint i; 
				for( i = 0; i < arr.size(); i++ ) 
				{ 
					RLP item = u[i]; 
					checkRLPAgainstJson(arr[i], item); 
				}  
			} 
			else 
			{ 
				BOOST_ERROR("Invalid Javascript object!");
			}  
			
		} 
	}
} 


BOOST_AUTO_TEST_CASE(rlp_encoding_test)
{
	cnote << "Testing RLP Encoding...";
	js::mValue v;
	eth::test::getRLPTestCases(v); 

	for (auto& i: v.get_obj())
	{
		js::mObject& o = i.second.get_obj();
		cnote << i.first;
		eth::test::checkRLPTestCase(o); 

		RLPStream s;
		eth::test::buildRLP(o["in"], s);

		std::string expectedText(o["out"].get_str()); 
		std::transform(expectedText.begin(), expectedText.end(), expectedText.begin(), ::tolower ); 

		const std::string& computedText = toHex(s.out()); 

		std::stringstream msg; 
		msg << "Encoding Failed: expected: " << expectedText << std::endl;
		msg << " But Computed: " << computedText; 

		BOOST_CHECK_MESSAGE(
			expectedText == computedText, 
			msg.str()
			); 
	}

}

BOOST_AUTO_TEST_CASE(rlp_decoding_test)
{
	cnote << "Testing RLP decoding..."; 
	// Uses the same test cases as encoding but in reverse. 
	// We read into the string of hex values, convert to bytes, 
	// and then compare the output structure to the json of the 
	// input object. 
	js::mValue v;
	eth::test::getRLPTestCases(v); 
	for (auto& i: v.get_obj())
	{
		js::mObject& o = i.second.get_obj();
		cnote << i.first;
		eth::test::checkRLPTestCase(o); 
		
		js::mValue& inputData = o["in"];
		bytes payloadToDecode = fromHex(o["out"].get_str()); 

		RLP payload(payloadToDecode); 

		eth::test::checkRLPAgainstJson(inputData, payload); 

	}
}


